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
}


SelectorLibraryTableModel::~SelectorLibraryTableModel() {
}

bool SelectorLibraryTableModel::isColumnInternal(int column) {
    return LibraryTableModel::isColumnInternal(column);
}

void SelectorLibraryTableModel::slotPlayingDeckChanged(int deck) {
    TrackPointer loaded_track = PlayerInfo::Instance().getTrackInfo(QString("[Channel%1]").arg(deck));
    if (loaded_track) {
        if (m_bFilterGenre) {
            // Genre - need a bool for checkbox value
            QString TrackGenre = loaded_track->getGenre();
            m_pFilterGenre = QString(
                "Genre == '%1'").arg(TrackGenre);
        } else {
            m_pFilterGenre = QString("1");
        }
        if (m_bFilterBpm) {
            // Bpm
            float TrackBpm = loaded_track->getBpm();
            m_pFilterBpm = QString(
                "Bpm > %1 AND Bpm < %2").arg(
                TrackBpm - 1).arg(TrackBpm + 1);
        } else {
            m_pFilterBpm = QString("1"); 
        }
    }
    //qDebug() << "slotPlayingDeckChanged(" << deck 
    //    << ") m_pFilterGenre = " << m_pFilterGenre
    //    << ", m_pFilterBpm = " << m_pFilterBpm;
    emit(doSearch(""));
}

void SelectorLibraryTableModel::search(const QString& searchText) {
    // qDebug() << "SelectorLibraryTableModel::search()" << searchText
    //          << QThread::currentThread();
    emit(doSearch(searchText));
}

void SelectorLibraryTableModel::slotSearch(const QString& searchText) {
    QString filterText = "";
    QString filterTextA = (m_bFilterGenre) ? m_pFilterGenre : "";
    QString filterTextB = (m_bFilterBpm) ? m_pFilterBpm : "";
    QString filterJoin = (filterTextA != NULL && filterTextB != NULL) ? " AND " : "";
    filterText = filterTextA + filterJoin + filterTextB;

    qDebug() << "slotSearch()" << filterText;
    BaseSqlTableModel::search(searchText, filterText);
}

/*
void SelectorLibraryTableModel::slotSearch(const QString& searchText) {
    QString filterText = QString("%1 AND %2").arg(m_pFilterGenre,m_pFilterBpm);
    QString filterText = "";
    if (m_bFilterGenre) {
        filterText = m_pFilterGenre;
    }
    if (m_bFilterBpm) {
        if (filterText != "") {
            filterText << " AND ";
        } 
        filterText << m_pFilterGenre;
    }
    qDebug() << "slotSearch() filterText = " << filterText; 
    BaseSqlTableModel::search(searchText, filterText);
}*/

void SelectorLibraryTableModel::filterByGenre(bool value) {
    m_bFilterGenre = value;
    qDebug() << "filterByGenre(" << value << ")";
    emit(doSearch(QString()));
}

void SelectorLibraryTableModel::filterByBpm(bool value) {
    m_bFilterBpm = value;
    qDebug() << "filterByBpm(" << value << ")";
    emit(doSearch(QString()));
}
