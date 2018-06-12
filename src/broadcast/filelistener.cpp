
#include "broadcast/filelistener.h"

FileListener::FileListener(const QString& path)
        : m_file(path) {
    m_file.open(QIODevice::WriteOnly |
            QIODevice::Text |
            QIODevice::Unbuffered);
}

FileListener::~FileListener() {
    m_file.resize(0);
}

void FileListener::broadcastCurrentTrack(TrackPointer pTrack) {
    if (!pTrack)
        return;
    QTextStream stream(&m_file);
    //Clear file
    m_file.resize(0);
    writeMetadata(stream, pTrack);
}

void FileListener::scrobbleTrack(TrackPointer pTrack) {
    Q_UNUSED(pTrack);
}

void FileListener::allTracksPaused() {
    m_file.resize(0);
}

std::unique_ptr<FileListener>
FileListener::makeFileListener(FileListenerType type,
        const QString& path) {
    switch (type) {
    case FileListenerType::SAMBroadcaster:
        return std::unique_ptr<FileListener>(new SAMFileListener(path));
    default:
        qWarning() << "Unrecognised FileListenerType";
        return std::unique_ptr<FileListener>();
    };
}

SAMFileListener::SAMFileListener(const QString& path)
        : FileListener(path) {
}

void SAMFileListener::writeMetadata(QTextStream& stream, TrackPointer pTrack) {
    stream << pTrack->getArtist() << " - " << pTrack->getTitle();
}
