
#include "metadatafileworker.h"

#include "moc_metadatafileworker.cpp"

MetadataFileWorker::MetadataFileWorker(const QString& filePath)
        : m_file(filePath) {
}

void MetadataFileWorker::slotDeleteFile() {
    m_file.remove();
}

void MetadataFileWorker::slotOpenFile() {
    m_file.open(QIODevice::ReadWrite |
            QIODevice::Truncate |
            QIODevice::Text |
            QIODevice::Unbuffered);
}

void MetadataFileWorker::slotMoveFile(QString destination) {
    m_file.remove();
    m_file.setFileName(destination);
    slotOpenFile();
}

void MetadataFileWorker::slotWriteMetadataToFile(QByteArray fileContents) {
    slotClearFile();
    m_file.write(fileContents);
}

void MetadataFileWorker::slotClearFile() {
    m_file.resize(0);
}
