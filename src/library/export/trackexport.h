#ifndef TRACKEXPORT_H
#define TRACKEXPORT_H

#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QThread>
#include <future>

#include "library/export/ui_dlgtrackexport.h"

// A QThread class for copying a list of files to a single destination directory.
// Currently does not preserve subdirectory relationships.  This class performs
// all copies in a blocking style within its own thread.
class TrackExport : public QThread {
    Q_OBJECT
  public:
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
    TrackExport(QString destDir, QList<QString> filenames)
            : m_destDir(destDir),
              m_filenames(filenames) { }
    virtual ~TrackExport() { };

    // exports ALL the tracks.  Thread joins on success or failure.
    void run() override;

    // Calling classes can call errorMessage after a failure for a user-friendly
    // message about what happened.
    QString errorMessage() const {
        return m_errorMessage;
    }

    // Cancels the export after the current copy operation.
    //May be called from another thread.
    void stop();

  signals:
    // Signals and slots necessarily make a copy of the items being passed,
    // so we have to use a bare pointer instead of a smart one. And QT's QFuture
    // is not quite what we want here, so we use the STL's future class.
    // Note that fully qualifying the Answer class name is required for the
    // signal to connect.
    void askOverwriteMode(QString filename,
                          std::promise<TrackExport::OverwriteAnswer>* promise);
    void progress(QString filename, int progress, int count);
    void canceled();

  private:
    void exportTrack(QString sourceFilename);

    // Emit a signal requesting overwrite mode, and block until we get an
    // answer.  Updates m_overwriteMode appropriately.
    OverwriteAnswer makeOverwriteRequest(QString filename);

    bool m_bStop = false;
    QString m_errorMessage;

    OverwriteMode m_overwriteMode = OverwriteMode::ASK;
    const QString m_destDir;
    const QList<QString> m_filenames;
};

#endif  // TRACKEXPORT_H
