// wlibrary.h
// Created 8/28/2009 by RJ Ryan (rryan@mit.edu)

#ifndef WLIBRARY_H
#define WLIBRARY_H

#include <QDomNode>
#include <QMap>
#include <QMutex>
#include <QStackedWidget>
#include <QString>

class WLibrary : public QStackedWidget {
    Q_OBJECT
public:
    WLibrary(QWidget* parent = NULL);
    virtual ~WLibrary();

    // registerView is used to add a view to the LibraryWidget which the widget
    // can disply on request via showView(). To switch to a given view, call
    // showView with the name provided here. WLibraryWidget takes ownership of
    // the view and is in charge of deleting it. Returns whether or not the
    // registration was successful.
    bool registerView(QString name, QWidget* view);

    // Apply skin-specific customizations to the library views. Will not affect
    // any views registered after calling setup(), so the Library must be bound
    // to this widget before calling setup().
    void setup(QDomNode node);

public slots:
    // Show the view registered with the given name. Does nothing if the current
    // view is the specified view, or if the name does not specify any
    // registered view.
    void switchToView(const QString& name);

    void search(const QString&);
    void searchStarting();
    void searchCleared();

private:
    QMutex m_mutex;
    QMap<QString, QWidget*> m_viewMap;
    QMap<QString, QString> m_searchMap;
    QString m_currentView;
};

#endif /* WLIBRARY_H */
