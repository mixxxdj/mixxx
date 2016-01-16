#include "library/export/trackexport.h"

#include <QFileInfo>
#include <QMessageBox>
#include <QDebug>

void TrackExport::run() {
    int i = 0;
    for (const auto& sourceFilename : m_filenames) {
        QFileInfo fileinfo(sourceFilename);
        emit(progress(fileinfo.fileName(), i, m_filenames.size()));
        exportTrack(sourceFilename);
        if (m_bStop) {
            emit(canceled());
            return;
        }
        ++i;
        emit(progress(fileinfo.fileName(), i, m_filenames.size()));
    }
}

void TrackExport::exportTrack(QString sourceFilename) {
    QFileInfo source_fileinfo(sourceFilename);
    const QString dest_filename =
            m_destDir + "/" + source_fileinfo.fileName();
    QFileInfo dest_fileinfo(dest_filename);

    // Give the user the option to overwrite existing files in the destination.
    if (dest_fileinfo.exists()) {
        switch (m_overwriteMode) {
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
            return;
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
        return;
    }
}

TrackExport::OverwriteAnswer TrackExport::makeOverwriteRequest(QString filename) {
    QScopedPointer<std::promise<OverwriteAnswer>>
            mode_promise(new std::promise<OverwriteAnswer>());
    std::future<OverwriteAnswer> mode_future = mode_promise->get_future();

    emit(askOverwriteMode(filename, mode_promise.data()));

    // Block until the user tells us the answer.
    std::future_status status(std::future_status::timeout);
    while (status != std::future_status::ready && !m_bStop) {
        status = mode_future.wait_for(std::chrono::milliseconds(500));
    }

    // We can be either canceled from the other thread, or as a return value
    // from this call.  First check for a call from the other thread.
    if (m_bStop) {
        return OverwriteAnswer::CANCEL;
    }

    if (!mode_future.valid()) {
        qWarning() << "TrackExport::makeOverwriteRequest invalid answer from future";
        m_errorMessage = tr("Error exporting tracks");
        m_bStop = true;
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

void TrackExport::stop() {
    m_bStop = true;
}
