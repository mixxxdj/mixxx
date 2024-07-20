#include "library/itunes/itunesiosassetexporter.h"

#import <AVFoundation/AVFoundation.h>

#include <QDir>
#include <QString>
#include <QUrl>
#include <QUrlQuery>
#include <QtGlobal>

ITunesIOSAssetExporter::ITunesIOSAssetExporter(const QDir& outputDir)
        : outputDir(outputDir) {
}

QString ITunesIOSAssetExporter::exportAsync(const QUrl& url) {
    AVURLAsset* asset = [[AVURLAsset alloc] initWithURL:url.toNSURL()
                                                options:nil];
    AVAssetExportSession* session = [[AVAssetExportSession alloc]
            initWithAsset:asset
               presetName:AVAssetExportPresetAppleM4A];

    QString name;
    if (url.scheme() == "ipod-library") {
        // TODO: Use a more descriptive name than the ID, e.g. including
        // title/artist?
        QUrlQuery query(url);
        name = query.queryItemValue("id");
    } else {
        name = QFileInfo(url.path()).baseName();
    }

    QString outputPath(outputDir.absolutePath() + "/" + name + ".m4a");

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
