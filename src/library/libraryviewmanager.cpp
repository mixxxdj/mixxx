#include "libraryviewmanager.h"
#include "util/assert.h"

LibraryViewManager::LibraryViewManager(QObject* parent)
        : QObject(parent) {

}

bool LibraryViewManager::initialize() {

    m_pButtonBar = new WButtonBar;
    m_pLeftPane = new QStackedWidget;


    for (int i = 0; i < RIGHT_PANE_COUNT; ++i) {
        m_rightPaneStack.append(new QStackedWidget);
        m_searchBar.append(new WSearchLineEdit);
        m_rightPane.append(new QWidget);

        QVBoxLayout* layout = new QVBoxLayout;
        layout->addWidget(m_searchBar[i]);
        layout->addWidget(m_rightPaneStack[i]);
        m_rightPane[i]->setLayout(layout);
        
        connect(m_searchBar[i], SIGNAL(search(QString)), 
                this, SLOT(onSearch(QString)));
    }
}

void LibraryViewManager::onSearch(QString& text) {
    LibraryViewFeature *feature = m_features[m_currentFeature[m_currentPane]];
    feature->onSearch(text, m_rightPaneStack[m_currentPane]->currentIndex());
}

void LibraryViewManager::addFeature(LibraryViewFeature* feature) {
    // Every feature ID will be it's position in the features' vector
    int ID = m_features.size();
    m_features.append(feature);
    m_pLeftPane->addWidget(feature->getLeftPane());

    for (QStackedWidget* pane : m_rightPaneStack) {
        pane->addWidget(feature->getRightPane());
    }

    m_pButtonBar->addItem(feature->getIcon(), feature->getTitle(), ID);
}

void LibraryViewManager::onFocusChange() {

}
