#include "broadcast/macos/macosmediaplayerservice.h"
#include "library/coverartcache.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "track/track.h"

#import <AppKit/AppKit.h>
#import <MediaPlayer/MediaPlayer.h>
#include <qpixmap.h>

#include "moc_macosmediaplayerservice.cpp"

constexpr int coverArtSize = 128;

MacOSMediaPlayerService::MacOSMediaPlayerService() {
    // Register command handlers.
    // At least one handler must be registered, otherwise the now playing info
    // will not show up.
    MPRemoteCommandCenter* center = [MPRemoteCommandCenter sharedCommandCenter];
    [center.playCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(
            MPRemoteCommandEvent* event) {
      // TODO: Implement play
      return MPRemoteCommandHandlerStatusCommandFailed;
    }];

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
}

namespace {
void setCoverArt(NSMutableDictionary* nowPlayingInfo, const QPixmap& pixmap) {
    if (@available(macOS 10.13.2, *)) {
        // Convert the Qt image to a Cocoa image
        CGImageRef cgImage = pixmap.toImage().toCGImage();
        NSImage* nsImage = [[NSImage alloc] initWithCGImage:cgImage
                                                       size:NSZeroSize];

        // Wrap the converted image in an artwork object as required
        // Since the block escapes, we need to de-stackify it with 'copy'
        auto fetchArtworkImage = ^NSImage*(CGSize size) {
            return nsImage;
        };
        MPMediaItemArtwork* artwork = [[MPMediaItemArtwork alloc]
                initWithBoundsSize:CGSizeMake(coverArtSize, coverArtSize)
                    requestHandler:[fetchArtworkImage copy]];
        [nowPlayingInfo setObject:artwork forKey:MPMediaItemPropertyArtwork];
    }
}
} // namespace

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

        // TODO: Use actual play time and (relative) tempo for rate
        [nowPlayingInfo setObject:@(0.0)
                           forKey:MPNowPlayingInfoPropertyElapsedPlaybackTime];
        [nowPlayingInfo setObject:@(1.0)
                           forKey:MPNowPlayingInfoPropertyDefaultPlaybackRate];
        [nowPlayingInfo setObject:@(1.0)
                           forKey:MPNowPlayingInfoPropertyPlaybackRate];

        [center setPlaybackState:MPNowPlayingPlaybackStatePlaying];
    }

    [center setNowPlayingInfo:nowPlayingInfo];
}

void MacOSMediaPlayerService::slotCoverFound(const QObject* pRequestor,
        const CoverInfo& coverInfo,
        const QPixmap& pixmap) {
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
