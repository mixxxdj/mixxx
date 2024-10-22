#include <QGuiApplication>

#include "window.h"

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);

    Window window;
    window.show();

    return app.exec();
}
