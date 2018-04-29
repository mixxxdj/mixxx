#include "library/traktor/traktor_cue.h"
#include <QSqlRecord>

TraktorCue::TraktorCue(int displ_order, int hotcue, double len, QString name,
        int repeats, double start, TraktorCueType type) : 
        displ_order(displ_order), hotcue(hotcue), len(len), name(name), repeats(repeats),
        start(start), type(type) {
}

TraktorCue::TraktorCue(const QXmlStreamAttributes &cue_attributes) :
    TraktorCue(
        cue_attributes.value("DISPL_ORDER").toString().toInt(),
        cue_attributes.value("HOTCUE").toString().toInt(),
        cue_attributes.value("LEN").toString().toDouble(),
        cue_attributes.value("NAME").toString(),
        cue_attributes.value("REPEATS").toString().toInt(),
        cue_attributes.value("START").toString().toDouble(),
        (TraktorCueType)cue_attributes.value("TYPE").toString().toInt()
    ) {
}

TraktorCue::TraktorCue(const QSqlRecord &record, const QSqlQuery &query) :
    TraktorCue(
        query.value(record.indexOf("displ_order")).toInt(),
        query.value(record.indexOf("hotcue")).toInt(),
        query.value(record.indexOf("len")).toDouble(),
        query.value(record.indexOf("name")).toString(),
        query.value(record.indexOf("repeats")).toInt(),
        query.value(record.indexOf("start")).toDouble(),
        (TraktorCueType)query.value(record.indexOf("type")).toInt()
    ) {
    track_id = query.value(record.indexOf("track_id")).toInt();
}

void TraktorCue::fillQuery(int track_id, QSqlQuery &query) {
    query.bindValue(":track_id", track_id);
    query.bindValue(":displ_order", displ_order);
    query.bindValue(":hotcue", hotcue);
    query.bindValue(":len", len);
    query.bindValue(":name", name);
    query.bindValue(":repeats", repeats);
    query.bindValue(":start", start);
    query.bindValue(":type", type);
}

CuePointer TraktorCue::toCue(int samplerate) {
    //Incomplete
    int hotcue = this->hotcue;
    Cue::CueType type;
    switch (this->type) {
        case HOTCUE:
            type = Cue::CueType::CUE;
            break;
        case LOAD:
            type = Cue::CueType::LOAD;
            break;
        case LOOP:
            type = Cue::CueType::LOOP;
            break;
        case AUTOGRID:
            type = Cue::CueType::BEAT;
            break;
        case FADEIN:
        case FADEOUT:
            qDebug() << "Fade cue points are not yet supported in mixxx";
            return CuePointer(NULL);
        default:
            qDebug() << "Unknown traktor cue type: " << this->type;
            return CuePointer(NULL);
    }
    TrackId track_id(this->track_id);
    QColor color("#FF0000");
    //start is a a float denoting milliseconds
    //convert to stereoSamplePointer
    int position = int(start/1000.0 * double(samplerate) * 2.0);
    int length = int(len/1000.0 * double(samplerate) * 2.0);

    return CuePointer(new Cue(-1, track_id, type, position, length, hotcue,
                name, color));
}

