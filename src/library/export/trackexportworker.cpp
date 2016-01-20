#include "library/export/trackexportworker.h"
#include "util/compatibility.h"

#include <QFileInfo>
#include <QMessageBox>
#include <QDebug>

void TrackExportWorker::run() {
    int i = 0;
    for (const auto& track : m_tracks) {
        auto fileinfo = track->getFileInfo();
        emit(progress(fileinfo.fileName(), i, m_tracks.size()));
        exportTrack(track);
        if (load_atomic(m_bStop)) {
            emit(canceled());
            return;
        }
        ++i;
        emit(progress(fileinfo.fileName(), i, m_tracks.size()));
    }
}

void TrackExportWorker::exportTrack(TrackPointer track) {
    QString sourceFilename = track->getLocation();
    auto source_fileinfo = track->getFileInfo();
    const QString dest_filename =
            QDir(m_destDir).filePath(source_fileinfo.fileName());
    QFileInfo dest_fileinfo(dest_filename);

    if (dest_fileinfo.exists()) {
        switch (m_overwriteMode) {
        // Give the user the option to overwrite existing files in the destination.
        case OverwriteMode::ASK:
            switch (makeOverwriteRequest(dest_filename)) {
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
        QFile dest_file(dest_filename);
        qDebug() << "Removing existing file" << dest_filename;
        if (!dest_file.remove()) {
            const QString error_message = tr(
                    "Error removing file %1: %2. Stopping.").arg(
                    dest_filename, dest_file.errorString());
            qWarning() << error_message;
            m_errorMessage = error_message;
            stop();
            return;
        }
    }

    qDebug() << "Copying" << sourceFilename << "to" << dest_filename;
    QFile source_file(sourceFilename);
    if (!source_file.copy(dest_filename)) {
        const QString error_message = tr(
                "Error exporting track %1 to %2: %3. Stopping.").arg(
                sourceFilename, dest_filename, source_file.errorString());
        qWarning() << error_message;
        m_errorMessage = error_message;
        stop();
        return;
    }
}

TrackExportWorker::OverwriteAnswer TrackExportWorker::makeOverwriteRequest(
        QString filename) {
    // QT's QFuture is not quite right for this type of threaded question-and-answer.
    // std::future works fine, even with signals and slots.
    QScopedPointer<std::promise<OverwriteAnswer>> mode_promise(
            new std::promise<OverwriteAnswer>());
    std::future<OverwriteAnswer> mode_future = mode_promise->get_future();

    emit(askOverwriteMode(filename, mode_promise.data()));

    // Block until the user tells us the answer.
    mode_future.wait();

    // We can be either canceled from the other thread, or as a return value
    // from this call.  First check for a call from the other thread.
    if (load_atomic(m_bStop)) {
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
        // Handle cancelation as a result of the question.
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
