
#include "broadcast/filelistener.h"

#include <QTextCodec>
#include <QtConcurrentRun>

#include "broadcast/filelistener.h"
#include "broadcast/metadatafileworker.h"
#include "moc_filelistener.cpp"
#include "preferences/metadatafilesettings.h"

FileListener::FileListener(UserSettingsPointer pConfig)
        : m_COsettingsChanged(kSettingsChanged),
          m_pConfig(pConfig),
          m_latestSettings(MetadataFileSettings::getPersistedSettings(pConfig)) {
    MetadataFileWorker* newWorker = new MetadataFileWorker(m_latestSettings.filePath);
    newWorker->moveToThread(&m_workerThread);

    connect(&m_workerThread, SIGNAL(finished()), newWorker, SLOT(deleteLater()));

    connect(this, SIGNAL(deleteFile()), newWorker, SLOT(slotDeleteFile()));

    connect(this, SIGNAL(deleteFile()), newWorker, SLOT(slotDeleteFile()));

    connect(this, SIGNAL(moveFile(QString)), newWorker, SLOT(slotMoveFile(QString)));

    connect(this, SIGNAL(writeMetadataToFile(QByteArray)), newWorker, SLOT(slotWriteMetadataToFile(QByteArray)));

    connect(this, SIGNAL(writeMetadataToFile(QByteArray)), newWorker, SLOT(slotWriteMetadataToFile(QByteArray)));

    connect(this, SIGNAL(clearFile()), newWorker, SLOT(slotClearFile()));

    connect(&m_COsettingsChanged, SIGNAL(valueChanged(double)), this, SLOT(slotFileSettingsChanged(double)));

    m_workerThread.start();
}

FileListener::~FileListener() {
    m_workerThread.quit();
    m_workerThread.wait();
}

void FileListener::slotBroadcastCurrentTrack(TrackPointer pTrack) {
    if (!pTrack)
        return;
    m_fileContents.title = pTrack->getTitle();
    m_fileContents.artist = pTrack->getArtist();
    QString writtenString(m_latestSettings.fileFormatString);
    writtenString.replace("$author", pTrack->getArtist()).replace("$title", pTrack->getTitle()) += '\n';
    QTextCodec* codec = QTextCodec::codecForName(m_latestSettings.fileEncoding);
    DEBUG_ASSERT(codec);
    QByteArray fileContents = codec->fromUnicode(writtenString);
    m_tracksPaused = false;
    emit writeMetadataToFile(fileContents);
}

void FileListener::slotScrobbleTrack(TrackPointer pTrack) {
    Q_UNUSED(pTrack);
}

void FileListener::slotAllTracksPaused() {
    m_tracksPaused = true;
    emit clearFile();
}

void FileListener::slotFileSettingsChanged(double value) {
    if (value) {
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
    }
    if (!m_tracksPaused && !m_fileContents.isEmpty()) {
        QTextCodec* codec = QTextCodec::codecForName(m_latestSettings.fileEncoding);
        DEBUG_ASSERT(codec);
        QString newContents(m_latestSettings.fileFormatString);
        newContents.replace("$author", m_fileContents.artist)
                .replace("$title", m_fileContents.title) += '\n';
        QByteArray contentsBinary = codec->fromUnicode(newContents);
        emit writeMetadataToFile(contentsBinary);
    }
}

