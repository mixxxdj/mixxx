#include "trackselector.h"
#include "library/autodj/autodjprocessor.h"



TrackSelector::TrackSelector(AutoDJProcessor* processor) :
   QObject(processor),
   m_processor(processor) {}
/*
TrackPointer TrackSelector::getNextTrackFromQueue() {
    assert(false);
}
*/

/* LinearSelector */

LinearSelector::LinearSelector(AutoDJProcessor* processor): TrackSelector(processor) { }


TrackPointer LinearSelector::getNextTrackFromQueue() {
    PlaylistTableModel* model(m_processor->getTableModel());

    while (true) {
        TrackPointer nextTrack = model->getTrack(model->index(0, 0));

        if (nextTrack) {
            if (nextTrack->exists()) {
                return nextTrack;
            } else {
                // Remove missing song from auto DJ playlist.
                model->removeTrack(model->index(0, 0));
            }
        } else {
            // We're out of tracks. Return the null TrackPointer.
            return nextTrack;
        }
    }

}


/* RandomSelector */
RandomSelector::RandomSelector(AutoDJProcessor* processor): TrackSelector(processor) { }


TrackPointer RandomSelector::getNextTrackFromQueue() {
    PlaylistTableModel* model(m_processor->getTableModel());

    while (true) {
        int trackCount = model->rowCount();
        int nextPos = qrand()%trackCount;
        qDebug() << "RandomSelector: chose track row:" << nextPos;
        TrackPointer nextTrack = model->getTrack(model->index(nextPos, 0));

        if (nextTrack) {
            if (nextTrack->exists()) {
                return nextTrack;
            } else {
                // Remove missing song from auto DJ playlist.
                model->removeTrack(model->index(nextPos, 0));
            }
        } else {
            // We're out of tracks. Return the null TrackPointer.
            return nextTrack;
        }
    }

}

