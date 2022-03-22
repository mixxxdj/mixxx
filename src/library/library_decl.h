#pragma once

class Library;

enum class LibraryRemovalType {
    KeepTracks,
    HideTracks,
    PurgeTracks
};

enum class FocusWidget {
    None,
    Searchbar,
    Sidebar,
    TracksTable, // or a feature root view (WLibraryTextBrowser)
    Count        // used for setting the number of PushButton states of
                 // m_pLibraryFocusedWidgetCO in librarycontrol.cpp
};
