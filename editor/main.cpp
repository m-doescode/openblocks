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
    // TODO: This is broken for some reason on Linux, Qt refuses to use any version newer than 3.2. Figure out why
    // format.setVersion(3, 3);
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
