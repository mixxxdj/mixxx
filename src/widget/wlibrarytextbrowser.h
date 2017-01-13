// wlibrarytextbrowser.h
// Created 10/23/2009 by RJ Ryan (rryan@mit.edu)

#ifndef WLIBRARYTEXTBROWSER_H
#define WLIBRARYTEXTBROWSER_H

#include <QTextBrowser>

#include "library/libraryview.h"

class WLibraryTextBrowser : public QTextBrowser, public LibraryView {
    Q_OBJECT
  public:
    explicit WLibraryTextBrowser(QWidget* parent = nullptr);

    void onShow() override {}
};

#endif /* WLIBRARYTEXTBROWSER_H */
