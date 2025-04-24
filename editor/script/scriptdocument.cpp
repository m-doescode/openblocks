#include "scriptdocument.h"

#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexer.h>
#include <qboxlayout.h>
#include <qfont.h>
#include <qdebug.h>
#include <qglobal.h>
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

    scintilla->setText(QString::fromStdString(script->source));

    connect(scintilla, &QsciScintilla::textChanged, [this]() {
        // this-> is important here, as otherwise it will refer to the
        // parameter passed in, which will get gc'ed eventually
        this->script->source = scintilla->text().toStdString();
    });
}

ScriptDocument::~ScriptDocument() {
}
