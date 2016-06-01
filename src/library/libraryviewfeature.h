#ifndef LIBRARYVIEWFEAUTRE_H
#define LIBRARYVIEWFEAUTRE_H

#include <QWidget>

class LibraryViewFeature {
    Q_OBJECT

  public:

    LibraryViewFeature();

    virtual QVariant getTitle() = 0;
    virtual QIcon getIcon() = 0;

    // A LibraryViewFeature always has a right and a left pane
    virtual QWidget* getLeftPane() = 0;

    // Since the right pane can be shown in two panes, this must return a new
    // widget every time and handle the proper connections with the left pane
    // Qt does not allow for a widget to have multiple parents
    virtual QWidget* getRightPane() = 0;

    // If it has not the search option, the search bar must be hidden
    virtual bool hasSearch() = 0;

  public slots:

    // Receives the text to search text and the widget that will be affected by 
    // the onSearch() event (because every feature will have multiple widgets
    // showing
    virtual void onSearch(QString& text, QWidget* content) = 0;

  signals:

    void focused();
};

#endif // LIBRARYVIEWFEAUTRE_H
