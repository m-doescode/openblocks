#include "commandedit.h"
#include "common.h"
#include "lua.h"
#include "objects/service/script/scriptcontext.h"
#include "luaapis.h" // IWYU pragma: keep
#include <qevent.h>
#include <qlineedit.h>
#include <qnamespace.h>

int script_errhandler(lua_State*);

CommandEdit::CommandEdit(QWidget* parent) : QLineEdit(parent) {
    connect(this, &QLineEdit::returnPressed, this, &CommandEdit::executeCommand);
}

CommandEdit::~CommandEdit() = default;

void CommandEdit::executeCommand() {
    std::string command = this->text().toStdString();
    
    // Execute via Lua
    auto context = gDataModel->GetService<ScriptContext>();
    lua_State* L = context->state;

    int top = lua_gettop(L);
    lua_State* Lt = lua_newthread(L);

    // Push wrapper as thread function
    lua_getfield(Lt, LUA_REGISTRYINDEX, "LuaPCallWrapper");

    // Load source code and push onto thread as upvalue for wrapper
    int status = luaL_loadstring(Lt, command.c_str());
    if (status != LUA_OK) {
        // Failed to parse/load chunk
        Logger::error(lua_tostring(Lt, -1));

        lua_settop(L, top);
        return;
    }

    // Push our error handler and then generate the wrapped function
    lua_pushcfunction(Lt, script_errhandler);
    lua_call(Lt, 2, 1);

    // Resume the thread
    lua_resume(Lt, 0);

    lua_pop(L, 1); // Pop the thread
    lua_settop(L, top);

    // Push to history
    if (commandHistory.size() == 0 || commandHistory.back() != command) {
        historyIndex = commandHistory.size();
        commandHistory.push_back(command);
    }
};

void CommandEdit::keyPressEvent(QKeyEvent* evt) {
    switch (evt->key()) {
    case Qt::Key_Up:
        if (historyIndex > 0)
            historyIndex--;

        if (commandHistory.size() > 0)
            this->setText(QString::fromStdString(commandHistory[historyIndex]));
        return;
    case Qt::Key_Down:
        if (historyIndex+1 < (int)commandHistory.size())
            historyIndex++;

        if (commandHistory.size() > 0)
            this->setText(QString::fromStdString(commandHistory[historyIndex]));
        return;
    }

    return QLineEdit::keyPressEvent(evt);
}