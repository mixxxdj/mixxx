#include "librarysidebarexpandedmanager.h"
#include "library/libraryfeature.h"

LibrarySidebarExpandedManager::LibrarySidebarExpandedManager(Library *pLibrary, 
                                                             QObject* parent)
        : LibraryPaneManager(-1, pLibrary, parent) {

}

void LibrarySidebarExpandedManager::bindPaneWidget(const parented_ptr<WBaseLibrary>&sidebarWidget,
                                                   KeyboardEventFilter* pKeyboard) {
    m_pPaneWidget = sidebarWidget.get();

    for (LibraryFeature* f : m_features) {        
        QWidget* pPane = f->createSidebarWidget(pKeyboard);
        if (pPane == nullptr) {
            continue;
        }
        pPane->setParent(m_pPaneWidget);
        m_pPaneWidget->registerView(f, pPane);
    }
}

