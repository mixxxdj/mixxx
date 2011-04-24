Configuring your Setup
**********************

This chapter describes some of the most common hardware setups that you
will choose from when preparing your DJ setup with Mixxx. We provide a couple of
options including time-coded records, MIDI control and keyboard.

Audio Output with Mixxx
=======================

Headphone cueing, or just cueing, is listening to the next song you would like to mix 
in in your headphones. The audience will not hear what you are cueing in your 
headphones. Being able to cue is a crucial aspect to DJing.

In order to cue with your computer, you will need **at least 2 separate audio
outputs**. Traditionally, a headphone jack on most laptops **is not a second
audio output**. Rather, plugging headphones into the headphone jack on most
laptops allows you to listen to the main output of your laptop in your
headphones. **Having a headphone jack alone will not allow you to cue.**

Controlling Mixxx
=================

Mixxx can be controlled with keyboard, mouse, MIDI controller, time-coded records
or a combination of these. The choice usually depends on your budget or area of 
application.  


Using your keyboard
-------------------

If you cannot afford a MIDI controller, controlling Mixxx through a keyboard
is a handy way. Unlike mouse control, the keyboard allows you to manage things 
simultaneously. For example, you can start playing a track on deck 1 whilst 
stopping deck 2 at the same time.

The default mapping for English keyboards is depicted in the figure below. The figure is divided 
into a left-hand side for deck 1 and right-hand side for deck 2. Please note that you can also 
access the functions through Mixxx's interface.


.. image:: ../_static/keyboard_mapping.png
   :width: 100%
   :alt: Keyboard shortcuts
   :align: center
   
+----------------------------------------+---------------------+---------------------+--------------+
| Function                               | Deck 1              | Deck 2              | Master       |
+========================================+=====================+=====================+==============+
| Crossfade Left                         |                     |                     |     G        |
+----------------------------------------+---------------------+---------------------+--------------+
| Crossfade Right                        |                     |                     |     H        |
+----------------------------------------+---------------------+---------------------+--------------+
| Playback                               | D                   | L                   |              |
+----------------------------------------+---------------------+---------------------+--------------+
| Set Cuepoint                           | Shift + D           | Shift + L           |              |
+----------------------------------------+---------------------+---------------------+--------------+
| Cue                                    | F                   | ;                   |              |
+----------------------------------------+---------------------+---------------------+--------------+
| Cue Go-to-and-stop                     | Shift + F           | Shift + ;           |              |
+----------------------------------------+---------------------+---------------------+--------------+
| Seek Backwards                         | A                   | J                   |              |
+----------------------------------------+---------------------+---------------------+--------------+
| Seek Forwards                          | S                   | K                   |              |
+----------------------------------------+---------------------+---------------------+--------------+
| Bass Kill                              | B                   | N                   |              |
+----------------------------------------+---------------------+---------------------+--------------+
| 4 beat loop                            | Q                   | U                   |              |
+----------------------------------------+---------------------+---------------------+--------------+
| double beat loop size                  | W                   | I                   |              |
+----------------------------------------+---------------------+---------------------+--------------+
| halve beat loop size                   | E                   | O                   |              |
+----------------------------------------+---------------------+---------------------+--------------+
| set loop point                         | 2                   | 7                   |              |
+----------------------------------------+---------------------+---------------------+--------------+
| set loop out point (activates loop)    | 3                   | 8                   |              |
+----------------------------------------+---------------------+---------------------+--------------+
| Exit loop                              | 4                   | 9                   |              |
+----------------------------------------+---------------------+---------------------+--------------+
| Activate/Disable Effects Unit          | 5                   | 0                   |              |
+----------------------------------------+---------------------+---------------------+--------------+
| Headphone Cue (P                       | T                   | Y                   |              |
+----------------------------------------+---------------------+---------------------+--------------+
| Tempo Adjust Up                        | F2                  | F6                  |              |
+----------------------------------------+---------------------+---------------------+--------------+
| Tempo Adjust Dow                       | F1                  | F5                  |              |
+----------------------------------------+---------------------+---------------------+--------------+
| Small Temporary Tempo Adjust Down      | Shift + F3          | Shift + F7          |              |
+----------------------------------------+---------------------+---------------------+--------------+
| Small Temporary Tempo Adjust Up        | Shift + F4          | Shift + F8          |              |
+----------------------------------------+---------------------+---------------------+--------------+
| Tempo (BPM) Sync                       | 1                   | 0                   |              |
+----------------------------------------+---------------------+---------------------+--------------+
| BPM Tap Tempo Adjust                   | Shift + 1           | Shift + 0           |              |
+----------------------------------------+---------------------+---------------------+--------------+
| Hotcue 1                               | Z                   | M                   |              |
+----------------------------------------+---------------------+---------------------+--------------+
| Hotcue 2                               | X                   | ,                   |              |
+----------------------------------------+---------------------+---------------------+--------------+
| Hotcue 3                               | C                   | .                   |              |
+----------------------------------------+---------------------+---------------------+--------------+
| Hotcue 4                               | V                   | /                   |              |
+----------------------------------------+---------------------+---------------------+--------------+
| Clear Hotcue 1                         | Shift + Z           | Shift + M           |              |
+----------------------------------------+---------------------+---------------------+--------------+
| Clear Hotcue 2                         | Shift + X           | Shift + <           |              |
+----------------------------------------+---------------------+---------------------+--------------+
| Clear Hotcue 3                         | Shift + C           | Shift + >           |              |
+----------------------------------------+---------------------+---------------------+--------------+
| Clear Hotcue 4                         | Shift + V           | Shift + ?           |              |
+----------------------------------------+---------------------+---------------------+--------------+
| Load selected track                    | left arrow          | right arrow         |              |
+----------------------------------------+---------------------+---------------------+--------------+
| Unload selected track                  | Shift + left arrow  | Shift + right arrow |              |
+----------------------------------------+---------------------+---------------------+--------------+

**Customizing** 

You can also customize the keyboard mapping. The shortcuts are defined in a text file, and can be changed 
by the user -
          
* Linux: /usr/share/mixxx/keyboard/Standard.kbd.cfg 
* MacOS X: <Mixxx bundle>/keyboard/Standard.kbd.cfg 
* Windows: <Mixxx dir>\keyboard\Standard.kbd.cfg  

Using a MIDI Controller
-----------------------

MIDI controllers are external hardware devices used that can be used to control audio applications, like Mixxx.
Many DJs prefer the “hands-on” feel of a MIDI controller with Mixxx because it can feel similar to using a real
mixer and turntables.

Mixxx can work with any MIDI controller that has drivers for your OS, as long as there is a MIDI mapping file
to tell Mixxx how to understand it. Mixxx comes bundled with a number of MIDI mapping presets for the devices 
listed below.

Often times these mappings are contributed by users, but the Mixxx team has no way of judging the quality of 
the mappings because we do not own the devices ourselves. There are two different levels of support for devices 
in Mixxx: Mixxx Certified Support and Community Support. Mixxx Certified mappings are verified by the Mixxx 
Team. Community Supported mappings are mappings provided by the Mixxx Community, but the Mixxx team is unable 
the verify their quality.

**Mixxx Certified Mappings**

* Hercules DJ Console RMX
* Hercules DJ Console MK2
* Hercules DJ Control MP3 e2
* Stanton SCS.3d
* Stanton SCS.3m
* Stanton SCS.1m
* M-Audio X-Session Pro
* DJ TechTools MIDIFighter
* Reloop Digitial Jockey 2 (Interface/Controller)

**Community Supported Mappings**

These mappings have been verified as working by the Mixxx community. However, they might have bugs or rough 
edges. If you run into issues with these mappings, please file a bug on our bug tracker or tell us about it on 
our mailing list, forums, or IRC channel.

* Hercules DJ Console MP3
* Hercules DJ Control Steel
* Hercules DJ Console Mac Edition
* Mixman DM2
* Tascam US-428
* M-Audio Xponent
* Evolution X-Session
* Ecler NUO4
* FaderFox DJ2
* Vestax VCI-100
* Numark Total Control
* Behringer BCD3000
* Akai MPD24

Before purchasing a controller to use with Mixxx, please consult our `Hardware Compatibility wiki page`_. It 
contains the most up-to-date documentation on which controllers work with Mixxx and what the caveats of each 
device are. Device support varies for each supported OS, so please make sure to consult the documentation.

.. _Hardware Compatibility wiki page: http://www.mixxx.org/wiki/doku.php/hardware_compatibility

Using Vinyl Timecode Records
----------------------------

Vinyl control allows a user to manipulate the playback of a song in Mixxx using a turntable as an interface. In 
effect, it simulates the sound and feel of having your MP3 collection on vinyl. 

**How does it work?**

Vinyl control uses special timecoded records which are placed on real turntables. The audio output of the 
turntables is plugged into a computer, on which Mixxx is running. When a record is played on one of the attached 
turntables, Mixxx decodes the timecode off the record, and uses information from that to manipulate whatever 
song is loaded. 

**What do I need to use it**?

It is possible to use Mixxx's vinyl control with several hardware setups, but the basic ones are:

**Setup 1: Vinyl DJ** Two timecoded records, two turntables with phono preamps (or line-out), and two sound 
inputs. You might skip the phono amplifiers if you use the snazzy software preamp in Mixxx. *This 
may not work for everyone - line-level signals are preferred and recommended.*

**Setup 2: CDJ** Two timecoded CDs, two CD decks, and two sound inputs.

For the sound inputs, you have two options: You can either use a propper DJ soundcard that has multiple 
stereo line inputs on it, or can use two soundcards (each with a single stereo line in). 

.. note:: For best scratch performance with vinyl control, your system must be able to handle setting the 
          latency to 10ms or less otherwise the scratch sound will start to become distorted as latencies (and 
          lag time) increase.

For timecoded records or CDs, you can use any of the records supported by Mixxx:

**Timecode Support**

+----------------------------------------+---------------------+
| Vinyl                                  | Responsivenes       |
+========================================+=====================+
| Serato CV02                            | Very high           |
+----------------------------------------+---------------------+
| Serato CD                              | Very high           |                   
+----------------------------------------+---------------------+
| Traktor Scratch                        | Very high           | 
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
to use CDs, you can download a free copy from `Rane`_. 

.. _Rane: http://serato.com/downloads/scratchlive-controlcd/  

Common Configurations
=====================

Laptop Only
-----------

Laptop and a USB Soundcard
--------------------------

Laptop, MIDI Controller, and USB Soundcard
------------------------------------------

Laptop, Two Turntables, and a Hardware Mixer
--------------------------------------------



