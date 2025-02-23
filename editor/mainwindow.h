#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "panes/explorerview.h"
#include "qbasictimer.h"
#include "qcoreevent.h"
#include "qmenu.h"
#include <QMainWindow>
#include <QLineEdit>

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
    void ConnectSelectionChangeHandler();
};
#endif // MAINWINDOW_H
