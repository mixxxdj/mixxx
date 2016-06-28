#include <QDebug>

#include "librarypanemanager.h"
#include "library/libraryfeature.h"
#include "widget/wtracktableview.h"
#include "widget/wbuttonbar.h"
#include "util/assert.h"

const QString LibraryPaneManager::m_sTrackViewName = QString("WTrackTableView");

LibraryPaneManager::LibraryPaneManager(int paneId, QObject* parent)
        : QObject(parent),
          m_pPaneWidget(nullptr),
          m_paneId(paneId) {
    qApp->installEventFilter(this);
}

LibraryPaneManager::~LibraryPaneManager() {
}

void LibraryPaneManager::bindPaneWidget(WBaseLibrary* pLibraryWidget,
                                        KeyboardEventFilter* pKeyboard) {
    //qDebug() << "LibraryPaneManager::bindLibraryWidget" << libraryWidget;
    m_pPaneWidget = pLibraryWidget;

    connect(this, SIGNAL(switchToView(const QString&)),
            m_pPaneWidget, SLOT(switchToView(const QString&)));
    connect(m_pPaneWidget, SIGNAL(focused()),
            this, SIGNAL(focused()));

    WLibrary* lib = qobject_cast<WLibrary*>(m_pPaneWidget);
    if (lib == nullptr) {
        return;
    }
    for (LibraryFeature* f : m_features) {
        f->bindPaneWidget(lib, pKeyboard, m_paneId);
    }
}

void LibraryPaneManager::bindSearchBar(WSearchLineEdit* pSearchLine) {
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

void LibraryPaneManager::addFeatures(const QList<LibraryFeature*>& features) {
    m_features.append(features);
}

WBaseLibrary* LibraryPaneManager::getPaneWidget() {
    return m_pPaneWidget;
}

void LibraryPaneManager::setFocusedFeature(const QString& featureName) {
    m_focusedFeature = featureName;
}

void LibraryPaneManager::setFocus() {
    //qDebug() << "LibraryPaneManager::setFocus";
    m_pPaneWidget->setProperty("showFocus", 1);
}

void LibraryPaneManager::clearFocus() {
    //qDebug() << "LibraryPaneManager::clearFocus";
    m_pPaneWidget->setProperty("showFocus", 0);
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
    m_pPaneWidget->setFocus();
}

void LibraryPaneManager::slotSwitchToView(LibraryFeature *pFeature) {
    
}

void LibraryPaneManager::slotRestoreSearch(const QString& text) {
    emit(restoreSearch(text));
}

bool LibraryPaneManager::eventFilter(QObject*, QEvent* event) {
    if (m_pPaneWidget == nullptr) {
        return false;
    }

    if (event->type() == QEvent::MouseButtonPress &&
        m_pPaneWidget->underMouse()) {
        emit(focused());
    }

    // Since this event filter is for the entire application (to handle the
    // mouse event), NEVER return true. If true is returned I will block all
    // application events and will block the entire application.
    return false;
}
