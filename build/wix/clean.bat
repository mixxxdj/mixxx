@ECHO OFF

del *.wixobj
del *.wixpdb
del *.log
del *.msi
del /Q subdirs\*.*
type NUL > subdirs\_EMPTY_