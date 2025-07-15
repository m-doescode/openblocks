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
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CompatibilityProfile); // Valid only in OpenGL 3.2+, see: https://stackoverflow.com/a/70519392/16255372
    QSurfaceFormat::setDefaultFormat(format);

    QApplication a(argc, argv);

    Logger::init();

    MainWindow w;
    w.show();
    int result = a.exec();

    Logger::finish();
    return result;
}
