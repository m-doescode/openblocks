#include "outputtextview.h"
#include "mainwindow.h"
#include "objects/script.h"
#include "panes/outputtextview.h"
#include <QEvent>
#include <QTextEdit>
#include <QWidget>
#include <QMouseEvent>
#include <qcursor.h>
#include <qmenu.h>
#include <qnamespace.h>
#include <string>

OutputTextView::OutputTextView(QWidget* parent) : QTextEdit(parent) {
    Logger::addLogListener(std::bind(&OutputTextView::handleLog, this, std::placeholders::_1, std::placeholders::_2));
    Logger::addLogListener(std::bind(&OutputTextView::handleLogTrace, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    ensureCursorVisible();

    QFont font("");
    font.setStyleHint(QFont::Monospace);
    setFont(font);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, [&](QPoint point) {
        QMenu *menu = createStandardContextMenu(point);

        menu->addAction("Clear Output", [&]() {
            clear();
            stackTraceScripts.clear();
        });

        menu->exec(mapToGlobal(point));
        delete menu;
    });
}

OutputTextView::~OutputTextView() = default;

void OutputTextView::handleLog(Logger::LogLevel logLevel, std::string message) {
    if (logLevel == Logger::LogLevel::DEBUG) return;

    // Skip if trace, as that is handled by handleLogTrace
    if (logLevel == Logger::LogLevel::TRACE && !message.starts_with("Stack"))
        return;

    // https://stackoverflow.com/a/61722734/16255372
    moveCursor(QTextCursor::MoveOperation::End);
    QTextCursor cursor = textCursor();
    QTextCharFormat format = cursor.charFormat();

    if (logLevel == Logger::LogLevel::TRACE) {
        format.setForeground(QColor(0, 127, 255));
    } else if (logLevel == Logger::LogLevel::WARNING) {
        format.setForeground(QColor(255, 127, 0));
        format.setFontWeight(QFont::Bold); // https://forum.qt.io/post/505920
    } else if (logLevel == Logger::LogLevel::ERROR || logLevel == Logger::LogLevel::FATAL_ERROR) {
        format.setForeground(QColor(255, 0, 0));
        format.setFontWeight(QFont::Bold);
    }

    cursor.insertText(message.c_str(), format);
    cursor.insertText("\n", QTextCharFormat());
}

void OutputTextView::handleLogTrace(std::string message, std::string source, int line, void* userData) {
    std::weak_ptr<Script>* script = (std::weak_ptr<Script>*)userData;

    // https://stackoverflow.com/a/61722734/16255372
    QTextCursor cursor = textCursor();
    QTextCharFormat format = cursor.charFormat();
    format.setForeground(QColor(0, 127, 255));

    if (userData != nullptr && !script->expired()) {
        int id = stackTraceScriptsLastId++;
        stackTraceScripts[id] = *script;

        format.setAnchor(true);
        format.setAnchorHref(QString::number(id));
    }

    cursor.insertText(message.c_str(), format);
    cursor.insertText("\n", QTextCharFormat());
}

// https://stackoverflow.com/a/61722734/16255372
void OutputTextView::mousePressEvent(QMouseEvent *e) {
    QString anchor = anchorAt(e->pos());
    if (anchor == "" || e->modifiers() & Qt::AltModifier) return QTextEdit::mousePressEvent(e);

    auto script = stackTraceScripts[anchor.toInt()];
    if (script.expired()) return QTextEdit::mousePressEvent(e);

    MainWindow* mainWnd = dynamic_cast<MainWindow*>(window());
    mainWnd->openScriptDocument(script.lock());
}

void OutputTextView::mouseReleaseEvent(QMouseEvent *e) {
    return QTextEdit::mouseReleaseEvent(e);
}

void OutputTextView::mouseMoveEvent(QMouseEvent *e) {
    QCursor cur = cursor();

    QString anchor = anchorAt(e->pos());
    if (anchor == "" || e->modifiers() & Qt::AltModifier) {
        viewport()->setCursor(Qt::IBeamCursor);
    } else {
        viewport()->setCursor(Qt::PointingHandCursor);
    }

    return QTextEdit::mouseMoveEvent(e);
}