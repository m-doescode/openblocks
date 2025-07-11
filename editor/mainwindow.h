#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "logger.h"
#include "placedocument.h"
#include "qbasictimer.h"
#include "qcoreevent.h"
#include "script/scriptdocument.h"
#include "undohistory.h"
#include <QMainWindow>
#include <QLineEdit>
#include <map>
#include <memory>
#include <qfiledialog.h>
#include <qmdisubwindow.h>

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

    UndoHistory undoManager;
    
    SelectedTool selectedTool;
    GridSnappingMode snappingMode;
    bool editSoundEffects = true;

    void openScriptDocument(std::shared_ptr<Script>);
    void closeScriptDocument(std::shared_ptr<Script>);

    void openFile(std::string path);

    Ui::MainWindow *ui;

    friend PlaceDocument;
private:
    PlaceDocument* placeDocument;

    void setUpCommandBar();
    void connectActionHandlers();
    void updateToolbars();
    void closeEvent(QCloseEvent* evt) override;
    ScriptDocument* findScriptWindow(std::shared_ptr<Script>);
    
    std::optional<std::string> openFileDialog(QString filter, QString defaultExtension, QFileDialog::AcceptMode acceptMode, QString title = "");
};
#endif // MAINWINDOW_H
