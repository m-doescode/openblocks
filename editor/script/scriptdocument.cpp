#include "scriptdocument.h"

#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexerlua.h>
#include <Qsci/qsciscintillabase.h>
#include <Qsci/qscistyle.h>
#include <map>
#include <qboxlayout.h>
#include <qcolor.h>
#include <qfont.h>
#include <qdebug.h>
#include <qglobal.h>
#include <qlayout.h>
#include "objects/script.h"

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

ScriptDocument::ScriptDocument(std::shared_ptr<Script> script, QWidget* parent):
    script(script), QMdiSubWindow(parent) {

    setWindowTitle(QString::fromStdString(script->name));

    // QFrame* frame = new QFrame;
    // QVBoxLayout* frameLayout = new QVBoxLayout;
    // frame->setLayout(frameLayout);
    scintilla = new QsciScintilla;
    // frameLayout->addWidget(scintilla);
    // setWidget(frame);
    setWidget(scintilla);

    QFont font;
    font.setFamily("Consolas");
    font.setStyleHint(QFont::Monospace);
    font.setPointSize(12);

    // scintilla->setMargins(2);
    scintilla->setScrollWidth(1); // Hide scrollbars on empty document, it will grow automatically
    scintilla->setMarginType(1, QsciScintilla::NumberMargin);
    scintilla->setMarginWidth(1, "0000");
    scintilla->setMarginsForegroundColor(palette().windowText().color());
    scintilla->setMarginsBackgroundColor(palette().window().color());
    scintilla->setCaretForegroundColor(palette().text().color());
    scintilla->setFont(font);

    scintilla->setText(QString::fromStdString(script->source));

    QsciLexerLua* lexer = new QsciLexerLua;
    lexer->setFont(font);
    scintilla->setLexer(lexer);

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
