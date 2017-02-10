#include "librarysidebarexpandedmanager.h"
#include "library/libraryfeature.h"

LibrarySidebarExpandedManager::LibrarySidebarExpandedManager(Library *pLibrary, 
                                                             QObject* parent)
        : LibraryPaneManager(-1, pLibrary, parent) {

}

void LibrarySidebarExpandedManager::bindPaneWidget(WBaseLibrary* sidebarWidget,
                                                   KeyboardEventFilter* pKeyboard) {
    m_pPaneWidget = sidebarWidget;

    for (LibraryFeature* f : m_features) {        
        auto pPane = f->createSidebarWidget(pKeyboard, m_pPaneWidget);
        if (pPane.get() == nullptr) {
            continue;
        }
        m_pPaneWidget->registerView(f, pPane.get());
    }
}

