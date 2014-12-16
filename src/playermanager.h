// playermanager.h
// Created 6/1/2010 by RJ Ryan (rryan@mit.edu)

#ifndef PLAYERMANAGER_H
#define PLAYERMANAGER_H

#include <QList>
#include <QMutex>

#include "configobject.h"
#include "trackinfoobject.h"

class ControlObject;
class Deck;
class Sampler;
class PreviewDeck;
class BaseTrackPlayer;

class Library;
class EngineMaster;
class AnalyserQueue;
class SoundManager;
class EffectsManager;
class TrackCollection;

// For mocking PlayerManager.
class PlayerManagerInterface {
  public:
    // Get a BaseTrackPlayer (i.e. a Deck or a Sampler) by its group
    virtual BaseTrackPlayer* getPlayer(QString group) const = 0;

    // Get the deck by its deck number. Decks are numbered starting with 1.
    virtual Deck* getDeck(unsigned int player) const = 0;

    // Returns the number of decks.
    virtual unsigned int numberOfDecks() const = 0;

    // Get the preview deck by its deck number. Preview decks are numbered
    // starting with 1.
    virtual PreviewDeck* getPreviewDeck(unsigned int libPreviewPlayer) const = 0;

    // Returns the number of preview decks.
    virtual unsigned int numberOfPreviewDecks() const = 0;

    // Get the sampler by its number. Samplers are numbered starting with 1.
    virtual Sampler* getSampler(unsigned int sampler) const = 0;

    // Returns the number of sampler decks.
    virtual unsigned int numberOfSamplers() const = 0;
};

class PlayerManager : public QObject, public PlayerManagerInterface {
    Q_OBJECT
  public:
    PlayerManager(ConfigObject<ConfigValue>* pConfig,
                  SoundManager* pSoundManager,
                  EffectsManager* pEffectsManager,
                  EngineMaster* pEngine);
    virtual ~PlayerManager();

    // Add a deck to the PlayerManager
    void addDeck();

    // Add number of decks according to configuration.
    void addConfiguredDecks();

    // Add a sampler to the PlayerManager
    void addSampler();

    // Add a PreviewDeck to the PlayerManager
    void addPreviewDeck();

    // Return the number of players. Thread-safe.
    static unsigned int numDecks();

    unsigned int numberOfDecks() const {
        return numDecks();
    }

    // Returns true if the group is a deck group. If index is non-NULL,
    // populates it with the deck number (1-indexed).
    static bool isDeckGroup(const QString& group, int* number=NULL);

    // Returns true if the group is a preview deck group. If index is non-NULL,
    // populates it with the deck number (1-indexed).
    static bool isPreviewDeckGroup(const QString& group, int* number=NULL);

    // Return the number of samplers. Thread-safe.
    static unsigned int numSamplers();

    unsigned int numberOfSamplers() const {
        return numSamplers();
    }

    // Return the number of preview decks. Thread-safe.
    static unsigned int numPreviewDecks();

    unsigned int numberOfPreviewDecks() const {
        return numPreviewDecks();
    }

    // Get a BaseTrackPlayer (i.e. a Deck or a Sampler) by its group
    BaseTrackPlayer* getPlayer(QString group) const;

    // Get the deck by its deck number. Decks are numbered starting with 1.
    Deck* getDeck(unsigned int player) const;

    PreviewDeck* getPreviewDeck(unsigned int libPreviewPlayer) const;

    // Get the sampler by its number. Samplers are numbered starting with 1.
    Sampler* getSampler(unsigned int sampler) const;

    // Binds signals between PlayerManager and Library. Does not store a pointer
    // to the Library.
    void bindToLibrary(Library* pLibrary);

    // Returns the group for the ith sampler where i is zero indexed
    static QString groupForSampler(int i) {
        return QString("[Sampler%1]").arg(i+1);
    }

    // Returns the group for the ith deck where i is zero indexed
    static QString groupForDeck(int i) {
        return QString("[Channel%1]").arg(i+1);
    }

    // Returns the group for the ith PreviewDeck where i is zero indexed
    static QString groupForPreviewDeck(int i) {
        return QString("[PreviewDeck%1]").arg(i+1);
    }

    // Used to determine if the user has configured an input for the given vinyl deck.
    bool hasVinylInput(int inputnum) const;

  public slots:
    // Slots for loading tracks into a Player, which is either a Sampler or a Deck
    void slotLoadTrackToPlayer(TrackPointer pTrack, QString group, bool play = false);
    void slotLoadToPlayer(QString location, QString group);

    // Slots for loading tracks to decks
    void slotLoadTrackIntoNextAvailableDeck(TrackPointer pTrack);
    // Loads the location to the deck. deckNumber is 1-indexed
    void slotLoadToDeck(QString location, int deckNumber);

    // Loads the location to the preview deck. previewDeckNumber is 1-indexed
    void slotLoadToPreviewDeck(QString location, int previewDeckNumber);
    // Slots for loading tracks to samplers
    void slotLoadTrackIntoNextAvailableSampler(TrackPointer pTrack);
    // Loads the location to the sampler. samplerNumber is 1-indexed
    void slotLoadToSampler(QString location, int samplerNumber);

    void slotNumDecksControlChanged(double v);
    void slotNumSamplersControlChanged(double v);
    void slotNumPreviewDecksControlChanged(double v);

  signals:
    void loadLocationToPlayer(QString location, QString group);

  private:
    TrackPointer lookupTrack(QString location);
    // Must hold m_mutex before calling this method. Internal method that
    // creates a new deck.
    void addDeckInner();
    // Must hold m_mutex before calling this method. Internal method that
    // creates a new sampler.
    void addSamplerInner();
    // Must hold m_mutex before calling this method. Internal method that
    // creates a new preview deck.
    void addPreviewDeckInner();

    // Used to protect access to PlayerManager state across threads.
    mutable QMutex m_mutex;

    ConfigObject<ConfigValue>* m_pConfig;
    SoundManager* m_pSoundManager;
    EffectsManager* m_pEffectsManager;
    EngineMaster* m_pEngine;
    AnalyserQueue* m_pAnalyserQueue;
    ControlObject* m_pCONumDecks;
    ControlObject* m_pCONumSamplers;
    ControlObject* m_pCONumPreviewDecks;

    QList<Deck*> m_decks;
    QList<Sampler*> m_samplers;
    QList<PreviewDeck*> m_preview_decks;
    QMap<QString, BaseTrackPlayer*> m_players;
};

#endif // PLAYERMANAGER_H
