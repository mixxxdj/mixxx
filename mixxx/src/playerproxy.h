/***************************************************************************
                          playerproxy.h  -  description
                             -------------------
    begin                : Fri Nov 21 2003
    copyright            : (C) 2003 by Tue Haste Andersen
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

#ifndef PLAYERPROXY_H
#define PLAYERPROXY_H

#include "player.h"

class PlayerProxy : public Player
{
public:
    PlayerProxy(ConfigObject<ConfigValue> *pConfig, ControlObject *pControl);
    ~PlayerProxy();
    bool initialize() { return false; };
    bool open();
    void close();
    void setDefaults();
    QStringList getInterfaces();
    QStringList getSampleRates();
    QString getSoundApiName();
    /** Return list of supported sound api's */
    static QStringList getSoundApiList();
    /** Select active sound api */
    bool setSoundApi(QString name);
    /** Satisfy virtual declaration in EngineObject */
    void process(const CSAMPLE *, const CSAMPLE *, const int) {};
    /** Static function to return active player class. Needed for ASIO implementation */
    static Player *getPlayer() { return m_pPlayer; }

protected:
    /** Pointer to active Player class */
    static Player *m_pPlayer;
};

#endif




