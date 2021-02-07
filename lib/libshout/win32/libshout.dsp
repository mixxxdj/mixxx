# Microsoft Developer Studio Project File - Name="libshout" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libshout - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libshout.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libshout.mak" CFG="libshout - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libshout - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libshout - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libshout - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\src" /I "..\src/httpp" /I "..\src/thread" /I "..\src/log" /I "..\src/avl" /I "..\src/net" /I "..\src/timings" /I "../" /I "../../pthreads" /I "../../oggvorbis-win32sdk-1.0.1/include" /I "../include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "_WIN32" /D VERSION=\"2.4.1\" /D LIBSHOUT_MAJOR=2 /D LIBSHOUT_MINOR=0 /D LIBSHOUT_MICRO=0 /D "HAVE_WINSOCK2_H" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libshout - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\src" /I "..\src/httpp" /I "..\src/thread" /I "..\src/log" /I "..\src/avl" /I "..\src/net" /I "..\src/timings" /I "../" /I "../../pthreads" /I "../../oggvorbis-win32sdk-1.0.1/include" /I "../include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "_WIN32" /D VERSION=\"2.0.0\" /D LIBSHOUT_MAJOR=2 /D LIBSHOUT_MINOR=0 /D LIBSHOUT_MICRO=0 /D "HAVE_WINSOCK2_H" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "libshout - Win32 Release"
# Name "libshout - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\avl\avl.c
# End Source File
# Begin Source File

SOURCE=..\src\avl\avl.h
# End Source File
# Begin Source File

SOURCE=..\src\httpp\httpp.c
# End Source File
# Begin Source File

SOURCE=..\src\httpp\httpp.h
# End Source File
# Begin Source File

SOURCE=..\src\mp3.c
# End Source File
# Begin Source File

SOURCE=..\src\ogg.c
# End Source File
# Begin Source File

SOURCE=..\src\net\resolver.c
# End Source File
# Begin Source File

SOURCE=..\src\net\resolver.h
# End Source File
# Begin Source File

SOURCE=..\src\shout.c
# End Source File
# Begin Source File

SOURCE=..\src\shout_private.h
# End Source File
# Begin Source File

SOURCE=..\src\net\sock.c
# End Source File
# Begin Source File

SOURCE=..\src\net\sock.h
# End Source File
# Begin Source File

SOURCE=..\src\thread\thread.c
# End Source File
# Begin Source File

SOURCE=..\src\thread\thread.h
# End Source File
# Begin Source File

SOURCE=..\src\timing\timing.c
# End Source File
# Begin Source File

SOURCE=..\src\timing\timing.h
# End Source File
# Begin Source File

SOURCE=..\src\util.c
# End Source File
# Begin Source File

SOURCE=..\src\util.h
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\include\os.h
# End Source File
# Begin Source File

SOURCE=..\include\shout\shout.h
# End Source File
# End Group
# End Target
# End Project
