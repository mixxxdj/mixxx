#include <QDebug>

#include "librarypanemanager.h"
#include "util/assert.h"

LibraryPaneManager::LibraryPaneManager(QObject* parent)
        : QObject(parent) {

}

bool LibraryPaneManager::initialize() {
    m_pLeftPane = nullptr;
    m_pRightPane = nullptr;
}

void LibraryPaneManager::bindLeftPane(WLibrary* leftWidget) {

}

void LibraryPaneManager::bindRightPane(WLibrary* rightWidget) {

}

void LibraryPaneManager::search(QString& text) {

}

void LibraryPaneManager::addFeature(LibraryFeature* feature) {


}

bool LibraryPaneManager::eventFilter(QObject* object, QEvent* event) {
    //QObject::eventFilter(object, event);

    if (event->type() == QEvent::FocusIn) {
        qDebug() << object;
    }
    return true;
}
