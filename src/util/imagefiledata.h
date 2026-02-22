#pragma once

#include <QByteArray>
#include <QImage>

class ImageFileData : public QImage {
  public:
    explicit ImageFileData(const QByteArray& coverArtBytes,
            const QByteArray& coverArtFormat = nullptr);

    static ImageFileData fromFilePath(const QString& coverArtFilePath);

    static QByteArray readFormatFrom(const QByteArray& coverArtBytes);

    bool saveFile(const QString& coverArtAbsoluteFilePath) const;
    bool saveFileAddSuffix(const QString& pathWithoutSuffix) const;

    QByteArray getCoverArtBytes() const {
        return m_coverArtBytes;
    }

  private:
    QByteArray m_coverArtBytes;
    QByteArray m_coverArtFormat;
};
