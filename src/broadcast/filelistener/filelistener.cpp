#include "broadcast/filelistener/filelistener.h"

#include <QTextCodec>
#include <QtConcurrentRun>

#include "broadcast/filelistener/metadatafileworker.h"
#include "moc_filelistener.cpp"
#include "preferences/metadatafilesettings.h"

FileListener::FileListener(UserSettingsPointer pConfig)
        : m_COsettingsChanged(kFileSettingsChanged),
          m_pConfig(pConfig),
          m_latestSettings(MetadataFileSettings::getPersistedSettings(pConfig)),
          m_filePathChanged(false),
          m_tracksPaused(false) {
    MetadataFileWorker* newWorker = new MetadataFileWorker(m_latestSettings.filePath);
    newWorker->moveToThread(&m_workerThread);

    connect(&m_workerThread, &QThread::finished, newWorker, &MetadataFileWorker::deleteLater);
    connect(&m_COsettingsChanged,
            &ControlPushButton::valueChanged,
            this,
            &FileListener::slotFileSettingsChanged);

    connect(this, &FileListener::deleteFile, newWorker, &MetadataFileWorker::slotDeleteFile);
    connect(this, &FileListener::moveFile, newWorker, &MetadataFileWorker::slotMoveFile);
    connect(this,
            &FileListener::writeMetadataToFile,
            newWorker,
            &MetadataFileWorker::slotWriteMetadataToFile);
    connect(this, &FileListener::clearFile, newWorker, &MetadataFileWorker::slotClearFile);

    m_workerThread.start();
}

FileListener::~FileListener() {
    emit clearFile();
    m_workerThread.quit();
    m_workerThread.wait();
}

void FileListener::slotBroadcastCurrentTrack(TrackPointer pTrack) {
    if (!m_latestSettings.enabled) {
        return;
    }
    if (!pTrack) {
        emit clearFile();
    } else {
        m_fileContents.title = pTrack->getTitle();
        m_fileContents.artist = pTrack->getArtist();
        QString writtenString(m_latestSettings.fileFormatString);
        writtenString.replace("$author", pTrack->getArtist())
                .replace("$title", pTrack->getTitle()) += '\n';
        QTextCodec* codec = QTextCodec::codecForName(m_latestSettings.fileEncoding);
        DEBUG_ASSERT(codec);
        QByteArray fileContents = codec->fromUnicode(writtenString);
        m_tracksPaused = false;
        emit writeMetadataToFile(fileContents);
    }
}

void FileListener::slotScrobbleTrack(TrackPointer pTrack) {
    Q_UNUSED(pTrack);
}

void FileListener::slotAllTracksPaused() {
    if (!m_latestSettings.enabled) {
        return;
    }
    m_tracksPaused = true;
    emit clearFile();
}

void FileListener::slotFileSettingsChanged(double value) {
    if (value >= 0) {
        FileSettings latestSettings = MetadataFileSettings::getLatestSettings();
        m_filePathChanged = latestSettings.filePath != m_latestSettings.filePath;
        m_latestSettings = latestSettings;
        updateStateFromSettings();
    }
}

void FileListener::updateStateFromSettings() {
    if (m_latestSettings.enabled) {
        updateFile();
    } else {
        emit deleteFile();
    }
}

void FileListener::updateFile() {
    if (m_filePathChanged) {
        emit moveFile(m_latestSettings.filePath);
        m_filePathChanged = false;
    }
    if (!m_tracksPaused && !m_fileContents.isEmpty()) {
        QTextCodec* codec = QTextCodec::codecForName(m_latestSettings.fileEncoding);
        if (!codec) {
            qWarning() << "Text codec selected from metadata broadcast settings doesn't exist";
            codec = QTextCodec::codecForName("UTF-8");
        }
        QString newContents(m_latestSettings.fileFormatString);
        newContents.replace("$author", m_fileContents.artist)
                .replace("$title", m_fileContents.title) += '\n';
        QByteArray contentsBinary = codec->fromUnicode(newContents);
        emit writeMetadataToFile(contentsBinary);
    }
}
