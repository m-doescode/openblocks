#include "mainwindow.h"

#include "logger.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QStyleFactory>
#include <QBasicTimer>
#include <QSurfaceFormat>
#include <qfont.h>
#include <qsurfaceformat.h>

int main(int argc, char *argv[])
{
    QSurfaceFormat format;
    format.setSamples(4);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication a(argc, argv);

    Logger::init();

    MainWindow w;
    w.show();
    int result = a.exec();

    Logger::finish();
    return result;
}
