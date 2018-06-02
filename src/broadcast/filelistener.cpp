
#include "broadcast/filelistener.h"

FileListener::FileListener(const QString& path)
        : m_file(path) {
    QFileInfo fileInfo(path);
    qDebug() << "Absolute path " << fileInfo.absoluteFilePath();
    qDebug() << "File exists: " << fileInfo.exists();
    if (!m_file.exists()) {
        m_file.open(QIODevice::WriteOnly);
    }
}

void FileListener::broadcastCurrentTrack(TrackPointer pTrack) {
    QTextStream stream(&m_file);
    m_file.resize(0);
    stream << "Now listening " << pTrack->getTitle();
    stream << " by " << pTrack->getArtist();
    qDebug() << "Text stream status: " << stream.status();
}

void FileListener::scrobbleTrack(TrackPointer pTrack) {
}