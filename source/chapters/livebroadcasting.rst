Live Broadcasting
*****************

Starting with version 1.9.0, Mixxx directly supports live broadcasting. This
allows you to connect directly to Shoutcast and Icecast servers. Using the
preferences dialogue, you can simply supply Mixxx with all information needed to
establish a server connection. To enable live broadcasting you can either use
the options menu or the checkbox within the preference dialogue.

.. figure:: ../_static/Mixxx-1.10-Preferences-Livebroadcasting.png
   :align: center
   :width: 100%
   :figwidth: 90%
   :alt: Mixxx preferences - Setting up live broadcasting
   :figclass: pretty-figures

   Mixxx preferences - Setting up live broadcasting

By default, Mixxx broadcasts artist and title information to your listeners. You
can disable this behavior by selecting “enable custom metadata”.

.. note:: For technical reasons, broadcasting artist and title information is not supported for OGG streams.


Icecast
=======

For an Icecast server, you'll need to provide the mount point (of the form
”/mount”).  You can enter the host as either a host name or an IP address. In
the “login” field, the default is to enter “source” – without this, you will not
connect successfully to the server. The password will be provided by your
streaming server provider, unless you run your own radio server.

.. note:: Do not enter a URL as the host! "http://example.com:8000" does not
          work. Use "example.com" in the host field and "8000" in the port field
          instead.

An Icecast server can stream either MP3 or Ogg. However, although Ogg is more
efficient and effective - you get a better sound than mp3 at a lower data rate -
not all players can play Ogg streams, so as a result MP3 is probably a better
choice unless you know your listeners can hear an Ogg stream successfully. You
may need the LAME libraries to stream in MP3. See :ref:`MP3 Streaming` for more
details.

Shoutcast
=========

If you connect to an Shoutcast server the default login name is “admin”. It is
not necessary to specify a mount point. The password will be provided by your
streaming server provider.

.. _MP3 Streaming:

MP3 streaming
=============

MP3 streaming is not supported out of the box in Mixxx since we do not license
the MP3 encoding patents. In order to enable MP3 streaming you must install the
LAME MP3 encoding tool yourself. For information about this, see the `Mixxx Wiki
page <http://mixxx.org/wiki/doku.php/internet_broadcasting#mp3_streaming>`_ on
the topic.

Linux
-----

On Ubuntu and GNU/Linux-based operating systems MP3 streams can be activated by
installing the package libmp3lame.  Dependent on your Linux distribution the
package might be slightly named different such as lame. ::

     sudo apt-get install libmp3lame0

Windows
-------

To activate MP3 streaming on Windows, follow these steps:

     1. Download LAME 3.98.4 binaries from http://lame.bakerweb.biz/ . The ZIP file includes x86 and x64 DLLs
     #. Unpack the archive
     #. If you have the 32-bit version of Mixxx, copy libmp3lame.dll from the x86 folder to the location you have installed Mixxx.
     #. If you have the 64-bit version of Mixxx, copy libmp3lame.dll from the x64 folder to the location you have installed Mixxx.
     #. Rename the DLL to lame_enc.dll

Please note that Audacity and other web sites provide lame binaries too. **DO
NOT USE THESE VERSIONS**.  If you do, Mixxx will show an error when activating
live broadcasting.

Mac OS X
--------

To activate MP3 streaming on Mac OS X, follow these steps:

     1. Download `LAME 3.98.4 <http://mir.cr/IOTD7VBU>`_ Intel (Mac OS X 10.5+ 32-bit & 64-bit) or `LAME 3.98.4 <http://mir.cr/YIBEU5R4>`_ PowerPC (Mac OS X 10.5 32-bit)
     #. Unpack and install the archive.

Another easy way to achieve MP3 streaming is to use `MacPorts
<http://www.macports.org/>`_ which is a repository manager (like apt on
Debian/Ubuntu) for Open Source software. Having installed this piece of
software, installing MP3 support is rather simple. ::

     sudo port install lame
