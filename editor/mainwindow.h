#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "logger.h"
#include "placedocument.h"
#include "qbasictimer.h"
#include "qcoreevent.h"
#include "script/scriptdocument.h"
#include <QMainWindow>
#include <QLineEdit>
#include <map>
#include <memory>
#include <qfiledialog.h>

enum SelectedTool {
    TOOL_SELECT,
    TOOL_MOVE,
    TOOL_SCALE,
    TOOL_ROTATE,

    TOOL_SMOOTH,
    TOOL_GLUE,
    TOOL_WELD,
    TOOL_STUDS,
    TOOL_INLETS,
    TOOL_UNIVERSAL,
    TOOL_HINGE,
    TOOL_MOTOR,
};

enum GridSnappingMode {
    SNAP_1_STUD,
    SNAP_05_STUDS,
    SNAP_OFF,
};

class Script;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
    SelectedTool selectedTool;
    GridSnappingMode snappingMode;
    bool editSoundEffects = true;

    void openScriptDocument(std::shared_ptr<Script>);
    void closeScriptDocument(std::shared_ptr<Script>);

    Ui::MainWindow *ui;
private:
    PlaceDocument* placeDocument;
    std::map<std::shared_ptr<Script>, ScriptDocument*> scriptDocuments;

    void updateToolbars();
    void closeEvent(QCloseEvent* evt) override;

    void connectActionHandlers();
    
    std::optional<std::string> openFileDialog(QString filter, QString defaultExtension, QFileDialog::AcceptMode acceptMode, QString title = "");
};
#endif // MAINWINDOW_H
