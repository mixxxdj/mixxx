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
#include "library/trackcollection.h"
#include "library/dao/cratedao.h"
#include "trackinfoobject.h"
#include "defs_promo.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "mixxxkeyboard.h"

QString PromoTracksFeature::m_sPromoLocalHTMLLocation;
QString PromoTracksFeature::m_sPromoRemoteHTMLLocation;
#define PROMO_BUNDLE_PATH (config->getResourcePath() + "promo/" + MIXXX_PROMO_VERSION + "/")
#define LOCAL_HTML_LOCATION (PROMO_BUNDLE_PATH + "index.html")

const QString PromoTracksFeature::m_sFeaturedArtistsViewName = "Featured Artists";
const QString PromoTracksFeature::m_sBundledSongsViewName = "Bundled Songs";
const QString PromoTracksFeature::m_sMyDownloadsViewName = "My Downloads";

PromoTracksFeature::PromoTracksFeature(QObject* parent,
                             ConfigObject<ConfigValue>* config,
                             TrackCollection* pTrackCollection,
                             bool firstRun)
        : LibraryFeature(parent),
          m_pConfig(config),
          m_pTrackCollection(pTrackCollection),
          m_pFeaturedArtistsView(NULL),
          m_pBundledSongsView(NULL),
          m_downloadsTableModel(this, pTrackCollection),
          m_bFirstRun(firstRun) {

    m_sPromoRemoteHTMLLocation = QString("http://promo.mixxx.org/%1/index.html").arg(MIXXX_PROMO_VERSION); //m_pConfig->getConfigPath() + "/promo/promotracks.html";
    m_sPromoLocalHTMLLocation = LOCAL_HTML_LOCATION;
    m_sPromoAutoloadLocation = m_pConfig->getResourcePath() + "/promo/" + MIXXX_PROMO_VERSION + "/autoload.dat";

    //Load the extra.dat file so we can peek at some extra information, such
    //as which songs to auto-load into Mixxx's players.
    QFile file(m_sPromoAutoloadLocation);
    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream extra(&file);

        //qDebug() << "PROMO: Autoload" << (file.exists() ? "" : "not") << "found";
        while (!extra.atEnd())
        {
            QString trackPath = extra.readLine();
            trackPath = m_pConfig->getResourcePath() + "/promo/" + MIXXX_PROMO_VERSION + "/" + trackPath;
            QFileInfo fileInfo(trackPath);
            trackPath = fileInfo.absoluteFilePath();
            //qDebug() << "PROMO: Auto-loading track" << trackPath;

            // Try to get TrackInfoObject* from library, identified by location.
            TrackDAO& trackDao = m_pTrackCollection->getTrackDAO();
            TrackPointer pTrack = trackDao.getTrack(trackDao.getTrackId(trackPath));
            // If not, create a new TrackInfoObject*
            if (pTrack == NULL)
            {
                // TODO(XXX) These tracks are probably getting leaked b/c
                // m_tracksToAutoLoad is never cleared.
                pTrack = TrackPointer(new TrackInfoObject(trackPath), &QObject::deleteLater);
            }

            m_tracksToAutoLoad.append(pTrack);
        }
        file.close();
    }


    /*  XXX: Re-enable all this code to get the tree children back for Promo 3.0
    //XXX: Factor this out and put it in bundledsongsfeature.cpp
    //If we've bundled songs with Mixxx, show the fancy bundled songs view
    if (QFile::exists(LOCAL_HTML_LOCATION)) {
        qDebug() << "Bundled tracks found at:" << LOCAL_HTML_LOCATION;
        childrenStringList << tr(m_sBundledSongsViewName.toUtf8().constData());
    }
    else {
        qDebug() << "No bundled tracks found, disabling view. Looked in:" << LOCAL_HTML_LOCATION;
    }

    QStringList childrenStringList;
    childrenStringList <<  tr(m_sMyDownloadsViewName.toUtf8().constData());
    m_childModel.setStringList(childrenStringList);

    CrateDAO& crateDAO = pTrackCollection->getCrateDAO();
    crateDAO.createCrate(tr(m_sMyDownloadsViewName.toUtf8().constData())); //XXX: hidden = false for debug
    m_downloadsTableModel.setTable(tr(m_sMyDownloadsViewName.toUtf8().constData()));
    */

}

PromoTracksFeature::~PromoTracksFeature() {
}

QVariant PromoTracksFeature::title() {
    return tr(m_sFeaturedArtistsViewName.toUtf8().constData());
}

QIcon PromoTracksFeature::getIcon() {
    return QIcon(":/images/library/ic_library_promotracks.png");
}

bool PromoTracksFeature::isSupported(ConfigObject<ConfigValue>* config) {
    m_sPromoLocalHTMLLocation = LOCAL_HTML_LOCATION;
    qDebug() << "Promo dir:" << m_sPromoLocalHTMLLocation;
    return (QFile::exists(m_sPromoLocalHTMLLocation));
}

QList<TrackPointer> PromoTracksFeature::getTracksToAutoLoad()
{
    return m_tracksToAutoLoad;
}

void PromoTracksFeature::bindWidget(WLibrarySidebar* sidebarWidget,
                                    WLibrary* libraryWidget,
                                    MixxxKeyboard* keyboard) {
    Q_UNUSED(sidebarWidget);

    QString libraryPath = m_pConfig->getValueString(ConfigKey("[Playlist]","Directory"));

    ConfigObject<ConfigValue>* config = m_pConfig; //Long story, macros macros macros
    m_pBundledSongsView = new BundledSongsWebView(libraryWidget, m_pTrackCollection,
                                                  PROMO_BUNDLE_PATH,
                                                  m_sPromoLocalHTMLLocation,
                                                  m_bFirstRun, m_pConfig);
    m_pBundledSongsView->installEventFilter(keyboard);

    libraryWidget->registerView(tr(m_sBundledSongsViewName.toUtf8().constData()), m_pBundledSongsView);
    connect(m_pBundledSongsView, SIGNAL(loadTrack(TrackPointer)),
            this, SIGNAL(loadTrack(TrackPointer)));
    connect(m_pBundledSongsView, SIGNAL(loadTrackToPlayer(TrackPointer, QString)),
            this, SIGNAL(loadTrackToPlayer(TrackPointer, QString)));

/*  XXX: Re-enable this code for Promo 3.0
    m_pFeaturedArtistsView = new FeaturedArtistsWebView(libraryWidget, libraryPath, m_sPromoRemoteHTMLLocation, new SongDownloader(this));
    libraryWidget->registerView(tr(m_sFeaturedArtistsViewName.toUtf8().constData()), m_pFeaturedArtistsView);
    connect(m_pFeaturedArtistsView, SIGNAL(loadTrack(TrackInfoObject*)),
            this, SIGNAL(loadTrack(TrackInfoObject*)));
    connect(m_pFeaturedArtistsView, SIGNAL(loadTrackToPlayer(TrackInfoObject*, QString)),
            this, SIGNAL(loadTrackToPlayer(TrackInfoObject*, QString)));
    */
}

TreeItemModel* PromoTracksFeature::getChildModel() {
    //XXX Promo 3.0:
    //return NULL;
    return &m_childModel;
}

void PromoTracksFeature::activate() {
    //XXX Promo 3.0:
    //emit(switchToView(tr(m_sFeaturedArtistsViewName.toUtf8().constData())));
    emit(switchToView(tr(m_sBundledSongsViewName.toUtf8().constData())));
}

void PromoTracksFeature::activateChild(const QModelIndex& index) {
    QString itemString = m_childModel.data(index, Qt::DisplayRole).toString();
    if (itemString == tr(m_sMyDownloadsViewName.toUtf8().constData())) {
        emit(showTrackModel(&m_downloadsTableModel));
    } else {
        emit(switchToView(itemString));
    }
}

void PromoTracksFeature::onRightClick(const QPoint& globalPos) {
    Q_UNUSED(globalPos);
}

void PromoTracksFeature::onRightClickChild(const QPoint& globalPos,
                                            QModelIndex index) {
    Q_UNUSED(globalPos);
    Q_UNUSED(index);
}

bool PromoTracksFeature::dropAccept(QList<QUrl> urls) {
    Q_UNUSED(urls);
    return false;
}

bool PromoTracksFeature::dropAcceptChild(const QModelIndex& index, QList<QUrl> urls) {
    Q_UNUSED(index);
    Q_UNUSED(urls);
    return false;
}

bool PromoTracksFeature::dragMoveAccept(QUrl url) {
    Q_UNUSED(url);
    return false;
}

bool PromoTracksFeature::dragMoveAcceptChild(const QModelIndex& index,
                                             QUrl url) {
    Q_UNUSED(index);
    Q_UNUSED(url);
    return false;
}
void PromoTracksFeature::onLazyChildExpandation(const QModelIndex &index){
    //Nothing to do because the childmodel is not of lazy nature.
}
