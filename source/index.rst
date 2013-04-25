.. Mixxx documentation master file, created by
   sphinx-quickstart on Mon Mar 14 02:25:59 2011.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Mixxx |version| User Manual
***************************

.. todo:: Release Checklist:

          * Disable the "For documentation writers" toctree from TOC in
            /index.rst
          * Temp delete this todo in /index.rst
          * Update the release and  version tags in /conf.py
          * Run "make html" to produce html output for http://mixxx.org/manual/
          * Run "make latexpdf" to produce pdf output for distribution
          * Run "make latexpdf" twice, or the TOC is missing from the resulting
            pdf

Mixxx is Free [#f1]_ DJ software for Windows, Mac OS X, and Linux.
**Mixxx gives you everything you need to become a superstar DJ** (except talent
and hard work). Using Mixxx, you can rock the party with your vinyl turntables,
your :term:`MIDI`/:term:`HID` controllers, or even just your keyboard.

That's right, this is you after learning how to DJ with Mixxx:

.. figure:: _static/armin-jesus.jpg
   :align: center
   :width: 60%
   :figwidth: 100%
   :alt: Armin van Buuren doing the Jesus pose.
   :figclass: pretty-figures

Table of Contents
-----------------

.. Temporarily disable quickstart since it won't be ready for 1.11.0
.. chapters/quickstart

.. toctree::
   :maxdepth: 2
   :glob:
   :numbered:

   chapters/introduction
   chapters/installation
   chapters/setup
   chapters/configuration
   chapters/user_interface
   chapters/library
   chapters/controlling_mixxx
   chapters/vinyl_control
   chapters/livebroadcasting
   chapters/djing_with_mixxx
   chapters/advanced_topics
   chapters/getting_involved
   chapters/appendix
   glossary

**For documentation writers**

.. toctree::
   :numbered:

   /todolist
   /manual_guidelines

.. [#f1] Yes, free with a capital F free. Free as in beer and free as in speech.
         For more information about what this means, see
         `Wikipedia <http://en.wikipedia.org/wiki/Open-source_software>`_.
