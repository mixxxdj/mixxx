@echo off
REM Batch to open all files that include gtest for editing

notepad src\controllers\legacycontrollersettings.h
notepad src\engine\controls\bpmcontrol.h
notepad src\engine\controls\cuecontrol.h
notepad src\engine\controls\enginecontrol.h
notepad src\engine\enginebuffer.h
notepad src\engine\sync\enginesync.h
notepad src\engine\sync\synccontrol.h
notepad src\library\scanner\libraryscanner.h
notepad src\library\trackcollection.h
notepad src\mixer\playermanager.h
notepad src\sources\soundsourceproxy.h
notepad src\track\serato\beatsimporter.h
notepad src\util\fileinfo.h

echo All files opened. Edit #include <gtest/gtest_prod.h> in each file.
pause
