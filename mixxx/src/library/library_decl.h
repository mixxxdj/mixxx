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
    ContextMenu, // See LibraryControl::getFocusedWidget() for detailed
    Dialog,      // descriptions
    Unknown,     // Unknown skin widget or unknown window
    Count        // Used for setting the number of PushButton states of
                 // m_pFocusedWidget in librarycontrol.cpp
};

struct LibraryScanResultSummary {
    QString durationString;
    bool autoscan;
    int numNewTracks;
    int numMovedTracks;
    int numMissingTracks;
    int numNewMissingTracks;
    int numRediscoveredTracks;
    int tracksTotal;
};
