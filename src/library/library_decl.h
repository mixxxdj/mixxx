#pragma once

class Library;

enum class LibraryRemovalType {
    KeepTracks,
    HideTracks,
    PurgeTracks
};

enum class FocusWidget {
    None,
    Searchbar,   // Try not to change the position/order of the library widgets,
    Sidebar,     //
    TracksTable, // tracks table or a feature root view (WLibraryTextBrowser)
    ContextMenu, // See LibraryControl::getFocusedWidget() for detailed descriptions
    Dialog,
    SearchRelatedMenu,
    Unknown, // Unknown skin widget or unknown window
    Count    // Used for setting the number of PushButton states of
             // m_pFocusedWidget in librarycontrol.cpp
};
