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
                            "mixxx.db.model.prepare") {

    connect(this, SIGNAL(doSearch(const QString&)),
            this, SLOT(slotSearch(const QString&)));

    // Getting info on current decks playing etc
    connect(&PlayerInfo::Instance(), SIGNAL(currentPlayingDeckChanged(int)),
           this, SLOT(slotPlayingDeckChanged(int)));

    m_bFilterGenre = true;
    m_bFilterBpm = false;
    m_bFilterYear = false;
    m_bFilterRating = false;
    m_bFilterKey = false;
    m_bFilterHarmonicKey = false;
    m_pChannelBpm = NULL;

    // TODO - move this somewhere sensible...
    m_pHarmonics["Abm"] = "'Dbm', 'C#m', 'B',  'Ebm', 'D#m'";
    m_pHarmonics["G#m"] = m_pHarmonics["Abm"];
    m_pHarmonics["Ebm"] = "'Abm', 'G#m', 'F#', 'Bbm', 'A#m'";
    m_pHarmonics["D#m"] = m_pHarmonics["Ebm"];
    m_pHarmonics["Bbm"] = "'Ebm', 'D#m', 'Db', 'C#',  'Fm'";
    m_pHarmonics["A#m"] = m_pHarmonics["Bbm"];
    m_pHarmonics["Fm"]  = "'Bbm', 'A#m', 'Ab', 'G#',  'Cm'";
    m_pHarmonics["Cm"]  = "'Fm',  'Eb',  'D#', 'Gm'";
    m_pHarmonics["Gm"]  = "'Cm',  'Bb',  'A#', 'Dm'";
    m_pHarmonics["Dm"]  = "'Gm',  'F',   'Am'";
    m_pHarmonics["Am"]  = "'Dm',  'C',   'Em'";
    m_pHarmonics["Em"]  = "'Am',  'G',   'Bm'";
    m_pHarmonics["Bm"]  = "'Em',  'D',   'F#m', 'Gbm'";
    m_pHarmonics["F#m"] = "'Bm',  'A',   'Dbm', 'C#m'";
    m_pHarmonics["Gbm"] = m_pHarmonics["F#m"];
    m_pHarmonics["Dbm"] = "'F#m', 'Gbm', 'F#m', 'E', 'Abm', 'G#m'";
    m_pHarmonics["C#m"] = m_pHarmonics["Dbm"];

    m_pHarmonics["B"]  = "'E',  'Abm', 'G#m', 'F#'";
    m_pHarmonics["F#"] = "'B',  'Ebm', 'D#m', 'Db', 'C#'";
    m_pHarmonics["Db"] = "'F#', 'Bbm', 'A#m', 'Ab', 'G#'";
    m_pHarmonics["C#"] = m_pHarmonics["Db"];
    m_pHarmonics["Ab"] = "'Db', 'C#',  'Fm',  'Eb', 'D#'";
    m_pHarmonics["G#"] = m_pHarmonics["Ab"];
    m_pHarmonics["Eb"] = "'Ab', 'G#',  'Cm',  'Bb'";
    m_pHarmonics["D#"] = m_pHarmonics["Eb"];
    m_pHarmonics["Bb"] = "'Eb', 'D#',  'Gm',  'F'";
    m_pHarmonics["A#"] = m_pHarmonics["Bb"];
    m_pHarmonics["F"]  = "'Bb', 'Dm',  'C'";
    m_pHarmonics["C"]  = "'F',  'Am',  'G'";
    m_pHarmonics["G"]  = "'C',  'Em',  'D'";
    m_pHarmonics["D"]  = "'G',  'Bm',  'A'";
    m_pHarmonics["A"]  = "'D',  'F#m', 'Gbm', 'E'";
    m_pHarmonics["E"]  = "'A',  'Dbm', 'C#m', 'B'";


}

SelectorLibraryTableModel::~SelectorLibraryTableModel() {
}

bool SelectorLibraryTableModel::isColumnInternal(int column) {
    return LibraryTableModel::isColumnInternal(column);
}

void SelectorLibraryTableModel::slotPlayingDeckChanged(int deck) {

    m_pChannel = QString("[Channel%1]").arg(deck);
    m_pLoadedTrack = PlayerInfo::Instance(
        ).getTrackInfo(m_pChannel);

    // get pitch slider value (deck rate)
    ControlObjectThreadMain* rateSlider = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(m_pChannel, "rate")));
    ControlObjectThreadMain* rateRange = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(m_pChannel, "rateRange")));
    ControlObjectThreadMain* rateDirection = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(m_pChannel, "rate_dir")));

    // disconnect the old pitch slider
    if (m_pChannelBpm) {
        m_pChannelBpm->disconnect(this);
    }
    // get the new pitch slider object
    m_pChannelBpm = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(m_pChannel, "bpm")));
    // listen for slider change events
    connect(m_pChannelBpm, SIGNAL(valueChanged(double)), this, 
        SLOT(slotChannel1BpmChanged(double)));

    //bpm = file_bpm * (1 + rateSlider * rateRange * rateDirection)
    //"rate", "rateRange", and "rate_dir"
    //see src/widget/wnumberrate.cpp for some stuff you could copy/paste

    if (m_pLoadedTrack) {

        // Genre
        QString TrackGenre = m_pLoadedTrack->getGenre();
        m_pFilterGenre = (TrackGenre != "") ? QString(
            "Genre == '%1'").arg(TrackGenre) : QString();

        // Bpm
        float trackBpm = m_pLoadedTrack->getBpm();
        float bpm = trackBpm * (1 + rateSlider->get() * rateRange->get() * rateDirection->get());
        //float trackBpm = pChannel1Bpm->get();
        m_pFilterBpm = (bpm > 0) ? QString(
            "Bpm > %1 AND Bpm < %2").arg(
            bpm - 1).arg(bpm + 1) : QString();

        // Year
        QString TrackYear = m_pLoadedTrack->getYear();
        m_pFilterYear = (TrackYear!="") ? QString(
            "Year == '%1'").arg(TrackYear) : QString();

        // Rating
        int TrackRating = m_pLoadedTrack->getRating();
        m_pFilterRating = (TrackRating > 0) ? QString(
            "Rating >= %1").arg(TrackRating) : QString();

        // Key
        QString TrackKey = m_pLoadedTrack->getKey();
        m_pFilterKey = (TrackKey!="") ? QString(
            "Key == '%1'").arg(TrackKey) : QString();

        // Harmonic Key
        //QString TrackKey = m_pLoadedTrack->getKey();
        QString keys = m_pHarmonics[TrackKey];
        m_pFilterHarmonicKey = (keys!="") ? QString(
            "Key in (%1)").arg(keys) : QString();

        // Hack

    }
    //qDebug() << "slotPlayingDeckChanged(" << deck;
    emit(doSearch(""));
}

void SelectorLibraryTableModel::slotChannel1BpmChanged(double value) {

    // get pitch slider value (deck rate)
    ControlObjectThreadMain* rateSlider = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(m_pChannel, "rate")));
    ControlObjectThreadMain* rateRange = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(m_pChannel, "rateRange")));
    ControlObjectThreadMain* rateDirection = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(m_pChannel, "rate_dir")));
    //ControlObjectThreadMain* pChannel1Bpm = new ControlObjectThreadMain(
    //    ControlObject::getControl(ConfigKey(m_pChannel, "bpm")));

    float trackBpm = m_pLoadedTrack->getBpm();
    float currentBpm = trackBpm * (1 + rateSlider->get() * rateRange->get() * rateDirection->get());
    //float trackBpm = pChannel1Bpm->get();
    m_pFilterBpm = (currentBpm > 0) ? QString(
        "Bpm > %1 AND Bpm < %2").arg(
        currentBpm - 1).arg(currentBpm + 1) : QString();

    float semitoneOffset = frequencyRatioToOctaveDifference(currentBpm, trackBpm);
    qDebug() << "slotChannelBpmChanged() semitones offset = " << semitoneOffset;

    QString TrackKey = adjustPitchBy(m_pLoadedTrack->getKey(), semitoneOffset);
    qDebug() << "slotChannelBpmChanged() adjusted key = " << TrackKey;
    // Key
    m_pFilterKey = (TrackKey!="") ? QString(
        "Key == '%1'").arg(TrackKey) : QString();

    // Harmonic Key
    //QString TrackKey = m_pLoadedTrack->getKey();
    QString keys = m_pHarmonics[TrackKey];
    m_pFilterHarmonicKey = (keys!="") ? QString(
        "Key in (%1)").arg(keys) : QString();

    //qDebug() << "getActiveProperties(" << deck;
    emit(doSearch(""));
}

QString SelectorLibraryTableModel::adjustPitchBy(QString pitch, int change) {
    if (pitch == "") return pitch;
    QList<QString> semitones_major;
    semitones_major <<"C"<<"C#"<<"D"<<"Eb"<<"E"<<"F"<<"F#"<<"G"<<"G#"<<"A"<<"Bb"<<"B";
    QList<QString> semitones_minor;
    semitones_minor <<"Cm"<<"C#m"<<"Dm"<<"Ebm"<<"Em"<<"Fm"<<"F#m"<<"Gm"<<"G#m"<<"Am"<<"Bbm"<<"Bm";

    qDebug() << "adjustPitchBy pitch = " << pitch;
    int position = semitones_major.indexOf(pitch);
    if (position>0){
        //qDebug() << "major adjustPitchBy position = " << position;
        int newpos = position + change;
        qDebug() << "major adjustPitchBy newpos = " << newpos;
        if (newpos >= 12) {
            newpos = newpos - 12;
        }
        QString newpitch = semitones_major.at(newpos);
        return newpitch;
    } else {
        position = semitones_minor.indexOf(pitch);
        if (position>0) {
            //qDebug() << "minor adjustPitchBy position = " << position;
            int newpos = position + change;
            qDebug() << "minor adjustPitchBy newpos = " << newpos;
            if (newpos >= 12) {
                newpos = newpos - 12;
            }
            QString newpitch = semitones_minor.at(newpos);
            return newpitch;
        } else {
            qDebug() << "Pitch " << pitch << " not found in config. Either change config or file meta data.";
            return pitch;
        }
    }
}

// calculate the new pitch
// To change pitch by one semitone, multiply or divide BPM by 
//    the 12th root of 2, depending on which direction you want
//    to go. The 12th root of 2 is approximately 1.0594631.
//    6% == 1 semitone
// float 12TH_ROOT_OF_2 = 2^(1/12); //1.05946309435929
float SelectorLibraryTableModel::frequencyRatioToOctaveDifference(
    float currentBpm, float originalBpm) {
    const int semitonesPerOctave = 12;
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
    if (m_bFilterBpm && m_pFilterBpm != "") { filters << m_pFilterBpm; }
    if (m_bFilterYear && m_pFilterYear != "") { filters << m_pFilterYear; }
    if (m_bFilterRating && m_pFilterRating != "") { filters << m_pFilterRating; }

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

void SelectorLibraryTableModel::filterByBpm(bool value) {
    m_bFilterBpm = value;
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
