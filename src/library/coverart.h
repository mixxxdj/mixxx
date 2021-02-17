#pragma once

#include <QImage>
#include <QString>
#include <QtDebug>

#include "util/cache.h"
#include "util/color/rgbcolor.h"
#include "util/imageutils.h"
#include "util/sandbox.h"

class CoverImageUtils {
  public:
    static mixxx::RgbColor::optional_t extractBackgroundColor(
            const QImage& image) {
        return mixxx::RgbColor::fromQColor(
                mixxx::extractImageBackgroundColor(image));
    }

    static QByteArray calculateDigest(
            const QImage& image) {
        return mixxx::digestImage(image);
    }

    static mixxx::cache_key_t cacheKeyFromDigest(
            const QByteArray& digest) {
        return mixxx::cacheKeyFromMessageDigest(digest);
    }

    static constexpr mixxx::cache_key_t defaultCacheKey() {
        return mixxx::invalidCacheKey();
    }
};

class CoverInfoRelative {
  public:
    // DO NOT CHANGE THESE CONSTANT VALUES. THEY ARE STORED IN THE DATABASE.
    enum Source {
        // We don't know where we got this cover.
        UNKNOWN = 0,
        // The cover was automatically detected by Mixxx.
        GUESSED = 1,
        // The cover was selected by the user (do not disturb).
        USER_SELECTED = 2
    };

    // DO NOT CHANGE THESE CONSTANT VALUES. THEY ARE STORED IN THE DATABASE.
    enum Type {
        // No cover information is known.
        NONE = 0,
        // Cover is located in the metadata of an audio file.
        METADATA = 1,
        // Cover is located in a standalone image file.
        FILE = 2
    };

    CoverInfoRelative();

    CoverInfoRelative(const CoverInfoRelative&) = default;
    CoverInfoRelative& operator=(const CoverInfoRelative&) = default;
    virtual ~CoverInfoRelative() = default;
    CoverInfoRelative(CoverInfoRelative&&) = default;
    CoverInfoRelative& operator=(CoverInfoRelative&&) = default;

    /*non-virtual*/ void reset() {
        // Slicing when invoked from a subclass is intended!
        // Only the contents of this base class are supposed
        // to be reset, i.e. trackLocation should be preserved.
        *this = CoverInfoRelative();
    }

    void setImage(
            const QImage& image = QImage());
    void setImageDigest(
            QByteArray imageDigest,
            quint16 legacyHash = defaultLegacyHash()) {
        m_imageDigest = std::move(imageDigest);
        m_legacyHash = legacyHash;
    }

    bool hasImage() const {
        return cacheKey() != CoverImageUtils::defaultCacheKey();
    }

    const QByteArray imageDigest() const {
        return m_imageDigest;
    }
    mixxx::cache_key_t cacheKey() const {
        if (m_imageDigest.isEmpty()) {
            // Legacy fallback, required for incremental migration
            return m_legacyHash;
        } else {
            return CoverImageUtils::cacheKeyFromDigest(m_imageDigest);
        }
    }

    static constexpr quint16 defaultLegacyHash() {
        return static_cast<quint16>(CoverImageUtils::defaultCacheKey());
    }

    quint16 legacyHash() const {
        return m_legacyHash;
    }

    Source source;
    Type type;

    /// An otional color that is calculated from the cover art
    /// image if available. Supposed to be used as a background
    /// color when displaying a placeholder for the actual
    /// image.
    mixxx::RgbColor::optional_t color;

    QString coverLocation; // relative path, from track location

  private:
    QByteArray m_imageDigest;
    // Only for backwards compatibility of database
    quint16 m_legacyHash;
};

bool operator==(const CoverInfoRelative& lhs, const CoverInfoRelative& rhs);
bool operator!=(const CoverInfoRelative& lhs, const CoverInfoRelative& rhs);
QDebug operator<<(QDebug dbg, const CoverInfoRelative& info);

class CoverInfo : public CoverInfoRelative {
  public:
    CoverInfo() {}

    CoverInfo(const CoverInfoRelative& base, const QString& tl)
        : CoverInfoRelative(base),
          trackLocation(tl) {
    }

    CoverInfo(const CoverInfo&) = default;
    CoverInfo& operator=(const CoverInfo&) = default;
    ~CoverInfo() override = default;
    CoverInfo(CoverInfo&&) = default;
    CoverInfo& operator=(CoverInfo&&) = default;

    struct LoadedImage final {
        enum class Result {
            Ok,
            NoImage,
            ErrorMetadataWithEmptyTrackLocation,
            ErrorRelativeFilePathWithEmptyTrackLocation,
            ErrorFilePathDoesNotExist,
            ErrorLoadingFailed, // if the image file is not readable or the format is not supported
            ErrorUnknown,       // should never happen
        };

        LoadedImage(const LoadedImage&) = default;
        LoadedImage(LoadedImage&&) = default;
        LoadedImage& operator=(const LoadedImage&) = default;
        LoadedImage& operator=(LoadedImage&&) = default;

        /// The loaded image if available.
        QImage image;

        /// Either the track location if the image was embedded in
        /// the metadata or the (absolute) path of the image file.
        QString filePath;

        /// The result of the operation.
        Result result;

      private:
        friend class CoverArt;
        friend class CoverInfo;
        LoadedImage(Result result)
                : result(result) {
        }
    };
    LoadedImage loadImage(
            const SecurityTokenPointer& pTrackLocationToken = SecurityTokenPointer()) const;

    /// Verify the image digest and update it if necessary.
    /// If the corresponding image has already been loaded it
    /// could be provided as a parameter to avoid reloading
    /// if actually needed.
    bool refreshImageDigest(
            const QImage& loadedImage = QImage(),
            const SecurityTokenPointer& pTrackLocationToken = SecurityTokenPointer());

    QString trackLocation;
};

bool operator==(const CoverInfo& a, const CoverInfo& b);
bool operator!=(const CoverInfo& a, const CoverInfo& b);
QDebug operator<<(QDebug dbg, const CoverInfoRelative& info);

QDebug operator<<(QDebug dbg, const CoverInfo::LoadedImage::Result& result);
QDebug operator<<(QDebug dbg, const CoverInfo::LoadedImage& loadedImage);

class CoverArt : public CoverInfo {
  public:
    CoverArt()
            : loadedImage(LoadedImage::Result::NoImage), // not loaded = no image
              resizedToWidth(0) {
    }
    CoverArt(
            const CoverInfo&& base,
            CoverInfo::LoadedImage&& loadedImage,
            int resizedToWidth)
            : CoverInfo(std::move(base)),
              loadedImage(std::move(loadedImage)),
              resizedToWidth(resizedToWidth) {
    }

    CoverArt(const CoverArt&) = default;
    ~CoverArt() override = default;
    CoverArt(CoverArt&&) = default;
    CoverArt& operator=(CoverArt&&) = default;

    // it is not a QPixmap, because it is not safe to use pixmaps
    // outside the GUI thread
    CoverInfo::LoadedImage loadedImage;
    int resizedToWidth;
};

QDebug operator<<(QDebug dbg, const CoverArt& art);
