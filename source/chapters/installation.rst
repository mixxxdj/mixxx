Installing Mixxx
****************

First, you need to download Mixxx. If you haven't done this already, head on
over to our `downloads page <http://mixxx.org/download.php>`_ and grab a copy
for your :term:`OS <operating system>`.

Windows
=======

Windows users can install Mixxx by double-clicking on the Mixxx installer
executable. Follow the step-by-step instructions in the installer. Mixxx is
supported on Windows XP, Vista, 7 & 8 with native 32 and 64-bit versions.

.. note:: If you aren't sure about 32-bit versus 64-bit, pick the 32-bit version.

Mac OS X
========

Mac OS X users on Snow Leopard or greater can install Mixxx from the `Mac App
Store <http://itunes.apple.com/us/app/mixxx/id413756578?mt=12>`_.
Alternatively, users can install Mixxx by downloading and
double-clicking the Mixxx DMG archive, and then dragging-and-dropping the Mixxx
bundle into their Applications folder. Mixxx requires an Intel Mac running Mac
OS X 10.5 or newer.

.. note:: The Mac App Store version of Mixxx does not have :term:`vinyl control`
          support enabled due to licensing restrictions.

GNU/Linux
=========

Official packages of Mixxx are only offered for Ubuntu Linux. However,
Mixxx can build on almost any Linux distribution.


Installation on Ubuntu
----------------------

Installing Mixxx on Ubuntu Linux is easy because we provide a handy Personal
Package Archive (ppa). Open a terminal and type the following commands::

    sudo apt-add-repository ppa:mixxx
    sudo apt-get update
    sudo apt-get install libportaudio2 mixxx

Installation on Other Distributions
-----------------------------------

Your distribution may maintain a non-official build that you may use.
Alternatively, you can build Mixxx from source. This should not be a terrible
process, and it's certainly easiest on GNU/Linux. For more information, see the
`Compiling on Linux<http://mixxx.org/wiki/doku.php/compiling_on_linux>`_ section
of the `Mixxx Wiki<http://mixxx.org/wiki>`_.

Building Mixxx from Source
==========================

If your operating system isn't on this list, then it's likely you're going to
have to build Mixxx from the source code. For more information, you should
probably head over to the `Mixxx Wiki <http://mixxx.org/wiki>`_.
