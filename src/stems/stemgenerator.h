#pragma once

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <memory>

#include "track/trackid.h"
#include "track/track.h"
#include "track/trackref.h"
#include "preferences/usersettings.h"
#include "stems/stemqueuemodel.h"

#if defined(MIXXX_USE_QML)
#include <QtQml/qqmlregistration.h>

// Only used as pointers here, so the full headers stay out of this file;
// stemgenerator.cpp includes them where they are actually needed.
class QJSEngine;
class QQmlEngine;
#endif

namespace mixxx {
class CoreServices;
} // namespace mixxx

class QAbstractItemModel;

/// Generates stem files for tracks, so they can be played as stems in Mixxx.
///
/// The heavy lifting is done by an external worker (``res/stems/stems_worker.py``,
/// ONNX Runtime based) that Mixxx runs as a child process and watches through a
/// tiny line protocol. Doing the inference in a child process keeps a multi
/// second long burst of work out of the audio thread and out of the application
/// binary, and lets the user pick the interpreter that has the model runtime
/// installed.
///
/// Jobs are queued, run one at a time (the GPU or the CPU are saturated anyway)
/// and, by default, are postponed while a deck is playing so the separation
/// never competes with playback for the GPU.
class StemGenerator : public QObject {
    Q_OBJECT
#if defined(MIXXX_USE_QML)
    QML_NAMED_ELEMENT(StemGenerator)
    QML_SINGLETON
#endif
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QString pythonPath READ pythonPath WRITE setPythonPath NOTIFY pythonPathChanged)
    Q_PROPERTY(QString ffmpegPath READ ffmpegPath WRITE setFfmpegPath NOTIFY ffmpegPathChanged)
    Q_PROPERTY(QString cacheDirectory READ cacheDirectory WRITE setCacheDirectory NOTIFY
                    cacheDirectoryChanged)
    Q_PROPERTY(bool writeNextToTrack READ writeNextToTrack WRITE setWriteNextToTrack NOTIFY
                    writeNextToTrackChanged)
    Q_PROPERTY(QString codec READ codec WRITE setCodec NOTIFY codecChanged)
    Q_PROPERTY(QString model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(QString provider READ provider WRITE setProvider NOTIFY providerChanged)
    Q_PROPERTY(bool pauseDuringPlayback READ pauseDuringPlayback WRITE setPauseDuringPlayback
                    NOTIFY pauseDuringPlaybackChanged)

    Q_PROPERTY(bool probing READ probing NOTIFY probeChanged)
    Q_PROPERTY(bool ready READ ready NOTIFY probeChanged)
    Q_PROPERTY(bool gpuAvailable READ gpuAvailable NOTIFY probeChanged)
    Q_PROPERTY(QString gpuName READ gpuName NOTIFY probeChanged)
    Q_PROPERTY(QString detectedProvider READ detectedProvider NOTIFY probeChanged)
    Q_PROPERTY(QStringList probeErrors READ probeErrors NOTIFY probeChanged)
    Q_PROPERTY(double estimatedFactor READ estimatedFactor NOTIFY probeChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool waitingForPlayback READ waitingForPlayback NOTIFY waitingForPlaybackChanged)
    Q_PROPERTY(int pendingCount READ pendingCount NOTIFY queueChanged)
    Q_PROPERTY(QString queueStatus READ queueStatus NOTIFY queueChanged)
    Q_PROPERTY(QAbstractItemModel* queue READ queue CONSTANT)

  public:
    explicit StemGenerator(UserSettingsPointer pConfig, QObject* pParent = nullptr);
    ~StemGenerator() override;

    void registerDependencies(mixxx::CoreServices* pCoreServices);
    /// Exposes the instance to QML; called once during startup.
    static void registerInstance(StemGenerator* pInstance);
    static StemGenerator* instance() {
        return s_pInstance;
    }
#if defined(MIXXX_USE_QML)
    static StemGenerator* create(QQmlEngine* pQmlEngine, QJSEngine* pJsEngine);
#endif

    bool enabled() const;
    void setEnabled(bool enabled);
    QString pythonPath() const;
    void setPythonPath(const QString& path);
    QString ffmpegPath() const;
    void setFfmpegPath(const QString& path);
    QString cacheDirectory() const;
    void setCacheDirectory(const QString& path);
    bool writeNextToTrack() const;
    void setWriteNextToTrack(bool writeNextToTrack);
    QString codec() const;
    void setCodec(const QString& codec);
    QString model() const;
    void setModel(const QString& model);
    QString provider() const;
    void setProvider(const QString& provider);
    bool pauseDuringPlayback() const;
    void setPauseDuringPlayback(bool pause);

    bool probing() const {
        return m_probing;
    }
    bool ready() const {
        return m_ready;
    }
    bool gpuAvailable() const {
        return m_gpuAvailable;
    }
    QString gpuName() const {
        return m_gpuName;
    }
    QString detectedProvider() const {
        return m_detectedProvider;
    }
    QStringList probeErrors() const {
        return m_probeErrors;
    }
    double estimatedFactor() const;
    bool busy() const;
    bool waitingForPlayback() const {
        return m_waitingForPlayback;
    }
    int pendingCount() const;
    QString queueStatus() const;
    QAbstractItemModel* queue() {
        return &m_queue;
    }

    /// Runs the worker with ``probe`` and updates the capability properties.
    Q_INVOKABLE void probe();
    /// Adds the given library tracks to the queue. With ``regenerate`` tracks
    /// that already have a stem file are queued again, overwriting it.
    void enqueueTracks(const QList<TrackId>& trackIds, bool regenerate = false);
    /// True when a stem file already exists for this track.
    Q_INVOKABLE bool hasStems(const QString& trackPath) const;
    /// The library track holding the stems of the given track: the sibling
    /// ".stem.mp4" when there is one, otherwise the track itself. Loading the
    /// stem file instead of the plain track is what lets a deck remix the
    /// track stem by stem without picking the stem file in the library.
    TrackPointer stemTrackFor(const TrackPointer& pTrack) const;
    /// Stem file that would be used for this track, empty when there is none.
    Q_INVOKABLE QString stemFileFor(const QString& trackPath) const;
    Q_INVOKABLE void cancel();
    Q_INVOKABLE void clearFinished();
    Q_INVOKABLE void openCacheFolder() const;
    Q_INVOKABLE void openTrackFolder(const QString& trackPath) const;
    Q_INVOKABLE void rescanLibrary();

  signals:
    void enabledChanged();
    void pythonPathChanged();
    void ffmpegPathChanged();
    void cacheDirectoryChanged();
    void writeNextToTrackChanged();
    void codecChanged();
    void modelChanged();
    void providerChanged();
    void pauseDuringPlaybackChanged();
    void probeChanged();
    void busyChanged();
    void waitingForPlaybackChanged();
    void queueChanged();

  private slots:
    void slotWorkerOutput();
    void slotWorkerFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void slotStartNextJob();

  private:
    void startNextJob();
    void failCurrentJob(const QString& error);
    QString resolvedPython() const;
    QString resolvedWorkerScript() const;
    QString outputPathForSource(const QString& sourcePath) const;
    QString cachePathForSource(const QString& sourcePath) const;
    QStringList workerEnvironmentValue() const;
    bool isAnyDeckPlaying() const;
    static QString displayTitleForTrack(const QString& trackPath);

    UserSettingsPointer m_pConfig;
    mixxx::CoreServices* m_pCoreServices{nullptr};
    StemQueueModel m_queue;
    QProcess m_worker;
    QTimer m_startTimer;

    int m_currentRow{-1};
    bool m_probing{false};
    bool m_ready{false};
    bool m_gpuAvailable{false};
    bool m_waitingForPlayback{false};
    QString m_gpuName;
    QString m_detectedProvider;
    QStringList m_probeErrors;
    QString m_outputBuffer;
    QString m_queueStatus;

    static inline StemGenerator* s_pInstance{nullptr};
};
