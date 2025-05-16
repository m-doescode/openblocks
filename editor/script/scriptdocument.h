#pragma once

#include "datatypes/signal.h"
#include <Qsci/qsciscintilla.h>
#include <memory>
#include <qmdisubwindow.h>

class Script;

class ScriptDocument : public QMdiSubWindow {
    std::shared_ptr<Script> script;
    SignalConnectionHolder scriptDeletionHandler;

    QsciScintilla* scintilla;
public:
    ScriptDocument(std::shared_ptr<Script> script, QWidget* parent = nullptr);
    ~ScriptDocument() override;
};