#include <QtDebug>
#include <QLatin1Literal>

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
    return typeToString(infoRelative.type) % QLatin1Literal(",") %
           sourceToString(infoRelative.source) % QLatin1Literal(",") %
           infoRelative.coverLocation % QLatin1Literal(",") %
           QLatin1Literal("0x") % QString::number(infoRelative.hash, 16);
}

QString coverInfoToString(const CoverInfo& info) {
    return coverInfoRelativeToString(info) % QLatin1Literal(",") %
           info.trackLocation % QLatin1Literal(",");
}
} // anonymous namespace


bool operator==(const CoverInfoRelative& a, const CoverInfoRelative& b) {
    return a.source == b.source &&
            a.type == b.type &&
            a.coverLocation == b.coverLocation &&
            a.hash == b.hash;
}

bool operator!=(const CoverInfoRelative& a, const CoverInfoRelative& b) {
    return !(a == b);
}

QDebug operator<<(QDebug dbg, const CoverInfoRelative& infoRelative) {
    return dbg.maybeSpace() << QString("CoverInfoRelative(%1)")
            .arg(coverInfoRelativeToString(infoRelative));
}

bool operator==(const CoverInfo& a, const CoverInfo& b) {
    return static_cast<CoverInfoRelative>(a) ==
                    static_cast<CoverInfoRelative>(b) &&
            a.trackLocation == b.trackLocation;
}

bool operator!=(const CoverInfo& a, const CoverInfo& b) {
    return !(a == b);
}

QDebug operator<<(QDebug dbg, const CoverInfo& info) {
    return dbg.maybeSpace() << QString("CoverInfo(%1)")
            .arg(coverInfoToString(info));
}

bool operator==(const CoverArt& a, const CoverArt& b) {
    // Only count image in the equality if both are non-null.
    return static_cast<CoverInfo>(a) ==
                    static_cast<CoverInfo>(b) &&
            (a.image.isNull() || b.image.isNull() ||
             a.image == b.image);
}

bool operator!=(const CoverArt& a, const CoverArt& b) {
    return !(a == b);
}

QDebug operator<<(QDebug dbg, const CoverArt& art) {
    return dbg.maybeSpace() << QString("CoverArt(%1,%2,%3)")
            .arg(coverInfoToString(art),
                 toDebugString(art.image.size()),
                 QString::number(art.resizedToWidth));
}

const quint16 CoverInfoRelative::kNullImageHash = CoverArtUtils::calculateHash(QImage());

