// wlibrarytextbrowser.h
// Created 10/23/2009 by RJ Ryan (rryan@mit.edu)

#ifndef WLIBRARYTEXTBROWSER_H
#define WLIBRARYTEXTBROWSER_H

#include <QTextBrowser>

#include "library/libraryview.h"

class WLibraryTextBrowser : public QTextBrowser, public virtual LibraryView {
    Q_OBJECT
  public:
    WLibraryTextBrowser(QWidget* parent = NULL);
    virtual ~WLibraryTextBrowser();

    virtual void setup(QDomNode node);
    void onSearchStarting();
    void onSearchCleared();
    void onSearch(const QString& text);
    void onShow();

};


#endif /* WLIBRARYTEXTBROWSER_H */
