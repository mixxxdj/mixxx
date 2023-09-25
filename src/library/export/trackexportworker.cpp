#include "library/export/trackexportworker.h"

#include <grantlee/context.h>
#include <grantlee/engine.h>
#include <grantlee/template.h>

#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>
#include <QRegularExpression>
#include <QString>
#include <QVariant>

#include "library/parser.h"
#include "moc_trackexportworker.cpp"
#include "track/track.h"
#include "util/fileutils.h"
#include "util/formatter.h"

namespace {

const auto kBadFileCharacters = QRegularExpression(
        QStringLiteral("\n"), QRegularExpression::MultilineOption);
const auto kResultSkipped = QStringLiteral("skipped");
const auto kResultCanceled = QStringLiteral("Export canceled");
const auto kResultEmptyPattern = QStringLiteral("empty pattern result, skipped");
const auto kResultOk = QStringLiteral("ok");
const auto kResultCantCreateDirectory = QStringLiteral("Could not create folder");
const auto kResultCantCreateFile = QStringLiteral("Could not create file");
const auto kDefaultPattern = QStringLiteral(
        "{{track.baseName}}{% if dup %}-{{dup|zeropad:\"4\"}}{% endif %}"
        ".{{track.extension}}");
const auto kEmptyMsg = QLatin1String("");
} // namespace

TrackExportWorker::TrackExportWorker(const QString& destDir,
        TrackPointerList& tracks,
        Grantlee::Context* context)
        : m_running(false),
          m_destDir(destDir),
          m_tracks(tracks),
          m_context(context) {
    qRegisterMetaType<TrackExportWorker::ExportResult>("TrackExportWorker::ExportResult");
    if (!m_context) {
        m_context = new Grantlee::Context();
    }
}

TrackExportWorker::~TrackExportWorker() {
    delete m_context;
}

// Iterate over a list of tracks and generate a minimal set of files to copy.
// Finds duplicate filenames.  Munges filenames if they refer to different files,
// and skips if they refer to the same disk location.  Returns a map from
// QString (the destination possibly-munged filenames) to QFileInfo (the source
// file information).
// Tracks for which a empty filename was generated will be added to the skippedTracks
// list.
QMap<QString, mixxx::FileInfo> TrackExportWorker::createCopylist(const TrackPointerList& tracks,
        TrackPointerList* skippedTracks) {
    // QMap is a non-obvious return value, but it's easy for callers to use
    // in practice and is the best object for producing the final list
    // efficiently.
    QMap<QString, mixxx::FileInfo> copylist;
    int index = 0;
    for (const auto& pTrack : tracks) {
        index++;
        if (!pTrack.get()) {
            qWarning() << "nullptr in tracklist";
            continue;
        }
        auto fileInfo = pTrack->getFileInfo();
        if (fileInfo.resolveCanonicalLocation().isEmpty()) {
            qWarning()
                    << "File not found or inaccessible while exporting"
                    << fileInfo;
            // Skip file
            continue;
        }

        const auto fileName = fileInfo.fileName();
        QString destFileName = generateFilename(pTrack, index, 0);
        if (destFileName.isEmpty()) {
            // qWarning() << "pattern generated empty filename for:" << it;
            skippedTracks->append(pTrack);
            continue;
        }
        int duplicateCounter = 0;
        do {
            const auto duplicateIter = copylist.find(destFileName);
            if (duplicateIter == copylist.end()) {
                // Usual case -- haven't seen this filename before, so add it.
                copylist[destFileName] = fileInfo;
                break;
            }
            if (fileInfo.canonicalLocation() == duplicateIter->canonicalLocation()) {
                // Silently ignore and skip duplicate files that point
                // to the same location on disk
                break;
            }
            if (++duplicateCounter >= 10000) {
                qWarning()
                        << "Failed to generate a unique file name from"
                        << fileName
                        << "while exporting"
                        << fileInfo.location();
                break;
            }
            // Next round
            destFileName = generateFilename(pTrack, index, duplicateCounter);
        } while (!destFileName.isEmpty());
    }
    return copylist;
}

void TrackExportWorker::setPattern(QString* pattern) {
    if (pattern == nullptr) {
        m_pattern = nullptr;
        if (!m_template.isNull()) {
            m_template_valid = false;
            m_template.reset();
        }
        return;
    }
    if (!m_engine) {
        m_engine = Formatter::getEngine(this);
        //  smartTrimEnabled would be good, but causes crashes on invalid pattern '{{ }}'
        // m_engine->setSmartTrimEnabled(true);
    }
    m_pattern = pattern;
    updateTemplate();
}

void TrackExportWorker::updateTemplate() {
    QString fullPattern;
    if (m_pattern) {
        QString trimmed = m_pattern->trimmed();
        fullPattern = m_destDir + QDir::separator().toLatin1() + trimmed;
    } else {
        fullPattern = m_destDir + QDir::separator().toLatin1() + kDefaultPattern;
    }
    m_template = m_engine->newTemplate(fullPattern, QStringLiteral("export"));
    if (m_template->error()) {
        m_errorMessage = m_template->errorString();
        m_template_valid = false;
    } else {
        m_errorMessage = QString();
        m_template_valid = true;
    }
}

void TrackExportWorker::run() {
    m_running = true;
    m_bStop = false;
    m_overwriteMode = OverwriteMode::ASK;
    int i = 0;
    auto skippedTracks = TrackPointerList();
    QMap<QString, mixxx::FileInfo> copy_list = createCopylist(m_tracks, &skippedTracks);
    int jobsTotal = copy_list.size();

    if (!m_playlist.isEmpty()) {
        jobsTotal++;
    }

    for (const TrackPointer& track : qAsConst(skippedTracks)) {
        QString fileName = track->fileName();
        emit progress(fileName, nullptr, 0, jobsTotal);
        emit result(TrackExportWorker::ExportResult::SKIPPED, kResultEmptyPattern);
    }

    for (auto it = copy_list.constBegin(); it != copy_list.constEnd(); ++it) {
        // We emit progress twice per loop, which may seem excessive, but it
        // guarantees that we emit a sane progress before we start and after
        // we end.  In between, each filename will get its own visible tick
        // on the bar, which looks really nice.
        QString fileName = it->fileName();
        QString target = it.key();
        emit progress(fileName, target, i, jobsTotal);
        copyFile((*it), target);
        if (m_bStop.loadAcquire()) {
            emit canceled();
            m_running = false;
            return;
        }
        ++i;
    }
    if (!m_playlist.isEmpty()) {
        const auto targetDir = QDir(m_destDir);
        const QString plsPath = targetDir.filePath(FileUtils::escapeFileName(m_playlist));
        QFileInfo plsPathFileinfo(plsPath);

        emit progress(QStringLiteral("export playlist"), plsPath, i, jobsTotal);

        QDir plsDir = plsPathFileinfo.absoluteDir();
        if (!plsDir.mkpath(plsDir.absolutePath())) {
            emit result(TrackExportWorker::ExportResult::FAILED, kResultCantCreateDirectory);
        } else {
            QStringList playlistItemLocations = QStringList();
            for (auto it = copy_list.constBegin(); it != copy_list.constEnd(); ++it) {
                // calculate the relative path between the exported file and the playlist
                QString relpath = plsPathFileinfo.dir().relativeFilePath(
                        targetDir.filePath(it.key()));
                playlistItemLocations.append(relpath);
            }
            bool playlistOk = Parser::exportPlaylistItemsIntoFile(
                    plsPath,
                    playlistItemLocations,
                    true);
            if (playlistOk) {
                emit result(TrackExportWorker::ExportResult::OK, kResultOk);
            } else {
                emit result(TrackExportWorker::ExportResult::FAILED,
                        kResultCantCreateDirectory);
            }
        }
        i++;
    }

    emit progress(kEmptyMsg, kEmptyMsg, i, jobsTotal);
    emit result(TrackExportWorker::ExportResult::EXPORT_COMPLETE, kResultOk);
    m_running = false;
}

// Returns the new filename for the track. Applies the pattern if set.
QString TrackExportWorker::generateFilename(TrackPointer track, int index, int dupCounter) {
    return FileUtils::safeFilename(applyPattern(track, index, dupCounter).trimmed());
}

// Applies the pattern on track
QString TrackExportWorker::applyPattern(
        TrackPointer track,
        int index,
        int duplicateCounter) {
    if (!m_template_valid) {
        return QString();
    }
    VERIFY_OR_DEBUG_ASSERT(!m_destDir.isEmpty()) {
        qWarning() << "empty target directory";
        return QString();
    }
    VERIFY_OR_DEBUG_ASSERT(m_engine) {
        qWarning() << "engine missing";
        return QString();
    }

    // fill the context with the proper variables.
    m_context->push();
    m_context->insert(QStringLiteral("directory"), m_destDir);
    // this is safe since the context stack is popped after rendering
    m_context->insert(QStringLiteral("track"), track.get());
    m_context->insert(QStringLiteral("index"), QVariant(index));
    m_context->insert(QStringLiteral("dup"), QVariant(duplicateCounter));

    QString newName = Formatter::renderFilenameEscape(m_template, *m_context);

    // remove the context stack so it is clean again
    m_context->pop();

    // replace bad filename characters with spaces
    return newName.trimmed();
}

void TrackExportWorker::copyFile(
        const mixxx::FileInfo& source_fileinfo,
        const QString& dest_filename) {
    QString sourceFilename = source_fileinfo.canonicalLocation();
    const QString dest_path = QDir(m_destDir).filePath(dest_filename);
    QFileInfo dest_fileinfo(dest_path);

    QDir destDir = dest_fileinfo.absoluteDir();
    if (!destDir.mkpath(destDir.absolutePath())) {
        emit result(TrackExportWorker::ExportResult::FAILED, kResultCantCreateDirectory);
    }

    if (dest_fileinfo.exists()) {
        switch (m_overwriteMode) {
        // Give the user the option to overwrite existing files in the destination.
        case OverwriteMode::ASK:
            switch (makeOverwriteRequest(dest_path)) {
            case OverwriteAnswer::SKIP:
            case OverwriteAnswer::SKIP_ALL:
                qDebug() << "skipping" << sourceFilename;
                emit result(TrackExportWorker::ExportResult::SKIPPED, kResultSkipped);
                return;
            case OverwriteAnswer::OVERWRITE:
            case OverwriteAnswer::OVERWRITE_ALL:
                break;
            case OverwriteAnswer::CANCEL:
                emit result(TrackExportWorker::ExportResult::EXPORT_COMPLETE, kResultCanceled);
                stop();
                return;
            }
            break;
        case OverwriteMode::SKIP_ALL:
            qDebug() << "skipping" << sourceFilename;
            emit result(TrackExportWorker::ExportResult::SKIPPED, kResultSkipped);
            return;
        case OverwriteMode::OVERWRITE_ALL:;
        }

        // Remove the existing file in preparation for overwriting.
        QFile dest_file(dest_path);
        qDebug() << "Removing existing file" << dest_path;
        if (!dest_file.remove()) {
            const QString error_message = tr(
                    "Error removing file %1: %2")
                                                  .arg(dest_path, dest_file.errorString());
            qWarning() << error_message;
            auto msg = QString("Failed: %1").arg(error_message);
            emit result(TrackExportWorker::ExportResult::FAILED, msg);
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
        auto msg = QString("Failed: %1").arg(error_message);
        emit result(TrackExportWorker::ExportResult::FAILED, msg);
        return;
    }
    emit result(TrackExportWorker::ExportResult::OK, kResultOk);
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
    if (m_bStop.loadAcquire()) {
        return OverwriteAnswer::CANCEL;
    }

    if (!mode_future.valid()) {
        qWarning() << "TrackExportWorker::makeOverwriteRequest invalid answer from future";
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
