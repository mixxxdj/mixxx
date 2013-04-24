.. _configuring-mixxx:

Configuring Mixxx
*****************

Audio Outputs
=============

Mixxx's mixing engine can be used two ways:

* Internal Mixer Mode: In this mode, Mixxx performs the mixing of the decks,
  microphone, and samplers in software and outputs them to a single output.

* External Mixer Mode: In this mode, Mixxx outputs the audio from each deck to a
  separate soundcard output. This allows you to route the deck outputs through a
  hardware mixer.

To enable external mixer mode, simply select a valid audio output for the
:guilabel:`Deck` outputs in :menuselection:`Preferences --> Sound Hardware -->
Output`. Similarly, to enable internal mixer mode assign a valid audio output to
the :guilabel:`Master` output.

Headphone Output
----------------

In both internal and external mixer mode, you can choose a headphone output for
:term:`pre-fader listening <PFL>` or :term:`headphone cueing <cueing>`. This
allows you to listen and synchronize the track you will play next in your
headphones before your audience hears the track.

.. _configuration-latency-samplerate-audioapi:

Latency, Sample Rate, and Audio API
===================================

*To achieve the best performance with Mixxx it is essential to configure your
latency, samplerate, and audio API*. These three factors largely determine
Mixxx's responsiveness and audio quality and the optimal settings will vary
based on your computer and hardware quality.

.. _configuration-latency:

Latency
-------

Latency is the lag time in milliseconds that it takes for Mixxx to process your
input (turning knobs, sliding the crossfader, etc.). For example, a latency of
36 ms indicates that it will take approximately 36 ms for Mixxx to stop the
audio after you toggle the play button. Additionally, latency determines how
quickly your :term:`Operating System` expects Mixxx to react. Lower latency
means Mixxx will be more responsive. On the other hand, setting your latency too
low may be too much for your computer to handle. In this situation, Mixxx
playback will be choppy and very clearly distorted as your computer will not be
able to keep up with how frequently Mixxx is processing audio.

A latency between 36-64 ms is acceptable if you are using Mixxx with a
keyboard/mouse or a MIDI controller. A latency below 10 ms is recommended when
vinyl control is used because Mixxx will feel unresponsive otherwise.

Keep in mind that *lower latencies require better soundcards and faster CPUs*
and that zero latency DJ software is a myth (although Mixxx is capable of
sub-1ms operation).

Sample Rate
-----------

.. versionchanged:: 1.11
   Mixxx automatically selects a default sample rate for your soundcard, most
   likely 44100 Hz

The sample rate setting in Mixxx determines how many samples per second are
produced by Mixxx. In general, a higher sample rate means that Mixxx produces
more audio data for your soundcard. This takes more CPU time, but in theory
produces higher audio quality. On high-wattage club sound systems, it may become
apparent if your audio sample rate is too low.

.. warning:: A sample rate of 96kHz takes Mixxx over twice as long to compute.
             Keep in mind that increasing the samplerate will increase CPU usage
             and likely raise the minimum latency you can achieve.

Audio API
---------

The Audio :term:`API` that Mixxx uses is the method by which Mixxx talks to your
:term:`Operating System` in order to deliver audio to your soundcard. Your
choice of Audio API can drastically affect Mixxx's performance on your
computer. *Therefore it is important to take care to choose the best Audio API
available to you.* Refer to the following table of Audio APIs to see what the
best choice is for your operating system.

+----------------------------------------+--------------+
| OS / Audio API                         | Quality      |
+========================================+==============+
| Windows / WMME                         | Poor         |
+----------------------------------------+--------------+
| Windows / DirectSound                  | Poor         |
+----------------------------------------+--------------+
| Windows / WASAPI                       | Good         |
+----------------------------------------+--------------+
| Windows / ASIO                         | Good         |
+----------------------------------------+--------------+
| Windows / WDDKMS                       | Good         |
+----------------------------------------+--------------+
| Mac OS X / CoreAudio                   | Good         |
+----------------------------------------+--------------+
| GNU Linux / OSS                        | OK           |
+----------------------------------------+--------------+
| GNU Linux / ALSA                       | Good         |
+----------------------------------------+--------------+
| GNU Linux / JACK (Advanced)            | Good         |
+----------------------------------------+--------------+

.. note:: On Windows, if an ASIO driver is not available for your operating
          system, you can try installing `ASIO4ALL <http://asio4all.com>`_,
          a low-latency audio driver for WDM audio devices.

.. note:: On GNU/Linux using JACK, make sure to start your JACK daemon *before*
          running Mixxx. Otherwise JACK will not appear as a Sound API in the
          preferences.

.. warning:: On GNU/Linux do *not* use the ``pulse`` device with the ALSA Audio
             API. This is an emulation layer for ALSA provided by PulseAudio and
             results in very poor performance. Make sure to run Mixxx using the
             ``pasuspender`` tool on GNU/Linux distributions that use
             PulseAudio.
