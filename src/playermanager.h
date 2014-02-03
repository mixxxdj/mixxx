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
class TrackCollection;

class PlayerManager : public QObject {
    Q_OBJECT
  public:
    PlayerManager(ConfigObject<ConfigValue>* pConfig,
                  SoundManager* pSoundManager,
                  EngineMaster* pEngine);
    virtual ~PlayerManager();

    // Add a deck to the PlayerManager
    void addDeck();

    // Add a sampler to the PlayerManager
    void addSampler();

    // Add a PreviewDeck to the PlayerManager
    void addPreviewDeck();

    // Return the number of players. Thread-safe.
    static unsigned int numDecks();

    // Returns true if the group is a deck group. If index is non-NULL,
    // populates it with the deck number (1-indexed).
    static bool isDeckGroup(const QString& group, int* number=NULL);

    // Return the number of samplers. Thread-safe.
    static unsigned int numSamplers();

    // Return the number of preview decks. Thread-safe.
    static unsigned int numPreviewDecks();

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

    class DeckOrderingManager {
      public:
        struct deck_order_t {
            deck_order_t() { }
            deck_order_t(QString l, QList<int> o) : label(l), load_order(o) { }

            QString label;
            QList<int> load_order;
        };

        typedef QHash<int, QList<deck_order_t> > orders_hash_t;

        DeckOrderingManager() {
            // Known orderings for 4-deck controllers.
            addOrdering(4, "ABCD");
            addOrdering(4, "CABD");
            addOrdering(4, "ACDB");
        }

        bool addOrdering(int deck_count, QString order) {
            deck_order_t new_order(order, makeLoadOrder(order));
            // ordering will be empty on error
            if (new_order.load_order.empty()) {
                return false;
            }
            m_hOrdersHash[deck_count].push_back(new_order);
            return true;
        }

        // Just take the first order in the list is default.  Make sure it's the natural ordering
        // "ABCD..."
        deck_order_t getDefaultOrder(int deck_count) {
            // Don't access the hash directly in case we need to generate a default order.
            return getDeckOrderings(deck_count).at(0);
        }

        const QList<deck_order_t> getDeckOrderings(int deck_count) {
            orders_hash_t::const_iterator it = m_hOrdersHash.find(deck_count);
            if (it == m_hOrdersHash.end()) {
                m_hOrdersHash[deck_count].push_back(makeDefaultOrder(deck_count));
            }
            return m_hOrdersHash[deck_count];
        }

      private:
        deck_order_t makeDefaultOrder(int deck_count) const {
            QString str_order;
            QList<int> int_order;
            for (int i = 0; i < deck_count; ++i) {
                str_order += 'A' + i;
                int_order.push_back(i);
            }
            return deck_order_t(str_order, int_order);
        }

        // Constructs a list of integers for load-order based on the string.
        // If the string has errors, then we return an empty list.
        QList<int> makeLoadOrder(QString str_order) {
            QList<int> int_order;
            for (int i = 0; i < str_order.length(); ++i) {
                int pos = str_order.indexOf('A' + i);
                if (pos == -1) {
                    return QList<int>();
                }
                int_order.push_back(pos);
            }
            return int_order;
        }

        orders_hash_t m_hOrdersHash;
    };

    static const QList<DeckOrderingManager::deck_order_t> getDeckOrderings(int deck_count) {
        return s_deckOrderingManager.getDeckOrderings(deck_count);
    }

    static const DeckOrderingManager::deck_order_t getDefaultOrder(int deck_count) {
        return s_deckOrderingManager.getDefaultOrder(deck_count);
    }

    void setDeckOrder(QString order);

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
    // When the skin changes, we need to change the orientations of the decks
    // because deck B might have moved from the right to the left.
    void reorientDecks();

    // Used to protect access to PlayerManager state across threads.
    mutable QMutex m_mutex;

    ConfigObject<ConfigValue>* m_pConfig;
    SoundManager* m_pSoundManager;
    EngineMaster* m_pEngine;
    AnalyserQueue* m_pAnalyserQueue;
    ControlObject* m_pCONumDecks;
    ControlObject* m_pCONumSamplers;
    ControlObject* m_pCONumPreviewDecks;

    QList<Deck*> m_decks;
    QList<Sampler*> m_samplers;
    QList<PreviewDeck*> m_preview_decks;
    QMap<QString, BaseTrackPlayer*> m_players;

    static DeckOrderingManager s_deckOrderingManager;
    static DeckOrderingManager::deck_order_t s_currentDeckOrder;
};

#endif // PLAYERMANAGER_H
