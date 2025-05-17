#include "scriptdocument.h"

#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexerlua.h>
#include <Qsci/qsciscintillabase.h>
#include <Qsci/qscistyle.h>
#include <Qsci/qsciapis.h>
#include <map>
#include <memory>
#include <qboxlayout.h>
#include <qcolor.h>
#include <qfont.h>
#include <qdebug.h>
#include <qglobal.h>
#include <qlayout.h>
#include <qtextformat.h>
#include "mainwindow.h"
#include "objects/script.h"
#include "datatypes/meta.h"

QsciAPIs* makeApis(QsciLexer*);

std::map<int, const QColor> DARK_MODE_COLOR_SCHEME = {{
    {QsciLexerLua::Comment, QColor("#808080")},
    {QsciLexerLua::LineComment, QColor("#808080")},
    {QsciLexerLua::Number, QColor("#6897BB")},
    {QsciLexerLua::Keyword, QColor("#CC7832")},
    {QsciLexerLua::String, QColor("#6A8759")},
    {QsciLexerLua::Character, QColor("#6A8759")},
    {QsciLexerLua::LiteralString, QColor("#6A8759")},
    {QsciLexerLua::Preprocessor, QColor("#FF00FF")}, // Obsolete since Lua 4.0, but whatever
    {QsciLexerLua::Operator, QColor("#FFFFFF")},
    {QsciLexerLua::Identifier, QColor("#FFFFFF")},
    {QsciLexerLua::UnclosedString, QColor("#6A8759")},
    {QsciLexerLua::BasicFunctions, QColor("#CC7832")},
    {QsciLexerLua::StringTableMathsFunctions, QColor("#CC7832")},
    {QsciLexerLua::CoroutinesIOSystemFacilities, QColor("#CC7832")},
    {QsciLexerLua::Label, QColor("#FFFFFF")},

}};

class ObLuaLexer : public QsciLexerLua {
    const char * keywords(int set) const override {
        // Taken from qscilexerlua.cpp

        if (set == 1)
            // Keywords.
            return
                "and break do else elseif end false for function if "
                "in local nil not or repeat return then true until "
                "while";

        if (set == 2)
            // Basic functions.
            return
                //"foreach foreachi getn "

                // Openblocks extensions
                "shared require game workspace "

                "_G _VERSION getfenv getmetatable ipairs loadstring "
                "next pairs pcall rawequal rawget rawset select "
                "setfenv setmetatable xpcall string table tonumber "
                "tostring type math newproxy coroutine io os";

        if (set == 3)
            // String, table and maths functions.
            return
                // "abs acos asin atan atan2 ceil cos deg exp floor "
                // "format frexp gsub ldexp log log10 max min mod rad "
                // "random randomseed sin sqrt tan "

                "string.byte string.char string.dump string.find "
                "string.len string.lower string.rep string.sub "
                "string.upper string.format string.gfind string.gsub "
                "table.concat table.foreach table.foreachi table.getn "
                "table.sort table.insert table.remove table.setn "
                "math.abs math.acos math.asin math.atan math.atan2 "
                "math.ceil math.cos math.deg math.exp math.floor "
                "math.frexp math.ldexp math.log math.log10 math.max "
                "math.min math.mod math.pi math.rad math.random "
                "math.randomseed math.sin math.sqrt math.tan";

        if (set == 4)
            // Coroutine, I/O and system facilities.
            return
                // "clock date difftime time "

                "coroutine.create coroutine.resume coroutine.status "
                "coroutine.wrap coroutine.yield os.clock os.date "
                "os.difftime os.time";

        return 0;
    };
};

ScriptDocument::ScriptDocument(std::shared_ptr<Script> script, QWidget* parent):
    script(script), QMdiSubWindow(parent) {

    setWindowTitle(QString::fromStdString(script->name));

    // Add detector for script deletion to automatically close this document
    scriptDeletionHandler = script->AncestryChanged->Connect([this, script](std::vector<Data::Variant> args) {
        std::weak_ptr<Instance> child = args[0].get<Data::InstanceRef>();
        std::weak_ptr<Instance> newParent = args[1].get<Data::InstanceRef>();
        if (child.expired() || child.lock() != script || !newParent.expired()) return;

        dynamic_cast<MainWindow*>(window())->closeScriptDocument(script);
    });

    QFrame* frame = new QFrame;
    QVBoxLayout* frameLayout = new QVBoxLayout;
    frameLayout->setMargin(0);
    frame->setLayout(frameLayout);
    scintilla = new QsciScintilla(this);
    
    frameLayout->addWidget(scintilla);
    setWidget(frame);

    // https://forum.qt.io/post/803690
    QFont findFont("<NONE>");
    findFont.setStyleHint(QFont::Monospace);
    QFontInfo info(findFont);

    QFont font;
    font.setFamily(info.family());
    font.setPointSize(10);
    font.setFixedPitch(true);

    // scintilla->setMargins(2);
    scintilla->setScrollWidth(1); // Hide scrollbars on empty document, it will grow automatically
    scintilla->setMarginType(1, QsciScintilla::NumberMargin);
    scintilla->setMarginWidth(1, "0000");
    scintilla->setMarginsForegroundColor(palette().windowText().color());
    scintilla->setMarginsBackgroundColor(palette().window().color());
    scintilla->setCaretForegroundColor(palette().text().color());
    scintilla->setFont(font);
    scintilla->setTabWidth(4);

    scintilla->setText(QString::fromStdString(script->source));

    ObLuaLexer* lexer = new ObLuaLexer;
    lexer->setFont(font);
    scintilla->setLexer(lexer);

    // Remove background highlight color
    lexer->setPaper(palette().light().color());
    QsciAPIs* api = makeApis(lexer);
    lexer->setAPIs(api);
    scintilla->setAutoCompletionSource(QsciScintilla::AcsAPIs);
    scintilla->setAutoCompletionThreshold(1);
    scintilla->setAutoCompletionCaseSensitivity(false);
    scintilla->setAutoCompletionReplaceWord(false);
    scintilla->setCallTipsVisible(-1);
    scintilla->setCallTipsPosition(QsciScintilla::CallTipsBelowText);

    // Set color scheme
    // https://stackoverflow.com/a/26318796/16255372
    for (auto& [style, color] : DARK_MODE_COLOR_SCHEME) {
        lexer->setColor(color, style);
    }

    // lexer->setAutoIndentStyle(QsciScintilla::AiOpening | QsciScintilla::AiMaintain | QsciScintilla::AiClosing);
    // scintilla->setAutoIndent(true);

    connect(scintilla, &QsciScintilla::textChanged, [this]() {
        // this-> is important here, as otherwise it will refer to the
        // parameter passed in, which will get gc'ed eventually
        this->script->source = scintilla->text().toStdString();
    });
}

ScriptDocument::~ScriptDocument() {
}

QsciAPIs* makeApis(QsciLexer* lexer) {
    QsciAPIs* apis = new QsciAPIs(lexer);

    apis->add("workspace");
    apis->add("game");
    apis->add("wait(seconds: number)\n\nPauses the current thread for the specified number of seconds");

    apis->prepare();

    return apis;
}