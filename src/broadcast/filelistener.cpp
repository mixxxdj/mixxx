
#include "broadcast/filelistener.h"

FileListener::FileListener(const QString& path)
        : m_file(path) {
    QFileInfo fileInfo(path);
    qDebug() << "Absolute path " << fileInfo.absoluteFilePath();
    qDebug() << "File exists: " << fileInfo.exists();
    m_file.open(QIODevice::WriteOnly |
                QIODevice::Text |
                QIODevice::Unbuffered);
    
}

void FileListener::broadcastCurrentTrack(TrackPointer pTrack) {
    if (!pTrack)
        return;
    QTextStream stream(&m_file);
    m_file.resize(0);
    stream << "Now listening " << pTrack->getTitle();
    stream << " by " << pTrack->getArtist();
}

void FileListener::scrobbleTrack(TrackPointer pTrack) {
}