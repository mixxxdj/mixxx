#include "util/cloudfilehelper.h"

#ifdef Q_OS_MACOS

#import <Foundation/Foundation.h>

#include <QCoreApplication>
#include <QMetaObject>
#include <QThreadPool>

#include "util/logger.h"

namespace {
mixxx::Logger kLogger("CloudFileHelper");

NSURL* toNSURL(const QUrl& url) {
    if (url.isEmpty()) {
        return nil;
    }
    QString localPath = url.toLocalFile();
    if (localPath.isEmpty()) {
        const QString str = url.toString();
        if (str.startsWith(QLatin1String("file://"))) {
            localPath = str.mid(7);
        } else if (str.startsWith(QLatin1Char('/'))) {
            localPath = str;
        } else if (url.scheme().isEmpty()) {
            localPath = url.path().isEmpty() ? str : url.path();
        }
    }

    if (!localPath.isEmpty()) {
        return [NSURL fileURLWithPath:localPath.toNSString()];
    }

    return [NSURL URLWithString:url.toString().toNSString()];
}
} // namespace

namespace mixxx {
namespace mac {

bool CloudFileHelper::isCloudFile(const QUrl& url) {
    @autoreleasepool {
        NSURL* nsUrl = toNSURL(url);
        if (!nsUrl || !nsUrl.scheme) {
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
        if (!nsUrl || !nsUrl.scheme) {
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
        if (!nsUrl || !nsUrl.scheme) {
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

        kLogger.debug() << "Triggered in-place download for cloud item:" << url.toLocalFile();
        return true;
    }
}

bool CloudFileHelper::ensureHydratedSync(const QUrl& url, QString* pErrorMessage) {
    @autoreleasepool {
        NSURL* nsUrl = toNSURL(url);
        if (!nsUrl || !nsUrl.scheme) {
            if (pErrorMessage) {
                *pErrorMessage = QStringLiteral("Invalid file URL");
            }
            return false;
        }

        if (!isCloudFile(url)) {
            return true;
        }

        // Safety guard: Never block the Main GUI thread!
        if ([NSThread isMainThread]) {
            return startDownloading(url, pErrorMessage);
        }

        kLogger.debug() << "Coordinating in-place hydration for:"
                        << (nsUrl.path ? nsUrl.path.UTF8String : url.toLocalFile().toUtf8().constData());

        // Ensure background downloading daemon is active for this item
        [[NSFileManager defaultManager] startDownloadingUbiquitousItemAtURL:nsUrl error:nil];

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

        kLogger.debug() << "File successfully hydrated in-place at canonical path:"
                        << (nsUrl.path ? nsUrl.path.UTF8String : url.toLocalFile().toUtf8().constData());
        return true;
    }
}

void CloudFileHelper::ensureHydratedAsync(
        const QList<QUrl>& urls,
        HydrationCallback completionCallback) {
    kLogger.debug() << "Asynchronously hydrating" << urls.size() << "URL(s)...";

    QThreadPool::globalInstance()->start([urls, cb = std::move(completionCallback)]() {
        QList<QUrl> readyUrls;
        QStringList errors;

        for (const QUrl& url : urls) {
            QString errorMsg;
            if (ensureHydratedSync(url, &errorMsg)) {
                readyUrls.append(url);
            } else {
                QString localPath = url.toLocalFile();
                if (localPath.isEmpty()) {
                    localPath = url.toString();
                }
                errors.append(QStringLiteral("%1: %2").arg(localPath, errorMsg));
            }
        }

        QMetaObject::invokeMethod(
                qApp,
                [cb = std::move(cb),
                 readyUrls = std::move(readyUrls),
                 errors = std::move(errors)]() {
                    kLogger.debug() << "Hydration completed. Ready:" << readyUrls.size()
                                    << "Errors:" << errors.size();
                    if (cb) {
                        cb(readyUrls, errors);
                    }
                },
                Qt::QueuedConnection);
    });
}

} // namespace mac
} // namespace mixxx

#endif // Q_OS_MACOS
