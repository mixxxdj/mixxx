/***************************************************************************
                          playerproxy.cpp  -  description
                             -------------------
    begin                : Fri Nov 21 2003
    copyright            : (C) 2003 by Tue and Ken Haste Andersen
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

#include "playerproxy.h"

#ifdef __PORTAUDIO__
    #include "playerportaudio.h"
#endif
#ifdef __JACK__
    #include "playerjack.h"
#endif

Player *PlayerProxy::m_pPlayer = 0;

PlayerProxy::PlayerProxy(ConfigObject<ConfigValue> *pConfig, ControlObject *pControl) : Player(pConfig, pControl)
{
    // Set API based on info stored in config database
    setSoundApi(m_pConfig->getValueString(ConfigKey("[Soundcard]","SoundApi")));

    if (!m_pPlayer)
    {
        // No valid API is stored in the database, so select the default
        QStringList api = getSoundApiList();
        QStringList::iterator it = api.begin();
        while (!m_pPlayer && (*it))
        {
            if (setSoundApi((*it)))
            {
                m_pConfig->set(ConfigKey("[Soundcard]","SoundApi"), ConfigValue((*it)));
                m_pPlayer->setDefaults();
            }
            *it++;
        }
    }
}

PlayerProxy::~PlayerProxy()
{
    if (m_pPlayer)
        delete m_pPlayer;
}

bool PlayerProxy::open()
{
    if (m_pPlayer)
        return m_pPlayer->open();
    else
        return false;
}

void PlayerProxy::close()
{
    if (m_pPlayer)
        m_pPlayer->close();
}

void PlayerProxy::setDefaults()
{
    if (m_pPlayer)
        m_pPlayer->setDefaults();
}

QStringList PlayerProxy::getInterfaces()
{
    if (m_pPlayer)
        return m_pPlayer->getInterfaces();
    else
        return QStringList();
}

QStringList PlayerProxy::getSampleRates()
{
    if (m_pPlayer)
        return m_pPlayer->getSampleRates();
    else
        return QStringList();
}

QString PlayerProxy::getSoundApiName()
{
    if (m_pPlayer)
        return m_pPlayer->getSoundApiName();
    else
        return QString();
}


QStringList PlayerProxy::getSoundApiList()
{
    QStringList result;
#ifdef __PORTAUDIO__
    result.append(PlayerPortAudio::getSoundApi());
#endif
#ifdef __JACK__
    result.append(PlayerJack::getSoundApi());
#endif

    return result;
}

bool PlayerProxy::setSoundApi(QString name)
{
    if (m_pPlayer)
        delete m_pPlayer;
    m_pPlayer = 0;

#ifdef __PORTAUDIO__
    if (!m_pPlayer && name == PlayerPortAudio::getSoundApi())
        m_pPlayer = new PlayerPortAudio(m_pConfig, m_pControl);
#endif

#ifdef __JACK__
    if (!m_pPlayer && name == PlayerJack::getSoundApi())
        m_pPlayer = new PlayerJack(m_pConfig, m_pControl);
#endif

    // Try initializing the selected API
    bool init = false;
    if (m_pPlayer)
        init = m_pPlayer->initialize();

    if (!init)
    {
        if (m_pPlayer)
            delete m_pPlayer;
        m_pPlayer = 0;
    }

    if (m_pPlayer)
        return true;
    else
        return false;
}

