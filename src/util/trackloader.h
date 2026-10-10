#ifndef MIXXX_TRACKLOADER_H
#define MIXXX_TRACKLOADER_H

#include <QString>
#include "track/track.h"
#include "control/controlobject.h"
#include "mixer/playermanager.h"
#include "preferences/configobject.h"

namespace mixxx {

struct TrackLoadDecision {
    bool canLoad;
    bool continuePlaying;
};

class TrackLoader {
public:
    static TrackLoadDecision canLoadTrackIntoPlayingDeck(const ConfigPointer& pConfig, const QString& group);
};

} 

#endif // MIXXX_TRACKLOADER_H