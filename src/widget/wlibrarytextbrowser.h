// wlibrarytextbrowser.h
// Created 10/23/2009 by RJ Ryan (rryan@mit.edu)

#ifndef WLIBRARYTEXTBROWSER_H
#define WLIBRARYTEXTBROWSER_H

#include <QByteArrayData>
#include <QString>
#include <QTextBrowser>

#include "library/libraryview.h"

class QObject;
class QWidget;

class WLibraryTextBrowser : public QTextBrowser, public LibraryView {
    Q_OBJECT
  public:
    explicit WLibraryTextBrowser(QWidget* parent = nullptr);
    void onShow() override {}
    bool hasFocus() const override;
};

#endif /* WLIBRARYTEXTBROWSER_H */
