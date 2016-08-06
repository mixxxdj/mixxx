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

QDebug operator<<(QDebug dbg, const CoverInfoRelative& infoRelative) {
    return dbg.maybeSpace() << QString("CoverInfoRelative(%1)")
            .arg(coverInfoRelativeToString(infoRelative));
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

const quint16 CoverInfoRelative::kNullImageHash = CoverArtUtils::calculateHash(QImage());

