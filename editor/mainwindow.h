#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "logger.h"
#include "placedocument.h"
#include "qbasictimer.h"
#include "qcoreevent.h"
#include <QMainWindow>
#include <QLineEdit>
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
};

enum GridSnappingMode {
    SNAP_1_STUD,
    SNAP_05_STUDS,
    SNAP_OFF,
};

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

    Ui::MainWindow *ui;
private:
    PlaceDocument* placeDocument;

    void updateToolbars();
    void closeEvent(QCloseEvent* evt) override;
    void handleLog(Logger::LogLevel, std::string);

    void connectActionHandlers();
    
    std::optional<std::string> openFileDialog(QString filter, QString defaultExtension, QFileDialog::AcceptMode acceptMode, QString title = "");
};
#endif // MAINWINDOW_H
