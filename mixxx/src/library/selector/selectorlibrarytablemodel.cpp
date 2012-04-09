// Created 3/17/2012 by Keith Salisbury (keithsalisbury@gmail.com)

#include <QObject>

#include "selectorlibrarytablemodel.h"
#include "library/trackcollection.h"
#include "basetrackplayer.h"
#include "playerinfo.h"

#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "trackinfoobject.h"


SelectorLibraryTableModel::SelectorLibraryTableModel(QObject* parent,
                                                   TrackCollection* pTrackCollection)
        : LibraryTableModel(parent, pTrackCollection,
                            "mixxx.db.model.selector") {

    connect(this, SIGNAL(doSearch(const QString&)),
            this, SLOT(slotSearch(const QString&)));

    // Getting info on current decks playing etc
    connect(&PlayerInfo::Instance(), SIGNAL(currentPlayingDeckChanged(int)),
           this, SLOT(slotPlayingDeckChanged(int)));

    m_bFilterGenre = true;
    m_bFilterBpm = false;
    m_iFilterBpmRange = 0;
    m_bFilterYear = false;
    m_bFilterRating = false;
    m_bFilterKey = false;
    m_bFilterHarmonicKey = false;
    m_channelBpm = NULL;

    initializeHarmonicsData();

}

SelectorLibraryTableModel::~SelectorLibraryTableModel() {
}

bool SelectorLibraryTableModel::isColumnInternal(int column) {
    return LibraryTableModel::isColumnInternal(column);
}

void SelectorLibraryTableModel::slotPlayingDeckChanged(int deck) {

    m_pChannel = QString("[Channel%1]").arg(deck);

    // disconnect the old pitch slider
    if (m_channelBpm) {
        m_channelBpm->disconnect(this);
    }
    // get the new pitch slider object
    m_channelBpm = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(m_pChannel, "bpm")));
    // listen for slider change events
    connect(m_channelBpm, SIGNAL(valueChanged(double)), this, 
        SLOT(slotChannel1BpmChanged(double)));

    m_pLoadedTrack = PlayerInfo::Instance().getTrackInfo(m_pChannel);
    if (m_pLoadedTrack) {

        // Genre
        QString TrackGenre = m_pLoadedTrack->getGenre();
        m_pFilterGenre = (TrackGenre != "") ? QString(
            "Genre == '%1'").arg(TrackGenre) : QString();

        // Year
        QString TrackYear = m_pLoadedTrack->getYear();
        m_pFilterYear = (TrackYear!="") ? QString(
            "Year == '%1'").arg(TrackYear) : QString();

        // Rating
        int TrackRating = m_pLoadedTrack->getRating();
        m_pFilterRating = (TrackRating > 0) ? QString(
            "Rating >= %1").arg(TrackRating) : QString();

        // Bpm and Keys
        float rate = getRate();
        setBpmFilter(rate);
        setKeyFilters(rate);

    }
    emit(doSearch(""));
}


void SelectorLibraryTableModel::slotChannel1BpmChanged(double value) {
    // Bpm and Keys
    float rate = getRate();
    setBpmFilter(rate);
    setKeyFilters(rate);
    emit(doSearch(""));
}

float SelectorLibraryTableModel::getRate() {
    // get pitch slider value (deck rate)
    ControlObjectThreadMain* rateSlider = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(m_pChannel, "rate")));
    ControlObjectThreadMain* rateRange = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(m_pChannel, "rateRange")));
    ControlObjectThreadMain* rateDirection = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(m_pChannel, "rate_dir")));

    float rate = (1 + rateSlider->get() * rateRange->get() * rateDirection->get());
    return rate;
}

void SelectorLibraryTableModel::setBpmFilter(float rate) {
    qDebug() << "setBpmFilters " << rate;

    float trackBpm = m_pLoadedTrack->getBpm();
    float currentBpm = trackBpm * rate;
    //float trackBpm = pChannel1Bpm->get();
    m_pFilterBpm = (currentBpm > 0) ? QString(
        "Bpm > %1 AND Bpm < %2").arg(
        floor(currentBpm - m_iFilterBpmRange)).arg(
        ceil(currentBpm + m_iFilterBpmRange)) : QString();
}

void SelectorLibraryTableModel::setKeyFilters(float rate) {

    qDebug() << "setKeyFilters " << rate;

    float trackBpm = m_pLoadedTrack->getBpm();
    float currentBpm = trackBpm * rate;

    float semitoneOffset = frequencyRatioToOctaveDifference(currentBpm, trackBpm);
    QString TrackKey = adjustPitchBy(m_pLoadedTrack->getKey(), semitoneOffset);
    // Key
    m_pFilterKey = (TrackKey!="") ? QString("Key == '%1'").arg(TrackKey) : QString();
    // Harmonic Key
    //QString hKeys = m_harmonics[TrackKey];

    QString hKeys;

    // determine major or minor key "m" 
    if (TrackKey.contains("m", Qt::CaseInsensitive)) {
        hKeys = getHarmonicKeys(m_minors,m_majors,TrackKey);
    } else {
        hKeys = getHarmonicKeys(m_majors,m_minors,TrackKey);
    }
    
    qDebug() << "setKeyFilters hKeys = " << hKeys;


    m_pFilterHarmonicKey = (hKeys!="") ? QString("Key in (%1)").arg(hKeys) : QString();
}

QString SelectorLibraryTableModel::getHarmonicKeys(QStringList keys1, QStringList keys2, QString key) const {
    int index = keys1.indexOf(key);
    if (index<0) return QString("");
    int lower = index-1;
    if (lower<0) lower += keys1.count(); 
    int upper = index+1;
    if (upper>=keys1.count()) upper -= keys1.count(); 
    qDebug() << "getHarmonicKeys index = " << index;
    qDebug() << "getHarmonicKeys lower = " << lower;
    qDebug() << "getHarmonicKeys upper = " << upper;
    return QString("'%1','%2','%3'").arg(keys1[lower],keys2[index],keys1[upper]);
}


QString SelectorLibraryTableModel::adjustPitchBy(QString pitch, int change) {
    if (pitch == "") return pitch;

    qDebug() << "adjustPitchBy pitch = " << pitch;
    int position = m_semitoneList.indexOf(pitch);
    if (position<0){
        qDebug() << "Pitch " << pitch << " not found in config.";
        return pitch;
    }

    int newpos = position + (change * 2);
    if (newpos >= 24) {
        newpos = newpos - 24;
    }
    QString newpitch = m_semitoneList.at(newpos);
    return newpitch;
}

// calculate the new pitch
// To change pitch by one semitone, multiply or divide BPM by 
//    the 12th root of 2, depending on which direction you want
//    to go. The 12th root of 2 is approximately 1.0594631.
//    6% == 1 semitone
// float 12TH_ROOT_OF_2 = 2^(1/12); //1.05946309435929
float SelectorLibraryTableModel::frequencyRatioToOctaveDifference(
    float currentBpm, float originalBpm) {
    // const int semitonesPerOctave = 12;
    float frequencyRatio = currentBpm / originalBpm;
    float semitones = 12 * log(frequencyRatio) / log(2);
    return semitones;
}

void SelectorLibraryTableModel::search(const QString& searchText) {
    // qDebug() << "SelectorLibraryTableModel::search()" << searchText
    //          << QThread::currentThread();
    emit(doSearch(searchText));
}

void SelectorLibraryTableModel::slotSearch(const QString& searchText) {
    QStringList filters;
    if (m_bFilterGenre && m_pFilterGenre != "") { filters << m_pFilterGenre; }
    if (m_bFilterYear && m_pFilterYear != "") { filters << m_pFilterYear; }
    if (m_bFilterRating && m_pFilterRating != "") { filters << m_pFilterRating; }

    if (m_bFilterBpm && m_pFilterBpm != "") { filters << m_pFilterBpm; }
    // hack if both
    if ((m_bFilterKey && m_pFilterKey != "") 
        && (m_bFilterHarmonicKey && m_pFilterHarmonicKey != "")) {
        filters << QString("(%1 OR %2)").arg(m_pFilterKey).arg(m_pFilterHarmonicKey);
    } else {
        if (m_bFilterKey && m_pFilterKey != "") { filters << m_pFilterKey; }
        if (m_bFilterHarmonicKey && m_pFilterHarmonicKey != "") { filters << m_pFilterHarmonicKey; }
    }

    //qDebug() << "slotSearch()m_pFilterGenre = [" << m_pFilterGenre << "] " << (m_pFilterGenre != "");   
    //qDebug() << "slotSearch()m_pFilterBpm = [" << m_pFilterBpm << "] " << (m_pFilterBpm != "");   
    //qDebug() << "slotSearch()m_pFilterYear = [" << m_pFilterYear << "] " << (m_pFilterYear != "");   

    QString filterText = filters.join(" AND ");
 
    qDebug() << "slotSearch()" << filterText;
    BaseSqlTableModel::search(searchText, filterText);
}

void SelectorLibraryTableModel::filterByGenre(bool value) {
    m_bFilterGenre = value;
    emit(doSearch(QString()));
}

void SelectorLibraryTableModel::filterByBpm(bool value, int range) {
    qDebug() << "filterByBpm bpm = " << value << " range = " << range;
    m_bFilterBpm = value;
    m_iFilterBpmRange = range;

    // Bpm and Keys
    float rate = getRate();
    setBpmFilter(rate);
    setKeyFilters(rate);

    emit(doSearch(QString()));
}

void SelectorLibraryTableModel::filterByYear(bool value) {
    m_bFilterYear = value;
    emit(doSearch(QString()));
}

void SelectorLibraryTableModel::filterByRating(bool value) {
    m_bFilterRating = value;
    emit(doSearch(QString()));
}

void SelectorLibraryTableModel::filterByKey(bool value) {
    m_bFilterKey = value;
    emit(doSearch(QString()));
}

void SelectorLibraryTableModel::filterByHarmonicKey(bool value) {
    m_bFilterHarmonicKey = value;
    emit(doSearch(QString()));
}

void SelectorLibraryTableModel::initializeHarmonicsData() {

    /*
    m_enharmonic_preference = "b" // # or b
    "A# = Bb"
    "C# = Db"
    "D# = Eb"
    "F# = Gb"
    "G# = Ab"

    m_enharmonic_preference = "#" // # or b
    "Ab = G#"
    "Bb = A#"
    "Db = C#"
    "Eb = D#"
    "Gb = F#"
    */
    
    /*
    m_majors = QString("C,G,D,A,E,B,F#,Db,Ab,Eb,Bb,F").split(",");
    m_minors = QString("Am,Em,Bm,F#m,Dbm,Abm,Ebm,Bbm,Fm,Cm,Gm,Dm").split(",");
    m_semitoneList = QString("C,Cm,C#,C#m,D,Dm,Eb,Ebm,E,Em,F,Fm,F#,F#m,G,Gm,G#,G#m,A,Am,Bb,Bbm,B,Bm").split(",");
    */

    if (m_enharmonic_preference == "#") {
        // prefer #'s
        m_majors = QString("C,G,D,A,E,B,F#,C#,G#,D#,A#,F").split(",");
        m_minors = QString("Am,Em,Bm,F#m,C#m,G#m,D#m,A#m,Fm,Cm,Gm,Dm").split(",");
        m_semitoneList = QString("C,Cm,C#,C#m,D,Dm,D#,D#m,E,Em,F,Fm,F#,F#m,G,Gm,G#,G#m,A,Am,A#,A#m,B,Bm").split(",");
    } else {
        // prefer b's
        m_majors = QString("C,G,D,A,E,B,Gb,Db,Ab,Eb,Bb,F").split(",");
        m_minors = QString("Am,Em,Bm,Gbm,Dbm,Abm,Ebm,Bbm,Fm,Cm,Gm,Dm").split(",");
        m_semitoneList = QString("C,Cm,Db,Dbm,D,Dm,Eb,Ebm,E,Em,F,Fm,Gb,Gbm,G,Gm,Ab,Abm,A,Am,Bb,Bbm,B,Bm").split(",");
    }


    // OK notation
    m_majors = QString("1,2,3,4,5,6,7,8,9,10,11,12").split("d,");
    m_minors = QString("1,2,3,4,5,6,7,8,9,10,11,12").split("m,");
    m_semitoneList = QString("1d,10m,8d,5m,3d,12m,10d,7m,5d,2m,12d,9m,7d,4m,2d,11m,9d,6m,4d,1m,11d,8m,6d,Bm").split(",");

QHash<QString, QStringList> hash;

hash["1d"] = "C";
hash["2d"] = "G";
hash["3d"] = "D";
hash["4d"] = "A";
hash["5d"] = "E";
hash["6d"] = "B";
hash["7d"] = "F#,Gb";
hash["8d"] = "C#,Db";
hash["9d"] = "G#,Ab";
hash["10d"] = "D#,Eb";
hash["11d"] = "A#,Bb";
hash["12d"] = "F";

hash["1m"] = "Am";
hash["2m"] = "Em";
hash["3m"] = "Bm";
hash["4m"] = "F#m,Gbm";
hash["5m"] = "C#m,Dbm";
hash["6m"] = "G#m,Abm";
hash["7m"] = "D#m,Ebm";
hash["8m"] = "A#m,Bbm";
hash["9m"] = "Fm";
hash["10m"] = "Cm";
hash["11m"] = "Gm";
hash["12m"] = "Dm";


}