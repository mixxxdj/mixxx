/**
* @file controllerpreset.h
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Mon 9 Apr 2012
* @brief Controller preset
*
* This class represents a controller preset, containing the data elements that
*   make it up.
*
*/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef CONTROLLERPRESET_H
#define CONTROLLERPRESET_H

#include <QtCore>

class ControllerPreset {

  public:
    ControllerPreset();
    ~ControllerPreset();

    /** addScriptFile(QString,QString)
        * Adds an entry to the list of script file names & associated list of function prefixes
        * @param filename Name of the XML file to add
        * @param functionprefix Function prefix to add
        */
    void addScriptFile(QString filename, QString functionprefix) {
        scriptFileNames.append(filename);
        scriptFunctionPrefixes.append(functionprefix);
    };

    QList<QString> scriptFileNames;
    QList<QString> scriptFunctionPrefixes;
};
#endif