#include "scriptdocument.h"

#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexer.h>
#include <qboxlayout.h>
#include <qfont.h>
#include <qlayout.h>
#include "objects/script.h"

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

    scintilla->setLexer();
}

ScriptDocument::~ScriptDocument() {
}
