/// libraryview.h
/// Created 8/28/2009 by RJ Ryan (rryan@mit.edu)
///
/// LibraryView is an abstract interface that all views to be used with the
/// Library widget should support.

#ifndef LIBRARYVIEW_H
#define LIBRARYVIEW_H

#include <QString>

#include "library/trackmodel.h"

class LibraryView {
  public:
    virtual ~LibraryView() {};

    virtual void onShow() = 0;
    virtual bool hasFocus() const = 0;
    /// Reimplement if LibraryView should be able to search
    virtual void onSearch(const QString& text) {Q_UNUSED(text);}

    /// If applicable, requests that the LibraryView load the selected
    /// track. Does nothing otherwise.
    virtual void loadSelectedTrack() {};

    virtual void slotAddToAutoDJBottom() {};
    virtual void slotAddToAutoDJTop() {};
    virtual void slotAddToAutoDJReplace() {};

    /// If applicable, requests that the LibraryView load the selected track to
    /// the specified group. Does nothing otherwise.
    virtual void loadSelectedTrackToGroup(QString group, bool play) {
        Q_UNUSED(group); Q_UNUSED(play);
    }

    /// If a selection is applicable for this view, request that the selection be
    /// increased or decreased by the provided delta. For example, for a value of
    /// 1, the view should move to the next selection in the list.
    virtual void moveSelection(int delta) {Q_UNUSED(delta);}

    virtual TrackModel::SortColumnId getColumnIdFromCurrentIndex() {
        return TrackModel::SortColumnId::Invalid;
    };
    /// If applicable, requests that the LibraryView changes the track color of
    /// the selected track. Does nothing otherwise.
    virtual void assignPreviousTrackColor(){};
    virtual void assignNextTrackColor(){};
};

#endif /* LIBRARYVIEW_H */
