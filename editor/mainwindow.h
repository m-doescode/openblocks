#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "explorermodel.h"
#include "qbasictimer.h"
#include "qcoreevent.h"
#include "qmenu.h"
#include <QMainWindow>
#include <QLineEdit>

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
    
private:
    ExplorerModel explorerModel;
    QMenu explorerMenu;
    Ui::MainWindow *ui;
    QBasicTimer timer;

    void timerEvent(QTimerEvent*) override;
};
#endif // MAINWINDOW_H
