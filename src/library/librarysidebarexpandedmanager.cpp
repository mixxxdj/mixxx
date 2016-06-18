#include "librarysidebarexpandedmanager.h"
#include "library/libraryfeature.h"

LibrarySidebarExpandedManager::LibrarySidebarExpandedManager(QObject* parent)
        : LibraryPaneManager(-1, parent) {

}

void LibrarySidebarExpandedManager::bindPaneWidget(WBaseLibrary* libraryWidget,
                                                   KeyboardEventFilter* pKeyboard) {

    m_pPaneWidget = libraryWidget;

    connect(this, SIGNAL(switchToView(const QString&)),
            m_pPaneWidget, SLOT(switchToView(const QString&)));

    for (LibraryFeature* f : m_features) {
        f->bindSidebarWidget(m_pPaneWidget, pKeyboard);
    }
}

