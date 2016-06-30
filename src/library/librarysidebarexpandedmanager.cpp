#include "librarysidebarexpandedmanager.h"
#include "library/libraryfeature.h"

LibrarySidebarExpandedManager::LibrarySidebarExpandedManager(QObject* parent)
        : LibraryPaneManager(-1, parent) {

}

void LibrarySidebarExpandedManager::bindPaneWidget(WBaseLibrary* sidebarWidget,
                                                   KeyboardEventFilter* pKeyboard) {

    m_pPaneWidget = sidebarWidget;

    connect(this, SIGNAL(switchToView(const QString&)),
            m_pPaneWidget, SLOT(switchToView(const QString&)));

    for (LibraryFeature* f : m_features) {
        //f->bindSidebarWidget(m_pPaneWidget, pKeyboard);
        
        QWidget* pPane = f->createSidebarWidget(pKeyboard);
        if (pPane == nullptr) {
            continue;
        }
        pPane->setParent(sidebarWidget);
        sidebarWidget->registerView(f->getViewName(), pPane);
        m_featuresWidget[f] = sidebarWidget->indexOf(pPane);
    }
}

