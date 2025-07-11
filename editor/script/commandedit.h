#pragma once

#include <qlineedit.h>
#include <vector>

struct lua_State;

class CommandEdit : public QLineEdit {
    Q_OBJECT

    std::vector<std::string> commandHistory;
    int historyIndex = 0;

    void executeCommand();
    void getOrCreateEnvironment(lua_State* L);
public:
    CommandEdit(QWidget* parent = nullptr);
    ~CommandEdit();

    void keyPressEvent(QKeyEvent *) override;
};