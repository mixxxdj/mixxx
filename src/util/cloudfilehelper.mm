#include "util/cloudfilehelper.h"

#ifdef Q_OS_MACOS

#import <Foundation/Foundation.h>

#include "util/logger.h"

namespace {
mixxx::Logger kLogger("CloudFileHelper");

NSURL* toNSURL(const QUrl& url) {
    if (url.isLocalFile()) {
        return [NSURL fileURLWithPath:url.toLocalFile().toNSString()];
    }
    return [NSURL URLWithString:url.toString().toNSString()];
}
} // namespace

namespace mixxx {
namespace mac {

bool CloudFileHelper::isCloudFile(const QUrl& url) {
    @autoreleasepool {
        NSURL* nsUrl = toNSURL(url);
        if (!nsUrl) {
            return false;
        }

        NSNumber* isUbiquitous = nil;
        NSError* error = nil;
        if ([nsUrl getResourceValue:&isUbiquitous
                             forKey:NSURLIsUbiquitousItemKey
                              error:&error] && isUbiquitous) {
            return [isUbiquitous boolValue];
        }

        // Fallback check via NSFileManager
        return [[NSFileManager defaultManager] isUbiquitousItemAtURL:nsUrl];
    }
}

CloudFileHelper::DownloadStatus CloudFileHelper::getDownloadStatus(const QUrl& url) {
    @autoreleasepool {
        NSURL* nsUrl = toNSURL(url);
        if (!nsUrl) {
            return DownloadStatus::NotUbiquitous;
        }

        NSNumber* isUbiquitous = nil;
        if (![nsUrl getResourceValue:&isUbiquitous forKey:NSURLIsUbiquitousItemKey error:nil] ||
                ![isUbiquitous boolValue]) {
            return DownloadStatus::NotUbiquitous;
        }

        NSString* downloadStatus = nil;
        if ([nsUrl getResourceValue:&downloadStatus
                             forKey:NSURLUbiquitousItemDownloadingStatusKey
                              error:nil]) {
            if ([downloadStatus isEqualToString:NSURLUbiquitousItemDownloadingStatusCurrent] ||
                    [downloadStatus isEqualToString:NSURLUbiquitousItemDownloadingStatusDownloaded]) {
                return DownloadStatus::Downloaded;
            } else if ([downloadStatus isEqualToString:NSURLUbiquitousItemDownloadingStatusNotDownloaded]) {
                return DownloadStatus::NotDownloaded;
            }
        }

        NSNumber* isDownloading = nil;
        if ([nsUrl getResourceValue:&isDownloading
                             forKey:NSURLUbiquitousItemIsDownloadingKey
                              error:nil] && [isDownloading boolValue]) {
            return DownloadStatus::Downloading;
        }

        return DownloadStatus::NotDownloaded;
    }
}

bool CloudFileHelper::startDownloading(const QUrl& url, QString* pErrorMessage) {
    @autoreleasepool {
        NSURL* nsUrl = toNSURL(url);
        if (!nsUrl) {
            if (pErrorMessage) {
                *pErrorMessage = QStringLiteral("Invalid URL");
            }
            return false;
        }

        NSError* error = nil;
        BOOL success = [[NSFileManager defaultManager] startDownloadingUbiquitousItemAtURL:nsUrl
                                                                                     error:&error];
        if (!success) {
            if (pErrorMessage && error) {
                *pErrorMessage = QString::fromCFString((__bridge CFStringRef)error.localizedDescription);
            }
            kLogger.warning() << "Failed to trigger download for" << url.toString()
                              << ":" << (error ? error.localizedDescription.UTF8String : "unknown error");
            return false;
        }

        kLogger.info() << "Triggered in-place download for cloud item:" << url.toLocalFile();
        return true;
    }
}

bool CloudFileHelper::ensureHydratedSync(const QUrl& url, QString* pErrorMessage) {
    @autoreleasepool {
        NSURL* nsUrl = toNSURL(url);
        if (!nsUrl) {
            if (pErrorMessage) {
                *pErrorMessage = QStringLiteral("Invalid file URL");
            }
            return false;
        }

        if (!isCloudFile(url)) {
            return true;
        }

        NSFileCoordinator* coordinator = [[NSFileCoordinator alloc] initWithFilePresenter:nil];
        NSError* coordError = nil;
        __block BOOL coordinatedSuccess = NO;

        [coordinator coordinateReadingItemAtURL:nsUrl
                                        options:0
                                          error:&coordError
                                     byAccessor:^(NSURL* newURL) {
            if (newURL) {
                coordinatedSuccess = YES;
            }
        }];

        if (coordError || !coordinatedSuccess) {
            if (pErrorMessage && coordError) {
                *pErrorMessage = QString::fromCFString((__bridge CFStringRef)coordError.localizedDescription);
            }
            kLogger.warning() << "Failed to coordinate hydration for" << url.toString()
                              << ":" << (coordError ? coordError.localizedDescription.UTF8String : "access failed");
            return false;
        }

        kLogger.info() << "File successfully hydrated in-place at canonical path:" << url.toLocalFile();
        return true;
    }
}

void CloudFileHelper::ensureHydratedAsync(
        const QList<QUrl>& urls,
        HydrationCallback completionCallback) {
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        QList<QUrl> readyUrls;
        QStringList errors;

        for (const QUrl& url : urls) {
            QString errorMsg;
            if (ensureHydratedSync(url, &errorMsg)) {
                readyUrls.append(url);
            } else {
                errors.append(QStringLiteral("%1: %2").arg(url.toLocalFile(), errorMsg));
            }
        }

        dispatch_async(dispatch_get_main_queue(), ^{
            if (completionCallback) {
                completionCallback(readyUrls, errors);
            }
        });
    });
}

} // namespace mac
} // namespace mixxx

#endif // Q_OS_MACOS
