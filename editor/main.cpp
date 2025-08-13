#include "mainwindow.h"

#include "logger.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QStyleFactory>
#include <QBasicTimer>
#include <QSurfaceFormat>
#include <cstdio>
#include <qfont.h>
#include <qsurfaceformat.h>
#include <miniaudio.h>
#include <qcursorconstraints.h>

ma_engine miniaudio;

int main(int argc, char *argv[])
{
    Logger::init();

    // Has to happen before Qt application initializes or we get an error in WASAPI initialization
    ma_result res = ma_engine_init(NULL, &miniaudio);
    if (res != MA_SUCCESS) {
        Logger::fatalErrorf("Failed to initialize Miniaudio withe error [%d]", res);
        panic();
    }

    QSurfaceFormat format;
    format.setSamples(4);
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CompatibilityProfile); // Valid only in OpenGL 3.2+, see: https://stackoverflow.com/a/70519392/16255372
    QSurfaceFormat::setDefaultFormat(format);

    QApplication a(argc, argv);
    QCursorConstraints::init();

    MainWindow w;
    w.show();
    int result = a.exec();

    ma_engine_uninit(&miniaudio);
    Logger::finish();
    return result;
}
