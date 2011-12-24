#ifndef BEATFACTORY_H
#define BEATFACTORY_H

#include "track/beats.h"
#include "track/beatgrid.h"
#include "track/beatmatrix.h"
#include "track/beatmap.h"

class BeatFactory {
  public:
    static BeatsPointer loadBeatsFromByteArray(TrackPointer pTrack,
                                               QString beatsVersion,
                                               QByteArray* beatsSerialized);
    static BeatsPointer makeBeatGrid(TrackPointer pTrack,
                                     double dBpm, double dFirstBeatSample);
    static BeatsPointer makeBeatMap (TrackPointer pTrack, QVector <double> beats, QString subVer);
  private:
    static void deleteBeats(Beats* pBeats);
};

#endif /* BEATFACTORY_H */
