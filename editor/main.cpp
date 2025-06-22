#include "mainwindow.h"

#include "logger.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QStyleFactory>
#include <QBasicTimer>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Logger::init();

    MainWindow w;
    w.show();
    int result = a.exec();

    Logger::finish();
    return result;
}
