#include "library/export/trackexport.h"

#include <QFileInfo>
#include <QMessageBox>
#include <QDebug>

bool TrackExport::exportTrackList(QList<QString> filenames) {
    int i = 0;
    for (const auto& sourceFilename : filenames) {
        if (!exportTrack(sourceFilename)) {
            // bail on error
            return false;
        }
        ++i;
        emit(progress(i, filenames.size()));
    }
    return true;
}

bool TrackExport::exportTrack(QString sourceFilename) {
    QFileInfo source_fileinfo(sourceFilename);
    const QString dest_filename =
            m_destDir + "/" + source_fileinfo.fileName();
    QFileInfo dest_fileinfo(dest_filename);

    // Give the user the option to overwrite existing files in the destination.
    if (dest_fileinfo.exists()) {
        if (m_overwriteMode == OverwriteMode::ASK) {
            switch (makeOverwriteRequest(dest_filename)) {
            case OverwriteAnswer::SKIP:
            case OverwriteAnswer::SKIP_ALL:
            case OverwriteAnswer::CANCEL:
                return true;
            case OverwriteAnswer::OVERWRITE:
            case OverwriteAnswer::OVERWRITE_ALL:
                break;
            }
        }

        // Remove the existing file.
        QFile dest_file(dest_filename);
        qDebug() << "Removing existing file" << dest_filename;
        if (!dest_file.remove()) {
            const QString error_message =
                tr("Error removing file %1: %2. Stopping.").arg(
                dest_filename, dest_file.errorString());
            qWarning() << error_message;
            // set some error message
            m_errorMessage = error_message;
            m_bStop = true;
            return false;
        }
    }

    qDebug() << "Copying" << sourceFilename << "to" << dest_filename;
    QFile source_file(sourceFilename);
    if (!source_file.copy(dest_filename)) {
        const QString error_message =
                tr("Error exporting track %1 to %2: %3. Stopping.").arg(
                sourceFilename, dest_filename, source_file.errorString());
        qWarning() << error_message;
        m_errorMessage = error_message;
        m_bStop = true;
        return false;
    }
    return true;
}

TrackExport::OverwriteAnswer TrackExport::makeOverwriteRequest(QString filename) {
    QScopedPointer<std::promise<OverwriteAnswer>> mode_promise;
    std::future<OverwriteAnswer> mode_future = mode_promise->get_future();
    emit(askOverwriteMode(filename, mode_promise.data()));

    // Block until the user tells us the answer.
    mode_future.wait();

    if (!mode_future.valid()) {
        qWarning() << "TrackExport::makeOverwriteRequest invalid answer from future";
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
        m_bStop = true;
        break;
    default:;
    }

    return answer;
}
