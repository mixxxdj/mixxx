#ifndef TRAKTOR_CUE_H
#define TRAKTOR_CUE_H

#include <QString>
#include <QXmlStreamAttributes>
#include <QSqlQuery>
#include "library/dao/cue.h"

#define TRAKTOR_CUE_QUERY "INSERT INTO traktor_cues (track_id, displ_order," \
                            "hotcue, len, name, repeats, start, type)" \
                            " VALUES (:track_id, :displ_order, :hotcue, :len," \
                            ":name, :repeats, :start, :type)"
enum TraktorCueType {
    HOTCUE,     //Normal Cue
    FADEIN,     //Triggered by fadeout in playing deck
    FADEOUT,    //Triggers fadein cue in other deck
    LOAD,       //Position to jump to when track is loaded into deck
    AUTOGRID,   //First beat of track
    LOOP        //Loop Cue
};

class TraktorCue {
  public:
    TraktorCue(const QXmlStreamAttributes &cue_attributes);
    TraktorCue(const QSqlRecord &record, const QSqlQuery &query);
    CuePointer toCue(int samplerate);
    void fillQuery(int track_id, QSqlQuery &query);
    void setTrackId(int track_id);

  private:
    TraktorCue(int displ_order, int hotcue, double len, QString name, int repeats, double start, TraktorCueType type);
    int track_id;
    int displ_order;
    int hotcue;
    double len;
    QString name;
    int repeats;
    double start;
    TraktorCueType type;
};

#endif // TRAKTOR_CUE_H
