#include "musicbrainz/musicbrainz.h"

#include <mutex> // std::once_flag/call_once

namespace mixxx {

namespace musicbrainz {

namespace {

std::once_flag s_registerMetaTypesOnceFlag;

void registerMetaTypes() {
    qRegisterMetaType<TrackRelease>();
    qRegisterMetaType<QList<TrackRelease>>();
}

} // anonymous namespace

void registerMetaTypesOnce() {
    std::call_once(s_registerMetaTypesOnceFlag, registerMetaTypes);
}

} // namespace musicbrainz

} // namespace mixxx
