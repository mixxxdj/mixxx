/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtCore>

#include "engineladspa.h"

#include "controlpotmeter.h"
#include "ladspa/ladspacontrol.h"
#include "sampleutil.h"

EngineLADSPA * EngineLADSPA::m_pEngine = NULL;

EngineLADSPA::EngineLADSPA()
{
    m_pEngine = this;

    m_bufferSize = 0;
    m_pBufferLeft[0] = NULL;
}

EngineLADSPA::~EngineLADSPA()
{
}

void EngineLADSPA::process(const CSAMPLE * pIn, const CSAMPLE * pOut, const int iBufferSize)
{
    if (iBufferSize != m_bufferSize)
    {
        m_bufferSize = iBufferSize;
        m_monoBufferSize = m_bufferSize / 2;
        //qDebug() << "LADSPA: setBufferSize: " << m_monoBufferSize << " (" << m_bufferSize << ")";
        LADSPAControl::setBufferSize(m_monoBufferSize);

        if (m_pBufferLeft[0] != NULL)
        {
            delete [] m_pBufferLeft[0];
            delete [] m_pBufferLeft[1];
            delete [] m_pBufferRight[0];
            delete [] m_pBufferRight[1];
        }
        m_pBufferLeft[0] = new CSAMPLE[m_monoBufferSize];
        m_pBufferLeft[1] = new CSAMPLE[m_monoBufferSize];
        m_pBufferRight[0] = new CSAMPLE[m_monoBufferSize];
        m_pBufferRight[1] = new CSAMPLE[m_monoBufferSize];
    }

    EngineLADSPAControlConnectionLinkedList::iterator connection = m_Connections.begin();
    while (connection != m_Connections.end())
    {
        if ((*connection)->remove)
        {
            EngineLADSPAControlConnection *con = *connection;
            delete con->control;
            delete con->potmeter;
            con->control = NULL;
            connection = m_Connections.erase(connection);
        }
        else
        {
            (*connection)->control->setValue((*connection)->potmeter->get());
            ++connection;
        }
    }

    SampleUtil::deinterleaveBuffer(m_pBufferLeft[0], m_pBufferRight[0],
                                   pIn, m_monoBufferSize);

    LADSPAInstanceLinkedList::iterator instance = m_Instances.begin();
    while (instance != m_Instances.end())
    {
        if ((*instance)->remove)
            instance = m_Instances.erase(instance);
        else
        {
            if ((*instance)->isEnabled())
            {
                //qDebug() << "enabled";
                CSAMPLE wet = (CSAMPLE)(*instance)->getWet();
                if ((*instance)->isInplaceBroken() || wet < 1.0)
                {
                    CSAMPLE dry = 1.0 - wet;
                    (*instance)->process(m_pBufferLeft[0], m_pBufferRight[0],
                                         m_pBufferLeft[1], m_pBufferRight[1],
                                         m_monoBufferSize);
                    qDebug() << "FXUNITS: EngineLADSPA::process: IN: " << *m_pBufferLeft[0] << "OUT: " << *m_pBufferLeft[1] << "BUF: " << iBufferSize;
                    // TODO: Use run_adding() if possible
                    SampleUtil::copy2WithGain(m_pBufferLeft[0], m_pBufferLeft[0], dry,
                                              m_pBufferLeft[1], wet, m_monoBufferSize);
                    SampleUtil::copy2WithGain(m_pBufferRight[0], m_pBufferRight[0], dry,
                                              m_pBufferRight[1], wet, m_monoBufferSize);
                }
                else
                {
                    qDebug() << "FXUNITS: EngineLADSPA::process: IN: " << *m_pBufferLeft[0] << "BUF: " << iBufferSize;
                    (*instance)->process(m_pBufferLeft[0], m_pBufferRight[0],
                                         m_pBufferLeft[0], m_pBufferRight[0],
                                         m_monoBufferSize);
                    qDebug() << "FXUNITS: EngineLADSPA::process: OUT: " << *m_pBufferLeft[0] << "BUF: " << iBufferSize;
                }
            }
            ++instance;
        }
    }

    CSAMPLE * pOutput = (CSAMPLE *)pOut;
    SampleUtil::interleaveBuffer(pOutput,
                                 m_pBufferLeft[0], m_pBufferRight[0],
                                 m_monoBufferSize);
}

void EngineLADSPA::addInstance(LADSPAInstance * instance)
{
    m_Instances.append(instance);
}

EngineLADSPAControlConnection * EngineLADSPA::addControl(ControlObject * potmeter, LADSPAControl * control)
{
    EngineLADSPAControlConnection * connection = new EngineLADSPAControlConnection;
    connection->potmeter = potmeter;
    connection->control = control;
    connection->remove = 0;
    m_Connections.append(connection);
    return connection;
}

EngineLADSPA * EngineLADSPA::getEngine()
{
    return m_pEngine;
}
