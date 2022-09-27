#include "library/export/coverartcopyworker.h"

#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>

#include "util/imagefiledata.h"
#include "util/safelywritablefile.h"

void CoverArtCopyWorker::run() {
    m_isCoverArtUpdated = false;
    copyFile(m_coverArtImage, m_coverArtAbsolutePath);
}

void CoverArtCopyWorker::copyFile(
        const ImageFileData& m_coverArtImage,
        const QString& m_coverArtAbsolutePath) {
    QFileInfo coverArtPathFileInfo(m_coverArtAbsolutePath);

    if (coverArtPathFileInfo.exists()) {
        switch (makeOverwriteRequest(m_coverArtAbsolutePath)) {
        case OverwriteAnswer::CANCEL:
            return;
        case OverwriteAnswer::OVERWRITE:
            break;
        }

        mixxx::SafelyWritableFile safelyWritableFile(m_coverArtAbsolutePath,
                mixxx::SafelyWritableFile::SafetyMode::REPLACE);

        DEBUG_ASSERT(!safelyWritableFile.fileName().isEmpty());
        if (m_coverArtImage.saveFile(safelyWritableFile.fileName())) {
            m_isCoverArtUpdated = true;
            qDebug() << "Cover art"
                     << m_coverArtAbsolutePath
                     << "copied successfully";
            safelyWritableFile.commit();
        } else {
            qWarning() << "Error while copying the cover art to" << safelyWritableFile.fileName();
        }
    } else {
        if (m_coverArtImage.saveFile(m_coverArtAbsolutePath)) {
            m_isCoverArtUpdated = true;
            qDebug() << "Cover art"
                     << m_coverArtAbsolutePath
                     << "copied successfully";
        } else {
            qWarning() << "Error while copying the cover art to" << m_coverArtAbsolutePath;
        }
    }
}

CoverArtCopyWorker::OverwriteAnswer CoverArtCopyWorker::makeOverwriteRequest(
        const QString& filename) {
    QScopedPointer<std::promise<OverwriteAnswer>> mode_promise(
            new std::promise<OverwriteAnswer>());
    std::future<OverwriteAnswer> mode_future = mode_promise->get_future();

    askOverWrite(filename, mode_promise.data());

    mode_future.wait();

    if (!mode_future.valid()) {
        qWarning() << "CoverArtCopyWorker::askOverWrite invalid answer from future";
        return OverwriteAnswer::CANCEL;
    }

    OverwriteAnswer answer = mode_future.get();
    switch (answer) {
    case OverwriteAnswer::CANCEL:
        qDebug() << "Cover art overwrite declined";
        break;
    default:;
    }

    return answer;
}

void CoverArtCopyWorker::askOverWrite(const QString& coverArtAbsolutePath,
        std::promise<CoverArtCopyWorker::OverwriteAnswer>* promise) {
    QFileInfo coverArtInfo(coverArtAbsolutePath);
    QString coverArtName = coverArtInfo.completeBaseName();
    QString coverArtFolder = coverArtInfo.absolutePath();
    QMessageBox overwrite_box(
            QMessageBox::Warning,
            tr("Cover Art File Already Exists"),
            tr("File: %1\n"
               "Folder: %2\n"
               "Override existing file?\n"
               "This can not be undone!")
                    .arg(coverArtName, coverArtFolder));
    overwrite_box.addButton(QMessageBox::Yes);
    overwrite_box.addButton(QMessageBox::No);

    switch (overwrite_box.exec()) {
    case QMessageBox::No:
        m_isCoverArtUpdated = false;
        promise->set_value(CoverArtCopyWorker::OverwriteAnswer::CANCEL);
        return;
    case QMessageBox::Yes:
        promise->set_value(CoverArtCopyWorker::OverwriteAnswer::OVERWRITE);
        return;
    }
}
