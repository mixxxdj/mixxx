
.. include:: /shortcuts.rstext

Advanced Topics
***************

.. _advanced-controller:

Adding support for your MIDI Controller
=======================================

So your shiny :term:`MIDI controller` isn't supported in Mixxx?  If you know
some basic Javascript, then adding support for your controller is not hard at
all!

Mixxx has an advanced scripting system that allows you to add support for MIDI
controllers by writing some simple Javascript. For more information, see the
`MIDI Scripting <http://mixxx.org/wiki/doku.php/midi_scripting>`_ topic on the
Mixxx Wiki.

.. _advanced-keyboard:

Making a Custom Keyboard Mapping
================================

The :ref:`default keyboard mappings<appendix-keyboard>` are defined in a text
file which can be found at the following location:

* Linux: ``/usr/local/share/mixxx/keyboard/en_US.kbd.cfg``
* Mac OS X: ``/Applications/Mixxx.app/Contents/Resources/keyboard/en_US.kbd.cfg``
* Windows: ``<Mixxx installation directory>\keyboard\en_US.kbd.cfg``

Depending on your systems language settings, Mixxx might use a different
file as default, e.g. ``de_DE.kbd.cfg`` for German or ``es_ES.kbd.cfg`` for
Spanish.

There are two ways to customize the default Mixxx keyboard mapping:

1. Edit your systems default mapping file directly, e.g. ``en_US.kbd.cfg``.
2. Copy the default mapping file to the following location:

* Linux: ``~/.mixxx/Custom.kbd.cfg``
* Mac OS X: ``/Library/Application\ Support/Mixxx/Custom.kbd.cfg``
* Windows: ``%USERPROFILE%\Local Settings\Application Data\Mixxx\Custom.kbd.cfg``

Then edit this file and save the changes. On the next startup, Mixxx will check
if ``Custom.kbd.cfg`` is present and load that file instead of the default
mapping file. This has the advantage, that you can always revert back to the
default mapping, just by deleting ``Custom.kbd.cfg``.

For a list of controls that can be used in a keyboard mapping, see the
`MixxxControls <http://www.mixxx.org/wiki/doku.php/mixxxcontrols>`_ topic on the
Mixxx Wiki.

You can download and share custom keyboard mappings in the
`Mixxx User customizations forum`_.

.. _Mixxx User customizations forum: http://mixxx.org/forums/viewforum.php?f=6

.. _advanced-jack-rack:

Effects via JACK Rack
=====================

Mixxx does not have an effects engine yet (work on one is in progress). To hold
you over, on GNU/Linux you can use a tool called JACK Rack. External mixer mode
can be used with `Jack
<http://en.wikipedia.org/wiki/JACK_Audio_Connection_Kit>`_ to route each deck
directly through `JACK Rack <http://jack-rack.sourceforge.net/>`_ effect racks,
or for more control you can use Ardour (or other DAW) using sends for
effects. This gives Mixxx access to the extensive collection of LADSPA
plugins. For more information, see the `Mixxx Wiki <http://mixxx.org/wiki>`_.

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

The Mixxx library is stored in the ``C:\Users\%USER%\Local Settings\Application
Data\Mixxx\`` folder. To delete your library on Windows, delete the
``mixxxdb.sqlite`` file in this folder.

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

