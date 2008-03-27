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
#include "ladspacontrol.h"

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
        qDebug() << "LADSPA: setBufferSize: " << m_monoBufferSize << " (" << m_bufferSize << ")";
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

    for (EngineLADSPAControlConnectionList::iterator connection = m_Connections.begin(); connection != m_Connections.end();)
    {
        if ((*connection)->remove)
        {
            EngineLADSPAControlConnection * con = *connection;
            connection++;
            delete con->control;
            delete con->potmeter;
            con->control = NULL;
            m_Connections.remove(con);
            delete con;
        }
        else
        {
            (*connection)->control->setValue((*connection)->potmeter->get());
            connection++;
        }
    }

    for (int i = 0; i < m_monoBufferSize; i++)
    {
        m_pBufferLeft[0][i] = pIn[2 * i];
        m_pBufferRight[0][i] = pIn[2 * i + 1];
    }

    int bufferNo = 0;

    for (LADSPAInstanceList::iterator instance = m_Instances.begin(); instance != m_Instances.end();)
    {
        if ((*instance)->remove)
        {
            LADSPAInstance * inst = *instance;
            instance++;
            m_Instances.remove(inst);
            delete inst;
        }
        else
        {
            if ((*instance)->isInplaceBroken())
            {
                (*instance)->process(m_pBufferLeft[bufferNo], m_pBufferRight[bufferNo], m_pBufferLeft[1 - bufferNo], m_pBufferRight[1 - bufferNo], m_monoBufferSize);
                bufferNo = 1 - bufferNo;
            }
            else
            {
                (*instance)->process(m_pBufferLeft[bufferNo], m_pBufferRight[bufferNo], m_pBufferLeft[bufferNo], m_pBufferRight[bufferNo], m_monoBufferSize);
            }
            instance++;
        }
    }

    CSAMPLE * pOutput = (CSAMPLE *)pOut;
    if (bufferNo == 0)
    {
        for (int i = 0; i < m_monoBufferSize; i++)
        {
            pOutput[2 * i] = m_pBufferLeft[0][i];
            pOutput[2 * i + 1] = m_pBufferRight[0][i];
        }
    }
    else
    {
        for (int i = 0; i < m_monoBufferSize; i++)
        {
            pOutput[2 * i] = m_pBufferLeft[1][i];
            pOutput[2 * i + 1] = m_pBufferRight[1][i];
        }
    }
}

void EngineLADSPA::addInstance(LADSPAInstance * instance)
{
    m_Instances.append(instance);
}

EngineLADSPAControlConnection * EngineLADSPA::addControl(ControlPotmeter * potmeter, LADSPAControl * control)
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
