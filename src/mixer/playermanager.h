#pragma once

#include <QList>
#include <QMap>
#include <QMutex>
#include <QObject>

#include "analyzer/trackanalysisscheduler.h"
#include "engine/channelhandle.h"
#include "preferences/usersettings.h"
#include "track/track_decl.h"
#include "util/parented_ptr.h"
#include "util/performancetimer.h"

class Auxiliary;
class BaseTrackPlayer;
class ControlObject;
class Deck;
class EffectsManager;
class EngineMaster;
class Library;
class Microphone;
class PreviewDeck;
class Sampler;
class SamplerBank;
class SoundManager;
class VisualsManager;
class ControlProxy;

// For mocking PlayerManager
class PlayerManagerInterface {
  public:
    virtual ~PlayerManagerInterface() = default;

    virtual BaseTrackPlayer* getPlayer(const QString& group) const = 0;
    virtual BaseTrackPlayer* getPlayer(const ChannelHandle& channelHandle) const = 0;

    // Get the deck by its deck number. Decks are numbered starting with 1.
    virtual Deck* getDeck(unsigned int player) const = 0;

    virtual unsigned int numberOfDecks() const = 0;

    // Get the preview deck by its deck number. Preview decks are numbered
    // starting with 1.
    virtual PreviewDeck* getPreviewDeck(unsigned int libPreviewPlayer) const = 0;

    virtual unsigned int numberOfPreviewDecks() const = 0;

    // Get the sampler by its number. Samplers are numbered starting with 1.
    virtual Sampler* getSampler(unsigned int sampler) const = 0;

    virtual unsigned int numberOfSamplers() const = 0;
};

class PlayerManager : public QObject, public PlayerManagerInterface {
    Q_OBJECT
  public:
    PlayerManager(UserSettingsPointer pConfig,
            SoundManager* pSoundManager,
            EffectsManager* pEffectsManager,
            VisualsManager* pVisualsManager,
            EngineMaster* pEngine);
    ~PlayerManager() override;

    // Add a deck to the PlayerManager
    // (currently unused, kept for consistency with other types)
    void addDeck();

    // Add number of decks according to configuration.
    void addConfiguredDecks();

    // Add a sampler to the PlayerManager
    void addSampler();

    // Load samplers from samplers.xml file in config directory
    void loadSamplers();

    // Add a PreviewDeck to the PlayerManager
    void addPreviewDeck();

    // Add a microphone to the PlayerManager
    void addMicrophone();

    // Add an auxiliary input to the PlayerManager
    void addAuxiliary();

    // Returns true if the group is a deck group. If index is non-NULL,
    // populates it with the deck number (1-indexed).
    static bool isDeckGroup(const QString& group, int* number = nullptr);

    // Returns true if the group is a sampler group. If index is non-NULL,
    // populates it with the deck number (1-indexed).
    static bool isSamplerGroup(const QString& group, int* number = nullptr);

    // Returns true if the group is a preview deck group. If index is non-NULL,
    // populates it with the deck number (1-indexed).
    static bool isPreviewDeckGroup(const QString& group, int* number = nullptr);

    // Get a BaseTrackPlayer (Deck, Sampler or PreviewDeck) by its group.
    // Auxiliaries and microphones are not players.
    BaseTrackPlayer* getPlayer(const QString& group) const override;
    // Get a BaseTrackPlayer (Deck, Sampler or PreviewDeck) by its handle.
    BaseTrackPlayer* getPlayer(const ChannelHandle& handle) const override;

    // Get the deck by its deck number. Decks are numbered starting with 1.
    Deck* getDeck(unsigned int player) const override;
    // Return the number of players. Thread-safe.
    static unsigned int numDecks();
    unsigned int numberOfDecks() const override {
        return numDecks();
    }

    PreviewDeck* getPreviewDeck(unsigned int libPreviewPlayer) const override;
    // Return the number of preview decks. Thread-safe.
    static unsigned int numPreviewDecks();
    unsigned int numberOfPreviewDecks() const override {
        return numPreviewDecks();
    }

    // Get the sampler by its number. Samplers are numbered starting with 1.
    Sampler* getSampler(unsigned int sampler) const override;
    // Return the number of samplers. Thread-safe.
    static unsigned int numSamplers();
    unsigned int numberOfSamplers() const override {
        return numSamplers();
    }

    // Get the microphone by its number. Microphones are numbered starting with 1.
    Microphone* getMicrophone(unsigned int microphone) const;

    // Get the auxiliary by its number. Auxiliaries are numbered starting with 1.
    Auxiliary* getAuxiliary(unsigned int auxiliary) const;

    // Binds signals between PlayerManager and Library. The library
    // must exist at least for the lifetime of this instance.
    void bindToLibrary(Library* pLibrary);

    // Returns the group for the ith sampler where i is zero indexed
    static QString groupForSampler(int i) {
        return QString("[Sampler%1]").arg(i + 1);
    }

    // Returns the group for the ith deck where i is zero indexed
    static QString groupForDeck(int i) {
        return QString("[Channel%1]").arg(i + 1);
    }

    // Returns the group for the ith PreviewDeck where i is zero indexed
    static QString groupForPreviewDeck(int i) {
        return QString("[PreviewDeck%1]").arg(i + 1);
    }

    // Returns the group for the ith Microphone where i is zero indexed
    static QString groupForMicrophone(int i) {
        // Before Mixxx had multiple microphone support the first microphone had
        // the group [Microphone]. For backwards compatibility we keep it that
        // way.
        QString group("[Microphone]");
        if (i > 0) {
            group = QString("[Microphone%1]").arg(i + 1);
        }
        return group;
    }

    // Returns the group for the ith Auxiliary where i is zero indexed
    static QString groupForAuxiliary(int i) {
        return QString("[Auxiliary%1]").arg(i + 1);
    }

    static QAtomicPointer<ControlProxy> m_pCOPNumDecks;
    static QAtomicPointer<ControlProxy> m_pCOPNumSamplers;
    static QAtomicPointer<ControlProxy> m_pCOPNumPreviewDecks;

  public slots:
    // Slots for loading tracks into a Player, which is either a Sampler or a Deck
    void slotLoadTrackToPlayer(TrackPointer pTrack, QString group, bool play = false);
    void slotLoadToPlayer(QString location, QString group);
    void slotCloneDeck(QString source_group, QString target_group);

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

    void slotChangeNumDecks(double v);
    void slotChangeNumSamplers(double v);
    void slotChangeNumPreviewDecks(double v);
    void slotChangeNumMicrophones(double v);
    void slotChangeNumAuxiliaries(double v);

  private slots:
    void slotAnalyzeTrack(TrackPointer track);

    void onTrackAnalysisProgress(TrackId trackId, AnalyzerProgress analyzerProgress);
    void onTrackAnalysisFinished();

  signals:
    void loadLocationToPlayer(QString location, QString group);

    // Emitted when the user tries to enable a microphone talkover control when
    // there is no input configured.
    void noMicrophoneInputConfigured();

    // Emitted when the user tries to enable an auxiliary master control when
    // there is no input configured.
    void noAuxiliaryInputConfigured();

    // Emitted when the user tries to enable deck passthrough when there is no
    // input configured.
    void noDeckPassthroughInputConfigured();

    // Emitted when the user tries to enable vinyl control when there is no
    // input configured.
    void noVinylControlInputConfigured();

    // Emitted when the number of decks changes.
    void numberOfDecksChanged(int decks);

    void trackAnalyzerProgress(TrackId trackId, AnalyzerProgress analyzerProgress);
    void trackAnalyzerIdle();

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
    // Must hold m_mutex before calling this method. Internal method that
    // creates a new microphone.
    void addMicrophoneInner();
    // Must hold m_mutex before calling this method. Internal method that
    // creates a new auxiliary.
    void addAuxiliaryInner();

    // Used to protect access to PlayerManager state across threads.
    mutable QMutex m_mutex;

    PerformanceTimer m_cloneTimer;
    QString m_lastLoadedPlayer;

    UserSettingsPointer m_pConfig;
    SoundManager* m_pSoundManager;
    EffectsManager* m_pEffectsManager;
    VisualsManager* m_pVisualsManager;
    EngineMaster* m_pEngine;
    SamplerBank* m_pSamplerBank;
    ControlObject* m_pCONumDecks;
    ControlObject* m_pCONumSamplers;
    ControlObject* m_pCONumPreviewDecks;
    ControlObject* m_pCONumMicrophones;
    ControlObject* m_pCONumAuxiliaries;
    parented_ptr<ControlProxy> m_pAutoDjEnabled;

    TrackAnalysisScheduler::Pointer m_pTrackAnalysisScheduler;

    QList<Deck*> m_decks;
    QList<Sampler*> m_samplers;
    QList<PreviewDeck*> m_previewDecks;
    QList<Microphone*> m_microphones;
    QList<Auxiliary*> m_auxiliaries;
    QMap<ChannelHandle, BaseTrackPlayer*> m_players;
};
