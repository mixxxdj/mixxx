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
}

LibraryPaneManager::~LibraryPaneManager() {
}

void LibraryPaneManager::bindPaneWidget(WBaseLibrary* pPaneWidget,
                                        KeyboardEventFilter* pKeyboard) {
    //qDebug() << "LibraryPaneManager::bindLibraryWidget" << libraryWidget;
    m_pPaneWidget = pPaneWidget;
    
    connect(pPaneWidget, SIGNAL(focused()),
            this, SLOT(slotPaneFocused()));
    connect(pPaneWidget, SIGNAL(collapsed()),
            this, SLOT(slotPaneCollapsed()));
    connect(pPaneWidget, SIGNAL(uncollapsed()),
            this, SLOT(slotPaneUncollapsed()));

    if (qobject_cast<WLibraryPane*>(pPaneWidget) == nullptr) {
        return;
    }
    for (LibraryFeature* f : m_features) {
        //f->bindPaneWidget(pPaneWidget, pKeyboard, m_paneId);
        
        QWidget* pFeaturePaneWidget = f->createPaneWidget(pKeyboard, m_paneId);
        if (pFeaturePaneWidget == nullptr) {
            continue;
        }
        pFeaturePaneWidget->setParent(pPaneWidget);
        pPaneWidget->registerView(f, pFeaturePaneWidget);
    }
}

void LibraryPaneManager::bindSearchBar(WSearchLineEdit* pSearchBar) {
    pSearchBar->installEventFilter(this);

    connect(pSearchBar, SIGNAL(search(const QString&)),
            this, SLOT(slotSearch(const QString&)));
    connect(pSearchBar, SIGNAL(searchCleared()),
            this, SLOT(slotSearchCleared()));
    connect(pSearchBar, SIGNAL(searchStarting()),
            this, SLOT(slotSearchStarting()));
    connect(pSearchBar, SIGNAL(focused()),
            this, SLOT(slotPaneFocused()));
    connect(pSearchBar, SIGNAL(cancel()),
            this, SLOT(slotSearchCancel()));

    m_pSearchBar = pSearchBar;
}

void LibraryPaneManager::setBreadCrumb(WLibraryBreadCrumb* pBreadCrumb) {
    m_pBreadCrumb = pBreadCrumb;
    connect(m_pBreadCrumb, SIGNAL(preselected(bool)),
            this, SLOT(slotPanePreselected(bool)));
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

void LibraryPaneManager::setFocused(bool value) {
    DEBUG_ASSERT_AND_HANDLE(m_pPaneWidget) {
        return;
    }
    
    m_pPaneWidget->setProperty("showFocus", (int) value);
}

void LibraryPaneManager::switchToFeature(LibraryFeature* pFeature) {
    m_pCurrentFeature = pFeature;
    if (!m_pPaneWidget.isNull()) {
        m_pPaneWidget->switchToFeature(pFeature);
    }
}

void LibraryPaneManager::restoreSearch(const QString& text) {
    if (!m_pSearchBar.isNull()) {
        m_pSearchBar->restoreSearch(text, m_pCurrentFeature);
    }
}

void LibraryPaneManager::restoreSaveButton() {
    if (!m_pSearchBar.isNull()) {
        m_pSearchBar->slotRestoreSaveButton();
    }
}

void LibraryPaneManager::showBreadCrumb(TreeItem *pTree) {
    DEBUG_ASSERT_AND_HANDLE(!m_pBreadCrumb.isNull()) {
        return;
    }
    
    m_pBreadCrumb->showBreadCrumb(pTree);
}

void LibraryPaneManager::showBreadCrumb(const QString &text, const QIcon& icon) {
    DEBUG_ASSERT_AND_HANDLE(!m_pBreadCrumb.isNull()) {
        return;
    }
    m_pBreadCrumb->showBreadCrumb(text, icon);
}

int LibraryPaneManager::getPaneId() const {
    return m_paneId;
}

void LibraryPaneManager::setPreselected(bool value) {
    if (!m_pBreadCrumb.isNull()) {
        m_pBreadCrumb->setPreselected(value);
    }
}

bool LibraryPaneManager::isPreselected() const {
    if (!m_pBreadCrumb.isNull()) {
        return m_pBreadCrumb->isPreselected();
    }
    return false;
}

void LibraryPaneManager::setPreviewed(bool value) {
    if (!m_pBreadCrumb.isNull()) {
        m_pBreadCrumb->setPreviewed(value);
    }
}

void LibraryPaneManager::slotPanePreselected(bool value) {
    // Preselect button clicked
    setPreselected(value);
    m_pLibrary->panePreselected(this, value);
}

void LibraryPaneManager::slotPaneCollapsed() {
    m_pLibrary->paneCollapsed(m_paneId);
}

void LibraryPaneManager::slotPaneUncollapsed() {
    m_pLibrary->paneUncollapsed(m_paneId);
}

void LibraryPaneManager::slotPaneFocused() {
    m_pLibrary->paneFocused(this);
}

void LibraryPaneManager::slotSearchCancel() {
    if (!m_pPaneWidget.isNull()) {
        QWidget* cw = m_pPaneWidget->currentWidget();
        if (cw != nullptr) {
            cw->setFocus();
        }
    }
}

void LibraryPaneManager::slotSearch(const QString& text) {
    DEBUG_ASSERT_AND_HANDLE(!m_pPaneWidget.isNull()) {
        return;    
    }
    m_pPaneWidget->search(text);
    m_pCurrentFeature->onSearch(text);
}

void LibraryPaneManager::slotSearchStarting() {
    if (!m_pPaneWidget.isNull()) {
        m_pPaneWidget->searchStarting();
    }
}

void LibraryPaneManager::slotSearchCleared() {
    if (!m_pPaneWidget.isNull()) {
        m_pPaneWidget->searchCleared();
    }
}

bool LibraryPaneManager::eventFilter(QObject*, QEvent* event) {
    if (m_pPaneWidget.isNull() || m_pSearchBar.isNull()) {
        return false;
    }

    if (event->type() == QEvent::MouseButtonPress &&
            (m_pPaneWidget->underMouse() || m_pSearchBar->underMouse())) {
        m_pLibrary->paneFocused(this);
    } else if (event->type() == QEvent::FocusIn) {
        m_pLibrary->paneFocused(this);
    }
    return false;
}

bool LibraryPaneManager::focusSearch() {
    if (m_pSearchBar.isNull()) {
        return false;
    }
    if (!m_pSearchBar->isEnabled()) {
        return false;
    }
    m_pSearchBar->setFocus();
    return true;
}
