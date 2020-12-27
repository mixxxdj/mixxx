#pragma once

#include <QFile>
#include <QObject>

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
