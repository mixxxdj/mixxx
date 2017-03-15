libebur128
==========

libebur128 is a library that implements the EBU R 128 standard for loudness
normalisation.

All source code is licensed under the MIT license. See COPYING file for
details.

See also [loudness-scanner tool](https://github.com/jiixyj/loudness-scanner).

News
----

v1.1.0 released:

 * Add `ebur128_relative_threshold()`
 * Add channel definitions from ITU R-REC-BS 1770-4 to channel enum
 * Fix some minor build issues

v1.0.3 released:

 * Fix build with recent speexdsp
 * Correct license file name
 * CMake option to disable static library
 * minimal-example.c: do not hard code program name in usage

Features
--------

* Portable ANSI C code
* Implements M, S and I modes
* Implements loudness range measurement (EBU - TECH 3342)
* True peak scanning
* Supports all samplerates by recalculation of the filter coefficients

Requirements
------------

* [libspeexdsp](http://www.speex.org/) - Needed for `ebur128_true_peak`.

Installation
------------

In the root folder, type:

    mkdir build
    cd build
    cmake ..
    make

If you want the git version, run simply:

    git clone git://github.com/jiixyj/libebur128.git

Usage
-----

Library usage should be pretty straightforward. All exported symbols are
documented in the ebur128.h header file. For a usage example, see
minimal-example.c in the tests folder.

On some operating systems, static libraries should be compiled as position
independent code. You can enable that by turning on `WITH_STATIC_PIC`.
