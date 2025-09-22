#pragma once

#include <gtest/gtest_prod.h>

#include <QList>
#include <QMap>
#include <QObject>

#include "analyzer/trackanalysisscheduler.h"
#include "engine/channelhandle.h"
#include "preferences/usersettings.h"
#include "track/track_decl.h"
#include "util/compatibility/qmutex.h"
#include "util/parented_ptr.h"
#include "util/performancetimer.h"

class Auxiliary;
class BaseTrackPlayer;
class ControlObject;
class Deck;
class EffectsManager;
class EngineMixer;
class Library;
class Microphone;
class PreviewDeck;
class Sampler;
class SamplerBank;
class SoundManager;
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
            EngineMixer* pEngine);
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

    // Returns the track that was last ejected or unloaded. Can return nullptr or
    // invalid TrackId in case of error.
    TrackPointer getLastEjectedTrack() const;
    TrackPointer getSecondLastEjectedTrack() const;

    // Get the microphone by its number. Microphones are numbered starting with 1.
    Microphone* getMicrophone(unsigned int microphone) const;

    // Get the auxiliary by its number. Auxiliaries are numbered starting with 1.
    Auxiliary* getAuxiliary(unsigned int auxiliary) const;

    // Binds signals between PlayerManager and Library. The library
    // must exist at least for the lifetime of this instance.
    void bindToLibrary(Library* pLibrary);

    QStringList getVisualPlayerGroups();

    // Returns the group for the ith sampler where i is zero indexed
    static QString groupForSampler(int i) {
        DEBUG_ASSERT(i >= 0);
        return QStringLiteral("[Sampler") + QString::number(i + 1) + QChar(']');
    }

    // Returns the group for the ith deck where i is zero indexed
    static QString groupForDeck(int i) {
        DEBUG_ASSERT(i >= 0);
        return QStringLiteral("[Channel") + QString::number(i + 1) + QChar(']');
    }

#ifdef __STEM__
    // Returns the group for the deck and stem where deckIndex and stemInde are zero based
    static QString groupForDeckStem(int deckIdx, int stemIdx) {
        // Removed hardoded 4 -> 5
        // DEBUG_ASSERT(deckIdx >= 0 && stemIdx >= 0 && stemIdx < 4);
        DEBUG_ASSERT(deckIdx >= 0 && stemIdx >= 0 && stemIdx < 5);
        return QStringLiteral("[Channel") + QString::number(deckIdx + 1) +
                QStringLiteral("_Stem") + QChar('1' + stemIdx) + QChar(']');
    }
#endif

    // Returns the group for the ith PreviewDeck where i is zero indexed
    static QString groupForPreviewDeck(int i) {
        DEBUG_ASSERT(i >= 0);
        return QStringLiteral("[PreviewDeck") + QString::number(i + 1) + QChar(']');
    }

    // Returns the group for the ith Microphone where i is zero indexed
    static QString groupForMicrophone(int i) {
        DEBUG_ASSERT(i >= 0);
        // Before Mixxx had multiple microphone support the first microphone had
        // the group [Microphone]. For backwards compatibility we keep it that
        // way.
        if (i > 0) {
            return QStringLiteral("[Microphone") + QString::number(i + 1) + QChar(']');
        } else {
            return QStringLiteral("[Microphone]");
        }
    }

    // Returns the group for the ith Auxiliary where i is zero indexed
    static QString groupForAuxiliary(int i) {
        DEBUG_ASSERT(i >= 0);
        return QStringLiteral("[Auxiliary") + QString::number(i + 1) + QChar(']');
    }

    static QAtomicPointer<ControlProxy> m_pCOPNumDecks;
    static QAtomicPointer<ControlProxy> m_pCOPNumSamplers;
    static QAtomicPointer<ControlProxy> m_pCOPNumPreviewDecks;

  public slots:
    // Slots for loading tracks into a Player, which is either a Sampler or a Deck
#ifdef __STEM__
    void slotLoadTrackToPlayer(TrackPointer pTrack,
            const QString& group,
            mixxx::StemChannelSelection stemMask,
            bool play);
#else
    void slotLoadTrackToPlayer(TrackPointer pTrack, const QString& group, bool play);
#endif
    void slotLoadLocationToPlayer(const QString& location, const QString& group, bool play);
    void slotLoadLocationToPlayerMaybePlay(const QString& location, const QString& group);

    void slotCloneDeck(const QString& source_group, const QString& target_group);

    // Slots for loading tracks to decks
    void slotLoadTrackIntoNextAvailableDeck(TrackPointer pTrack);
    void slotLoadLocationIntoNextAvailableDeck(const QString& location, bool play = false);
    // Loads the location to the deck. deckNumber is 1-indexed
    void slotLoadToDeck(const QString& location, int deckNumber);

    // Loads the location to the preview deck. previewDeckNumber is 1-indexed
    void slotLoadToPreviewDeck(const QString& location, int previewDeckNumber);
    // Slots for loading tracks to samplers
    void slotLoadTrackIntoNextAvailableSampler(TrackPointer pTrack);
    // Loads the location to the sampler. samplerNumber is 1-indexed
    void slotLoadToSampler(const QString& location, int samplerNumber);

    void slotChangeNumDecks(double v);
    void slotChangeNumSamplers(double v);
    void slotChangeNumPreviewDecks(double v);
    void slotChangeNumMicrophones(double v);
    void slotChangeNumAuxiliaries(double v);

  protected slots:
    FRIEND_TEST(PlayerManagerTest, UnEjectInvalidTrackIdTest);
    void slotSaveEjectedTrack(TrackPointer track);

  private slots:
    void slotAnalyzeTrack(TrackPointer track);

    void onTrackAnalysisProgress(TrackId trackId, AnalyzerProgress analyzerProgress);
    void onTrackAnalysisFinished();

  signals:
    void loadLocationToPlayer(const QString& location, const QString& group, bool play);

    // Emitted when the user tries to enable a microphone talkover control when
    // there is no input configured.
    void noMicrophoneInputConfigured();

    // Emitted when the user tries to enable an auxiliary `main_mix` control
    // when there is no input configured.
    void noAuxiliaryInputConfigured();

    // Emitted when the user tries to enable deck passthrough when there is no
    // input configured.
    void noDeckPassthroughInputConfigured();

    // Emitted when the user tries to enable vinyl control when there is no
    // input configured.
    void noVinylControlInputConfigured();

    // Emitted when the number of decks changes.
    void numberOfDecksChanged(int decks);
    void numberOfSamplersChanged(int samplers);

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
    mutable QT_RECURSIVE_MUTEX m_mutex;

    PerformanceTimer m_cloneTimer;
    QString m_lastLoadedPlayer;

    UserSettingsPointer m_pConfig;
    Library* m_pLibrary;
    SoundManager* m_pSoundManager;
    EffectsManager* m_pEffectsManager;
    EngineMixer* m_pEngine;
    SamplerBank* m_pSamplerBank;
    std::unique_ptr<ControlObject> m_pCONumDecks;
    std::unique_ptr<ControlObject> m_pCONumSamplers;
    std::unique_ptr<ControlObject> m_pCONumPreviewDecks;
    std::unique_ptr<ControlObject> m_pCONumMicrophones;
    std::unique_ptr<ControlObject> m_pCONumAuxiliaries;
    parented_ptr<ControlProxy> m_pAutoDjEnabled;

    TrackAnalysisScheduler::Pointer m_pTrackAnalysisScheduler;

    TrackId m_secondLastEjectedTrackId;
    TrackId m_lastEjectedTrackId;

    QList<Deck*> m_decks;
    QList<Sampler*> m_samplers;
    QList<PreviewDeck*> m_previewDecks;
    QList<Microphone*> m_microphones;
    QList<Auxiliary*> m_auxiliaries;
    QMap<ChannelHandle, BaseTrackPlayer*> m_players;
};
