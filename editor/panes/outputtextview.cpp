#include "outputtextview.h"
#include "logger.h"
#include "mainwindow.h"
#include "panes/outputtextview.h"
#include <QEvent>
#include <QTextEdit>
#include <QWidget>
#include <QMouseEvent>
#include <qcursor.h>
#include <qfont.h>
#include <qfontdatabase.h>
#include <qmenu.h>
#include <qnamespace.h>
#include <string>

OutputTextView::OutputTextView(QWidget* parent) : QTextEdit(parent) {
    Logger::addLogListener(std::bind(&OutputTextView::handleLog, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
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

void OutputTextView::handleLog(Logger::LogLevel logLevel, std::string message, Logger::ScriptSource source) {
    if (logLevel == Logger::LogLevel::DEBUG) return;

    // https://stackoverflow.com/a/61722734/16255372
    moveCursor(QTextCursor::MoveOperation::End);
    QTextCursor cursor = textCursor();
    QTextCharFormat format = cursor.charFormat();
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    format.setFont(font);

    if (logLevel == Logger::LogLevel::TRACE) {
        format.setForeground(QColor(0, 127, 255));
    } else if (logLevel == Logger::LogLevel::WARNING) {
        format.setForeground(QColor(255, 127, 0));
        format.setFontWeight(QFont::Bold); // https://forum.qt.io/post/505920
    } else if (logLevel == Logger::LogLevel::ERROR || logLevel == Logger::LogLevel::FATAL_ERROR) {
        format.setForeground(QColor(255, 0, 0));
        format.setFontWeight(QFont::Bold);
    }

    // Add anchor point if source is provided
    if (source.script != nullptr) {
        int id = stackTraceScriptsLastId++;
        stackTraceScripts[id] = source.script;

        format.setAnchor(true);
        format.setAnchorHref(QString::number(id) + ":" + QString::number(source.line));
    }

    cursor.insertText(message.c_str(), format);
    cursor.insertText("\n", QTextCharFormat());
}

// https://stackoverflow.com/a/61722734/16255372
void OutputTextView::mousePressEvent(QMouseEvent *e) {
    QString anchor = anchorAt(e->pos());
    if (anchor == "" || e->modifiers() & Qt::AltModifier) return QTextEdit::mousePressEvent(e);

    int idx = anchor.indexOf(":");
    int id = anchor.mid(0, idx).toInt(), line = anchor.mid(idx+1).toInt();
    auto script = stackTraceScripts[id];
    if (script.expired()) return QTextEdit::mousePressEvent(e);

    MainWindow* mainWnd = dynamic_cast<MainWindow*>(window());
    mainWnd->openScriptDocument(script.lock(), line);
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