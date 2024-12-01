#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QStyleFactory>
#include <QBasicTimer>

#include "physics/simulation.h"
#include "qcoreevent.h"
#include "qobject.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    MainWindow w;
    w.show();
    return a.exec();
}
