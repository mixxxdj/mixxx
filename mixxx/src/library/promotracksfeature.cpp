// promotracksfeature.cpp
// FORK FORK FORK on 11/1/2009 by Albert Santoni (alberts@mixxx.org)
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "library/promotracksfeature.h"
#include "library/promotrackswebview.h"
#include "library/proxytrackmodel.h"
#include "library/trackcollection.h"
#include "trackinfoobject.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"

const QString PromoTracksFeature::m_sPromoTracksViewName = tr("Featured Artists");
QString PromoTracksFeature::m_sPromoLocalHTMLLocation;
QString PromoTracksFeature::m_sPromoRemoteHTMLLocation;
#define LOCAL_HTML_LOCATION (config->getConfigPath() + "/promo/1.8.0~beta1/index.html")

PromoTracksFeature::PromoTracksFeature(QObject* parent,
                             ConfigObject<ConfigValue>* config,
                             TrackCollection* pTrackCollection)
        : LibraryFeature(parent),
          m_pConfig(config),
          m_pPromoTracksView(NULL),
          m_pTrackCollection(pTrackCollection) {

    m_sPromoRemoteHTMLLocation = "http://promo.mixxx.org/1.8.0~beta1/index.html"; //m_pConfig->getConfigPath() + "/promo/promotracks.html";
    m_sPromoLocalHTMLLocation = LOCAL_HTML_LOCATION;
    m_sPromoAutoloadLocation = m_pConfig->getConfigPath() + "/promo/1.8.0~beta1/autoload.dat";

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
            qDebug() << "PROMO: Auto-loading track" << trackPath;
            m_tracksToAutoLoad.append(new TrackInfoObject(trackPath));
        }
        file.close();
    }
}

PromoTracksFeature::~PromoTracksFeature() {
}

QVariant PromoTracksFeature::title() {
    return m_sPromoTracksViewName;
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
                               WLibrary* libraryWidget) {

    m_pPromoTracksView = new PromoTracksWebView(libraryWidget, m_pConfig->getConfigPath(), m_sPromoLocalHTMLLocation, m_sPromoRemoteHTMLLocation);

    libraryWidget->registerView(m_sPromoTracksViewName, m_pPromoTracksView);
    connect(m_pPromoTracksView, SIGNAL(loadTrack(TrackInfoObject*)),
            this, SIGNAL(loadTrack(TrackInfoObject*)));
    connect(m_pPromoTracksView, SIGNAL(loadTrackToPlayer(TrackInfoObject*, int)),
            this, SIGNAL(loadTrackToPlayer(TrackInfoObject*, int)));
}

QAbstractItemModel* PromoTracksFeature::getChildModel() {
    return &m_childModel;
}

void PromoTracksFeature::activate() {
    //qDebug() << "PromoTracksFeature::activate()";
    //emit(showTrackModel(m_pPromoTracksTableModelProxy));
    
    if (m_pPromoTracksView->page()->bytesReceived() == 0) {
        qDebug() << "PROMO: Loading local page";
        m_pPromoTracksView->load(m_sPromoLocalHTMLLocation);
    }
    emit(switchToView(m_sPromoTracksViewName));
}

void PromoTracksFeature::activateChild(const QModelIndex& index) {

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
