#include "library/itunes/itunesiosassetexporter.h"

#import <AVFoundation/AVFoundation.h>
#import <MediaPlayer/MediaPlayer.h>

#include <QDir>
#include <QString>
#include <QUrl>
#include <QUrlQuery>
#include <QtGlobal>

ITunesIOSAssetExporter::ITunesIOSAssetExporter(const QDir& outputDir)
        : outputDir(outputDir) {
}

QString ITunesIOSAssetExporter::exportAsync(const QUrl& url) {
    AVURLAsset* asset;
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

        // TODO: Use a more descriptive name than the ID, e.g. including
        // title/artist?
        baseName = persistentID;
    } else {
        asset = [[AVURLAsset alloc] initWithURL:url.toNSURL() options:nil];
        baseName = QFileInfo(url.path()).baseName();
    }

    AVAssetExportSession* session = [[AVAssetExportSession alloc]
            initWithAsset:asset
               presetName:AVAssetExportPresetAppleM4A];

    QString outputPath(outputDir.absolutePath() + "/" + baseName + ".m4a");

    session.outputFileType = AVFileTypeAppleM4A;
    session.outputURL = [NSURL fileURLWithPath:outputPath.toNSString()];

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
                       << "completed with unknown status:" << session.status;
            break;
        }
    }];

    return outputPath;
}
