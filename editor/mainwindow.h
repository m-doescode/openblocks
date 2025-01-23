#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "panes/explorerview.h"
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
    
    Ui::MainWindow *ui;
private:
    QBasicTimer timer;

    void timerEvent(QTimerEvent*) override;
};
#endif // MAINWINDOW_H
