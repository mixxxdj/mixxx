#include "library/export/coverartcopyworker.h"

#include <QBuffer>
#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>

void CoverArtCopyWorker::run() {
    copyFile(m_coverArtImage, m_coverArtCopyPath);

    if (m_bStop.loadAcquire()) {
        emit canceled();
        return;
    }
}

void CoverArtCopyWorker::copyFile(
        const QImage& m_coverArtImage,
        const QString& m_coverArtCopyPath) {
    QFileInfo dest_fileinfo(m_coverArtCopyPath);

    if (dest_fileinfo.exists()) {
        switch (makeOverwriteRequest(m_coverArtCopyPath)) {
        case OverwriteAnswer::UPDATE:
        case OverwriteAnswer::CANCEL:
            stop();
            return;
        case OverwriteAnswer::OVERWRITE:
            break;
        }

        QFile dest_file(m_coverArtCopyPath);
        qDebug() << "Removing existing cover art" << m_coverArtCopyPath;
        if (!dest_file.remove()) {
            const QString error_message =
                    tr("Error removing cover art %1: %2. Stopping.")
                            .arg(m_coverArtCopyPath, dest_file.errorString());
            qWarning() << error_message;
            m_errorMessage = error_message;
            stop();
            return;
        }
    }

    QByteArray coverArtByteArray;
    QBuffer bufferCoverArt(&coverArtByteArray);
    bufferCoverArt.open(QIODevice::WriteOnly);
    m_coverArtImage.save(&bufferCoverArt, "JPG");
    qDebug() << "Copying cover art to" << m_coverArtCopyPath;
    QFile coverArtFile(m_coverArtCopyPath);
    coverArtFile.open(QIODevice::WriteOnly);
    if (!coverArtFile.write(coverArtByteArray)) {
        const QString error_message = tr(
                "Error copying cover art to %1: Stopping.")
                                              .arg(m_coverArtCopyPath);
        qWarning() << error_message;
        m_errorMessage = error_message;
        stop();
    }

    bufferCoverArt.close();
    coverArtFile.close();
    stop();
    return;
}

CoverArtCopyWorker::OverwriteAnswer CoverArtCopyWorker::makeOverwriteRequest(
        const QString& filename) {
    QScopedPointer<std::promise<OverwriteAnswer>> mode_promise(
            new std::promise<OverwriteAnswer>());
    std::future<OverwriteAnswer> mode_future = mode_promise->get_future();

    emit askOverwrite(filename, mode_promise.data());

    mode_future.wait();

    if (m_bStop.loadAcquire()) {
        return OverwriteAnswer::CANCEL;
    }

    if (!mode_future.valid()) {
        qWarning() << "CoverArtCopyWorker::makeOverwriteRequest invalid answer from future";
        m_errorMessage = tr("Error copying cover art");
        stop();
        return OverwriteAnswer::CANCEL;
    }

    OverwriteAnswer answer = mode_future.get();
    switch (answer) {
    case OverwriteAnswer::CANCEL:
        m_errorMessage = tr("Copying process was canceled");
        stop();
        break;
    default:;
    }

    return answer;
}

void CoverArtCopyWorker::stop() {
    m_bStop = true;
}
