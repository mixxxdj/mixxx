// libraryview.h
// Created 8/28/2009 by RJ Ryan (rryan@mit.edu)
//
// LibraryView is an abstract interface that all views to be used with the
// Library widget should support.

#ifndef LIBRARYVIEW_H
#define LIBRARYVIEW_H

#include <QString>

class LibraryView {
  public:
    virtual void onShow() = 0;
    // reimplement if LibraryView should be able to search
    virtual void onSearch(const QString& text) {Q_UNUSED(text);}

    // If applicable, requests that the LibraryView load the selected
    // track. Does nothing otherwise.
    virtual void loadSelectedTrack() {};

    // If applicable, requests that the LibraryView load the selected track to
    // the specified group. Does nothing otherwise.
    virtual void loadSelectedTrackToGroup(QString group, bool play) {
        Q_UNUSED(group); Q_UNUSED(play);
    }

    // If a selection is applicable for this view, request that the selection be
    // increased or decreased by the provided delta. For example, for a value of
    // 1, the view should move to the next selection in the list.
    virtual void moveSelection(int delta) {Q_UNUSED(delta);}
};

#endif /* LIBRARYVIEW_H */
