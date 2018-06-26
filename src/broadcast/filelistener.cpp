
#include "broadcast/filelistener.h"

#include <QTextCodec>
#include <QtConcurrentRun>

#include "broadcast/filelistener.h"
#include "broadcast/metadatafileworker.h"
#include "preferences/metadatafilesettings.h"
#include "moc_filelistener.cpp"


FileListener::FileListener(UserSettingsPointer pConfig)
        : m_COsettingsChanged(kSettingsChanged),
          m_pConfig(pConfig),
          m_latestSettings(MetadataFileSettings::getPersistedSettings(pConfig)) {

    MetadataFileWorker *newWorker = new MetadataFileWorker(m_latestSettings.filePath);
    newWorker->moveToThread(&m_workerThread);

    connect(&m_workerThread,SIGNAL(finished()),
            newWorker,SLOT(deleteLater()));

    connect(this,SIGNAL(deleteFile()),
            newWorker,SLOT(slotDeleteFile()));

    connect(this,SIGNAL(openFile()),
            newWorker,SLOT(slotOpenFile()));

    connect(this,SIGNAL(moveFile(QString)),
            newWorker,SLOT(slotMoveFile(QString)));

    connect(this,SIGNAL(writeMetadataToFile(QByteArray)),
            newWorker,SLOT(slotWriteMetadataToFile(QByteArray)));

    connect(this,SIGNAL(clearFile()),
            newWorker,SLOT(slotClearFile()));

    connect(&m_COsettingsChanged,SIGNAL(valueChanged(double)),
            this,SLOT(slotFileSettingsChanged(double)));

    updateStateFromSettings();

    m_workerThread.start();
}

FileListener::~FileListener() {
    m_workerThread.quit();
    m_workerThread.wait();
}


void FileListener::slotBroadcastCurrentTrack(TrackPointer pTrack) {
    if (!pTrack)
        return;
    QString writtenString(m_latestSettings.fileFormatString);
    writtenString.replace("author", pTrack->getArtist()).replace("title", pTrack->getTitle()) += '\n';
    m_fileContents = writtenString;
    QTextCodec* codec = QTextCodec::codecForName(m_latestSettings.fileEncoding);
    DEBUG_ASSERT(codec);
    QByteArray fileContents = codec->fromUnicode(m_fileContents);
    tracksPaused = false;
    emit writeMetadataToFile(fileContents);
}

void FileListener::slotScrobbleTrack(TrackPointer pTrack) {
    Q_UNUSED(pTrack);
}

void FileListener::slotAllTracksPaused() {
    tracksPaused = true;
    emit clearFile();
}

void FileListener::slotFileSettingsChanged(double value) {
    if (value) {
        FileSettings latestSettings = MetadataFileSettings::getLatestSettings();
        filePathChanged = latestSettings.filePath != m_latestSettings.filePath;
        m_latestSettings = latestSettings;
        updateStateFromSettings();
    }
}

void FileListener::updateStateFromSettings() {
    if (m_latestSettings.enabled) {
        updateFile();
    }
    else  {
        emit deleteFile();
    }
}

void FileListener::updateFile() {
    if (fileOpen) {
        if (filePathChanged) {
            emit moveFile(m_latestSettings.filePath);
        }
        else if (!tracksPaused) {
            QTextCodec *codec = QTextCodec::codecForName(m_latestSettings.fileEncoding);
            DEBUG_ASSERT(codec);
            QByteArray fileContents = codec->fromUnicode(m_fileContents);
            emit writeMetadataToFile(fileContents);
        }
    }
    else {
        emit openFile();
        fileOpen = true;
    }
}

