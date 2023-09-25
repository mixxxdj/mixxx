#pragma once

#include <grantlee/template.h>

#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QThread>
#include <future>

#include "track/track.h"
#include "util/fileinfo.h"

namespace Grantlee {
class Context;
class Engine;
} // namespace Grantlee
// A QThread class for copying a list of files to a single destination directory.
// Currently does not preserve subdirectory relationships.  This class performs
// all copies in a blocking style within its own thread.  May be canceled from
// another thread.
class TrackExportWorker : public QThread {
    Q_OBJECT
  public:
    enum ExportResult {
        OK,
        SKIPPED,
        FAILED,
        EXPORT_COMPLETE
    };
    Q_ENUM(ExportResult)

    enum class OverwriteMode {
        ASK,
        OVERWRITE_ALL,
        SKIP_ALL,
    };

    // Class used to answer the question about what the user would prefer to
    // do in case a file of the same name exists at the destination
    enum class OverwriteAnswer {
        SKIP,
        SKIP_ALL,
        OVERWRITE,
        OVERWRITE_ALL,
        CANCEL = -1,
    };

    // Constructor does not validate the destination directory.  Calling classes
    // should do that.
    // pattern will
    TrackExportWorker(const QString& destDir,
            TrackPointerList& tracks,
            Grantlee::Context* context = nullptr);

    virtual ~TrackExportWorker();

    // exports ALL the tracks.  Thread joins on success or failure.
    void run() override;

    bool isRunning() {
        return m_running;
    };
    // Calling classes can call errorMessage after a failure for a user-friendly
    // message about what happened.
    QString errorMessage() const {
        return m_errorMessage;
    }

    void setDestDir(const QString& destDir) {
        m_destDir = destDir;
        updateTemplate();
    }

    /// Sets the filename pattern
    void setPattern(QString* pattern);

    /// returns the current filename pattern
    QString* getPattern() {
        return m_pattern;
    }

    /// Sets the filename for the playlist to generate
    void setPlaylist(const QString& playlist) {
        m_playlist = playlist;
    }
    /// returns the playlist filename
    QString getPlaylist() {
        return m_playlist;
    }

    // Returns the new filename for the track. Applies the pattern if set.
    QString generateFilename(TrackPointer track, int index = 0, int dupCounter = 0);

    /// Applies the filename pattern on track
    QString applyPattern(TrackPointer track, int index, int duplicateCounter = 0);

    // Cancels the export after the current copy operation.
    // May be called from another thread.
    void stop();

  signals:
    // Signals and slots necessarily make a copy of the items being passed,
    // so we have to use a bare pointer instead of a smart one. And QT's QFuture
    // is not quite what we want here, so we use the STL's future class.
    // Note that fully qualifying the Answer class name is required for the
    // signal to connect.
    void askOverwriteMode(
            const QString& filename,
            std::promise<TrackExportWorker::OverwriteAnswer>* promise);
    void progress(const QString& from, const QString& to, int progress, int count);
    void result(TrackExportWorker::ExportResult result, const QString& msg);
    void canceled();

  private:
    // Copies the file at source_fileinfo to the destination directory with the
    // name given by dest_filename (not a full path).  If the destination file
    // exists, will emit an overwrite request signal to ask how to proceed.
    // On unrecoverable error, sets the error message and stops the export
    // process entirely.
    void copyFile(const mixxx::FileInfo& source_fileinfo,
            const QString& dest_filename);
    QMap<QString, mixxx::FileInfo> createCopylist(
            const TrackPointerList& tracks,
            TrackPointerList* skippedTracks);
    void updateTemplate();

    // Emit a signal requesting overwrite mode, and block until we get an
    // answer.  Updates m_overwriteMode appropriately.
    OverwriteAnswer makeOverwriteRequest(const QString& filename);

    QAtomicInt m_bStop = false;
    bool m_running = false;
    QString m_errorMessage;

    OverwriteMode m_overwriteMode = OverwriteMode::ASK;
    QString m_destDir;
    TrackPointerList m_tracks;
    QString* m_pattern;
    Grantlee::Context* m_context;
    Grantlee::Template m_template;
    bool m_template_valid{false};
    Grantlee::Engine* m_engine{nullptr};
    QString m_playlist;
};
