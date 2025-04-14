#include "broadcast/filelistener/metadatafileworker.h"

#include "moc_metadatafileworker.cpp"

MetadataFileWorker::MetadataFileWorker(const QString& filePath)
        : m_file(filePath) {
}

void MetadataFileWorker::slotDeleteFile() {
    m_file.remove();
}

void MetadataFileWorker::slotMoveFile(const QString& destination) {
    m_file.remove();
    m_file.setFileName(destination);
}

void MetadataFileWorker::slotWriteMetadataToFile(const QByteArray& fileContents) {
    m_file.open(QIODevice::WriteOnly |
            QIODevice::Text |
            QIODevice::Unbuffered);
    m_file.write(fileContents);
    m_file.close();
}

void MetadataFileWorker::slotClearFile() {
    m_file.resize(0);
}
