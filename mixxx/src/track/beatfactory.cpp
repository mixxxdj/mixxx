#include <QtDebug>

#include "track/beatgrid.h"
#include "track/beatmatrix.h"
#include "track/beatmap.h"
#include "track/beatfactory.h"

BeatsPointer BeatFactory::loadBeatsFromByteArray(TrackPointer pTrack,
                                                 QString beatsVersion,
                                                 QByteArray* beatsSerialized) {

    if (beatsVersion == BEAT_GRID_VERSION) {
        BeatGrid* pGrid = new BeatGrid(pTrack, beatsSerialized);
        pGrid->moveToThread(pTrack->thread());
        pGrid->setParent(pTrack.data());
        qDebug() << "Successfully deserialized BeatGrid";
        return BeatsPointer(pGrid, &BeatFactory::deleteBeats);
    } else if (beatsVersion == BEAT_MATRIX_VERSION) {
        BeatMatrix* pMatrix = new BeatMatrix(pTrack, beatsSerialized);
        pMatrix->moveToThread(pTrack->thread());
        pMatrix->setParent(pTrack.data());
        qDebug() << "Successfully deserialized BeatMatrix";
        return BeatsPointer(pMatrix, &BeatFactory::deleteBeats);
    } else if (beatsVersion == BEAT_MAP_VERSION) {
        BeatMap* pMap = new BeatMap(pTrack, beatsSerialized);
        pMap->moveToThread(pTrack->thread());
        pMap->setParent(pTrack.data());
        qDebug() << "Successfully deserialized BeatMap";
        return BeatsPointer(pMap, &BeatFactory::deleteBeats);
    }
    qDebug() << "BeatFactory::loadBeatsFromByteArray could not parse serialized beats.";
    return BeatsPointer();
}

BeatsPointer BeatFactory::makeBeatGrid(TrackPointer pTrack, double dBpm, double dFirstBeatSample) {
    BeatGrid* pGrid = new BeatGrid(pTrack);
    pGrid->setGrid(dBpm, dFirstBeatSample);
    return BeatsPointer(pGrid, &BeatFactory::deleteBeats);
}

BeatsPointer BeatFactory::makeBeatMap(TrackPointer pTrack, QVector<double> beats) {
    BeatMap* pBeatMap = new BeatMap(pTrack, beats);
    return BeatsPointer(pBeatMap, &BeatFactory::deleteBeats);
}

void BeatFactory::deleteBeats(Beats* pBeats) {
    // This assumes all Beats* variants multiply-inherit from QObject. Kind of
    // ugly. Oh well.
    QObject* pObject = dynamic_cast<QObject*>(pBeats);

    if (pObject != NULL) {
        pObject->deleteLater();
    }
}
