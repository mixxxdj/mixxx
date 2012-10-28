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

The Library displays a sortable list of all the tracks in your music
library. Mixxx imports your music library automatically when it is run for the
first time, and automatically detects newly added tracks on each subsequent
run. If you want to manually refresh your library without exiting (for example
because you added or moved files) you can do this with Library→Rescan Library in
the menu.

To load a track into a deck, you can either simply drag it to the waveform
display or use the context menu (right-click on a track). The right-click
context menu also allows you to add a track to the Auto DJ queue, playlists, or
crates. Also see the chapter :ref:`djing-loading-tracks`.

Search
======

The Search box in the top-left filters the current library view for tracks that match
your search query. The library search include some nice search features, see 
the chapter :ref:`djing-finding-tracks` for details.

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
enabling automatic mixing. This is useful for taking a break from live mixing 
or for using Mixxx as media player. Also see the chapter :ref:`djing-auto-dj`.

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
playlist they cannot contain duplicate entries and do not support drag-and-drop
within them. Crates can be created by right-clicking on “Crates” in the sidebar
and selecting “New Crate”.

Browse
======

Browse mode works like a file-manager and allows you to load tracks that are not
necessarily already in your Mixxx library.

Recordings
==========

In this section of the library you can start and stop recordings well as view
previous recordings and the dates they were made. Also see the chapter 
:ref:`djing-recording-your-mix`.

History
=======

New in Mixx 1.11: The history section automatically keeps a list of tracks you 
play in your DJ sets.This is handy for remembering what worked in your DJ sets, 
posting set-lists, or reporting your plays to licensing organizations. Every time 
you start Mixxx, a new history section is created. You can export it as a playlist 
in various formats or play it again with Auto DJ. You can join the current history 
session with a previous one by right-clicking and selecting "Join with previous".

Analyze
=======

The Analyze view allows you to see a list of either all tracks in the library or 
tracks added to the library within the last 7 days. You can select certain tracks 
and run BPM and beatgrid detection on them in advance. Waveforms will be generated 
as part of a analysis as well. Also see the chapter :ref:`djing-bpm-detection`.

iTunes, Traktor, and Rhythmbox Libraries
========================================

The iTunes (Windows and Mac OS X only), Traktor (Windows and Mac OS X only), and
Rhythmbox (Linux only) views allow you to view the music libraries you have
created in 3rd party applications.
