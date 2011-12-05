Advanced Topics
***************

Adding support for your MIDI Controller
=======================================

So your shiny :term:`MIDI controller` isn't supported in Mixxx?  If you know
some basic Javascript, then adding support for your controller is not hard at
all!

Mixxx has an advanced scripting system that allows you to add support for MIDI
controllers by writing some simple Javascript. For more information, see the
`MIDI Scripting <http://mixxx.org/wiki/doku.php/midi_scripting>`_ topic on the
Mixxx Wiki.

Making a Custom Keyboard Mapping
================================

Editing the keyboard shortcuts is very easy. For more information, see the
`Mixxx Wiki <http://mixxx.org/wiki/>`_.

Effects via JACK Rack
=====================

Mixxx does not have an effects engine yet (work on one is in progress). To hold
you over, on GNU/Linux you can use a tool called JACK Rack. External mixer mode
can be used with `Jack
<http://en.wikipedia.org/wiki/JACK_Audio_Connection_Kit>`_ to route each deck
directly through `JACK Rack <http://jack-rack.sourceforge.net/>`_ effect racks,
or for more control you can use Ardour (or other DAW) using sends for
effects. This gives Mixxx access to the extensive collection of LADSPA
plugins. For more informatoin, see the `Mixxx Wiki <http://mixxx.org/wiki>`_.

Deleting Your Library
=====================

.. warning:: Deleting your library will lose all of your metadata. This includes
             saved **hotcues**, **loops**, **comments**, **ratings**, and other
             library related metadata. Only delete your library if you are fine
             with losing these.

The library file is stored in the following places depending on your
:term:`operating system`.

Windows
-------

The Mixxx library is stored in the ``C:\Users\%USER%\Application Data\Mixxx\``
folder. To delete your library on Windows, delete the ``mixxxdb.sqlite`` file in
this folder.

Mac OS X
--------

The Mixxx library is stored in the ``Library/Application Support/Mixxx`` folder
in your home directory. To delete your library on Mac OS X type the following
command into a terminal: ::

     rm ~/Library/Application\ Support/Mixxx/mixxxdb.sqlite


GNU/Linux
---------

The Mixxx library is stored in the ``.mixxx`` folder in your home directory. To
delete your library on GNU/Linux type the following command into a terminal: ::

     rm ~/.mixxx/mixxxdb.sqlite
