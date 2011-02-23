#ifndef BEATFACTORY_H
#define BEATFACTORY_H

#include "beats.h"
#include "beatgrid.h"
#include "beatmatrix.h"

class BeatFactory {
  public:
    static BeatsPointer loadBeatsFromByteArray(TrackPointer pTrack,
                                               QString beatsVersion,
                                               QByteArray* beatsSerialized);
    static BeatsPointer makeBeatGrid(TrackPointer pTrack,
                                     double dBpm, double dFirstBeatSample);
  private:
    static void deleteBeats(Beats* pBeats);
};

#endif /* BEATFACTORY_H */
