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
class VinylControlManager;
class TrackCollection;

class PlayerManager : public QObject {
    Q_OBJECT
  public:
    PlayerManager(ConfigObject<ConfigValue>* pConfig,
                  SoundManager* pSoundManager,
                  EngineMaster* pEngine,
                  VinylControlManager* pVCManager);
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

    class DeckOrderingManager {
      public:
        typedef QPair<QString, QList<int> > t_deck_order;
        typedef QHash<int, QList<t_deck_order> > t_orders_hash;

        DeckOrderingManager() {
            addOrdering(4, "ABCD");
            addOrdering(4, "CABD");
            addOrdering(4, "ACDB");
        }

        bool addOrdering(int deck_count, QString order) {
            qDebug() << "AAAAAAAAAAAASSSSSSSSSSSSSSSSSSSSSSSSSSKKKKKKKKKKKKKKKKKED TO ADD " << order;
            t_deck_order new_order;
            new_order.first = order;
            new_order.second = makeLoadOrder(order);
            // ordering will be empty on error
            if (new_order.second.empty()) {
                return false;
            }
            m_hOrdersHash[deck_count].push_back(new_order);
            return true;
        }

        // The first order in the list is default
        t_deck_order getDefaultOrder(int deck_count) {
            t_orders_hash::const_iterator it = m_hOrdersHash.find(deck_count);
            if (it == m_hOrdersHash.end()) {
                m_hOrdersHash[deck_count].push_back(makeDefaultOrder(deck_count));
            }
            return m_hOrdersHash[deck_count].at(0);
        }

        const QList<t_deck_order> getDeckOrderings(int deck_count) const {
            return m_hOrdersHash[deck_count];
        }

        const QList<int> getLoadOrder(QString deckstring) const {
            t_orders_hash::const_iterator it = m_hOrdersHash.find(deckstring.length());
            if (it == m_hOrdersHash.end()) {
                return QList<int>();
            }
            foreach(const t_deck_order& order, *it) {
                if (order.first == deckstring) {
                    return order.second;
                }
            }
        }

      private:
        t_deck_order makeDefaultOrder(int deck_count) const {
            QString str_order;
            QList<int> int_order;
            for (int i = 0; i < deck_count; ++i) {
                str_order += 'A' + i;
                int_order.push_back(i);
            }
            return qMakePair(str_order, int_order);
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

        t_orders_hash m_hOrdersHash;
    };

    static const QList<DeckOrderingManager::t_deck_order> getDeckOrderings(int deck_count) {
        return s_deckOrderingManager.getDeckOrderings(deck_count);
    }

    // Returns a list of lists of possible orders to load tracks into decks.
    // Controllers label their channels differently and these are the three combinations
    // we've encountered.
    /*static const QList<QList<int> > getAvailableDeckOrderings() {
        if (PlayerManager::deckOrderings.count() == 0) {
            QList<int> order;
            // Corresponds to ABCD
            order << 0 << 1 << 2 << 3;
            PlayerManager::deckOrderings.push_back(order);
            order.clear();
            // Corresponds to CABD
            order << 1 << 2 << 0 << 3;
            PlayerManager::deckOrderings.push_back(order);
            order.clear();
            // Corresponds to ACDB
            order << 0 << 3 << 1 << 2;
            PlayerManager::deckOrderings.push_back(order);
        }
        return PlayerManager::deckOrderings;
    }*/

    //const QList<int> getDeckOrdering();

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
    void slotSkinNumDecksControlChanged(double v);

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
    EngineMaster* m_pEngine;
    VinylControlManager* m_pVCManager;
    AnalyserQueue* m_pAnalyserQueue;
    ControlObject* m_pCONumDecks;
    ControlObject* m_pCONumSamplers;
    ControlObject* m_pCONumPreviewDecks;
    ControlObject* m_pCOSkinNumDecks;
    ControlObject* m_pCOSkinNumSamplers;
    ControlObject* m_pCOSkinNumPreviewDecks;

    QList<Deck*> m_decks;
    QList<Sampler*> m_samplers;
    QList<PreviewDeck*> m_preview_decks;
    QMap<QString, BaseTrackPlayer*> m_players;

    static DeckOrderingManager s_deckOrderingManager;
    static DeckOrderingManager::t_deck_order s_currentDeckOrder;
};

#endif // PLAYERMANAGER_H
