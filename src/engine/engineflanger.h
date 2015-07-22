/***************************************************************************
                          engineflanger.h  -  description
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

#ifndef ENGINEFLANGER_H
#define ENGINEFLANGER_H

#include <QMap>
#include <QSharedPointer>

#include "engine/engineobject.h"

class ControlPotmeter;
class ControlPushButton;
class EngineFlangerControls;

const int max_delay = 5000;

class EngineFlanger : public EngineObject {
    Q_OBJECT
  public:
    EngineFlanger(const char* group);
    virtual ~EngineFlanger();

    void process(const CSAMPLE* pIn, CSAMPLE* pOut, const int iBufferSize);

  private:
    ControlPushButton* m_pFlangerEnable;
    CSAMPLE* m_pDelay_buffer;
    int  m_LFOamplitude;
    int m_average_delay_length;
    int m_time;
    int m_delay_pos;
    QSharedPointer<EngineFlangerControls> m_pControls;
};

#endif
