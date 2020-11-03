#include "library/coverart.h"

#include <QDebugStateSaver>

#include "library/coverartutils.h"
#include "track/track.h"
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

CoverInfo::LoadedImage CoverInfo::loadImage(
        const SecurityTokenPointer& pTrackLocationToken) const {
    LoadedImage loadedImage(LoadedImage::Result::ErrorUnknown);
    if (type == CoverInfo::METADATA) {
        VERIFY_OR_DEBUG_ASSERT(!trackLocation.isEmpty()) {
            loadedImage.result = LoadedImage::Result::ErrorMetadataWithEmptyTrackLocation;
            return loadedImage;
        }
        loadedImage.filePath = trackLocation;
        loadedImage.image = CoverArtUtils::extractEmbeddedCover(
                TrackFile(trackLocation),
                pTrackLocationToken);
        if (loadedImage.image.isNull()) {
            // TODO: extractEmbeddedCover() should indicate if no image
            // is available or if loading the embedded image failed.
            // Until then we assume optimistically that no image is
            // available instead of presuming that an error occurred.
            loadedImage.result = LoadedImage::Result::NoImage;
        } else {
            loadedImage.result = LoadedImage::Result::Ok;
        }
    } else if (type == CoverInfo::FILE) {
        auto coverFile = QFileInfo(coverLocation);
        if (coverFile.isRelative()) {
            VERIFY_OR_DEBUG_ASSERT(!trackLocation.isEmpty()) {
                // This is not expected to happen, because every track
                // must have a valid location, i.e. a file path. Most
                // likely a programming error, but might also be caused
                // by yet unknown circumstances.
                loadedImage.result = LoadedImage::Result::
                        ErrorRelativeFilePathWithEmptyTrackLocation;
                return loadedImage;
            }
            // Compose track directory with relative path
            const auto trackFile = TrackFile(trackLocation);
            DEBUG_ASSERT(trackFile.asFileInfo().isAbsolute());
            coverFile = QFileInfo(
                    trackFile.directory(),
                    coverLocation);
        }
        DEBUG_ASSERT(coverFile.isAbsolute());
        loadedImage.filePath = coverFile.filePath();
        if (!coverFile.exists()) {
            loadedImage.result = LoadedImage::Result::ErrorFilePathDoesNotExist;
            return loadedImage;
        }
        SecurityTokenPointer pToken =
                Sandbox::openSecurityToken(
                        coverFile,
                        true);
        if (loadedImage.image.load(loadedImage.filePath)) {
            DEBUG_ASSERT(!loadedImage.image.isNull());
            loadedImage.result = LoadedImage::Result::Ok;
        } else {
            DEBUG_ASSERT(loadedImage.image.isNull());
            loadedImage.result = LoadedImage::Result::ErrorLoadingFailed;
        }
    } else if (type == CoverInfo::NONE) {
        loadedImage.result = LoadedImage::Result::NoImage;
    } else {
        DEBUG_ASSERT(!"unhandled CoverInfo::Type");
    }
    return loadedImage;
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
    if (image.isNull()) {
        image = loadImage(pTrackLocationToken).image;
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

QDebug operator<<(QDebug dbg, const CoverInfo::LoadedImage::Result& result) {
    switch (result) {
    case CoverInfo::LoadedImage::Result::Ok:
        return dbg << "Ok";
    case CoverInfo::LoadedImage::Result::NoImage:
        return dbg << "NoImage";
    case CoverInfo::LoadedImage::Result::ErrorMetadataWithEmptyTrackLocation:
        return dbg << "ErrorMetadataWithEmptyTrackLocation";
    case CoverInfo::LoadedImage::Result::ErrorRelativeFilePathWithEmptyTrackLocation:
        return dbg << "ErrorRelativeFilePathWithEmptyTrackLocation";
    case CoverInfo::LoadedImage::Result::ErrorFilePathDoesNotExist:
        return dbg << "ErrorFilePathDoesNotExist";
    case CoverInfo::LoadedImage::Result::ErrorLoadingFailed:
        return dbg << "ErrorLoadingFailed";
    case CoverInfo::LoadedImage::Result::ErrorUnknown:
        return dbg << "ErrorUnknown";
    }
    DEBUG_ASSERT(!"unreachable");
    return dbg;
}

QDebug operator<<(QDebug dbg, const CoverInfo::LoadedImage& loadedImage) {
    const QDebugStateSaver saver(dbg);
    dbg = dbg.maybeSpace() << "CoverInfo::LoadedImage";
    return dbg.nospace()
            << '{'
            << loadedImage.filePath
            << ','
            << loadedImage.image.size()
            << ','
            << loadedImage.result
            << '}';
}

QDebug operator<<(QDebug dbg, const CoverArt& art) {
    const QDebugStateSaver saver(dbg);
    dbg = dbg.maybeSpace() << "CoverArt";
    return dbg.nospace()
            << '{'
            << static_cast<const CoverInfo&>(art)
            << ','
            << art.loadedImage
            << ','
            << art.resizedToWidth
            << '}';
}
