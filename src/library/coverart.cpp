#include <QtDebug>

#include "library/coverart.h"
#include "library/coverartutils.h"
#include "util/debug.h"

namespace {

QString sourceToString(CoverInfo::Source source) {
    switch (source) {
        case CoverInfo::UNKNOWN:
            return "UNKNOWN";
        case CoverInfo::GUESSED:
            return "GUESSED";
        case CoverInfo::USER_SELECTED:
            return "USER_SELECTED";
    }
    return "INVALID INFO VALUE";
}

QString typeToString(CoverInfo::Type type) {
    switch (type) {
        case CoverInfo::NONE:
            return "NONE";
        case CoverInfo::METADATA:
            return "METADATA";
        case CoverInfo::FILE:
            return "FILE";
    }
    return "INVALID TYPE VALUE";
}

QString coverInfoRelativeToString(const CoverInfoRelative& infoRelative) {
    return typeToString(infoRelative.type) % "," %
           sourceToString(infoRelative.source) % "," %
           infoRelative.coverLocation % "," %
           "0x" % QString::number(infoRelative.hash, 16);
}

QString coverInfoToString(const CoverInfo& info) {
    return coverInfoRelativeToString(info) % "," %
           info.trackLocation % ",";
}
} // anonymous namespace

//static
quint16 CoverImageUtils::calculateHash(
        const QImage& image) {
    const auto hash = qChecksum(
            reinterpret_cast<const char*>(image.constBits()),
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
            image.sizeInBytes()
#else
            image.byteCount()
#endif
            );
    DEBUG_ASSERT(image.isNull() || isValidHash(hash));
    DEBUG_ASSERT(!image.isNull() || hash == defaultHash());
    return hash;
}

CoverInfoRelative::CoverInfoRelative()
        : source(UNKNOWN),
          type(NONE),
          hash(CoverImageUtils::defaultHash()) {
}

bool operator==(const CoverInfoRelative& a, const CoverInfoRelative& b) {
    return a.source == b.source &&
            a.type == b.type &&
            a.hash == b.hash &&
            a.coverLocation == b.coverLocation;
}

bool operator!=(const CoverInfoRelative& a, const CoverInfoRelative& b) {
    return !(a == b);
}

QDebug operator<<(QDebug dbg, const CoverInfoRelative& infoRelative) {
    return dbg.maybeSpace() << QString("CoverInfoRelative(%1)")
            .arg(coverInfoRelativeToString(infoRelative));
}

QImage CoverInfo::loadImage(
        const SecurityTokenPointer& pTrackLocationToken) const {
    if (type == CoverInfo::METADATA) {
        if (trackLocation.isEmpty()) {
            qDebug() << "CoverArtUtils::loadCover METADATA cover with empty trackLocation.";
            return QImage();
        }
        return CoverArtUtils::extractEmbeddedCover(
                TrackFile(trackLocation),
                pTrackLocationToken);
    } else if (type == CoverInfo::FILE) {
        if (trackLocation.isEmpty()) {
            qDebug() << "CoverArtUtils::loadCover FILE cover with empty trackLocation."
                     << "Relative paths will not work.";
            SecurityTokenPointer pToken =
                    Sandbox::openSecurityToken(
                            QFileInfo(coverLocation),
                            true);
            return QImage(coverLocation);
        }

        const QFileInfo coverFile(
                TrackFile(trackLocation).directory(),
                coverLocation);
        const QString coverFilePath = coverFile.filePath();
        if (!coverFile.exists()) {
            qDebug() << "CoverArtUtils::loadCover FILE cover does not exist:"
                     << coverFilePath;
            return QImage();
        }
        SecurityTokenPointer pToken =
                Sandbox::openSecurityToken(coverFile, true);
        return QImage(coverFilePath);
    } else if (type == CoverInfo::NONE) {
        return QImage();
    } else {
        qDebug() << "CoverArtUtils::loadCover unhandled type";
        DEBUG_ASSERT(false);
        return QImage();
    }
}

bool operator==(const CoverInfo& a, const CoverInfo& b) {
    return static_cast<const CoverInfoRelative&>(a) ==
                    static_cast<const CoverInfoRelative&>(b) &&
            a.trackLocation == b.trackLocation;
}

bool operator!=(const CoverInfo& a, const CoverInfo& b) {
    return !(a == b);
}

QDebug operator<<(QDebug dbg, const CoverInfo& info) {
    return dbg.maybeSpace() << QString("CoverInfo(%1)")
            .arg(coverInfoToString(info));
}

QDebug operator<<(QDebug dbg, const CoverArt& art) {
    return dbg.maybeSpace() << QString("CoverArt(%1,%2,%3)")
            .arg(coverInfoToString(art),
                 toDebugString(art.image.size()),
                 QString::number(art.resizedToWidth));
}
