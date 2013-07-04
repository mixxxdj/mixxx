#include <QtDebug>

#include "track/timbre.h"
#include "track/timbrefactory.h"

Timbre TimbreFactory::loadTimbreFromByteArray(TrackPointer pTrack,
                                       QString timbreVersion,
                                       QString timbreSubVersion,
                                       QByteArray* timbreSerialized) {
    Q_UNUSED(pTrack);
    if (timbreVersion == TIMBRE_VERSION) {
        Timbre timbre(timbreSerialized);
        timbre.setSubVersion(timbreSubVersion);
        qDebug() << "Successfully deserialized TimbreModel";
        return timbre;
    }

    return Keys();
}
