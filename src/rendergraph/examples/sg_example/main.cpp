#include <QGuiApplication>
#include <QtQuick/QQuickView>
#include <iostream>

#include "customitem.h"

int main(int argc, char** argv) {
    QGuiApplication app(argc, argv);

    QQuickView view;
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.setSource(QUrl("qrc:///example/qml/main.qml"));
    view.show();

    return app.exec();
}
