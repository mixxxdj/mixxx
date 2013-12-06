// Created 3/17/2012 by Keith Salisbury (keithsalisbury@gmail.com)

#include <QObject>
#include <QStringBuilder>

#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "basetrackplayer.h"
#include "playerinfo.h"

#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "trackinfoobject.h"

#include "track/keyutils.h"
#include "track/timbreutils.h"

#include "util/timer.h"

#include "library/selector/selector_preferences.h"
#include "library/selector/selectorsimilarity.h"
#include "library/selector/selectorlibrarytablemodel.h"

using mixxx::track::io::key::ChromaticKey;
using mixxx::track::io::key::ChromaticKey_IsValid;

SelectorLibraryTableModel::SelectorLibraryTableModel(QObject* parent,
                                                     ConfigObject<ConfigValue>* pConfig,
                                                     TrackCollection* pTrackCollection)
                         : LibraryTableModel(parent, pTrackCollection,
                                             "mixxx.db.model.selector"),
                           m_pConfig(pConfig),
                           m_selectorFilters(parent, pConfig),
                           m_selectorSimilarity(parent, pTrackCollection,
                                                pConfig, m_selectorFilters) {
    setTableModel();

    // Detect when deck has changed
    // connect(&PlayerInfo::Instance(), SIGNAL(currentPlayingDeckChanged(int)),
    //        this, SLOT(slotPlayingDeckChanged(int)));

    connect(this, SIGNAL(filtersChanged()),
            this, SLOT(slotFiltersChanged()));

    m_channelBpm = NULL;
    m_channelKey = NULL;
    m_bActive = false;

    clearSeedTrackInfo();
}

SelectorLibraryTableModel::~SelectorLibraryTableModel() {
}


void SelectorLibraryTableModel::setTableModel(int id){
    Q_UNUSED(id);

    QStringList columns;
    columns << "library." % LIBRARYTABLE_ID << "'' as preview" << "0.0 as score";

    QSqlQuery query(m_pTrackCollection->getDatabase());
    QString queryStringClear = "DROP TABLE IF EXISTS " SELECTOR_TABLE;
    QString queryStringCreate = "CREATE TEMPORARY TABLE " SELECTOR_TABLE " AS "
            "SELECT " % columns.join(", ") %
            " FROM library INNER JOIN track_locations "
            "ON library.location = track_locations.id "
            "WHERE (" % LibraryTableModel::DEFAULT_LIBRARYFILTER % ")";
    if (!query.exec(queryStringClear)) {
        LOG_FAILED_QUERY(query);
    }
    if (!query.exec(queryStringCreate)) {
        LOG_FAILED_QUERY(query);
    }

    QStringList tableColumns;
    tableColumns << LIBRARYTABLE_ID;
    tableColumns << "preview";
    tableColumns << "score";
    setTable(SELECTOR_TABLE, LIBRARYTABLE_ID, tableColumns,
             m_pTrackCollection->getTrackSource("default"));

    initHeaderData();

    setSearch("");
    setDefaultSort(fieldIndex("score"), Qt::DescendingOrder);
}

void SelectorLibraryTableModel::active(bool value) {
    m_bActive = value;
}

void SelectorLibraryTableModel::setSeedTrack(TrackPointer pSeedTrack) {
    setTableModel();
    m_pSeedTrack = pSeedTrack;
    if (!m_pSeedTrack.isNull()) {
        m_sSeedTrackInfo = m_pSeedTrack->getInfo();
        m_sSeedTrackGenre = m_pSeedTrack->getGenre();
        m_fSeedTrackBpm = m_pSeedTrack->getBpm();
        m_seedTrackKey = m_pSeedTrack->getKey();
        m_pSeedTrackTimbre = m_pSeedTrack->getTimbre();
    } else {
        clearSeedTrackInfo();
    }
    emit(loadStoredFilterSettings());
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
    if (!m_pSeedTrack.isNull()) {
//        qDebug() << m_selectorSimilarity.getTopFollowupTrack(m_pSeedTrack->getId());


        QList<int> trackIds;
        for (int i = 0, n = rowCount(); i < n; i++) {
            trackIds << index(i, fieldIndex(LIBRARYTABLE_ID)).data().toInt();
        }

        QList<ScorePair> results =
            m_selectorSimilarity.calculateSimilarities(m_pSeedTrack->getId(),
                                                       trackIds);

        QVariantList queryTrackIds;
        QVariantList queryScores;
        foreach (ScorePair pair, results) {
            queryTrackIds << pair.id;
            queryScores << pair.score;
        }

        QSqlQuery query(m_pTrackCollection->getDatabase());
        query.prepare("UPDATE " SELECTOR_TABLE " SET score=:score "
                      "WHERE " % LIBRARYTABLE_ID % "=:id;");
        query.bindValue(":score", queryScores);
        query.bindValue(":id", queryTrackIds);
        if (!query.execBatch()) {
            qDebug() << query.lastError();
        } else {
            select(); // update the view
        }
    }
}

void SelectorLibraryTableModel::calculateAllSimilarities(
        const QString& filename) {
    QTime timer;
    timer.start();

    QFile file(filename);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);

    QStringList similarityTypes = m_selectorSimilarity.getSimilarityTypes();

    // output csv header
    out << "filename1,filename2," % similarityTypes.join(",") % "\n";

    for (int i = 0; i < rowCount(); i++) {
        TrackPointer pTrack_i = getTrack(index(i, fieldIndex(LIBRARYTABLE_ID)));
        QString sTrack_i = pTrack_i->getFilename();
        for (int j = i + 1; j < rowCount(); j++) {
            TrackPointer pTrack_j = getTrack(index(j, fieldIndex(LIBRARYTABLE_ID)));
            QString sTrack_j = pTrack_j->getFilename();
            QHash<QString, double> comparison =
                    m_selectorSimilarity.compareTracks(pTrack_i, pTrack_j);

            QStringList scores;
            foreach (QString similarityType, similarityTypes) {
                scores << QString::number(comparison[similarityType]);
            }

            out << sTrack_i % "," % sTrack_j % "," %
                   scores.join(",") % "\n";
            if (i % 100 == 0 && j % 100 == 0) {
                qDebug() << QString::number(i*j) << "/" <<
                            QString::number(rowCount()*rowCount()) <<
                            "processed";
            }
        }
    }

    file.close();
    qDebug() << "calculateAllSimilarities:" << timer.elapsed() << "ms";
}

SelectorFilters& SelectorLibraryTableModel::getFilters() {
    return m_selectorFilters;
}

void SelectorLibraryTableModel::applyFilters() {
    updateFilterText();
}

// PRIVATE SLOTS

void SelectorLibraryTableModel::slotPlayingDeckChanged(int deck) {
    m_pChannel = QString("[Channel%1]").arg(deck + 1);

    // disconnect the old pitch slider
    if (m_channelBpm) {
        disconnect(m_channelBpm, 0, this, 0);
    }

    if (m_channelKey) {
        disconnect(m_channelKey, 0, this, 0);
    }

    if (deck >= 0) {
        m_channelBpm = new ControlObjectThreadMain(m_pChannel, "bpm");
        m_channelKey = new ControlObjectThreadMain(m_pChannel, "key");

        // listen for slider change events
        connect(m_channelBpm, SIGNAL(valueChanged(double)), this,
            SLOT(slotChannelBpmChanged(double)));
        connect(m_channelKey, SIGNAL(valueChanged(double)), this,
            SLOT(slotChannelKeyChanged(double)));
    }

    // m_pLoadedTrack = PlayerInfo::Instance().getCurrentPlayingTrack();

    setSeedTrack(m_pLoadedTrack);
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
    qDebug() << m_filterString;
    search("");
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
    m_pSeedTrackTimbre = TimbrePointer();
}

void SelectorLibraryTableModel::updateFilterText() {
    if (!m_bActive) return;
    QString filterString = m_selectorFilters.getFilterString(m_pSeedTrack);
    if (m_filterString != filterString) {
        m_filterString = filterString;
        emit(filtersChanged());
    }
}
