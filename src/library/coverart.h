#ifndef COVERART_H
#define COVERART_H

#include <QImage>
#include <QString>
#include <QObject>
#include <QtDebug>
#include <QtGlobal>

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

    CoverInfoRelative()
            : source(UNKNOWN),
              type(NONE),
              // This default value is fine: qChecksum(NULL, 0) is 0.
              hash(0) {
    }

    bool operator==(const CoverInfoRelative& other) const {
        return other.source == source &&
                other.type == type &&
                other.coverLocation == coverLocation &&
                other.hash == hash;
    }
    bool operator!=(const CoverInfoRelative& other) const {
        return !(*this == other);
    }

    Source source;
    Type type;
    QString coverLocation; // relative path, from track location
    quint16 hash;
};

class CoverInfo : public CoverInfoRelative {
  public:
    CoverInfo() {}

    CoverInfo(const CoverInfoRelative& base, const QString& tl)
        : CoverInfoRelative(base),
          trackLocation(tl) {
    }

    bool operator==(const CoverInfo& other) const {
        return static_cast<CoverInfoRelative>(other) == 
                        static_cast<CoverInfoRelative>(*this) &&
                other.trackLocation == trackLocation;
    }
    bool operator!=(const CoverInfo& other) const {
        return !(*this == other);
    }

    QString trackLocation;
};

class CoverArt : public CoverInfo {
  public:
    CoverArt() {}

    CoverArt(const CoverInfo& base, const QImage& img)
        : CoverInfo(base),
          image(img) {
    }

    bool operator==(const CoverArt& other) const {
        // Only count image in the equality if both are non-null.
        return static_cast<CoverInfo>(other) ==
                        static_cast<CoverInfo>(*this) &&
                (other.image.isNull() || image.isNull() ||
                 other.image == image);
    }
    bool operator!=(const CoverArt& other) const {
        return !(*this == other);
    }

    QImage image;
};

QDebug operator<<(QDebug dbg, const CoverInfoRelative& info);
QDebug operator<<(QDebug dbg, const CoverInfo& info);
QDebug operator<<(QDebug dbg, const CoverArt& art);

#endif /* COVERART_H */
