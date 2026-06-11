#include "library/itunes/itunesiosassetexporter.h"

#import <AVFoundation/AVFoundation.h>
#import <MediaPlayer/MediaPlayer.h>
#import <dispatch/dispatch.h>

#include <QDir>
#include <QString>
#include <QUrl>
#include <QUrlQuery>
#include <QtGlobal>

ITunesIOSAssetExporter::ITunesIOSAssetExporter(const QDir& outputDir)
        : outputDir(outputDir) {
    if (!outputDir.exists()) {
        qInfo() << "Creating output directory for exported iTunes assets at"
                << outputDir;
        outputDir.mkpath(".");
    }
}

QString ITunesIOSAssetExporter::exportAsset(const QUrl& url) {
    AVURLAsset* asset = nil;
    NSMutableArray<AVMetadataItem*>* metadata = [[NSMutableArray alloc] init];
    QString baseName;

    if (url.scheme() == "ipod-library") {
        // We perform an additional lookup through Media Player instead of just
        // constructing the AVURLAsset to retrieve additional metadata.

        QUrlQuery query(url);
        QString persistentID = query.queryItemValue("id");

        MPMediaQuery* mediaQuery = [MPMediaQuery songsQuery];
        [mediaQuery
                addFilterPredicate:
                        [MPMediaPropertyPredicate
                                predicateWithValue:persistentID.toNSString()
                                       forProperty:
                                               MPMediaItemPropertyPersistentID]];

        MPMediaItem* item = [mediaQuery.items firstObject];
        if (item == nil) {
            qCritical() << "Could not find item with ID in local music library"
                        << persistentID;
            return QString();
        }

        asset = [[AVURLAsset alloc] initWithURL:item.assetURL options:nil];
        baseName = persistentID;

        // Add metadata tags for the exported audio file

        AVMutableMetadataItem* titleItem = [[AVMutableMetadataItem alloc] init];
        titleItem.keySpace = AVMetadataKeySpaceCommon;
        titleItem.key = AVMetadataCommonKeyTitle;
        titleItem.value = item.title;
        [metadata addObject:titleItem];

        AVMutableMetadataItem* artistItem =
                [[AVMutableMetadataItem alloc] init];
        artistItem.keySpace = AVMetadataKeySpaceCommon;
        artistItem.key = AVMetadataCommonKeyArtist;
        artistItem.value = item.artist;
        [metadata addObject:artistItem];

        AVMutableMetadataItem* albumItem = [[AVMutableMetadataItem alloc] init];
        albumItem.keySpace = AVMetadataKeySpaceCommon;
        albumItem.key = AVMetadataCommonKeyAlbumName;
        albumItem.value = item.albumTitle;
        [metadata addObject:albumItem];

        AVMutableMetadataItem* genreItem = [[AVMutableMetadataItem alloc] init];
        genreItem.keySpace = AVMetadataKeySpaceiTunes;
        genreItem.key = AVMetadataiTunesMetadataKeyUserGenre;
        genreItem.value = item.genre;
        [metadata addObject:genreItem];

        AVMutableMetadataItem* releaseDateItem =
                [[AVMutableMetadataItem alloc] init];
        releaseDateItem.keySpace = AVMetadataKeySpaceiTunes;
        releaseDateItem.key = AVMetadataiTunesMetadataKeyReleaseDate;
        releaseDateItem.value = item.releaseDate;
        [metadata addObject:releaseDateItem];
    } else {
        asset = [[AVURLAsset alloc] initWithURL:url.toNSURL() options:nil];
        baseName = QFileInfo(url.path()).baseName();
    }

    QString outputPath(outputDir.absolutePath() + "/" + baseName + ".m4a");

    if (!QFileInfo::exists(outputPath)) {
        AVAssetExportSession* session = [[AVAssetExportSession alloc]
                initWithAsset:asset
                   presetName:AVAssetExportPresetAppleM4A];

        session.metadata = metadata;
        session.outputFileType = AVFileTypeAppleM4A;
        session.outputURL = [NSURL fileURLWithPath:outputPath.toNSString()];

        dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);

        [session exportAsynchronouslyWithCompletionHandler:^{
            switch (session.status) {
            case AVAssetExportSessionStatusCompleted:
                qInfo() << "Successfully exported asset to" << outputPath;
                break;
            case AVAssetExportSessionStatusFailed:
                qWarning() << "Exporting asset to" << outputPath
                           << "failed:" << [session.error localizedDescription];
                break;
            default:
                qWarning() << "Exporting asset to" << outputPath
                           << "completed with unknown status:"
                           << session.status;
                break;
            }

            dispatch_semaphore_signal(semaphore);
        }];

        dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);
    }

    return outputPath;
}
