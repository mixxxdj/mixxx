/// libraryview.h
/// Created 8/28/2009 by RJ Ryan (rryan@mit.edu)
///
/// LibraryView is an abstract interface that all views to be used with the
/// Library widget should support.

#pragma once

#include <QString>

#include "library/trackmodel.h"

class LibraryView {
  public:
    virtual ~LibraryView() {
    }

    virtual void onShow() = 0;
    virtual bool hasFocus() const = 0;
    virtual void setFocus() {
    }
    /// Reimplement if LibraryView should be able to search
    virtual void onSearch(const QString& text) {Q_UNUSED(text);}

    virtual void pasteFromSidebar() {
    }

    virtual void saveCurrentViewState() {
    }
    /// @brief restores current view state.
    /// @return true if restore succeeded
    virtual bool restoreCurrentViewState() {
        return false;
    };

    virtual TrackModel::SortColumnId getColumnIdFromCurrentIndex() {
        return TrackModel::SortColumnId::Invalid;
    }
};
