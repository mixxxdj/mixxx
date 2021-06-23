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

quint16 calculateLegacyHash(
        const QImage& image) {
    auto legacyHash = qChecksum(
            reinterpret_cast<const char*>(image.constBits()),
            image.sizeInBytes());
    // In rare cases the calculated checksum could be equal to the
    // reserved value CoverInfo::defaultLegacyHash() which might cause
    // unexpected behavior. In this case we simply invert all bits to
    // get a hash value that is considered valid.
    // See also (same special case handling): mixxx::cacheKeyFromMessageDigest()
    // https://mixxx.discourse.group/t/mixxx-2-2-4-reads-id3v1-tags-mixxx-2-3-or-2-4-does-not/21041/11
    if (legacyHash == CoverInfo::defaultLegacyHash() && !image.isNull()) {
        legacyHash = ~legacyHash;
    }
    DEBUG_ASSERT(image.isNull() || legacyHash != CoverInfo::defaultLegacyHash());
    DEBUG_ASSERT(!image.isNull() || legacyHash == CoverInfo::defaultLegacyHash());
    return legacyHash;
}

} // anonymous namespace

CoverInfoRelative::CoverInfoRelative()
        : source(UNKNOWN),
          type(NONE),
          m_legacyHash(defaultLegacyHash()) {
}

void CoverInfoRelative::setImage(
        const QImage& image) {
    color = CoverImageUtils::extractBackgroundColor(image);
    m_imageDigest = CoverImageUtils::calculateDigest(image);
    DEBUG_ASSERT(image.isNull() == m_imageDigest.isEmpty());
    m_legacyHash = calculateLegacyHash(image);
    DEBUG_ASSERT(image.isNull() == (m_legacyHash == defaultLegacyHash()));
    DEBUG_ASSERT(image.isNull() != hasImage());
}

bool operator==(const CoverInfoRelative& lhs, const CoverInfoRelative& rhs) {
    return lhs.source == rhs.source &&
            lhs.type == rhs.type &&
            lhs.color == rhs.color &&
            lhs.legacyHash() == rhs.legacyHash() &&
            lhs.imageDigest() == rhs.imageDigest() &&
            lhs.coverLocation == rhs.coverLocation;
}

QDebug operator<<(QDebug dbg, const CoverInfoRelative& info) {
    const QDebugStateSaver saver(dbg);
    dbg = dbg.maybeSpace() << "CoverInfoRelative";
    return dbg.nospace()
            << '{'
            << typeToString(info.type)
            << ','
            << sourceToString(info.source)
            << ','
            << info.color
            << ','
            << info.coverLocation
            << ','
            << info.imageDigest()
            << ','
            << info.legacyHash()
            << '}';
}

CoverInfo::LoadedImage CoverInfo::loadImage(
        const SecurityTokenPointer& pTrackLocationToken) const {
    LoadedImage loadedImage(LoadedImage::Result::ErrorUnknown);
    if (type == CoverInfo::METADATA) {
        VERIFY_OR_DEBUG_ASSERT(!trackLocation.isEmpty()) {
            loadedImage.result = LoadedImage::Result::ErrorMetadataWithEmptyTrackLocation;
            return loadedImage;
        }
        loadedImage.location = trackLocation;
        loadedImage.image = CoverArtUtils::extractEmbeddedCover(
                mixxx::FileAccess(
                        mixxx::FileInfo(trackLocation),
                        pTrackLocationToken));
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
        auto coverFile = mixxx::FileInfo(coverLocation);
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
            const auto fileInfo = mixxx::FileInfo(trackLocation);
            coverFile = mixxx::FileInfo(
                    fileInfo.locationPath(),
                    coverLocation);
        }
        loadedImage.location = coverFile.location();
        if (!coverFile.exists()) {
            loadedImage.result = LoadedImage::Result::ErrorFilePathDoesNotExist;
            return loadedImage;
        }
        SecurityTokenPointer pToken =
                Sandbox::openSecurityToken(
                        &coverFile,
                        true);
        if (loadedImage.image.load(loadedImage.location)) {
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

bool CoverInfo::refreshImageDigest(
        const QImage& loadedImage,
        const SecurityTokenPointer& pTrackLocationToken) {
    if (!imageDigest().isEmpty()) {
        // Assume that a non-empty digest has been calculated from
        // the corresponding image. Otherwise we would refresh all
        // digests over and over again.
        // Invalid legacy hash values are ignored. These will only
        // be refreshed when opened with a previous version.
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
    setImage(image);
    return true;
}

bool operator==(const CoverInfo& lhs, const CoverInfo& rhs) {
    return static_cast<const CoverInfoRelative&>(lhs) ==
            static_cast<const CoverInfoRelative&>(rhs) &&
            lhs.trackLocation == rhs.trackLocation;
}

bool operator!=(const CoverInfo& lhs, const CoverInfo& rhs) {
    return !(lhs == rhs);
}

QDebug operator<<(QDebug dbg, const CoverInfo& info) {
    const QDebugStateSaver saver(dbg);
    dbg = dbg.maybeSpace() << "CoverInfo";
    return dbg.nospace()
            << '{'
            << static_cast<const CoverInfoRelative&>(info)
            << ','
            << info.trackLocation
            << '}';
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
            << loadedImage.location
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
