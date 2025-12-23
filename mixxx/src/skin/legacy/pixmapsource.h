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
    const QString& getPath() const;

  private:
    enum class Type {
        SVG,
        BITMAP
    };

    QString m_path;
    Type m_eType;
};
