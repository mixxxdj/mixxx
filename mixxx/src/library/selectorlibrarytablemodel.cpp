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


}

SelectorLibraryTableModel::~SelectorLibraryTableModel() {
}

bool SelectorLibraryTableModel::isColumnInternal(int column) {
    return LibraryTableModel::isColumnInternal(column);
}

void SelectorLibraryTableModel::slotPlayingDeckChanged(int deck) {


    // TODO - move this somewhere sensible...
    QHash<QString, QString> harmonics;
    harmonics["Abm"] = "'Dbm', 'C#m', 'B',  'Ebm', 'D#m'";
    harmonics["G#m"] = harmonics["Abm"];
    harmonics["Ebm"] = "'Abm', 'G#m', 'F#', 'Bbm', 'A#m'";
    harmonics["D#m"] = harmonics["Ebm"];
    harmonics["Bbm"] = "'Ebm', 'D#m', 'Db', 'C#',  'Fm'";
    harmonics["A#m"] = harmonics["Bbm"];
    harmonics["Fm"]  = "'Bbm', 'A#m', 'Ab', 'G#',  'Cm'";
    harmonics["Cm"]  = "'Fm',  'Eb',  'D#', 'Gm'";
    harmonics["Gm"]  = "'Cm',  'Bb',  'A#', 'Dm'";
    harmonics["Dm"]  = "'Gm',  'F',   'Am'";
    harmonics["Am"]  = "'Dm',  'C',   'Em'";
    harmonics["Em"]  = "'Am',  'G',   'Bm'";
    harmonics["Bm"]  = "'Em',  'D',   'F#m', 'Gbm'";
    harmonics["F#m"] = "'Bm',  'A',   'Dbm', 'C#m'";
    harmonics["Gbm"] = harmonics["F#m"];
    harmonics["Dbm"] = "'F#m', 'Gbm', 'F#m', 'E', 'Abm', 'G#m'";
    harmonics["C#m"] = harmonics["Dbm"];

    harmonics["B"]  = "'E',  'Abm', 'G#m', 'F#'";
    harmonics["F#"] = "'B',  'Ebm', 'D#m', 'Db', 'C#'";
    harmonics["Db"] = "'F#', 'Bbm', 'A#m', 'Ab', 'G#'";
    harmonics["C#"] = harmonics["Db"];
    harmonics["Ab"] = "'Db', 'C#',  'Fm',  'Eb', 'D#'";
    harmonics["G#"] = harmonics["Ab"];
    harmonics["Eb"] = "'Ab', 'G#',  'Cm',  'Bb'";
    harmonics["D#"] = harmonics["Eb"];
    harmonics["Bb"] = "'Eb', 'D#',  'Gm',  'F'";
    harmonics["A#"] = harmonics["Bb"];
    harmonics["F"]  = "'Bb', 'Dm',  'C'";
    harmonics["C"]  = "'F',  'Am',  'G'";
    harmonics["G"]  = "'C',  'Em',  'D'";
    harmonics["D"]  = "'G',  'Bm',  'A'";
    harmonics["A"]  = "'D',  'F#m', 'Gbm', 'E'";
    harmonics["E"]  = "'A',  'Dbm', 'C#m', 'B'";

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
        float TrackBpm = m_pLoadedTrack->getBpm();
        float bpm = TrackBpm * (1 + rateSlider->get() * rateRange->get() * rateDirection->get());
        //float TrackBpm = pChannel1Bpm->get();
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
        QString keys = harmonics[TrackKey];
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
    ControlObjectThreadMain* pChannel1Bpm = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(m_pChannel, "bpm")));

    float TrackBpm = m_pLoadedTrack->getBpm();
    float bpm = TrackBpm * (1 + rateSlider->get() * rateRange->get() * rateDirection->get());
    //float TrackBpm = pChannel1Bpm->get();
    m_pFilterBpm = (bpm > 0) ? QString(
        "Bpm > %1 AND Bpm < %2").arg(
        bpm - 1).arg(bpm + 1) : QString();

    //qDebug() << "getActiveProperties(" << deck;
    emit(doSearch(""));
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
