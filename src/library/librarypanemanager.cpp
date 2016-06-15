#include <QDebug>

#include "librarypanemanager.h"
#include "util/assert.h"

const QString LibraryPaneManager::m_sTrackViewName = QString("WTrackTableView");

LibraryPaneManager::LibraryPaneManager(QObject* parent)
        : QObject(parent),
          m_pPaneWidget(nullptr){
    qApp->installEventFilter(this);
}

LibraryPaneManager::~LibraryPaneManager() {
}

void LibraryPaneManager::bindPaneWidget(WBaseLibrary* libraryWidget,
                                        KeyboardEventFilter* pKeyboard,
                                        FeaturePane pane) {
    //qDebug() << "LibraryPaneManager::bindLibraryWidget" << libraryWidget;

    m_pPaneWidget = libraryWidget;

    connect(this, SIGNAL(switchToView(const QString&)),
        m_pPaneWidget, SLOT(switchToView(const QString&)));

    switch (pane) {
        case FeaturePane::SidebarExpanded:
            //qDebug() << "LibraryPaneManager::bindLibraryWidget:SidebarExpanded";
            for (LibraryFeature* f : m_features) {
                f->bindSidebarWidget(m_pPaneWidget, pKeyboard);
            }
            break;
        case FeaturePane::TrackTable:
            //qDebug() << "LibraryPaneManager::bindLibraryWidget:TrackTable";
            WLibrary* lib = qobject_cast<WLibrary*>(m_pPaneWidget);
            if (lib == nullptr) {
                return;
            }
            for (LibraryFeature* f : m_features) {
                f->bindPaneWidget(lib, pKeyboard);
            }
            break;
    }
}

void LibraryPaneManager::bindSearchBar(WSearchLineEdit *pSearchLine) {
    pSearchLine->installEventFilter(this);
    
    connect(pSearchLine, SIGNAL(search(const QString&)),
            this, SIGNAL(search(const QString&)));
    connect(pSearchLine, SIGNAL(searchCleared()),
            this, SIGNAL(searchCleared()));
    connect(pSearchLine, SIGNAL(searchStarting()),
            this, SIGNAL(searchStarting()));
    connect(this, SIGNAL(restoreSearch(const QString&)),
            pSearchLine, SLOT(restoreSearch(const QString&)));
}

void LibraryPaneManager::addFeature(LibraryFeature* feature) {
    DEBUG_ASSERT_AND_HANDLE(feature) {
        return;
    }
    
    m_features.append(feature);
}

void LibraryPaneManager::addFeatures(const QList<LibraryFeature *> &features) {
    m_features.append(features);
}

WBaseLibrary *LibraryPaneManager::getPaneWidget() {
    return m_pPaneWidget;
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
    if (m_pPaneWidget == nullptr) return false;
    
    if (event->type() == QEvent::MouseButtonPress && 
            m_pPaneWidget->underMouse()) {
            emit(focused());
    }
    
    // Since this event filter is for the entire application (to handle the
    // mouse event), NEVER return true. If true is returned I will block all
    // application events and will block the entire application.
    return false;
}
