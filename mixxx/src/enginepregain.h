/***************************************************************************
                          enginepregain.h  -  description
                             -------------------
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
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

#ifndef ENGINEPREGAIN_H
#define ENGINEPREGAIN_H

#include "engineobject.h"

class ControlEngine;

class EnginePregain : public EngineObject {
public:
  EnginePregain(const char *group);
  ~EnginePregain();
    void notify(double) {};
  CSAMPLE *process(const CSAMPLE*, const int);

private:
  ControlEngine* potmeterPregain;
  CSAMPLE *buffer;
};

#endif
