The Library
***********

.. figure:: ../_static/Mixxx-1.10-Library.png
   :align: center
   :width: 100%
   :figwidth: 100%
   :alt: The Mixxx Library
   :figclass: pretty-figures

   The Mixxx Library

The library manages all your music files. This is where you can find the tracks
you want to play and load them into a channel. Alternatively, you can also use
your external filemanager and drop files onto the waveform display. The sidebar
on the left contains different collections of music. The view on the right
displays the tracks in those collections.

The Search box in the top-left the current view for tracks that match your query.

The Library displays a sortable list of all the tracks in your music
library. Mixxx imports your music library automatically when it is run for the
first time, and automatically detects newly added tracks on each subsequent
run. If you want to manually refresh your library without exiting (for example
because you added or moved files) you can do this with Library→Rescan Library in
the menu.

To load a track into a deck, you can either simply drag it to the waveform
display or use the context menu (right-click on a track). The right-click
context menu also allows you to add a track to the Auto DJ queue, playlists, or
crates.

Missing Tracks
==============

The Missing Tracks view is accessible by expanding Library tree item in the
sidebar. Any tracks that were previously loaded into your library, but were later
detected to be missing from your hard disk by Mixxx will appear here. Mixxx does
not automatically delete records of missing tracks so that extra metadata Mixxx
might have (such as hot cues and the BPM) will not be lost if the file is
replaced.

Auto DJ
=======

The Auto DJ queue is a special playlist that contains extra controls for
enabling automatic mixing. Toggling the “Enable Auto DJ” button within this view
will tell Mixxx to automatically load the next track from this playlist when the
current track is nearly finished, and crossfade into it. Mixxx will continue to
automatically mix until the Auto DJ playlist is empty.

Playlists
=========

Playlists can be created by right-clicking on the “Playlists” sidebar item, and
selecting “New Playlist”. Tracks can be added to a playlist by finding them in
the Library, and drag-and-dropping them onto the name of a playlist in the
sidebar, or by selecting a track in the library and right-clicking on
it. Playlists are not directly loadable into Mixxx's decks as Mixxx is
primarily intended for live, attended performance use. However, you can add the
contents of a playlist to the Auto DJ queue, and use automatic mixing.

Crates
======

Crates are unordered collections of tracks, and are similar to playlists. Unlike
playlist they cannot contain duplicate entries and do not support drack-and-drop
within them. Crates can be created by right-clicking on “Crates” in the sidebar
and selecting “New Crate”.

Browse
======

Browse mode works like a file-manager and allows you to load tracks that are not
necessarily already in your Mixxx library.

Recordings
==========

In this section of the library you can start and stop recordings well as view
previous recordings and the dates they were made.

Analyze
=======

The Analyze view allows you to see a list of recently added tracks, and to run
BPM detection on them in advance.

iTunes, Traktor, and Rhythmbox Libraries
========================================

The iTunes (Windows and Mac OS X only), Traktor (Windows and Mac OS X only), and
Rhythmbox (Linux only) views allow you to view the music libraries you have
created in 3rd party applications.
