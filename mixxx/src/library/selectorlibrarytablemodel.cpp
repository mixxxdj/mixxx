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
    harmonics["Abm"] = "'Dbm', 'B',  'Ebm'";
    harmonics["Ebm"] = "'Abm', 'F#', 'Bbm'";
    harmonics["Bbm"] = "'Ebm', 'Db', 'Fm'";
    harmonics["Fm"]  = "'Bbm', 'Ab', 'Cm'";
    harmonics["Cm"]  = "'Fm',  'Eb', 'Gm'";
    harmonics["Gm"]  = "'Cm',  'Bb', 'Dm'";
    harmonics["Dm"]  = "'Gm',  'F',  'Am'";
    harmonics["Am"]  = "'Dm',  'C',  'Em'";
    harmonics["Em"]  = "'Am',  'G',  'Bm'";
    harmonics["Bm"]  = "'Em',  'D',  'F#m'";
    harmonics["F#m"] = "'Bm',  'A',  'Dbm'";
    harmonics["Dbm"] = "'F#m', 'E',  'Abm'";

    harmonics["B"]  = "'E',  'Abm', 'F#'";
    harmonics["F#"] = "'B',  'Ebm', 'Db'";
    harmonics["Db"] = "'F#', 'Bbm', 'Ab'";
    harmonics["Ab"] = "'Db', 'Fm',  'Eb'";
    harmonics["Eb"] = "'Ab', 'Cm',  'Bb'";
    harmonics["Bb"] = "'Eb', 'Gm',  'F'";
    harmonics["F"]  = "'Bb', 'Dm',  'C'";
    harmonics["C"]  = "'F',  'Am',  'G'";
    harmonics["G"]  = "'C',  'Em',  'D'";
    harmonics["D"]  = "'G',  'Bm',  'A'";
    harmonics["A"]  = "'D',  'F#m', 'E'";
    harmonics["E"]  = "'A',  'Dbm', 'B'";

    TrackPointer loaded_track = PlayerInfo::Instance(
        ).getTrackInfo(QString("[Channel%1]").arg(deck));
    if (loaded_track) {

        // Genre
        QString TrackGenre = loaded_track->getGenre();
        m_pFilterGenre = (TrackGenre != "") ? QString(
            "Genre == '%1'").arg(TrackGenre) : QString();

        // Bpm
        float TrackBpm = loaded_track->getBpm();
        m_pFilterBpm = (TrackBpm > 0) ? QString(
            "Bpm > %1 AND Bpm < %2").arg(
            TrackBpm - 1).arg(TrackBpm + 1) : QString();

        // Year
        QString TrackYear = loaded_track->getYear();
        m_pFilterYear = (TrackYear!="") ? QString(
            "Year == '%1'").arg(TrackYear) : QString();

        // Rating
        int TrackRating = loaded_track->getRating();
        m_pFilterRating = (TrackRating > 0) ? QString(
            "Rating >= %1").arg(TrackRating) : QString();

        // Key
        QString TrackKey = loaded_track->getKey();
        m_pFilterKey = (TrackKey!="") ? QString(
            "Key == '%1'").arg(TrackKey) : QString();

        // Harmonic Key
        //QString TrackKey = loaded_track->getKey();
        QString keys = harmonics[TrackKey];
        m_pFilterHarmonicKey = (keys!="") ? QString(
            "Key in (%1)").arg(keys) : QString();

        // Hack

    }
    //qDebug() << "slotPlayingDeckChanged(" << deck;
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
