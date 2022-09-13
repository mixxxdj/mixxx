#include "library/export/coverartcopyworker.h"

#include <QBuffer>
#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>

#include "library/export/coverartsaver.h"
#include "util/safelywritablefile.h"

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
        case OverwriteAnswer::CANCEL:
            stop();
            return;
        case OverwriteAnswer::OVERWRITE:
            break;
        }

        mixxx::SafelyWritableFile safelyWritableFile(m_coverArtCopyPath, true);
        if (!safelyWritableFile.isReady()) {
            // Change the Feedback because Temp file might exists.
            qDebug()
                    << "Unable to copy cover art into file"
                    << m_coverArtCopyPath
                    << "- Please check file permissions and storage space";
            return;
        }
        qDebug() << "SafelyWritableFile called, there should be temp file in the folder.";
        QThread::msleep(5000);

        CoverArtSaver coverArtSaver(safelyWritableFile.fileName(), m_coverArtImage);
        if (coverArtSaver.saveCoverArt()) {
            qDebug() << "coverArtSaver saved the cover art successfully.";
        } else {
            qDebug() << "Error";
        }
        stop();
    } else {
        CoverArtSaver coverArtSaver(m_coverArtCopyPath, m_coverArtImage);
        if (coverArtSaver.saveCoverArt()) {
            qDebug() << "coverArtSaver saved the cover art successfully.";
        } else {
            qDebug() << "Error";
        }
    }

    qDebug() << "Worker stopped, there should be only the new cover on the folder";
    stop();
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
        m_errorMessage = tr("Updating cover art is canceled.");
        stop();
        break;
    default:;
    }

    return answer;
}

void CoverArtCopyWorker::stop() {
    m_bStop = true;
}
