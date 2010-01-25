// promotracksfeature.cpp
// FORK FORK FORK on 11/1/2009 by Albert Santoni (alberts@mixxx.org)
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "library/promotracksfeature.h"
#include "library/promotrackswebview.h"
#include "library/proxytrackmodel.h"
#include "library/trackcollection.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"

const QString PromoTracksFeature::m_sPromoTracksViewName = tr("Featured Artists");

PromoTracksFeature::PromoTracksFeature(QObject* parent,
                             ConfigObject<ConfigValue>* pConfig,
                             TrackCollection* pTrackCollection)
        : LibraryFeature(parent),
          m_pConfig(pConfig),
          m_pTrackCollection(pTrackCollection) {
}

PromoTracksFeature::~PromoTracksFeature() {
}

QVariant PromoTracksFeature::title() {
    return m_sPromoTracksViewName;
}

QIcon PromoTracksFeature::getIcon() {
    return QIcon();
}

bool PromoTracksFeature::isSupported() {
    return (QFile::exists(MIXXX_PROMO_HTML_LOCATION));
}

void PromoTracksFeature::bindWidget(WLibrarySidebar* sidebarWidget,
                               WLibrary* libraryWidget) {

    PromoTracksWebView* pPromoTracksView = new PromoTracksWebView(libraryWidget);

    libraryWidget->registerView(m_sPromoTracksViewName, pPromoTracksView);
    connect(pPromoTracksView, SIGNAL(loadTrack(TrackInfoObject*)),
            this, SIGNAL(loadTrack(TrackInfoObject*)));
    connect(pPromoTracksView, SIGNAL(loadTrackToPlayer(TrackInfoObject*, int)),
            this, SIGNAL(loadTrackToPlayer(TrackInfoObject*, int)));
}

QAbstractItemModel* PromoTracksFeature::getChildModel() {
    return &m_childModel;
}

void PromoTracksFeature::activate() {
    //qDebug() << "PromoTracksFeature::activate()";
    //emit(showTrackModel(m_pPromoTracksTableModelProxy));
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
