#pragma once

#include <QFile>
#include <QObject>

/// The object, created by this class is intended to run in an extra thread.
/// This allows non blocking file access.
class MetadataFileWorker : public QObject {
    Q_OBJECT
  public:
    explicit MetadataFileWorker(const QString& filePath);
  public slots:
    void slotDeleteFile();
    void slotMoveFile(const QString& destination);
    void slotWriteMetadataToFile(const QByteArray& fileContents);
    void slotClearFile();

  private:
    QFile m_file;
};
