#include <QDebug>

#include "library/libraryfeature.h"
#include "library/library.h"
#include "library/librarypanemanager.h"
#include "util/assert.h"
#include "widget/wbuttonbar.h"
#include "widget/wlibrarybreadcrumb.h"
#include "widget/wtracktableview.h"

LibraryPaneManager::LibraryPaneManager(int paneId, Library *pLibrary, QObject* parent)
        : QObject(parent),
          m_pPaneWidget(nullptr),
          m_pBreadCrumb(nullptr),
          m_paneId(paneId),
          m_pLibrary(pLibrary) {
    qApp->installEventFilter(this);
}

LibraryPaneManager::~LibraryPaneManager() {
}

void LibraryPaneManager::bindPaneWidget(WBaseLibrary* pLibraryWidget,
                                        KeyboardEventFilter* pKeyboard) {
    //qDebug() << "LibraryPaneManager::bindLibraryWidget" << libraryWidget;
    m_pPaneWidget = pLibraryWidget;
    
    connect(m_pPaneWidget, SIGNAL(focused()),
            this, SLOT(slotPaneFocused()));
    connect(m_pPaneWidget, SIGNAL(collapsed()),
            this, SLOT(slotPaneCollapsed()));
    connect(m_pPaneWidget, SIGNAL(uncollapsed()),
            this, SLOT(slotPaneUncollapsed()));

    WLibrary* lib = qobject_cast<WLibrary*>(m_pPaneWidget);
    if (lib == nullptr) {
        return;
    }
    for (LibraryFeature* f : m_features) {
        //f->bindPaneWidget(lib, pKeyboard, m_paneId);
        
        QWidget* pPane = f->createPaneWidget(pKeyboard, m_paneId);
        if (pPane == nullptr) {
            continue;
        }
        pPane->setParent(lib);
        lib->registerView(f, pPane);
    }
}

void LibraryPaneManager::bindSearchBar(WSearchLineEdit* pSearchBar) {
    pSearchBar->installEventFilter(this);

    connect(pSearchBar, SIGNAL(search(const QString&)),
            this, SIGNAL(search(const QString&)));
    connect(pSearchBar, SIGNAL(searchCleared()),
            this, SIGNAL(searchCleared()));
    connect(pSearchBar, SIGNAL(searchStarting()),
            this, SIGNAL(searchStarting()));
    
    m_pSearchBar = pSearchBar;
}

void LibraryPaneManager::setBreadCrumb(WLibraryBreadCrumb *pBreadCrumb) {
    m_pBreadCrumb = pBreadCrumb;
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

void LibraryPaneManager::setCurrentFeature(LibraryFeature* pFeature) {
    m_pCurrentFeature = pFeature;
}

LibraryFeature *LibraryPaneManager::getCurrentFeature() const {
    return m_pCurrentFeature;
}

void LibraryPaneManager::setFocus() {
    //qDebug() << "LibraryPaneManager::setFocus";
    DEBUG_ASSERT_AND_HANDLE(m_pPaneWidget) {
        return;
    }
    
    m_pPaneWidget->setProperty("showFocus", 1);
}

void LibraryPaneManager::clearFocus() {
    //qDebug() << "LibraryPaneManager::clearFocus";
    m_pPaneWidget->setProperty("showFocus", 0);
}

void LibraryPaneManager::switchToFeature(LibraryFeature* pFeature) {
    DEBUG_ASSERT_AND_HANDLE(!m_pPaneWidget.isNull() && pFeature) {
        return;
    }
    
    m_pCurrentFeature = pFeature;
    m_pPaneWidget->switchToFeature(pFeature);
}

void LibraryPaneManager::restoreSearch(const QString& text) {
    if (!m_pSearchBar.isNull()) {
        m_pSearchBar->restoreSearch(text);
    }
}

void LibraryPaneManager::showBreadCrumb(TreeItem *pTree) {
    DEBUG_ASSERT_AND_HANDLE(!m_pBreadCrumb.isNull()) {
        return;
    }
    
    m_pBreadCrumb->showBreadCrumb(pTree);
}

void LibraryPaneManager::slotPaneCollapsed() {
    m_pLibrary->paneCollapsed(m_paneId);
}

void LibraryPaneManager::slotPaneUncollapsed() {
    m_pLibrary->paneUncollapsed(m_paneId);
}

void LibraryPaneManager::slotPaneFocused() {
    m_pLibrary->slotPaneFocused(this);
}

bool LibraryPaneManager::eventFilter(QObject*, QEvent* event) {
    if (m_pPaneWidget.isNull()) {
        return false;
    }

    if (event->type() == QEvent::MouseButtonPress &&
        m_pPaneWidget->underMouse()) {
        m_pLibrary->slotPaneFocused(this);
    }

    // Since this event filter is for the entire application (to handle the
    // mouse event), NEVER return true. If true is returned I will block all
    // application events and will block the entire application.
    return false;
}
