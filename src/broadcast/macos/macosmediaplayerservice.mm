#include "broadcast/macos/macosmediaplayerservice.h"
#include "library/autodj/autodjprocessor.h"
#include "library/coverartcache.h"
#include "mixer/deck.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "track/track.h"
#include "track/track_decl.h"
#include "util/assert.h"

#import <AppKit/AppKit.h>
#import <MediaPlayer/MediaPlayer.h>
#include <utility>

#include <QPixmap>
#include <cstdlib>

#include "moc_macosmediaplayerservice.cpp"

constexpr int coverArtSize = 128;

namespace {

MPRemoteCommandHandlerStatus commandHandlerStatusFor(bool success) {
    return success ? MPRemoteCommandHandlerStatusSuccess
                   : MPRemoteCommandHandlerStatusCommandFailed;
}

void setCoverArt(NSMutableDictionary* nowPlayingInfo, const QPixmap& pixmap) {
    if (@available(macOS 10.13.2, *)) {
        // Convert the Qt image to a Cocoa image
        CGImageRef cgImage = pixmap.toImage().toCGImage();
        NSImage* nsImage = [[NSImage alloc] initWithCGImage:cgImage
                                                       size:NSZeroSize];

        // Wrap the converted image in an artwork object as required
        // Since the block escapes, we need to de-stackify it with 'copy'
        auto fetchArtworkImage = ^NSImage*(CGSize) {
            return nsImage;
        };
        MPMediaItemArtwork* artwork = [[MPMediaItemArtwork alloc]
                initWithBoundsSize:CGSizeMake(coverArtSize, coverArtSize)
                    requestHandler:[fetchArtworkImage copy]];
        [nowPlayingInfo setObject:artwork forKey:MPMediaItemPropertyArtwork];
    }
}

} // anonymous namespace

MacOSMediaPlayerService::MacOSMediaPlayerService(
        PlayerManagerInterface* pPlayerManager) {
    // Set up deck attributes to track and control playback state
    for (unsigned i = 1; i <= pPlayerManager->numberOfDecks(); i++) {
        auto pAttributes =
                std::make_unique<DeckAttributes>(i, pPlayerManager->getDeck(i));
        connect(pAttributes.get(),
                &DeckAttributes::playChanged,
                this,
                &MacOSMediaPlayerService::slotPlayChanged);
        connect(pAttributes.get(),
                &DeckAttributes::playPositionChanged,
                this,
                &MacOSMediaPlayerService::slotPlayPositionChanged);
        m_deckAttributes.emplace_back(std::move(pAttributes));
    }

    // Set up control proxies for Auto-DJ (used to implemented 'skip to next
    // track')
    m_pCPAutoDjEnabled =
            new ControlProxy(ConfigKey("[AutoDJ]", "enabled"), this);
    m_pCPFadeNow = new ControlProxy(ConfigKey("[AutoDJ]", "fade_now"), this);

    // Write up player info so we get notified about track updates
    connect(&PlayerInfo::instance(),
            &PlayerInfo::currentPlayingTrackChanged,
            this,
            &MacOSMediaPlayerService::slotBroadcastCurrentTrack);

    // Wire up cover art cache so loaded covers get passed to our slot
    connect(CoverArtCache::instance(),
            &CoverArtCache::coverFound,
            this,
            &MacOSMediaPlayerService::slotCoverFound);

    // Register command handlers for controlling Mixxx's playback state
    // externally (i.e. via the command center). At least one handler must be
    // registered, otherwise the now playing info will not show up.
    setupCommandHandlers();
}

void MacOSMediaPlayerService::setupCommandHandlers() {
    MPRemoteCommandCenter* center = [MPRemoteCommandCenter sharedCommandCenter];

    [center.playCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(
            MPRemoteCommandEvent*) {
        bool success = updatePlayState(true);
        return commandHandlerStatusFor(success);
    }];

    [center.pauseCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(
            MPRemoteCommandEvent*) {
        bool success = updatePlayState(false);
        return commandHandlerStatusFor(success);
    }];

    [center.togglePlayPauseCommand
            addTargetWithHandler:^MPRemoteCommandHandlerStatus(
                    MPRemoteCommandEvent*) {
                bool success = togglePlayState();
                return commandHandlerStatusFor(success);
            }];

    [center.changePlaybackPositionCommand
            addTargetWithHandler:^MPRemoteCommandHandlerStatus(
                    MPRemoteCommandEvent* event) {
                auto positionEvent =
                        (MPChangePlaybackPositionCommandEvent*)event;
                bool success = updatePlayPosition(positionEvent.positionTime);
                return commandHandlerStatusFor(success);
            }];

    [center.nextTrackCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(
            MPRemoteCommandEvent*) {
        bool success = skipToNextTrack();
        return commandHandlerStatusFor(success);
    }];
}

void MacOSMediaPlayerService::slotBroadcastCurrentTrack(TrackPointer pTrack) {
    MPNowPlayingInfoCenter* center = [MPNowPlayingInfoCenter defaultCenter];
    NSMutableDictionary* nowPlayingInfo = [[NSMutableDictionary alloc] init];

    if (pTrack == nullptr) {
        [center setPlaybackState:MPNowPlayingPlaybackStatePaused];
    } else {
        // Request cover art, possibly loaded asynchronously
        if (@available(macOS 10.13.2, *)) {
            CoverArtCache::requestTrackCover(this, pTrack);
        }

        // Retrieve general track metadata
        NSURL* url = [NSURL fileURLWithPath:pTrack->getLocation().toNSString()];
        NSString* title = pTrack->getTitle().toNSString();
        NSString* artist = pTrack->getArtist().toNSString();
        NSString* album = pTrack->getAlbum().toNSString();
        NSString* albumArtist = pTrack->getAlbumArtist().toNSString();
        NSNumber* duration = @(pTrack->getDuration());

        [nowPlayingInfo setObject:@(MPNowPlayingInfoMediaTypeAudio)
                           forKey:MPNowPlayingInfoPropertyMediaType];
        [nowPlayingInfo setObject:url forKey:MPNowPlayingInfoPropertyAssetURL];
        [nowPlayingInfo setObject:@(false)
                           forKey:MPNowPlayingInfoPropertyIsLiveStream];
        [nowPlayingInfo setObject:title forKey:MPMediaItemPropertyTitle];
        [nowPlayingInfo setObject:artist forKey:MPMediaItemPropertyArtist];
        [nowPlayingInfo setObject:album forKey:MPMediaItemPropertyAlbumTitle];
        [nowPlayingInfo setObject:albumArtist
                           forKey:MPMediaItemPropertyAlbumArtist];
        [nowPlayingInfo setObject:duration
                           forKey:MPMediaItemPropertyPlaybackDuration];

        // Use actual playback rate (based on bpm adjustment)
        [nowPlayingInfo setObject:@(1.0)
                           forKey:MPNowPlayingInfoPropertyDefaultPlaybackRate];
        [nowPlayingInfo setObject:@(1.0)
                           forKey:MPNowPlayingInfoPropertyPlaybackRate];

        [center setPlaybackState:MPNowPlayingPlaybackStatePlaying];
    }

    [center setNowPlayingInfo:nowPlayingInfo];

    m_lastSentPosition = 0;
}

DeckAttributes* MacOSMediaPlayerService::getCurrentDeck() {
    int i = PlayerInfo::instance().getCurrentPlayingDeck();
    if (i >= 0) {
        return m_deckAttributes[i].get();
    } else {
        return nullptr;
    }
}

bool MacOSMediaPlayerService::isCurrentDeck(DeckAttributes* attributes) {
    return attributes->index ==
            PlayerInfo::instance().getCurrentPlayingDeck() + 1;
}

bool MacOSMediaPlayerService::togglePlayState() {
    DeckAttributes* deck = getCurrentDeck();
    if (deck != nullptr) {
        return updatePlayState(!deck->isPlaying());
    }
    return false;
}

bool MacOSMediaPlayerService::updatePlayState(bool playing) {
    DeckAttributes* deck = getCurrentDeck();
    if (deck != nullptr) {
        if (playing) {
            deck->play();
        } else {
            deck->stop();
        }
        return true;
    }
    return false;
}

bool MacOSMediaPlayerService::updatePlayPosition(double absolutePosition) {
    DeckAttributes* deck = getCurrentDeck();
    if (deck != nullptr) {
        TrackPointer track = deck->getLoadedTrack();
        DEBUG_ASSERT(track != nullptr);
        DEBUG_ASSERT(track->getDuration() > 0);
        double position = absolutePosition / track->getDuration();
        deck->setPlayPosition(position);
        return true;
    }
    return false;
}

bool MacOSMediaPlayerService::skipToNextTrack() {
    if (m_pCPAutoDjEnabled->toBool()) {
        m_pCPFadeNow->set(true);
        return true;
    }
    return false;
}

void MacOSMediaPlayerService::slotPlayChanged(
        DeckAttributes* attributes, bool playing) {
    if (isCurrentDeck(attributes)) {
        MPNowPlayingInfoCenter* center = [MPNowPlayingInfoCenter defaultCenter];
        [center setPlaybackState:playing ? MPNowPlayingPlaybackStatePlaying
                                         : MPNowPlayingPlaybackStatePaused];
    }
}

void MacOSMediaPlayerService::slotPlayPositionChanged(
        DeckAttributes* attributes, double position) {
    // We throttle play position updates by keeping track of the last sent
    // position to avoid spamming the now playing center.
    bool hasChangedEnough = std::abs(position - m_lastSentPosition) > 0.01;

    if (isCurrentDeck(attributes) && hasChangedEnough) {
        m_lastSentPosition = position;

        MPNowPlayingInfoCenter* center = [MPNowPlayingInfoCenter defaultCenter];
        NSDictionary* existingInfo = [center nowPlayingInfo];
        NSMutableDictionary* nowPlayingInfo = existingInfo == nil
                ? [[NSMutableDictionary alloc] init]
                : [[NSMutableDictionary alloc] initWithDictionary:existingInfo];

        // Compute and set absolute position from track duration
        TrackPointer track = attributes->getLoadedTrack();
        DEBUG_ASSERT(track != nullptr);
        [nowPlayingInfo setObject:@(position * track->getDuration())
                           forKey:MPNowPlayingInfoPropertyElapsedPlaybackTime];

        [center setNowPlayingInfo:nowPlayingInfo];
    }
}

void MacOSMediaPlayerService::slotCoverFound(
        const QObject* pRequestor, const CoverInfo&, const QPixmap& pixmap) {
    if (pRequestor != this) {
        return;
    }

    MPNowPlayingInfoCenter* center = [MPNowPlayingInfoCenter defaultCenter];
    NSDictionary* existingInfo = [center nowPlayingInfo];
    NSMutableDictionary* nowPlayingInfo = existingInfo == nil
            ? [[NSMutableDictionary alloc] init]
            : [[NSMutableDictionary alloc] initWithDictionary:existingInfo];

    // Update cover art in now playing with our found image
    if (!pixmap.isNull()) {
        setCoverArt(nowPlayingInfo, pixmap);
    }

    [center setNowPlayingInfo:nowPlayingInfo];
}

void MacOSMediaPlayerService::slotAllTracksPaused() {
    MPNowPlayingInfoCenter* center = [MPNowPlayingInfoCenter defaultCenter];
    [center setPlaybackState:MPNowPlayingPlaybackStatePaused];
}
