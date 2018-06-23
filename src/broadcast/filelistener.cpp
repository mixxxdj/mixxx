
#include "broadcast/filelistener.h"

#include <QTextCodec>
#include <QtConcurrentRun>
#include <functional>

#include "moc_filelistener.cpp"
#include "preferences/dialog/dlgprefbroadcast.h"

FileListener::FileListener(UserSettingsPointer pConfig)
        : m_pFile(new QFile, [](QFile* file) -> void {
              file->resize(0);
              delete file;
          }),
          m_COsettingsChanged(kSettingsChanged),
          m_pConfig(pConfig),
          m_latestSettings(MetadataFileSettings::getPersistedSettings(pConfig)) {
    connect(&m_COsettingsChanged, SIGNAL(valueChanged(double)), this, SLOT(slotFileSettingsChanged(double)));
    updateStateFromSettings();
}

void FileListener::slotBroadcastCurrentTrack(TrackPointer pTrack) {
    if (!pTrack)
        return;
    QString writtenString(m_latestSettings.fileFormatString);
    writtenString.replace("author", pTrack->getArtist()).replace("title", pTrack->getTitle()) += '\n';
    m_fileContents = writtenString;
    QTextCodec* codec = QTextCodec::codecForName(m_latestSettings.fileEncoding);
    DEBUG_ASSERT(codec);
    QByteArray* fileContents = new QByteArray;
    *fileContents = codec->fromUnicode(m_fileContents);
    QtConcurrent::run(&FileListener::writeMetadataToFile, fileContents, m_pFile);
}

void FileListener::slotScrobbleTrack(TrackPointer pTrack) {
    Q_UNUSED(pTrack);
}

void FileListener::slotAllTracksPaused() {
    m_pFile->resize(0);
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
    } else {
        m_pFile->setFileName(m_latestSettings.filePath);
        if (m_pFile->exists()) {
            m_pFile->remove();
        }
    }
}

void FileListener::updateFile() {
    if (m_pFile->isOpen()) {
        if (filePathChanged) {
            m_pFile->remove();
            m_pFile->setFileName(m_latestSettings.filePath);
            m_pFile->open(QIODevice::ReadWrite |
                    QIODevice::Truncate |
                    QIODevice::Text |
                    QIODevice::Unbuffered);
        }
        QTextCodec* codec = QTextCodec::codecForName(m_latestSettings.fileEncoding);
        DEBUG_ASSERT(codec);
        QByteArray* fileContents = new QByteArray;
        *fileContents = codec->fromUnicode(m_fileContents);
        QtConcurrent::run(&FileListener::writeMetadataToFile, fileContents, m_pFile);
    } else {
        m_pFile->setFileName(m_latestSettings.filePath);
        m_pFile->open(QIODevice::ReadWrite |
                QIODevice::Truncate |
                QIODevice::Text |
                QIODevice::Unbuffered);
    }
}

void FileListener::writeMetadataToFile(const QByteArray* contents, std::shared_ptr<QFile> file) {
    file->resize(0);
    file->write(*contents);
    delete contents;
}
