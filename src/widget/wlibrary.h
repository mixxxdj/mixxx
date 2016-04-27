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
#include "widget/wbasewidget.h"

class KeyboardEventFilter;

class WLibrary : public QStackedWidget, public WBaseWidget {
    Q_OBJECT
  public:
    explicit WLibrary(QWidget* parent);

    // registerView is used to add a view to the LibraryWidget which the widget
    // can disply on request via showView(). To switch to a given view, call
    // showView with the name provided here. WLibraryWidget takes ownership of
    // the view and is in charge of deleting it. Returns whether or not the
    // registration was successful. Registered widget must implement the
    // LibraryView interface.
    bool registerView(QString name, QWidget* view);

    LibraryView* getActiveView() const;

  public slots:
    // Show the view registered with the given name. Does nothing if the current
    // view is the specified view, or if the name does not specify any
    // registered view.
    void switchToView(const QString& name);

    void search(const QString&);

  protected:
    bool event(QEvent* pEvent) override;

  private:
    QMutex m_mutex;
    QMap<QString, QWidget*> m_viewMap;
};

#endif /* WLIBRARY_H */
