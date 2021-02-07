#pragma once

#include <QString>
#include <QByteArray>

// A class representing an image source for a pixmap
// A bundle of a file path, raw data or inline svg
class PixmapSource final {
  public:
    PixmapSource();
    PixmapSource(const QString& filepath);

    bool isEmpty() const;
    bool isSVG() const;
    bool isBitmap() const;
    void setSVG(const QByteArray& content);
    const QString& getPath() const;
    const QByteArray& getSvgSourceData() const;
    QString getId() const;

  private:
    enum Type {
        SVG,
        BITMAP
    };

    QString m_path;
    QByteArray m_svgSourceData;
    enum Type m_eType;
};
