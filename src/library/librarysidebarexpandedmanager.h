#ifndef LIBRARYSIDEBAREXPANDEDMANAGER_H
#define LIBRARYSIDEBAREXPANDEDMANAGER_H
#include "library/librarypanemanager.h"

class LibrarySidebarExpandedManager : public LibraryPaneManager
{
  public:
    LibrarySidebarExpandedManager(Library* pLibrary, QObject* parent = nullptr);

    void bindPaneWidget(WBaseLibrary* sidebarWidget,
                        KeyboardEventFilter* pKeyboard) override;
};

#endif // LIBRARYSIDEBAREXPANDEDMANAGER_H
