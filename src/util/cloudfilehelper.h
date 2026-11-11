#pragma once

#include <QList>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QtGlobal>
#include <functional>

namespace mixxx {
namespace mac {

class CloudFileHelper {
  public:
    enum class DownloadStatus {
        Downloaded,     // Fully downloaded locally
        Downloading,    // Download is currently in progress
        NotDownloaded,  // Dataless / dehydrated placeholder
        NotUbiquitous   // Standard local file (not managed by cloud)
    };

    using HydrationCallback = std::function<void(
            const QList<QUrl>& hydratedUrls,
            const QStringList& errors)>;

#ifdef Q_OS_MACOS
    /// Checks if a file URL is managed by iCloud Drive / FileProvider.
    static bool isCloudFile(const QUrl& url);

    /// Returns the current cloud download status for the file.
    static DownloadStatus getDownloadStatus(const QUrl& url);

    /// Triggers in-place download via NSFileManager without blocking the UI.
    static bool startDownloading(const QUrl& url, QString* pErrorMessage = nullptr);

    /// Synchronously coordinates and ensures the file is fully hydrated in-place on disk.
    static bool ensureHydratedSync(const QUrl& url, QString* pErrorMessage = nullptr);

    /// Asynchronously ensures a list of URLs are fully hydrated in-place on a background queue,
    /// then calls completionCallback on the Qt main thread.
    static void ensureHydratedAsync(
            const QList<QUrl>& urls,
            HydrationCallback completionCallback);
#else
    // Inline stubs for Linux & Windows: zero-cost pass-through
    static bool isCloudFile(const QUrl& /*url*/) {
        return false;
    }

    static DownloadStatus getDownloadStatus(const QUrl& /*url*/) {
        return DownloadStatus::NotUbiquitous;
    }

    static bool startDownloading(const QUrl& /*url*/, QString* /*pErrorMessage*/ = nullptr) {
        return true;
    }

    static bool ensureHydratedSync(const QUrl& /*url*/, QString* /*pErrorMessage*/ = nullptr) {
        return true;
    }

    static void ensureHydratedAsync(
            const QList<QUrl>& urls,
            HydrationCallback completionCallback) {
        if (completionCallback) {
            completionCallback(urls, {});
        }
    }
#endif
};

} // namespace mac
} // namespace mixxx
