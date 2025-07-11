#pragma once

#include "logger.h"
#include <memory>
#include <qobjectdefs.h>
#include <qtextedit.h>

class Script;

class OutputTextView : public QTextEdit {
    Q_OBJECT
private:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;

    void handleLog(Logger::LogLevel, std::string, Logger::ScriptSource source);

    std::map<int, std::weak_ptr<Script>> stackTraceScripts;
    int stackTraceScriptsLastId = 0;
public:
    OutputTextView(QWidget* parent = nullptr);
    ~OutputTextView() override;
};