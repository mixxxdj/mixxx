#include <QtDebug>

#include "track/beatgrid.h"
#include "track/beatmatrix.h"
#include "track/beatfactory.h"

BeatsPointer BeatFactory::loadBeatsFromByteArray(TrackPointer pTrack,
                                                 QString beatsVersion,
                                                 QByteArray* beatsSerialized) {
    // TODO(XXX) make it so the version strings are not in multiple places
    if (beatsVersion == "BeatGrid-1.0") {
        BeatGrid* pGrid = new BeatGrid(pTrack, beatsSerialized);
        pGrid->moveToThread(pTrack->thread());
        pGrid->setParent(pTrack.data());
        qDebug() << "Successfully deserialized BeatGrid";
        return BeatsPointer(pGrid, &BeatFactory::deleteBeats);
    } else if (beatsVersion == "BeatMatrix-1.0") {
        BeatMatrix* pMatrix = new BeatMatrix(pTrack, beatsSerialized);
        pMatrix->moveToThread(pTrack->thread());
        pMatrix->setParent(pTrack.data());
        qDebug() << "Successfully deserialized BeatMatrix";
        return BeatsPointer(pMatrix, &BeatFactory::deleteBeats);
    }
    qDebug() << "BeatFactory::loadBeatsFromByteArray could not parse serialized beats.";
    return BeatsPointer();
}

BeatsPointer BeatFactory::makeBeatGrid(TrackPointer pTrack, double dBpm, double dFirstBeatSample) {
    BeatGrid* pGrid = new BeatGrid(pTrack);
    pGrid->setGrid(dBpm, dFirstBeatSample);
    return BeatsPointer(pGrid, &BeatFactory::deleteBeats);
}

void BeatFactory::deleteBeats(Beats* pBeats) {
    // This assumes all Beats* variants multiply-inherit from QObject. Kind of
    // ugly. Oh well.
    QObject* pObject = dynamic_cast<QObject*>(pBeats);

    if (pObject != NULL) {
        pObject->deleteLater();
    }
}
