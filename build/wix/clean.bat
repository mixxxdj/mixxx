@ECHO OFF

if exist *.wixobj del *.wixobj
if exist *.wixpdb del *.wixpdb
if exist *.log del *.log
if exist *.msi del *.msi
if exist subdirs\*.wxs del /Q subdirs\*.wxs
if exist subdirs\*.wixobj del /Q subdirs\*.wixobj
type NUL > subdirs\_EMPTY_