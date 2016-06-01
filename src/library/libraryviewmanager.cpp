#include "libraryviewmanager.h"

LibraryViewManager::LibraryViewManager(QObject* parent)
        : QObject(parent) {

}

void LibraryViewManager::addFeature(LibraryViewFeature* feature) {
    // Every feature ID will be it's position in the features' vector
    int ID = m_pFeatures.size();
    m_pFeatures.append(feature);
    
    m_pLeftPane->addWidget(feature->getLeftPane());

    for (QStackedWidget* stack : m_pRightPane) {
        stack->addWidget(feature->getRightPane());
    }

    m_pButtonBar->addItem(feature->getIcon(), feature->getTitle(), ID);
}

void LibraryViewManager::onFocusChange() {

}
