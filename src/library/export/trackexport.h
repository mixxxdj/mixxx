#ifndef TRACKEXPORT_H
#define TRACKEXPORT_H

#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <future>

#include "library/export/ui_dlgtrackexport.h"

// A class for copying a list of files to a single destination directory.
// Currently does not preserve subdirectory relationships.  This class performs
// all copies in a block style, so it should be spawned in a separate thread.
class TrackExport : public QObject{
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
    TrackExport(QString destDir) : m_destDir(destDir) { }
    virtual ~TrackExport() { };

    // exports ALL the tracks.
    bool exportTrackList(QList<QString> filenames);

    // Calling classes can call errorMessage after a failure for a user-friendly
    // message about what happened.
    QString errorMessage() const {
        return m_errorMessage;
    }

  signals:
    // Signals and slots necessarily make a copy of the items being passed,
    // so we have to use a bare pointer instead of a smart one. And QT's QFuture
    // is not quite what we want here, so we use the STL's future class.
    void askOverwriteMode(QString filename,
                          std::promise<OverwriteAnswer>* promise);
    void progress(int progress, int count);

  private:
    bool exportTrack(QString sourceFilename);

    // Emit a signal requesting overwrite mode, and block until we get an
    // answer.  Updates m_overwriteMode appropriately.
    OverwriteAnswer makeOverwriteRequest(QString filename);

    bool m_bStop = false;
    QString m_errorMessage;

    OverwriteMode m_overwriteMode = OverwriteMode::ASK;
    const QString m_destDir;
};

#endif  // TRACKEXPORT_H
