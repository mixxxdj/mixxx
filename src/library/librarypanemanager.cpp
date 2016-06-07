#include <QDebug>

#include "librarypanemanager.h"
#include "util/assert.h"

LibraryPaneManager::LibraryPaneManager(QObject* parent)
        : QObject(parent) {

}

LibraryPaneManager::~LibraryPaneManager() {
    for (LibraryFeature* f : m_features) {
        delete f;
    }
    m_features.clear();
}

bool LibraryPaneManager::initialize() {
    m_pSidebarExpanded = nullptr;
    m_pLibraryWidget = nullptr;
}

void LibraryPaneManager::bindSidebarExpanded(WLibrary* leftWidget,
                                             KeyboardEventFilter* pKeyboard) {

}

void LibraryPaneManager::bindLibraryWidget(WLibrary* rightWidget,
                                           KeyboardEventFilter* pKeyboard) {
    m_pLibraryWidget = rightWidget;

    for (LibraryFeature* f : m_features) {
        f->bindLibraryWidget(m_pLibraryWidget, pKeyboard);
    }
}

void LibraryPaneManager::search(QString& text) {

}

void LibraryPaneManager::addFeature(LibraryFeature* feature) {
    m_features.append(feature);

    connect(feature, SIGNAL(showTrackModel(QAbstractItemModel*)),
            this, SLOT(slotShowTrackModel(QAbstractItemModel*)));
    connect(feature, SIGNAL(switchToView(const QString&)),
            this, SLOT(slotSwitchToView(const QString&)));
    connect(feature, SIGNAL(loadTrack(TrackPointer)),
            this, SLOT(slotLoadTrack(TrackPointer)));
    connect(feature, SIGNAL(loadTrackToPlayer(TrackPointer, QString, bool)),
            this, SLOT(slotLoadTrackToPlayer(TrackPointer, QString, bool)));
    connect(feature, SIGNAL(restoreSearch(const QString&)),
            this, SLOT(slotRestoreSearch(const QString&)));
    connect(feature, SIGNAL(enableCoverArtDisplay(bool)),
            this, SIGNAL(enableCoverArtDisplay(bool)));
    connect(feature, SIGNAL(trackSelected(TrackPointer)),
            this, SIGNAL(trackSelected(TrackPointer)));
}

bool LibraryPaneManager::eventFilter(QObject* object, QEvent* event) {
    //QObject::eventFilter(object, event);

    if (event->type() == QEvent::FocusIn) {
        qDebug() << object;
    }
    return true;
}
