// Created 3/17/2012 by Keith Salisbury (keithsalisbury@gmail.com)

#include <QObject>

#include "selectorlibrarytablemodel.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "basetrackplayer.h"
#include "playerinfo.h"

#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "trackinfoobject.h"

#include "track/keyutils.h"

using mixxx::track::io::key::ChromaticKey;
using mixxx::track::io::key::ChromaticKey_IsValid;

const bool sDebug = true;
const QString tableName = "selector_table";
const double maxBpmDiff = 10.0;

SelectorLibraryTableModel::SelectorLibraryTableModel(QObject* parent,
                                                   TrackCollection* pTrackCollection)
        : LibraryTableModel(parent, pTrackCollection,
                            "mixxx.db.model.selector") {
    setTableModel();

    // Detect when deck has changed
    connect(&PlayerInfo::Instance(), SIGNAL(currentPlayingDeckChanged(int)),
           this, SLOT(slotPlayingDeckChanged(int)));

    connect(this, SIGNAL(filtersChanged()),
            this, SLOT(slotFiltersChanged()));

    connect(this, SIGNAL(resetFilters()),
            this, SLOT(slotResetFilters()));

    connect(this, SIGNAL(filtersChanged()),
            this, SLOT(slotFiltersChanged()));

    m_channelBpm = NULL;
    m_channelKey = NULL;
    m_bActive = false;

    slotResetFilters();
    clearSeedTrackInfo();
}

SelectorLibraryTableModel::~SelectorLibraryTableModel() {
}


void SelectorLibraryTableModel::setTableModel(int id){
    Q_UNUSED(id);

    QStringList columns;
    columns << "library."+LIBRARYTABLE_ID << "'' as preview" << "0.0 as score";

    QSqlQuery query(m_pTrackCollection->getDatabase());
    QString queryString = "CREATE TEMPORARY TABLE IF NOT EXISTS "+tableName+" AS "
            "SELECT " + columns.join(", ") +
            " FROM library INNER JOIN track_locations "
            "ON library.location = track_locations.id "
            "WHERE (" + LibraryTableModel::DEFAULT_LIBRARYFILTER + ")";
    query.prepare(queryString);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    QStringList tableColumns;
    tableColumns << LIBRARYTABLE_ID;
    tableColumns << "preview";
    tableColumns << "score";
    setTable(tableName, LIBRARYTABLE_ID, tableColumns,
             m_pTrackCollection->getTrackSource("default"));

    initHeaderData();

    setSearch("");
    setDefaultSort(fieldIndex("score"), Qt::DescendingOrder);
}

bool SelectorLibraryTableModel::isColumnInternal(int column) {
    return LibraryTableModel::isColumnInternal(column);
}

int SelectorLibraryTableModel::rowCount() {
    return BaseSqlTableModel::rowCount();
}

void SelectorLibraryTableModel::active(bool value) {
    m_bActive = value;
}

void SelectorLibraryTableModel::setSeedTrack(TrackPointer pSeedTrack) {
    m_pSeedTrack = pSeedTrack;
    if (!m_pSeedTrack.isNull()) {
        m_sSeedTrackInfo = m_pSeedTrack->getInfo();
        m_sSeedTrackGenre = m_pSeedTrack->getGenre();
        m_fSeedTrackBpm = m_pSeedTrack->getBpm();
        m_seedTrackKey = m_pSeedTrack->getKey();
    } else {
        clearSeedTrackInfo();
        emit(resetFilters());
    }
    emit(seedTrackInfoChanged());
}

QString SelectorLibraryTableModel::getSeedTrackInfo() {
    return m_sSeedTrackInfo;
}

bool SelectorLibraryTableModel::seedTrackGenreExists() {
    return m_sSeedTrackGenre != QString();
}

bool SelectorLibraryTableModel::seedTrackBpmExists() {
    return m_fSeedTrackBpm > 0;
}

bool SelectorLibraryTableModel::seedTrackKeyExists() {
    return ChromaticKey_IsValid(m_seedTrackKey) &&
               m_seedTrackKey != mixxx::track::io::key::INVALID;
}

void SelectorLibraryTableModel::calculateSimilarity() {
    qDebug() << "SelectorLibraryTableModel::calculateSimilarity()";
    if (!m_pSeedTrack.isNull()) {
        QSqlQuery query(m_pTrackCollection->getDatabase());
        query.prepare("UPDATE " + tableName + " SET score=:score "
                      "WHERE " + LIBRARYTABLE_ID + "=:id;");
        QVariantList scores;
        QVariantList trackIds;

        for (int i = 0, n = rowCount(); i < n; i++) {
            QModelIndex index = createIndex(i, fieldIndex(LIBRARYTABLE_ID));
            int trackId = getTrackId(index);
            QVariant score = scoreTrack(index);

            // if the score could not be calculated (i.e. a field is missing),
            // skip it
            if (!score.isValid())
                continue;

            trackIds << QVariant(trackId);
            scores << score;
        }

        query.bindValue(":score", scores);
        query.bindValue(":id", trackIds);
        if (!query.execBatch())
            qDebug() << query.lastError();
        else
            select(); // update the view
    }
}

void SelectorLibraryTableModel::filterByGenre(bool value) {
    m_bFilterGenre = value;
    updateFilterText();
}

void SelectorLibraryTableModel::filterByBpm(bool value, int range) {
    m_bFilterBpm = value;
    m_iFilterBpmRange = range;
    updateFilterText();
}


void SelectorLibraryTableModel::filterByKey(bool value) {
    m_bFilterKey = value;
    updateFilterText();
}

void SelectorLibraryTableModel::filterByKey4th(bool value) {
    m_bFilterKey4th = value;
    updateFilterText();
}

void SelectorLibraryTableModel::filterByKey5th(bool value) {
    m_bFilterKey5th = value;
    updateFilterText();
}

void SelectorLibraryTableModel::filterByKeyRelative(bool value) {
    m_bFilterKeyRelative = value;
    updateFilterText();
}

// PRIVATE SLOTS

void SelectorLibraryTableModel::slotPlayingDeckChanged(int deck) {
    if (deck > 0) {
        m_pChannel = QString("[Channel%1]").arg(deck);

        // disconnect the old pitch slider
        if (m_channelBpm) {
            disconnect(m_channelBpm, 0, this, 0);
        }

        if (m_channelKey) {
            disconnect(m_channelKey, 0, this, 0);
        }

        // get the new pitch slider object
        m_channelBpm = new ControlObjectThreadMain(
                    ControlObject::getControl(ConfigKey(m_pChannel, "bpm")));

        m_channelKey = new ControlObjectThreadMain(
                    ControlObject::getControl(ConfigKey(m_pChannel, "key")));

        // listen for slider change events
        connect(m_channelBpm, SIGNAL(valueChanged(double)), this,
            SLOT(slotChannelBpmChanged(double)));
        connect(m_channelKey, SIGNAL(valueChanged(double)), this,
            SLOT(slotChannelKeyChanged(double)));

        m_pLoadedTrack = PlayerInfo::Instance().getTrackInfo(m_pChannel);
    } else {
        m_pLoadedTrack = TrackPointer();
    }

    setSeedTrack(m_pLoadedTrack);
    emit(seedTrackInfoChanged());
    updateFilterText();
}

void SelectorLibraryTableModel::slotChannelBpmChanged(double value) {
    qDebug() << "BPM changed to " << value;
    if (m_pLoadedTrack == m_pSeedTrack) {
        m_fSeedTrackBpm = value;
    }
    updateFilterText();
}

void SelectorLibraryTableModel::slotChannelKeyChanged(double value) {
    qDebug() << "Key changed to " << value;
    if (m_pLoadedTrack == m_pSeedTrack) {
        m_seedTrackKey = KeyUtils::keyFromNumericValue(value);
    }
    updateFilterText();
}

void SelectorLibraryTableModel::slotFiltersChanged() {
    search("");
}

void SelectorLibraryTableModel::slotResetFilters() {
    m_bFilterGenre = false;
    m_bFilterBpm = false;
    m_iFilterBpmRange = 0;
    m_bFilterKey = false;
    m_bFilterKey4th = false;
    m_bFilterKey5th = false;
    m_bFilterKeyRelative = false;
    m_filterString = QString();
}

void SelectorLibraryTableModel::search(const QString& text) {
    setSearch(text, m_filterString);
    select();
}

// PRIVATE METHODS

void SelectorLibraryTableModel::initHeaderData() {
    // call the base class method first
    BaseSqlTableModel::initHeaderData();
    setHeaderData(fieldIndex("score"),
                  Qt::Horizontal, tr("Score"));
}

void SelectorLibraryTableModel::clearSeedTrackInfo() {
    m_sSeedTrackInfo = QString();
    m_sSeedTrackGenre = QString();
    m_fSeedTrackBpm = 0;
    m_seedTrackKey = mixxx::track::io::key::INVALID;
}

QVariant SelectorLibraryTableModel::scoreTrack(const QModelIndex& index) {
    // return a QVariant::Double from 0 (no match) to 100 (perfect match)
    // or QVariant::Invalid if the field is missing
    // assume that seed track info is valid

    QVariant score;

    QVariant bpm = data(index.sibling(index.row(), fieldIndex("bpm")));
    if (bpm.isValid()) {
        double bpmDiff = abs(bpm.toDouble() - m_fSeedTrackBpm);
        double scoreValue = ((maxBpmDiff - bpmDiff) / maxBpmDiff) * 100.0;

        if (scoreValue > 100.0) scoreValue = 100.0;
        else if (scoreValue < 0.0) scoreValue = 0.0;

        score.setValue(scoreValue);
    }

    return score;
}

void SelectorLibraryTableModel::updateFilterText() {
	if (!m_bActive) return;
    if (m_pSeedTrack) {
		QStringList filters;

        // Genre
        if (m_bFilterGenre) {
            QString TrackGenre = m_sSeedTrackGenre;
            if (TrackGenre != "")
                filters << QString("Genre == '%1'").arg(TrackGenre);
        }

        // BPM
        if (m_bFilterBpm) {
            if (m_fSeedTrackBpm > 0)
                filters << QString("(Bpm > %1 AND Bpm < %2)").arg(
                    floor(m_fSeedTrackBpm - m_iFilterBpmRange)).arg(
                    ceil(m_fSeedTrackBpm + m_iFilterBpmRange));
        } 

        // Keys

        QList<ChromaticKey> hKeys = getHarmonicKeys(m_seedTrackKey);

        if (!hKeys.isEmpty()) {
            // string business is a hack until filters use ChromaticKey directly
            QStringList keyNames;

            while (!hKeys.isEmpty()) {
                ChromaticKey key = hKeys.takeFirst();
                keyNames.append(QString("'%1'").arg(KeyUtils::keyToString(key)));
            }
            if (!keyNames.isEmpty()) {
                QString keyString = keyNames.join(",");
                qDebug() << "Match keys " << keyString;
                filters << QString("Key in (%1)").arg(keyString);
            }
        }
                    
        QString filterString = filters.join(" AND ");
        if (m_filterString != filterString) {
            m_filterString = filterString;
            emit(filtersChanged());
        }

    } else { // no seed track
        emit(filtersChanged());
    }
}

QList<ChromaticKey> SelectorLibraryTableModel::getHarmonicKeys(ChromaticKey key) {
    QList<ChromaticKey> keys;

    if (seedTrackKeyExists()) {
        if (m_bFilterKey) {
            keys.append(key);
        }
        if (m_bFilterKey4th) {
            keys.append(KeyUtils::scaleKeySteps(key, 5));
        }
        if (m_bFilterKey5th) {
            keys.append(KeyUtils::scaleKeySteps(key, 7));
        }
        if (m_bFilterKeyRelative) {
            keys.append(KeyUtils::keyToRelativeMajorOrMinor(key));
        }
    }

    return keys;
}
