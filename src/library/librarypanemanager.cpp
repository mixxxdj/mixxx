#include <QDebug>

#include "librarypanemanager.h"
#include "util/assert.h"

const QString LibraryPaneManager::m_sTrackViewName = QString("WTrackTableView");

LibraryPaneManager::LibraryPaneManager(QObject* parent)
        : QObject(parent),
          m_pLibraryWidget(nullptr),
          m_pTrackTable(nullptr),
          m_pSearchLine(nullptr) {

}

LibraryPaneManager::~LibraryPaneManager() {
}

void LibraryPaneManager::bindLibraryWidget(WLibrary* libraryWidget,
                                           KeyboardEventFilter* pKeyboard, 
                                           FeaturePane pane) {
    //qDebug() << "LibraryPaneManager::bindLibraryWidget" << libraryWidget;
    
    m_pLibraryWidget = libraryWidget;
    m_pLibraryWidget->installEventFilter(this);

    connect(this, SIGNAL(switchToView(const QString &)),
            m_pLibraryWidget, SLOT(switchToView(const QString &)));

    switch (pane) {
        case FeaturePane::SidebarExpanded:
            //qDebug() << "LibraryPaneManager::bindLibraryWidget:SidebarExpanded";
            for (LibraryFeature* f : m_features) {
                f->bindSidebarWidget(m_pLibraryWidget, pKeyboard);
            }
            break;
        case FeaturePane::TrackTable:
            //qDebug() << "LibraryPaneManager::bindLibraryWidget:TrackTable";
            for (LibraryFeature* f : m_features) {
                f->bindLibraryWidget(m_pLibraryWidget, pKeyboard);
            }
            break;
    }
}

void LibraryPaneManager::bindTrackTable(WTrackTableView *pTrackTable) {
    m_pTrackTable = pTrackTable;
    m_pTrackTable->installEventFilter(this);
    
    
    connect(this, SIGNAL(showTrackModel(QAbstractItemModel*)),
            m_pTrackTable, SLOT(loadTrackModel(QAbstractItemModel*)));
    connect(this, SIGNAL(searchStarting()),
            m_pTrackTable, SLOT(onSearchStarting()));
    connect(this, SIGNAL(searchCleared()),
            m_pTrackTable, SLOT(onSearchCleared()));
}

void LibraryPaneManager::bindSearchBar(WSearchLineEdit *pSearchLine) {
    m_pSearchLine = pSearchLine;
    m_pSearchLine->installEventFilter(this);
    
    connect(m_pSearchLine, SIGNAL(search(const QString&)),
            this, SIGNAL(search(const QString&)));
    connect(m_pSearchLine, SIGNAL(searchCleared()),
            this, SIGNAL(searchCleared()));
    connect(m_pSearchLine, SIGNAL(searchStarting()),
            this, SIGNAL(searchStarting()));
    connect(this, SIGNAL(restoreSearch(const QString&)),
            m_pSearchLine, SLOT(restoreSearch(const QString&)));
}

void LibraryPaneManager::addFeature(LibraryFeature* feature) {
    DEBUG_ASSERT_AND_HANDLE(feature) {
        return;
    }
    
    m_features.append(feature);

    
    /*connect(feature, SIGNAL(showTrackModel(QAbstractItemModel*)),
            this, SLOT(slotShowTrackModel(QAbstractItemModel*)));
    connect(feature, SIGNAL(switchToView(const QString&)),
            this, SLOT(slotSwitchToView(const QString&)));*/
    /*connect(feature, SIGNAL(loadTrack(TrackPointer)),
            this, SLOT(slotLoadTrack(TrackPointer)));
    connect(feature, SIGNAL(loadTrackToPlayer(TrackPointer, QString, bool)),
            this, SLOT(slotLoadTrackToPlayer(TrackPointer, QString, bool)));*/
    /*connect(feature, SIGNAL(restoreSearch(const QString&)),
            this, SLOT(slotRestoreSearch(const QString&)));*/
    /*connect(feature, SIGNAL(enableCoverArtDisplay(bool)),
            this, SIGNAL(enableCoverArtDisplay(bool)));*/
    /*connect(feature, SIGNAL(trackSelected(TrackPointer)),
            this, SIGNAL(trackSelected(TrackPointer)));*/
}

void LibraryPaneManager::addFeatures(const QList<LibraryFeature *> &features) {
    for (LibraryFeature* f : features) {
        addFeature(f);
    }
}

void LibraryPaneManager::slotShowTrackModel(QAbstractItemModel* model) {
    //qDebug() << "LibraryPaneManager::slotShowTrackModel" << model;
    TrackModel* trackModel = dynamic_cast<TrackModel*>(model);
    DEBUG_ASSERT_AND_HANDLE(trackModel) {
        return;
    }
    emit(showTrackModel(model));
    emit(switchToView(m_sTrackViewName));
    emit(restoreSearch(trackModel->currentSearch()));
}

void LibraryPaneManager::slotSwitchToView(const QString& view) {
    //qDebug() << "LibraryPaneManager::slotSwitchToView" << view;
    emit(switchToView(view));
}

void LibraryPaneManager::slotRestoreSearch(const QString& text) {
    emit(restoreSearch(text));
}

bool LibraryPaneManager::eventFilter(QObject*, QEvent* event) {

    if (event->type() == QEvent::FocusIn) {
        emit(focused());
    }
    return false;
}
