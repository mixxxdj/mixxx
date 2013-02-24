.. _controlling mixxx:

Controlling Mixxx
*****************

Mixxx can be controlled with a keyboard, a mouse, MIDI/HID controllers, 
time-code records/CDs, or a combination of these. The choice usually depends on 
your budget or style of DJing.


.. _control-keyboard:

Using a Keyboard
================

.. figure:: ../_static/Mixxx-111-Keyboard-Mapping.png
   :align: center
   :width: 100%
   :figwidth: 100%
   :alt: Keyboard shortcuts
   :figclass: pretty-figures

   Mixxx Keyboard shortcuts (for en-us keyboard layout) 

Controlling Mixxx with a keyboard is handy. Unlike mouse control, the keyboard 
allows you to manage things simultaneously. For example, you can start playing 
a track on deck 1 whilst stopping deck 2 at the same time.

The default mapping for English keyboards is depicted in the figure above. It's
divided into a left-hand side for deck 1 and right-hand side for deck 2. Please
note that you can also access the functions through Mixxx's interface.

.. note:: For some user groups, i.e. those using midi controllers or vinyl 
          control it might be useful to enable/disable the keyboard mapping at 
          runtime. You can do so by clicking 
          ``Options`` -> ``Enable keyboard shortcuts`` in the menu. 

.. hint::  If you hover with the mouse over a control (e.g the crossfader) in 
           the Mixxx user interface the tooltip lists the keyboard shortcuts of 
           the control. 

.. seealso:: For a list of default shortcuts, go to: :ref:`appendix-keyboard`

Customizing the keyboard mapping
--------------------------------

Mixxx allows you to customize the keyboard control. For more informations, go to: 

* :ref:`advanced-keyboard`

.. _control-midi:

Using a MIDI Controller
=======================

:term:`MIDI controllers <MIDI Controller>` are external hardware devices used
that can be used to control audio applications.  Many DJs prefer the hands-on
feel of a MIDI controller with Mixxx because it can feel similar to using a real
mixer and turntables.

Here are the steps for using one:

#. Connect your controller(s) to your computer
#. Open Preferences and click *MIDI Controllers*
#. Select your controller on the left and the right pane will change
#. Click *Enable* and choose the appropriate mapping from the *presets* combobox
#. Click OK and Mixxx can now be controlled by your controller(s).

Supported controllers
---------------------

Mixxx can use any :term:`MIDI`/:term:`HID` controller that is recognized by your 
:term:`OS <operating system>` (some may require drivers), as long as there is a 
MIDI/HID mapping file to tell Mixxx how to understand it. Mixxx comes bundled 
with a number of mappings for various devices. There are two levels of 
controller mappings:

* **Mixxx Certified Mappings** - These mappings are verified by the Mixxx 
  Development Team.
* **Community Supported Mappings** - These mappings are provided and have been 
  verified as working by the Mixxx community, but the Mixxx Team is unable to 
  verify their quality because we don't have the devices ourselves. They might 
  have bugs or rough edges.

If you run into issues with any of these mappings, please file a bug on our 
`bug tracker`_ or tell us about it on our mailing list, forums, or :term:`IRC` 
channel. Device support varies for each supported :term:`OS <operating system>`, 
so make sure to consult the documentation of the device.

.. seealso:: Before purchasing a controller to use with Mixxx, consult our 
             `Hardware Compatibility wiki page`_. It contains the most 
             up-to-date information about which controllers work with Mixxx and 
             the details of each.

.. _Hardware Compatibility wiki page: http://www.mixxx.org/wiki/doku.php/hardware_compatibility
.. _Bug tracker: http://bugs.launchpad.net/mixxx
.. _Controller presets forum: http://mixxx.org/forums/viewforum.php?f=7

.. _control-timecode:

Using Timecode Vinyl Records and CDs
====================================

:term:`Vinyl control` allows a user to manipulate the playback of a track in
Mixxx using a turntable or DJ CD player as an interface.  In effect, it
simulates the sound and feel of having your music collection on vinyl.

How does it work?
-----------------

Vinyl control uses special timecode records which are placed on real
turntables. The audio output of the turntables is plugged into a computer on
which Mixxx is running. When a record is played on one of the attached
turntables, Mixxx reads the timecode from the record and uses the information to
manipulate whatever track is loaded.

What do I need to use it?
-------------------------

It is possible to use Mixxx's vinyl control with several hardware setups, but
the basic ones are:

**Setup 1: Vinyl DJ** Two timecode vinyls, two turntables with phono
pre-amplifiers (or line-out), and two sound inputs.  You can try skipping the
phono pre-amps if you use the software pre-amp in Mixxx on the Vinyl Control
preferences pane.  *This may not work for everyone - line-level signals are
preferred and recommended.*

**Setup 2: CDJ** Two timecode CDs, two CD decks, and two sound inputs.

For the sound inputs, you have two options: You can either use a proper DJ sound card that has multiple
stereo line inputs on it, or can use two sound cards (each with a single stereo line in).
A single multi-channel sound card is recommended.

.. note:: For best scratch performance with vinyl control, we recommend using a
          system capable of :term:`latencies <latency>` of 10ms. With higher
          latencies the scratch sound will start to become distorted.

For timecode records or CDs, you can use any of the records supported by Mixxx:

Timecode Support
----------------

+----------------------------------------+---------------------+
| Type                                   | Responsiveness      |
+========================================+=====================+
| Serato CV02 Vinyl                      | Very high           |
+----------------------------------------+---------------------+
| Serato CV02 CD                         | Very high           |
+----------------------------------------+---------------------+
| Traktor Scratch Vinyl MK1              | Very high           |
+----------------------------------------+---------------------+
| FinalScratch (Standard)                | Not supported       |
+----------------------------------------+---------------------+
| FinalScratch (Scratch)                 | Not supported       |
+----------------------------------------+---------------------+
| MixVibes DVS CD                        | Not supported       |
+----------------------------------------+---------------------+
| MixVibes DVS Vinyl                     | Not supported       |
+----------------------------------------+---------------------+

At the present time, Serato records are recommended if you are looking to buy vinyl. If you want
to use CDs, you can download a free copy from `Serato`_.

.. _Serato: http://serato.com/downloads/scratchlive-controlcd/

