#include "EngineSimulator.h"
#include <QtWidgets/QApplication>

// Ö÷º¯Êý
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    EngineSimulator window;
    window.show();
    return app.exec();
}
