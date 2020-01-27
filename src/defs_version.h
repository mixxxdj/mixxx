/***************************************************************************
                          defs_version.h  -  description
                          --------------
    copyright            : (C) 2010 by Sean Pappalardo
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// Doing this in its own file avoids needlessly rebuilding everything when just
// the version number changes
//
// Some notes on versioning:
// - For Windows, src/SConscript loads this and produces src/mixxx.rc.include
//   which is included by src/mixxx.rc when rc.exe compiles it.
//
// - Windows versions in mixxx.rc need to match major.minor.patch
//   https://msdn.microsoft.com/en-us/library/aa370859(v=vs.85).aspx
//   The SConscript reads VERSION and strips off ~ and - parts when it writes
//   mixxx.rc.include.
//
//   If you change the VERSION format we use, make sure mixxx.rc is valid!
//
// - Debian versions are compared (i.e. version A comes before version B) in a
//   very specific order. We need to be careful to prevent situations where our
//   betas or release candidates are considered "newer" than the release itself.
//   https://www.debian.org/doc/debian-policy/ch-controlfields.html#s-f-Version
//
// Here's how we should format our versions:
//
//  ===================================
//  |  MAJOR.MINOR.PATCH~(betaX|rcX)  |
//  ===================================
//
// The build server will append an Ubuntu version to this package name such as
// "-0ubuntu1~trusty1". So the final Debian package version will be something
// like:
//
// MAJOR.MINOR.PATCH~beta1-0ubuntu1~precise1
//
// You can use dpkg --compare-versions to test that this works as desired:
//
// $ dpkg --compare-versions 2.0.0~rc1-0ubuntu1 lt 2.0.0-0ubuntu1 && echo true
//
#ifndef MIXXX_VERSION
    // In that case, this value is used

    // READ ABOVE CAREFULLY BEFORE CHANGING!
    // USE 3 DIGITS VERSION ONLY !
    // CHANGE IN ACTUAL VERSION FILE
    #include "_version.h"
#endif
