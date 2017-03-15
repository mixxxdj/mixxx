@ECHO OFF

if exist *.wixobj del *.wixobj
if exist *.wixpdb del *.wixpdb
if exist *.log del *.log
if exist *.msi del *.msi
if exist subdirs\*.* del /Q subdirs\*.*
type NUL > subdirs\_EMPTY_