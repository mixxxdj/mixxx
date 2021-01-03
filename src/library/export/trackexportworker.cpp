#include "library/export/trackexportworker.h"

#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>

#include "moc_trackexportworker.cpp"
#include "track/track.h"

namespace {

QString rewriteFilename(const QFileInfo& fileinfo, int index) {
    // We don't have total control over the inputs, so definitely
    // don't use .arg().arg().arg().
    const QString index_str = QString("%1").arg(index, 4, 10, QChar('0'));
    return QString("%1-%2.%3").arg(fileinfo.baseName(), index_str, fileinfo.completeSuffix());
}

// Iterate over a list of tracks and generate a minimal set of files to copy.
// Finds duplicate filenames.  Munges filenames if they refer to different files,
// and skips if they refer to the same disk location.  Returns a map from
// QString (the destination possibly-munged filenames) to QFileinfo (the source
// file information).
QMap<QString, TrackFile> createCopylist(const TrackPointerList& tracks) {
    // QMap is a non-obvious return value, but it's easy for callers to use
    // in practice and is the best object for producing the final list
    // efficiently.
    QMap<QString, TrackFile> copylist;
    for (const auto& it : tracks) {
        if (it->getCanonicalLocation().isEmpty()) {
            qWarning()
                    << "File not found or inaccessible while exporting"
                    << it->getFileInfo();
            // Skip file
            continue;
        }

        // When obtaining the canonical location the file info of the
        // track might have been refreshed. Get it now.
        const auto trackFile = it->getFileInfo();

        const auto fileName = trackFile.fileName();
        auto destFileName = fileName;
        int duplicateCounter = 0;
        do {
            const auto duplicateIter = copylist.find(destFileName);
            if (duplicateIter == copylist.end()) {
                // Usual case -- haven't seen this filename before, so add it.
                copylist[destFileName] = trackFile;
                break;
            }
            if (trackFile.canonicalLocation() == duplicateIter->canonicalLocation()) {
                // Silently ignore and skip duplicate files that point
                // to the same location on disk
                break;
            }
            if (++duplicateCounter >= 10000) {
                qWarning()
                        << "Failed to generate a unique file name from"
                        << fileName
                        << "while exporting"
                        << trackFile.location();
                break;
            }
            // Next round
            destFileName = rewriteFilename(trackFile.asFileInfo(), duplicateCounter);
        } while (!destFileName.isEmpty());
    }
    return copylist;
}

}  // namespace

void TrackExportWorker::run() {
    int i = 0;
    QMap<QString, TrackFile> copy_list = createCopylist(m_tracks);
    for (auto it = copy_list.constBegin(); it != copy_list.constEnd(); ++it) {
        // We emit progress twice per loop, which may seem excessive, but it
        // guarantees that we emit a sane progress before we start and after
        // we end.  In between, each filename will get its own visible tick
        // on the bar, which looks really nice.
        emit progress(it->fileName(), i, copy_list.size());
        copyFile((*it).asFileInfo(), it.key());
        if (atomicLoadAcquire(m_bStop)) {
            emit canceled();
            return;
        }
        ++i;
        emit progress(it->fileName(), i, copy_list.size());
    }
}

void TrackExportWorker::copyFile(const QFileInfo& source_fileinfo,
                                 const QString& dest_filename) {
    QString sourceFilename = source_fileinfo.canonicalFilePath();
    const QString dest_path = QDir(m_destDir).filePath(dest_filename);
    QFileInfo dest_fileinfo(dest_path);

    if (dest_fileinfo.exists()) {
        switch (m_overwriteMode) {
        // Give the user the option to overwrite existing files in the destination.
        case OverwriteMode::ASK:
            switch (makeOverwriteRequest(dest_path)) {
            case OverwriteAnswer::SKIP:
            case OverwriteAnswer::SKIP_ALL:
                qDebug() << "skipping" << sourceFilename;
                return;
            case OverwriteAnswer::OVERWRITE:
            case OverwriteAnswer::OVERWRITE_ALL:
                break;
            case OverwriteAnswer::CANCEL:
                m_errorMessage = tr("Export process was canceled");
                stop();
                return;
            }
            break;
        case OverwriteMode::SKIP_ALL:
            qDebug() << "skipping" << sourceFilename;
            return;
        case OverwriteMode::OVERWRITE_ALL:;
        }

        // Remove the existing file in preparation for overwriting.
        QFile dest_file(dest_path);
        qDebug() << "Removing existing file" << dest_path;
        if (!dest_file.remove()) {
            const QString error_message = tr(
                    "Error removing file %1: %2. Stopping.").arg(
                    dest_path, dest_file.errorString());
            qWarning() << error_message;
            m_errorMessage = error_message;
            stop();
            return;
        }
    }

    qDebug() << "Copying" << sourceFilename << "to" << dest_path;
    QFile source_file(sourceFilename);
    if (!source_file.copy(dest_path)) {
        const QString error_message = tr(
                "Error exporting track %1 to %2: %3. Stopping.").arg(
                sourceFilename, dest_path, source_file.errorString());
        qWarning() << error_message;
        m_errorMessage = error_message;
        stop();
        return;
    }
}

TrackExportWorker::OverwriteAnswer TrackExportWorker::makeOverwriteRequest(
        const QString& filename) {
    // QT's QFuture is not quite right for this type of threaded question-and-answer.
    // std::future works fine, even with signals and slots.
    QScopedPointer<std::promise<OverwriteAnswer>> mode_promise(
            new std::promise<OverwriteAnswer>());
    std::future<OverwriteAnswer> mode_future = mode_promise->get_future();

    emit askOverwriteMode(filename, mode_promise.data());

    // Block until the user tells us the answer.
    mode_future.wait();

    // We can be either canceled from the other thread, or as a return value
    // from this call.  First check for a call from the other thread.
    if (atomicLoadAcquire(m_bStop)) {
        return OverwriteAnswer::CANCEL;
    }

    if (!mode_future.valid()) {
        qWarning() << "TrackExportWorker::makeOverwriteRequest invalid answer from future";
        m_errorMessage = tr("Error exporting tracks");
        stop();
        return OverwriteAnswer::CANCEL;
    }

    OverwriteAnswer answer = mode_future.get();
    switch (answer) {
    case OverwriteAnswer::SKIP_ALL:
        m_overwriteMode = OverwriteMode::SKIP_ALL;
        break;
    case OverwriteAnswer::OVERWRITE_ALL:
        m_overwriteMode = OverwriteMode::OVERWRITE_ALL;
        break;
    case OverwriteAnswer::CANCEL:
        // Handle cancellation as a result of the question.
        m_errorMessage = tr("Export process was canceled");
        stop();
        break;
    default:;
    }

    return answer;
}

void TrackExportWorker::stop() {
    // We'll wait for the current file to finish copying, then stop.
    m_bStop = true;
}
