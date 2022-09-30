#include "library/export/coverartcopyworker.h"

#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>

#include "util/fileaccess.h"
#include "util/imagefiledata.h"
#include "util/safelywritablefile.h"

void CoverArtCopyWorker::run() {
    // Create a security token for the file.
    auto selectedCoverFileAccess = mixxx::FileAccess(mixxx::FileInfo(m_selectedCoverArtFilePath));

    ImageFileData imageFileData = ImageFileData::fromFilePath(m_selectedCoverArtFilePath);
    if (imageFileData.isNull()) {
        // TODO(rryan): feedback
        return;
    }

    m_coverInfo.type = CoverInfo::FILE;
    m_coverInfo.source = CoverInfo::USER_SELECTED;
    m_coverInfo.coverLocation = m_selectedCoverArtFilePath;
    m_coverInfo.setImage(imageFileData);

    if (QFileInfo(m_oldCoverArtFilePath).canonicalPath() ==
            selectedCoverFileAccess.info().canonicalLocationPath()) {
        qDebug() << "Track and selected cover art are in the same path:"
                 << selectedCoverFileAccess.info().canonicalLocationPath()
                 << "Cover art updated without copying";
        emit coverArtUpdated(m_coverInfo);
        return;
    }

    copyFile(m_selectedCoverArtFilePath, m_oldCoverArtFilePath);
}

void CoverArtCopyWorker::copyFile(
        const QString& m_selectedCoverArtFilePath,
        const QString& m_oldCoverArtFilePath) {
    QFileInfo coverArtPathFileInfo(m_oldCoverArtFilePath);
    ImageFileData imageFileData = ImageFileData::fromFilePath(m_selectedCoverArtFilePath);
    QString errorMessage = tr("Error while copying the cover art to: %1")
                                   .arg(m_oldCoverArtFilePath);
    if (coverArtPathFileInfo.exists()) {
        switch (makeOverwriteRequest(m_oldCoverArtFilePath)) {
        case OverwriteAnswer::Cancel:
            return;
        case OverwriteAnswer::Overwrite:
            break;
        }

        mixxx::SafelyWritableFile safelyWritableFile(m_oldCoverArtFilePath,
                mixxx::SafelyWritableFile::SafetyMode::Replace);

        DEBUG_ASSERT(!safelyWritableFile.fileName().isEmpty());
        if (imageFileData.saveFile(safelyWritableFile.fileName())) {
            qDebug() << "Cover art"
                     << m_oldCoverArtFilePath
                     << "copied successfully";
            safelyWritableFile.commit();
        } else {
            emit coverArtCopyFailed(errorMessage);
            return;
        }
    } else {
        if (imageFileData.saveFile(m_oldCoverArtFilePath)) {
            qDebug() << "Cover art"
                     << m_oldCoverArtFilePath
                     << "copied successfully";
        } else {
            emit coverArtCopyFailed(errorMessage);
            return;
        }
    }
    emit coverArtUpdated(m_coverInfo);
}

CoverArtCopyWorker::OverwriteAnswer CoverArtCopyWorker::makeOverwriteRequest(
        const QString& filename) {
    QScopedPointer<std::promise<OverwriteAnswer>> mode_promise(
            new std::promise<OverwriteAnswer>());
    std::future<OverwriteAnswer> mode_future = mode_promise->get_future();
    emit askOverwrite(filename, mode_promise.data());

    mode_future.wait();

    if (!mode_future.valid()) {
        qWarning() << "CoverArtCopyWorker::askOverwrite invalid answer from future";
        return OverwriteAnswer::Cancel;
    }

    OverwriteAnswer answer = mode_future.get();
    switch (answer) {
    case OverwriteAnswer::Cancel:
        qDebug() << "Cover art overwrite declined";
        break;
    default:;
    }

    return answer;
}
