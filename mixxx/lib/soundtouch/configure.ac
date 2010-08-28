dnl SoundTouch configure.ac, by David W. Durham
dnl
dnl $Id: configure.ac 38 2008-12-25 17:00:23Z oparviai $
dnl
dnl This file is part of SoundTouch, an audio processing library for pitch/time adjustments
dnl 
dnl SoundTouch is free software; you can redistribute it and/or modify it under the
dnl terms of the GNU General Public License as published by the Free Software
dnl Foundation; either version 2 of the License, or (at your option) any later
dnl version.
dnl 
dnl SoundTouch is distributed in the hope that it will be useful, but WITHOUT ANY
dnl WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
dnl FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
dnl details.
dnl 
dnl You should have received a copy of the GNU General Public License along with
dnl this program; if not, write to the Free Software Foundation, Inc., 59 Temple
dnl Place - Suite 330, Boston, MA  02111-1307, USA
# Process this file with autoconf to produce a configure script.

AC_INIT(SoundTouch, 1.4.0, [http://www.surina.net/soundtouch])
AC_CONFIG_AUX_DIR(config)
AM_CONFIG_HEADER([include/soundtouch_config.h])
AM_INIT_AUTOMAKE
AC_DISABLE_SHARED	dnl This makes libtool only build static libs 
#AC_GNU_SOURCE 		dnl enable posix extensions in glibc

AC_LANG(C++)





dnl ############################################################################
dnl # Checks for programs                                                      #
dnl ############################################################################
AC_PROG_CXX
#AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXXCPP
AC_PROG_INSTALL
#AC_PROG_LN_S
AC_PROG_MAKE_SET

AM_PROG_LIBTOOL dnl turn on using libtool




dnl ############################################################################
dnl # Checks for header files                                                  #
dnl ############################################################################
AC_HEADER_STDC
#AC_HEADER_SYS_WAIT
#	add any others you want to check for here
#AC_CHECK_HEADERS([fcntl.h limits.h stddef.h stdlib.h string.h sys/ioctl.h sys/vfs.h unistd.h])






dnl ############################################################################
dnl # Checks for typedefs, structures, and compiler characteristics            $
dnl ############################################################################
AC_C_CONST
AC_C_INLINE
#AC_TYPE_OFF_T
#AC_TYPE_SIZE_T


AC_ARG_ENABLE(integer-samples,
              [AC_HELP_STRING([--enable-integer-samples],
                              [use integer samples instead of floats
[default=yes]])],,
              [enable_integer_samples=no])

      
if test "x$enable_integer_samples" = "xyes"; then
        echo "****** Integer sample type enabled ******"
        AC_DEFINE(INTEGER_SAMPLES,1,[Use Integer as Sample type])
else
        echo "****** Float sample type enabled ******"
        AC_DEFINE(FLOAT_SAMPLES,1,[Use Float as Sample type])
fi



dnl ############################################################################
dnl # Checks for library functions/classes                                     #
dnl ############################################################################
AC_FUNC_MALLOC
AC_TYPE_SIGNAL

dnl make -lm get added to the LIBS
AC_CHECK_LIB(m, sqrt,,AC_MSG_ERROR([compatible libc math library not found])) 

dnl add whatever functions you might want to check for here
#AC_CHECK_FUNCS([floor ftruncate memmove memset mkdir modf pow realpath sqrt strchr strdup strerror strrchr strstr strtol])







dnl ############################################################################
dnl # Internationaliation and Localiation                                    #
dnl ############################################################################
#AM_GNU_GETTEXT_VERSION([0.11.5])
#AM_GNU_GETTEXT([external])





dnl ############################################################################
dnl # Final                                                                    #
dnl ############################################################################

AC_CONFIG_FILES([
	Makefile
	source/Makefile
		source/SoundTouch/Makefile
		source/SoundStretch/Makefile
	include/Makefile
])

AC_OUTPUT(
	soundtouch-1.4.pc
)

dnl use 'echo' to put stuff here if you want a message to the builder
