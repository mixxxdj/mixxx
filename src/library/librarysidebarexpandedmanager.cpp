#include "librarysidebarexpandedmanager.h"
#include "library/libraryfeature.h"

LibrarySidebarExpandedManager::LibrarySidebarExpandedManager(QObject* parent)
        : LibraryPaneManager(-1, parent) {

}

void LibrarySidebarExpandedManager::bindPaneWidget(WBaseLibrary* sidebarWidget,
                                                   KeyboardEventFilter* pKeyboard) {
    m_pPaneWidget = sidebarWidget;

    for (LibraryFeature* f : m_features) {        
        QWidget* pPane = f->createSidebarWidget(pKeyboard);
        if (pPane == nullptr) {
            continue;
        }
        pPane->setParent(sidebarWidget);
        sidebarWidget->registerView(f->getFeatureName(), pPane);
        m_featuresWidget[f] = sidebarWidget->indexOf(pPane);
    }
}

