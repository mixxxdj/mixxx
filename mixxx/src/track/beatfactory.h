#ifndef BEATFACTORY_H
#define BEATFACTORY_H

#include "track/beats.h"
#include "track/beatgrid.h"
#include "track/beatmatrix.h"

class BeatFactory {
  public:
    static BeatsPointer loadBeatsFromByteArray(TrackPointer pTrack,
                                               QString beatsVersion,
                                               QByteArray* beatsSerialized);
    static BeatsPointer makeBeatGrid(TrackPointer pTrack,
                                     double dBpm, double dFirstBeatSample);
    static BeatsPointer makeBeatMatrix (TrackPointer pTrack, QVector <double> beats);

  private:
    static void deleteBeats(Beats* pBeats);
};

#endif /* BEATFACTORY_H */
