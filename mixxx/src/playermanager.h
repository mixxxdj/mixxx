// playermanager.h
// Created 6/1/2010 by RJ Ryan (rryan@mit.edu)

#ifndef PLAYERMANAGER_H
#define PLAYERMANAGER_H

#include <QList>

#include "configobject.h"
#include "trackinfoobject.h"

class ControlObject;
class Deck;
class Sampler;
class BaseTrackPlayer;

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

    // Add a deck to the PlayerManager
    Deck* addDeck();

    // Add a sampler to the PlayerManager
    Sampler* addSampler();

    // Return the number of players
    unsigned int numDecks() const;

    // Return the number of samplers
    unsigned int numSamplers() const;

    // Get a BaseTrackPlayer (i.e. a Deck or a Sampler) by its group
    BaseTrackPlayer* getPlayer(QString group) const;

    // Get the deck by its deck number. Decks are numbered starting with 1.
    Deck* getDeck(unsigned int player) const;

    // Get the sampler by its number. Samplers are numbered starting with 1.
    Sampler* getSampler(unsigned int sampler) const;

    // Returns the group for the ith sampler where i is zero indexed
    static QString groupForSampler(int i) {
        return QString("[Sampler%1]").arg(i+1);
    }

    // Returns the group for the ith deck where i is zero indexed
    static QString groupForDeck(int i) {
        return QString("[Channel%1]").arg(i+1);
    }

  public slots:
    // Slots for loading tracks into a Player, which is either a Sampler or a Deck
    void slotLoadTrackToPlayer(TrackPointer pTrack, QString group);
    void slotLoadToPlayer(QString location, QString group);

    // Slots for loading tracks to decks
    void slotLoadTrackIntoNextAvailableDeck(TrackPointer pTrack);
    void slotLoadToDeck(QString location, int deckNumber);

    // Slots for loading tracks to samplers
    void slotLoadTrackIntoNextAvailableSampler(TrackPointer pTrack);
    void slotLoadToSampler(QString location, int samplerNumber);

  private:
    TrackPointer lookupTrack(QString location);
    ConfigObject<ConfigValue>* m_pConfig;
    EngineMaster* m_pEngine;
    Library* m_pLibrary;
    AnalyserQueue* m_pAnalyserQueue;
    ControlObject* m_pCONumDecks;
    ControlObject* m_pCONumSamplers;

    QList<Deck*> m_decks;
    QList<Sampler*> m_samplers;
    QMap<QString, BaseTrackPlayer*> m_players;
};

#endif /* PLAYERMANAGER_H */
