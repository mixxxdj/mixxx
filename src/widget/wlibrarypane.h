// wlibrary.h
// Created 8/28/2009 by RJ Ryan (rryan@mit.edu)

#ifndef WLIBRARY_H
#define WLIBRARY_H

#include <QMap>
#include <QMutex>
#include <QStackedWidget>
#include <QString>
#include <QEvent>

#include "library/libraryview.h"
#include "widget/wbaselibrary.h"

class KeyboardEventFilter;

class WLibraryPane : public WBaseLibrary {
    Q_OBJECT
  public:
    explicit WLibraryPane(QWidget* parent);

    // registerView is used to add a view to the LibraryWidget which the widget
    // can disply on request via showView(). To switch to a given view, call
    // showView with the name provided here. WLibraryWidget takes ownership of
    // the view and is in charge of deleting it. Returns whether or not the
    // registration was successful. Registered widget must implement the
    // LibraryView interface.
    bool registerView(LibraryFeature* pFeature, QWidget* pView);

    LibraryView* getActiveView() const;

    // Show the view registered with the given feature. Does nothing if the current
    // view is the specified view, or if the name does not specify any
    // registered view.
    void switchToFeature(LibraryFeature* pFeature) override;
    void search(const QString& name) override;
    void searchCleared() override;
    void searchStarting() override;
};

#endif /* WLIBRARY_H */
