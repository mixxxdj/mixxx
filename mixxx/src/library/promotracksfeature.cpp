/***************************************************************************
                          promotracksfeature.cpp
                             -------------------
    begin                : Jan 2010
    copyright            : (C) 2010 Albert Santoni
    email                : alberts@mixxx.org
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QtDebug>

#include "library/songdownloader.h"
#include "library/promotracksfeature.h"
#include "library/bundledsongswebview.h"
#include "library/featuredartistswebview.h"
#include "library/proxytrackmodel.h"
#include "library/trackcollection.h"
#include "library/dao/cratedao.h"
#include "trackinfoobject.h"
#include "defs_version.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "mixxxkeyboard.h"

QString PromoTracksFeature::m_sPromoLocalHTMLLocation;
QString PromoTracksFeature::m_sPromoRemoteHTMLLocation;
#define PROMO_BUNDLE_PATH (config->getConfigPath() + "/promo/" + VERSION + "/")
#define LOCAL_HTML_LOCATION (PROMO_BUNDLE_PATH + "index.html")

const QString PromoTracksFeature::m_sFeaturedArtistsViewName = tr("Featured Artists");
const QString PromoTracksFeature::m_sBundledSongsViewName = tr("Bundled Songs");
const QString PromoTracksFeature::m_sMyDownloadsViewName = tr("My Downloads");

PromoTracksFeature::PromoTracksFeature(QObject* parent,
                             ConfigObject<ConfigValue>* config,
                             TrackCollection* pTrackCollection,
                             bool firstRun)
        : LibraryFeature(parent),
          m_pConfig(config),
          m_pFeaturedArtistsView(NULL),
          m_pBundledSongsView(NULL),
          m_pTrackCollection(pTrackCollection),
          m_downloadsTableModel(this, pTrackCollection),
          m_bFirstRun(firstRun) {

    m_sPromoRemoteHTMLLocation = QString("http://promo.mixxx.org/%1/index.html").arg(VERSION); //m_pConfig->getConfigPath() + "/promo/promotracks.html";
    m_sPromoLocalHTMLLocation = LOCAL_HTML_LOCATION;
    m_sPromoAutoloadLocation = m_pConfig->getConfigPath() + "/promo/" + VERSION + "/autoload.dat";

    //Load the extra.dat file so we can peek at some extra information, such
    //as which songs to auto-load into Mixxx's players.
    QFile file(m_sPromoAutoloadLocation);
    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream extra(&file);

        qDebug() << "PROMO: Autoload" << (file.exists() ? "" : "not") << "found";
        while (!extra.atEnd())
        {
            QString trackPath = extra.readLine();
            trackPath = m_pConfig->getConfigPath() + "/promo/" + VERSION + "/" + trackPath;
            qDebug() << "PROMO: Auto-loading track" << trackPath;
            m_tracksToAutoLoad.append(new TrackInfoObject(trackPath));
        }
        file.close();
    }


    /*  XXX: Re-enable all this code to get the tree children back for Promo 3.0
    //XXX: Factor this out and put it in bundledsongsfeature.cpp
    //If we've bundled songs with Mixxx, show the fancy bundled songs view
    if (QFile::exists(LOCAL_HTML_LOCATION)) {
        qDebug() << "Bundled tracks found at:" << LOCAL_HTML_LOCATION;
        childrenStringList << m_sBundledSongsViewName;
    }
    else {
        qDebug() << "No bundled tracks found, disabling view. Looked in:" << LOCAL_HTML_LOCATION;
    }

    QStringList childrenStringList;
    childrenStringList <<  m_sMyDownloadsViewName;
    m_childModel.setStringList(childrenStringList);

    CrateDAO& crateDAO = pTrackCollection->getCrateDAO();
    crateDAO.createCrate(m_sMyDownloadsViewName); //XXX: hidden = false for debug
    m_downloadsTableModel.setTable(m_sMyDownloadsViewName);
    */

}

PromoTracksFeature::~PromoTracksFeature() {
}

QVariant PromoTracksFeature::title() {
    return m_sFeaturedArtistsViewName;
}

QIcon PromoTracksFeature::getIcon() {
    return QIcon();
}

bool PromoTracksFeature::isSupported(ConfigObject<ConfigValue>* config) {
    m_sPromoLocalHTMLLocation = LOCAL_HTML_LOCATION;
    qDebug() << "Promo dir:" << m_sPromoLocalHTMLLocation;
    return (QFile::exists(m_sPromoLocalHTMLLocation));
}

QList<TrackInfoObject*> PromoTracksFeature::getTracksToAutoLoad()
{
    return m_tracksToAutoLoad;
}

void PromoTracksFeature::bindWidget(WLibrarySidebar* sidebarWidget,
                                    WLibrary* libraryWidget,
                                    MixxxKeyboard* keyboard) {
    QString libraryPath = m_pConfig->getValueString(ConfigKey("[Playlist]","Directory"));

    ConfigObject<ConfigValue>* config = m_pConfig; //Long story, macros macros macros
    m_pBundledSongsView = new BundledSongsWebView(libraryWidget, PROMO_BUNDLE_PATH, m_sPromoLocalHTMLLocation, m_bFirstRun, m_pConfig);

    libraryWidget->registerView(m_sBundledSongsViewName, m_pBundledSongsView);
    connect(m_pBundledSongsView, SIGNAL(loadTrack(TrackInfoObject*)),
            this, SIGNAL(loadTrack(TrackInfoObject*)));
    connect(m_pBundledSongsView, SIGNAL(loadTrackToPlayer(TrackInfoObject*, int)),
            this, SIGNAL(loadTrackToPlayer(TrackInfoObject*, int)));

/*  XXX: Re-enable this code for Promo 3.0
    m_pFeaturedArtistsView = new FeaturedArtistsWebView(libraryWidget, libraryPath, m_sPromoRemoteHTMLLocation, new SongDownloader(this)); 
    libraryWidget->registerView(m_sFeaturedArtistsViewName, m_pFeaturedArtistsView);
    connect(m_pFeaturedArtistsView, SIGNAL(loadTrack(TrackInfoObject*)),
            this, SIGNAL(loadTrack(TrackInfoObject*)));
    connect(m_pFeaturedArtistsView, SIGNAL(loadTrackToPlayer(TrackInfoObject*, int)),
            this, SIGNAL(loadTrackToPlayer(TrackInfoObject*, int)));
    */
}

QAbstractItemModel* PromoTracksFeature::getChildModel() {
    //XXX Promo 3.0:
    //return NULL;
    return &m_childModel;
}

void PromoTracksFeature::activate() {
    //XXX Promo 3.0:
    //emit(switchToView(m_sFeaturedArtistsViewName));
    emit(switchToView(m_sBundledSongsViewName));
}

void PromoTracksFeature::activateChild(const QModelIndex& index) {
    QString itemString = m_childModel.data(index, Qt::DisplayRole).toString();
    if (itemString == m_sMyDownloadsViewName)
    {
        emit(showTrackModel(&m_downloadsTableModel));
    }
    else
        emit(switchToView(itemString));
}

void PromoTracksFeature::onRightClick(const QPoint& globalPos) {
}

void PromoTracksFeature::onRightClickChild(const QPoint& globalPos,
                                            QModelIndex index) {
}

bool PromoTracksFeature::dropAccept(QUrl url) {
    return false;
}

bool PromoTracksFeature::dropAcceptChild(const QModelIndex& index, QUrl url) {
    return false;
}

bool PromoTracksFeature::dragMoveAccept(QUrl url) {
    return false;
}

bool PromoTracksFeature::dragMoveAcceptChild(const QModelIndex& index,
                                              QUrl url) {
    return false;
}
