// playermanager.h
// Created 6/1/2010 by RJ Ryan (rryan@mit.edu)

#ifndef PLAYERMANAGER_H
#define PLAYERMANAGER_H

#include <QList>

#include "configobject.h"
#include "trackinfoobject.h"

class Player;
class Library;
class EngineMaster;
class AnalyserQueue;

class PlayerManager : public QObject {
    Q_OBJECT
  public:
    PlayerManager(ConfigObject<ConfigValue> *pConfig,
                  EngineMaster* pEngine,
                  Library* pLibrary);
    virtual ~PlayerManager();

    // Add a player to the PlayerManager
    Player* addPlayer();

    // Return the number of players
    int numPlayers();

    // Get the player by its deck number. Decks are numbered starting with 1.
    Player* getPlayer(int player);
    Player* getPlayer(QString group);

  public slots:
    void slotLoadTrackToPlayer(TrackPointer pTrack, int player);
    void slotLoadTrackIntoNextAvailablePlayer(TrackPointer pTrack);
    void slotLoadToPlayer(QString location, int player);
    void slotLoadToPlayer(QString location, QString group);

  private:
    TrackPointer lookupTrack(QString location);
    ConfigObject<ConfigValue>* m_pConfig;
    EngineMaster* m_pEngine;
    Library* m_pLibrary;
    AnalyserQueue* m_pAnalyserQueue;
    QList<Player*> m_players;
};

#endif /* PLAYERMANAGER_H */
