#pragma once

#include <memory>

#include "control/controlobject.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "proto/keys.pb.h"
#include "track/keyutils.h"
#include "track/track.h"

/// The deck controls that KeyHighlightManager and PlayerInfo bind to, for tests
/// without an engine or a PlayerManager, and helpers to drive them. Derive the
/// test fixture from MixxxTest (or a subclass) first and from this class
/// second, so the controls are created after MixxxTest set up the control
/// registry and destroyed before it tears it down. Call PlayerInfo::create()
/// and KeyHighlightManager::createInstance() afterwards: their proxies bind on
/// construction. The manager creates the key_highlight controls itself.
class KeyHighlightTestControls {
  protected:
    static constexpr int kDecks = 2;

    KeyHighlightTestControls() {
        m_pNumDecks = std::make_unique<ControlObject>(
                ConfigKey(QStringLiteral("[App]"), QStringLiteral("num_decks")));
        // PlayerInfo binds to these, and setTrackInfo() reads them to find the
        // loudest deck. Their values don't matter here.
        m_pCrossfader = std::make_unique<ControlObject>(ConfigKey(
                QStringLiteral("[Master]"), QStringLiteral("crossfader")));
        m_pNumSamplers = std::make_unique<ControlObject>(ConfigKey(
                QStringLiteral("[App]"), QStringLiteral("num_samplers")));
        m_pNumPreviewDecks = std::make_unique<ControlObject>(ConfigKey(
                QStringLiteral("[App]"), QStringLiteral("num_preview_decks")));
        for (int i = 0; i < kDecks; ++i) {
            const QString group = PlayerManager::groupForDeck(i);
            // The playing key, the loaded track's stored key and the
            // displayed BPM.
            m_key[i] = std::make_unique<ControlObject>(
                    ConfigKey(group, QStringLiteral("key")));
            m_fileKey[i] = std::make_unique<ControlObject>(
                    ConfigKey(group, QStringLiteral("file_key")));
            m_visualBpm[i] = std::make_unique<ControlObject>(
                    ConfigKey(group, QStringLiteral("visual_bpm")));
            m_playerInfoEnv[i][0] = std::make_unique<ControlObject>(
                    ConfigKey(group, QStringLiteral("play")));
            m_playerInfoEnv[i][1] = std::make_unique<ControlObject>(
                    ConfigKey(group, QStringLiteral("pregain")));
            m_playerInfoEnv[i][2] = std::make_unique<ControlObject>(
                    ConfigKey(group, QStringLiteral("volume")));
            m_playerInfoEnv[i][3] = std::make_unique<ControlObject>(
                    ConfigKey(group, QStringLiteral("orientation")));
        }
        m_pNumDecks->set(kDecks);
    }

    static ConfigKey highlightKey(int deck) {
        return ConfigKey(PlayerManager::groupForDeck(deck), QStringLiteral("key_highlight"));
    }

    void setDeckKey(int deck, mixxx::track::io::key::ChromaticKey k) {
        m_key[deck]->set(KeyUtils::keyToNumericValue(k));
    }
    void setDeckFileKey(int deck, mixxx::track::io::key::ChromaticKey k) {
        m_fileKey[deck]->set(KeyUtils::keyToNumericValue(k));
    }
    // Sets the stored key and the playing key. Pass a different playing key
    // to simulate a pitched deck.
    void setDeckKeys(int deck,
            mixxx::track::io::key::ChromaticKey fileKey,
            mixxx::track::io::key::ChromaticKey playingKey) {
        setDeckFileKey(deck, fileKey);
        setDeckKey(deck, playingKey);
    }
    // Requires a KeyHighlightManager, which creates the control.
    void setHighlight(int deck, bool on) {
        ControlObject::set(highlightKey(deck), on ? 1.0 : 0.0);
    }
    bool isHighlightOn(int deck) const {
        return ControlObject::toBool(highlightKey(deck));
    }
    void setDeckBpm(int deck, double bpm) {
        m_visualBpm[deck]->set(bpm);
    }
    // Loads a track on a deck, or ejects it with an invalid id. The manager
    // only reads the track id, so a dummy track stands in for a library track.
    TrackPointer loadTrack(int deck, TrackId trackId) {
        TrackPointer pTrack;
        if (trackId.isValid()) {
            pTrack = Track::newDummy(
                    QStringLiteral("/test/keyhighlight/track.mp3"), trackId);
        }
        PlayerInfo::instance().setTrackInfo(PlayerManager::groupForDeck(deck), pTrack);
        return pTrack;
    }
    TrackPointer loadTrack(int deck, int trackId) {
        return loadTrack(deck, TrackId(QVariant(trackId)));
    }
    void ejectTrack(int deck) {
        loadTrack(deck, TrackId());
    }

    std::unique_ptr<ControlObject> m_pNumDecks;
    std::unique_ptr<ControlObject> m_pCrossfader;
    std::unique_ptr<ControlObject> m_pNumSamplers;
    std::unique_ptr<ControlObject> m_pNumPreviewDecks;
    std::unique_ptr<ControlObject> m_key[kDecks];
    std::unique_ptr<ControlObject> m_fileKey[kDecks];
    std::unique_ptr<ControlObject> m_visualBpm[kDecks];
    // play/pregain/volume/orientation per deck, for PlayerInfo.
    std::unique_ptr<ControlObject> m_playerInfoEnv[kDecks][4];
};
