#include "library/export/trackexportworker.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>

#include "moc_trackexportworker.cpp"
#include "track/track.h"
#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("TrackExportWorker");

QString rewriteFilename(const mixxx::FileInfo& fileinfo, int index) {
    const QString index_str = QString("%1").arg(index, 4, 10, QChar('0'));
    return QString("%1-%2.%3").arg(fileinfo.baseName(), index_str, fileinfo.completeSuffix());
}

QMap<QString, mixxx::FileInfo> createCopylist(
        const TrackPointerList& tracks,
        QStringList& failedTracks) {
    QMap<QString, mixxx::FileInfo> copylist;
    for (const auto& pTrack : tracks) {
        VERIFY_OR_DEBUG_ASSERT(pTrack != nullptr) {
            continue;
        }
        auto fileInfo = pTrack->getFileInfo();
        if (fileInfo.resolveCanonicalLocation().isEmpty()) {
            kLogger.warning()
                    << "File not found or inaccessible while exporting"
                    << fileInfo;
            const QString location = fileInfo.location();
            failedTracks.append(location.isEmpty() ? pTrack->getTitle() : location);
            continue;
        }

        const auto fileName = fileInfo.fileName();
        auto destFileName = fileName;
        int duplicateCounter = 0;
        do {
            const auto duplicateIter = copylist.find(destFileName);
            if (duplicateIter == copylist.end()) {
                copylist[destFileName] = fileInfo;
                break;
            }
            if (fileInfo.canonicalLocation() == duplicateIter->canonicalLocation()) {
                break;
            }
            if (++duplicateCounter >= 10000) {
                kLogger.warning()
                        << "Failed to generate a unique file name from"
                        << fileName
                        << "while exporting"
                        << fileInfo.location();
                failedTracks.append(fileInfo.location());
                break;
            }
            destFileName = rewriteFilename(fileInfo, duplicateCounter);
        } while (!destFileName.isEmpty());
    }
    return copylist;
}

} // namespace

void TrackExportWorker::run() {
    m_failedTracks.clear();
    m_errorMessage.clear();

    QMap<QString, mixxx::FileInfo> copylist = createCopylist(m_tracks, m_failedTracks);

    int totalCount = copylist.size() + m_failedTracks.size();
    int currentCount = m_failedTracks.size();

    emit progress(QString(), currentCount, totalCount);

    for (auto it = copylist.constBegin(); it != copylist.constEnd(); ++it) {
        if (m_bStop) {
            emit canceled();
            return;
        }

        copyFile(it.value(), it.key());

        if (m_bStop) {
            return;
        }

        currentCount++;
        emit progress(it.key(), currentCount, totalCount);
    }
}

void TrackExportWorker::copyFile(
        const mixxx::FileInfo& source_fileinfo,
        const QString& dest_filename) {
    QString sourceFilename = source_fileinfo.canonicalLocation();
    const QString dest_path = QDir(m_destDir).filePath(dest_filename);
    QFileInfo dest_fileinfo(dest_path);

    if (dest_fileinfo.exists()) {
        switch (m_overwriteMode) {
        case OverwriteMode::ASK:
            switch (makeOverwriteRequest(dest_path)) {
            case OverwriteAnswer::SKIP:
            case OverwriteAnswer::SKIP_ALL:
                kLogger.debug() << "skipping" << sourceFilename;
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
            kLogger.debug() << "skipping" << sourceFilename;
            return;
        case OverwriteMode::OVERWRITE_ALL:;
        }

        QFile dest_file(dest_path);
        kLogger.debug() << "removing existing file" << dest_path;
        if (!dest_file.remove()) {
            const QString error_message = tr(
                    "Error removing file %1: %2. Stopping.")
                                                  .arg(
                                                          dest_path, dest_file.errorString());
            kLogger.warning() << error_message;
            m_failedTracks.append(sourceFilename);
            m_errorMessage = error_message;
            stop();
            return;
        }
    }

    kLogger.debug() << "copying" << sourceFilename << "to" << dest_path;
    QFile source_file(sourceFilename);
    if (!source_file.copy(dest_path)) {
        const QString error_message = tr(
                "Error exporting track %1 to %2: %3. Stopping.")
                                              .arg(
                                                      sourceFilename, dest_path, source_file.errorString());
        kLogger.warning() << error_message;
        m_failedTracks.append(sourceFilename);
        m_errorMessage = error_message;
        stop();
        return;
    }
}

TrackExportWorker::OverwriteAnswer TrackExportWorker::makeOverwriteRequest(
        const QString& filename) {
    QScopedPointer<std::promise<OverwriteAnswer>> mode_promise(
            new std::promise<OverwriteAnswer>());
    std::future<OverwriteAnswer> mode_future = mode_promise->get_future();

    emit askOverwriteMode(filename, mode_promise.data());

    mode_future.wait();

    if (m_bStop.loadAcquire()) {
        return OverwriteAnswer::CANCEL;
    }

    if (!mode_future.valid()) {
        kLogger.warning() << "invalid answer from future";
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
        m_errorMessage = tr("Error exporting tracks");
        stop();
        break;
    default:;
    }

    return answer;
}

void TrackExportWorker::stop() {
    m_bStop = true;
}