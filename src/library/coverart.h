#pragma once

#include <QImage>
#include <QString>
#include <QtDebug>

#include "util/sandbox.h"

class CoverImageUtils {
  public:
    static quint16 calculateHash(
            const QImage& image);

    static constexpr quint16 defaultHash() {
        return 0;
    }

    static constexpr bool isValidHash(
            quint16 hash) {
        return hash != defaultHash();
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

    // all-default memory management
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

    Source source;
    Type type;
    QString coverLocation; // relative path, from track location
    quint16 hash;
};

bool operator==(const CoverInfoRelative& a, const CoverInfoRelative& b);
bool operator!=(const CoverInfoRelative& a, const CoverInfoRelative& b);
QDebug operator<<(QDebug dbg, const CoverInfoRelative& info);

class CoverInfo : public CoverInfoRelative {
  public:
    CoverInfo() {}

    CoverInfo(const CoverInfoRelative& base, const QString& tl)
        : CoverInfoRelative(base),
          trackLocation(tl) {
    }

    // all-default memory management
    CoverInfo(const CoverInfo&) = default;
    CoverInfo& operator=(const CoverInfo&) = default;
    virtual ~CoverInfo() override = default;
    CoverInfo(CoverInfo&&) = default;
    CoverInfo& operator=(CoverInfo&&) = default;

    QImage loadImage(
            const SecurityTokenPointer& pTrackLocationToken = SecurityTokenPointer()) const;

    // Verify the image hash and update it if necessary.
    // If the corresponding image has already been loaded it
    // could be provided as a parameter to avoid reloading
    // if actually needed.
    bool refreshImageHash(
            const QImage& loadedImage = QImage(),
            const SecurityTokenPointer& pTrackLocationToken = SecurityTokenPointer());

    QString trackLocation;
};

bool operator==(const CoverInfo& a, const CoverInfo& b);
bool operator!=(const CoverInfo& a, const CoverInfo& b);
QDebug operator<<(QDebug dbg, const CoverInfo& info);

class CoverArt : public CoverInfo {
  public:
    CoverArt()
        : resizedToWidth(0) {
    }

    CoverArt(const CoverInfo& base, const QImage& img, int rtw)
        : CoverInfo(base),
          image(img),
          resizedToWidth(rtw) {
    }

    // all-default memory management
    CoverArt(const CoverArt&) = default;
    virtual ~CoverArt() override = default;
    CoverArt(CoverArt&&) = default;
    CoverArt& operator=(CoverArt&&) = default;

    // it is not a QPixmap, because it is not safe to use pixmaps
    // outside the GUI thread
    QImage image;
    int resizedToWidth;
};

QDebug operator<<(QDebug dbg, const CoverArt& art);
