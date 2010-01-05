// libraryview.h
// Created 8/28/2009 by RJ Ryan (rryan@mit.edu)
//
// LibraryView is an abstract interface that all views to be used with the
// Library widget should support.

#ifndef LIBRARYVIEW_H
#define LIBRARYVIEW_H

#include <QDomNode>

class LibraryView {
public:
    virtual void setup(QDomNode node) = 0;
    virtual void onSearchStarting() = 0;
    virtual void onSearchCleared() = 0;
    virtual void onSearch(const QString& text) = 0;
    virtual void onShow() = 0;
    virtual QWidget* getWidgetForMIDIControl()  = 0;
};

#endif /* LIBRARYVIEW_H */
