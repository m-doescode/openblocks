#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "logger.h"
#include "panes/explorerview.h"
#include "qbasictimer.h"
#include "qcoreevent.h"
#include "qmenu.h"
#include <QMainWindow>
#include <QLineEdit>
#include <qfiledialog.h>

enum SelectedTool {
    SELECT,
    MOVE,
    SCALE,
    ROTATE,
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

    Ui::MainWindow *ui;
private:
    QBasicTimer timer;

    void updateToolbars();
    void timerEvent(QTimerEvent*) override;
    void closeEvent(QCloseEvent* evt) override;
    void handleLog(Logger::LogLevel, std::string);
    
    std::optional<std::string> openFileDialog(QString filter, QString defaultExtension, QFileDialog::AcceptMode acceptMode, QString title = "");
};
#endif // MAINWINDOW_H
