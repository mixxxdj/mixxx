#include <QtDebug>

#include "library/coverart.h"
#include "library/coverartutils.h"
#include "util/debug.h"

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

QDebug operator<<(QDebug dbg, const CoverInfo& info) {
    return dbg.maybeSpace() << QString("CoverInfo(%1,%2,%3,%4,%5)")
            .arg(typeToString(info.type),
                 sourceToString(info.source),
                 info.coverLocation,
                 QString::number(info.hash), 
                 info.trackLocation);
}

QDebug operator<<(QDebug dbg, const CoverArt& art) {
    return dbg.maybeSpace() << QString("CoverArt(%1,%2)")
            .arg(toDebugString(art.image.size()),
                 toDebugString(art.info));
}

const quint16 CoverInfo::kNullImageHash = CoverArtUtils::calculateHash(QImage());

