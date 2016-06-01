#include <QDebug>

#include "libraryviewmanager.h"
#include "util/assert.h"

LibraryViewManager::LibraryViewManager(QObject* parent)
        : QObject(parent) {

}

bool LibraryViewManager::initialize() {

    m_pButtonBar = new WButtonBar;
    m_pLeftPane = new QStackedWidget;
    m_features.resize(RIGHT_PANE_COUNT);

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
    return true;
}

void LibraryViewManager::onSearch(QString& text) {
    LibraryViewFeature* feature = m_features[m_currentPane][m_currentFeature[m_currentPane]];
    feature->onSearch(text);
}

void LibraryViewManager::addFeature(LibraryViewFeature* feature, int pane) {
    if (pane < 0 || pane >= RIGHT_PANE_COUNT) {
        return;
    }

    m_features[pane].append(feature);

    m_pLeftPane->addWidget(feature->getLeftPane());
    m_rightPaneStack[pane]->addWidget(feature->getRightPane());

    m_pButtonBar->addItem(feature->getIcon(), feature->getTitle(), feature->getName());
}

bool LibraryViewManager::eventFilter(QObject* object, QEvent* event) {
    //QObject::eventFilter(object, event);
    
    if (event->type() == QEvent::FocusIn) {
        qDebug() << object;
    }
    return true;
}
