#include "util/imagefiledata.h"

#include <QBuffer>
#include <QFile>
#include <QFileInfo>
#include <QImageReader>

#include "util/assert.h"

ImageFileData::ImageFileData(
        const QByteArray& coverArtBytes, const QByteArray& coverArtFormat)
        : QImage(QImage::fromData(coverArtBytes)),
          m_coverArtBytes(coverArtBytes) {
    m_coverArtFormat = coverArtFormat.isEmpty() ? readFormatFrom(m_coverArtBytes) : coverArtFormat;
}

//static
ImageFileData ImageFileData::fromFilePath(const QString& coverArtFilePath) {
    QFile coverArtFile = QFile(coverArtFilePath);
    if (!coverArtFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open cover art file";
        return ImageFileData(QByteArray{}, QByteArray{});
    }
    QByteArray coverArtBytes = coverArtFile.readAll();
    QByteArray coverArtFormat = readFormatFrom(coverArtBytes);
    return ImageFileData(coverArtBytes, coverArtFormat);
}

//static
QByteArray ImageFileData::readFormatFrom(const QByteArray& coverArtBytes) {
    QByteArray bytes = coverArtBytes;
    QBuffer b = QBuffer(&bytes);
    if (!b.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open buffer";
        return QByteArray{};
    }
    auto reader = QImageReader(&b);
    QByteArray coverArtFormat = reader.format();
    VERIFY_OR_DEBUG_ASSERT(!coverArtFormat.isEmpty()) {
        return QByteArray{};
    }
    return coverArtFormat;
}

bool ImageFileData::saveFile(const QString& coverArtAbsoluteFilePath) const {
    if (m_coverArtBytes.isEmpty()) {
        return save(coverArtAbsoluteFilePath);
    }

    if (QFileInfo(coverArtAbsoluteFilePath).suffix() != m_coverArtFormat) {
        qWarning() << "Cover Art format and extension does not match!"
                   << "Cover art format:"
                   << m_coverArtFormat
                   << "File extension:"
                   << QFileInfo(coverArtAbsoluteFilePath).suffix();
        return save(coverArtAbsoluteFilePath);
    }

    QFile coverArtFile(coverArtAbsoluteFilePath);
    if (!coverArtFile.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open cover art file"
                   << coverArtAbsoluteFilePath;
        return false;
    }
    if (!coverArtFile.write(m_coverArtBytes)) {
        qWarning() << "Failed to write cover art"
                   << coverArtAbsoluteFilePath;
        return false;
    }

    return true;
}

bool ImageFileData::saveFileAddSuffix(const QString& pathWithoutSuffix) const {
    VERIFY_OR_DEBUG_ASSERT(!m_coverArtBytes.isEmpty() || !m_coverArtFormat.isEmpty()) {
        return false;
    }
    QFile coverArtFile(pathWithoutSuffix + '.' + m_coverArtFormat);
    if (!coverArtFile.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open cover art file"
                   << pathWithoutSuffix;
        return false;
    }
    if (!coverArtFile.write(m_coverArtBytes)) {
        qWarning() << "Failed to write cover art"
                   << pathWithoutSuffix;
        return false;
    }
    return true;
}
