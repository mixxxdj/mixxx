#include "library/coverart.h"

#include "library/coverartutils.h"
#include "util/debug.h"
#include "util/logger.h"

namespace {

mixxx::Logger kLogger("CoverArt");

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
        VERIFY_OR_DEBUG_ASSERT(!trackLocation.isEmpty()) {
            kLogger.warning()
                    << "loadImage"
                    << type
                    << "cover with empty trackLocation";
            return QImage();
        }
        return CoverArtUtils::extractEmbeddedCover(
                TrackFile(trackLocation),
                pTrackLocationToken);
    } else if (type == CoverInfo::FILE) {
        auto coverFile = QFileInfo(coverLocation);
        if (coverFile.isRelative()) {
            VERIFY_OR_DEBUG_ASSERT(!trackLocation.isEmpty()) {
                // This is not expected to happen, because every track
                // must have a valid location, i.e. a file path. Most
                // likely a programming error, but might also be caused
                // by yet unknown circumstances.
                kLogger.warning()
                        << "loadImage"
                        << type
                        << "cover with empty track location"
                        << "and relative file path:"
                        << coverFile.filePath();
                return QImage();
            }
            // Compose track directory with relative path
            const auto trackFile = TrackFile(trackLocation);
            DEBUG_ASSERT(trackFile.asFileInfo().isAbsolute());
            coverFile = QFileInfo(
                    trackFile.directory(),
                    coverLocation);
        }
        DEBUG_ASSERT(coverFile.isAbsolute());
        if (!coverFile.exists()) {
            // Disabled because this code can cause high CPU and thus possibly
            // xruns as it might print the warning repeatedly.
            // ToDo: Print warning about missing cover image only once.
            //            kLogger.warning()
            //                    << "loadImage"
            //                    << type
            //                    << "cover does not exist:"
            //                    << coverFile.filePath();
            return QImage();
        }
        SecurityTokenPointer pToken =
                Sandbox::openSecurityToken(
                        coverFile,
                        true);
        return QImage(coverFile.filePath());
    } else if (type == CoverInfo::NONE) {
        return QImage();
    } else {
        DEBUG_ASSERT(!"unhandled CoverInfo::Type");
        return QImage();
    }
}

bool CoverInfo::refreshImageHash(
        const QImage& loadedImage,
        const SecurityTokenPointer& pTrackLocationToken) {
    if (CoverImageUtils::isValidHash(hash)) {
        // Trust that a valid hash has been calculated from the
        // corresponding image. Otherwise we would refresh all
        // hashes over and over again.
        return false;
    }
    QImage image = loadedImage;
    if (loadedImage.isNull()) {
        image = loadImage(pTrackLocationToken);
    }
    if (image.isNull() && type != CoverInfo::NONE) {
        kLogger.warning()
                << "Resetting cover info"
                << *this;
        reset();
        return true;
    }
    hash = CoverImageUtils::calculateHash(image);
    DEBUG_ASSERT(image.isNull() || CoverImageUtils::isValidHash(hash));
    DEBUG_ASSERT(!image.isNull() || hash == CoverImageUtils::defaultHash());
    return true;
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
