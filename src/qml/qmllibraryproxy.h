#pragma once
#include <qqmlintegration.h>

#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <Qt>
#include <memory>

#include "library/dao/directorydao.h"
#include "library/scanner/libraryscanner.h"
#include "library/trackcollectionmanager.h"
#include "qml/qmllibrarysource.h"
#include "qml/qmllibrarytracklistmodel.h"
#include "util/parented_ptr.h"

class Library;
class KeyboardEventFilter;

namespace mixxx {
class LibraryExporter;
namespace qml {

class QmlLibrarySource : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString path MEMBER m_path CONSTANT)
    Q_PROPERTY(uint totalSecond MEMBER m_totalSecond CONSTANT)
    Q_PROPERTY(uint trackCount MEMBER m_trackCount CONSTANT)
    QML_NAMED_ELEMENT(LibrarySource)
  public:
    QmlLibrarySource(const DirectoryDAO::RootDirectoryInfo& record)
            : m_path(record.path),
              m_totalSecond(record.totalSecond),
              m_trackCount(record.trackCount) {
    }

  private:
    QString m_path;
    uint m_totalSecond;
    uint m_trackCount;
};

class QmlLibraryScannerProxy : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool running READ isRunning NOTIFY stateChanged)
    Q_PROPERTY(bool cancelling READ isCancelling NOTIFY stateChanged)
    QML_NAMED_ELEMENT(LibraryScanner)
    QML_UNCREATABLE("Only accessible via Mixxx.Library.scanner")
  public:
    QmlLibraryScannerProxy(
            LibraryScanner* libraryScanner,
            TrackCollectionManager* trackCollectionManager,
            QObject* parent);

    bool isRunning() const {
        return m_running;
    }

    bool isCancelling() const {
        return m_cancelling;
    }

    Q_INVOKABLE void start() {
        m_running = true;
        m_pLibraryScanner->scan();
        emit stateChanged();
    }

    Q_INVOKABLE void cancel() {
        m_cancelling = true;
        emit stateChanged();
        emit requestCancel();
    }

  signals:
    void progress(const QString& path);
    void started();
    void finished();

    void stateChanged();

    void requestStart();
    void requestCancel();

  private:
    LibraryScanner* m_pLibraryScanner;
    bool m_running;
    bool m_cancelling;
};

class QmlLibraryProxy : public QObject {
    Q_OBJECT
    Q_PROPERTY(mixxx::qml::QmlLibraryTrackListModel* model MEMBER m_pModelProperty CONSTANT)
    Q_PROPERTY(QQmlListProperty<mixxx::qml::QmlLibrarySource> sources READ sources CONSTANT)
    Q_PROPERTY(mixxx::qml::QmlLibraryScannerProxy* scanner MEMBER m_pScanner CONSTANT)
    Q_PROPERTY(bool libraryScanActive READ libraryScanActive NOTIFY libraryScanActiveChanged)
    Q_PROPERTY(bool enginePrimeExportAvailable READ enginePrimeExportAvailable CONSTANT)
    QML_NAMED_ELEMENT(Library)
    QML_SINGLETON

  public:
    enum class AddResult {
        Ok,
        AlreadyWatching,
        InvalidOrMissingDirectory,
        UnreadableDirectory,
        SqlError,
    };
    Q_ENUM(AddResult);
    enum class RemoveResult {
        Ok,
        NotFound,
        SqlError,
    };
    Q_ENUM(RemoveResult);
    enum class RelocateResult {
        Ok,
        InvalidOrMissingDirectory,
        UnreadableDirectory,
        SqlError,
    };
    Q_ENUM(RelocateResult);
    enum class SourceRemovalType {
        KeepTracks,
        HideTracks,
        PurgeTracks
    };
    Q_ENUM(SourceRemovalType);

    explicit QmlLibraryProxy(QObject* parent = nullptr);
    ~QmlLibraryProxy() override;

    static QmlLibraryProxy* create(QQmlEngine* pQmlEngine, QJSEngine* pJsEngine);
    static void registerLibrary(std::shared_ptr<Library> pLibrary) {
        s_pLibrary = std::move(pLibrary);
    }

    static Library* get() {
        return s_pLibrary.get();
    }

    QQmlListProperty<QmlLibrarySource> sources() {
        return {this,
                nullptr,
                nullptr,
                &QmlLibraryProxy::sources_count,
                &QmlLibraryProxy::sources_at,
                &QmlLibraryProxy::sources_clear};
    }

    Q_INVOKABLE mixxx::qml::QmlLibraryProxy::AddResult addSource(const QUrl& newPath);
    Q_INVOKABLE mixxx::qml::QmlLibraryProxy::RemoveResult removeSource(
            const QUrl& oldPath,
            mixxx::qml::QmlLibraryProxy::SourceRemovalType type);
    Q_INVOKABLE mixxx::qml::QmlLibraryProxy::RelocateResult relinkSource(
            const QUrl& oldPath, const QUrl& newPath);

    static void registerKeyboardEventFilter(std::shared_ptr<KeyboardEventFilter> pKeyboard) {
        s_pKeyboard = std::move(pKeyboard);
    }

    static KeyboardEventFilter* getKeyboard() {
        return s_pKeyboard.get();
    }

    QmlLibraryTrackListModel* model() const;
    bool libraryScanActive() const {
        return m_pTrackCollectionManager &&
                m_pTrackCollectionManager->isLibraryScanActive();
    }
    bool enginePrimeExportAvailable() const;

    Q_INVOKABLE void analyze(const mixxx::qml::QmlTrackProxy* track) const;
    Q_INVOKABLE void createCrate();
    Q_INVOKABLE void createPlaylist();
    Q_INVOKABLE void exportLibrary();
    Q_INVOKABLE void rescanLibrary();
    Q_INVOKABLE void searchInCurrentView();
    Q_INVOKABLE void searchInTracksLibrary();
    Q_INVOKABLE void showAutoDJ();
    Q_INVOKABLE QString deckHotcueLabel(
            mixxx::qml::QmlTrackProxy* track,
            int hotcueNumber) const;
    Q_INVOKABLE bool setDeckHotcueLabel(
            mixxx::qml::QmlTrackProxy* track,
            int hotcueNumber,
            const QString& label);
    Q_INVOKABLE bool setDeckHotcueType(
            mixxx::qml::QmlTrackProxy* track,
            const QString& group,
            int hotcueNumber,
            const QString& action);
    Q_INVOKABLE void cleanupDeckHotcuePopup(
            mixxx::qml::QmlTrackProxy* track,
            int hotcueNumber);

  signals:
    void libraryScanActiveChanged();
    void libraryScanSummaryAvailable(
            const QString& title,
            const QString& text,
            const QString& informativeText);

  private:
    void deliverPendingLibraryScanSummary();
    static inline std::shared_ptr<Library> s_pLibrary;

    /// This needs to be a plain pointer because it's used as a `Q_PROPERTY` member variable.
    QmlLibraryTrackListModel* m_pModelProperty;
    QmlLibraryScannerProxy* m_pScanner;
    TrackCollectionManager* m_pTrackCollectionManager;

    static qsizetype sources_count(QQmlListProperty<QmlLibrarySource>* property);
    static QmlLibrarySource* sources_at(
            QQmlListProperty<QmlLibrarySource>* property, qsizetype index);
    static void sources_clear(QQmlListProperty<QmlLibrarySource>* property);
    static inline std::shared_ptr<KeyboardEventFilter> s_pKeyboard;
#ifdef __ENGINEPRIME__
    std::unique_ptr<mixxx::LibraryExporter> m_pLibraryExporter;
#endif
};

} // namespace qml
} // namespace mixxx
