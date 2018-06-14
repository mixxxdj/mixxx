
#include "broadcast/filelistener.h"

#include "moc_filelistener.cpp"

FileListener::FileListener(UserSettingsPointer pConfig)
        : m_filePathChanged(getFileModifiedControlKey()),
          m_pConfig(pConfig) {
    QString filePath = pConfig->getValue(getFilePathConfigKey(),
            "./NowPlaying.txt");
    QObject::connect(&m_filePathChanged, SIGNAL(valueChanged(double)), this, SLOT(slotFilePathChanged(double)));
    m_file.setFileName(filePath);
    m_file.open(QIODevice::ReadWrite |
            QIODevice::Truncate |
            QIODevice::Text |
            QIODevice::Unbuffered);
}

FileListener::~FileListener() {
    m_file.resize(0);
}

void FileListener::slotBroadcastCurrentTrack(TrackPointer pTrack) {
    if (!pTrack)
        return;
    QTextStream stream(&m_file);
    //Clear file
    m_file.resize(0);
    writeMetadata(stream, pTrack);
}

void FileListener::slotScrobbleTrack(TrackPointer pTrack) {
    Q_UNUSED(pTrack);
}

void FileListener::slotAllTracksPaused() {
    m_file.resize(0);
}

std::unique_ptr<FileListener>
FileListener::makeFileListener(FileListenerType type,
        UserSettingsPointer pConfig) {
    switch (type) {
    case FileListenerType::SAMBroadcaster:
        return std::unique_ptr<FileListener>(new SAMFileListener(pConfig));
    }
    return {};
}

void FileListener::slotFilePathChanged(double value) {
    if (value > 0.0) {
        QString newPath = m_pConfig->getValueString(getFilePathConfigKey());
        if (newPath.size() == 0) {
            qDebug() << "Received value changed from nowPlaying.txt control object"
                        " yet QString is empty";
            return;
        }
        if (!m_file.seek(0)) {
            qDebug() << "Couldn't seek start of NowPlaying.txt file";
            return;
        }
        QByteArray fileContents = m_file.readAll();
        m_file.remove();
        m_file.setFileName(newPath);
        m_file.open(QIODevice::ReadWrite |
                QIODevice::Truncate |
                QIODevice::Text |
                QIODevice::Unbuffered);
        m_file.write(fileContents);
    }
}

ConfigKey FileListener::getFileModifiedControlKey() {
    return ConfigKey("[Livemetadata]", "nowPlayingFilePathChanged");
}

ConfigKey FileListener::getFilePathConfigKey() {
    return ConfigKey("[Livemetadata]", "NowPlayingFilePath");
}

SAMFileListener::SAMFileListener(UserSettingsPointer pConfig)
        : FileListener(pConfig) {
}

void SAMFileListener::writeMetadata(QTextStream& stream, TrackPointer pTrack) {
    stream << pTrack->getArtist() << " - " << pTrack->getTitle();
}
