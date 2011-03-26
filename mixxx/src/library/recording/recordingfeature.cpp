// recordingfeature.cpp
// Created 03/26/2010 by Tobias Rafreider

#include <QStringList>
#include <QTreeView>
#include <QDirModel>
#include <QStringList>
#include <QFileInfo>
#include <QDesktopServices>

#include "trackinfoobject.h"
#include "library/treeitem.h"
#include "library/recording/recordingfeature.h"
#include "library/trackcollection.h"
#include "library/dao/trackdao.h"

RecordingFeature::RecordingFeature(QObject* parent, ConfigObject<ConfigValue>* pConfig, TrackCollection* pTrackCollection)
        : LibraryFeature(parent),
          m_pConfig(pConfig),
          m_browseModel(this),
          m_proxyModel(&m_browseModel),
          m_pTrackCollection(pTrackCollection) {

    m_proxyModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel.setSortCaseSensitivity(Qt::CaseInsensitive);

}

RecordingFeature::~RecordingFeature() {

}

QVariant RecordingFeature::title() {
    return QVariant(tr("Recordings"));
}

QIcon RecordingFeature::getIcon() {
    return QIcon(":/images/library/ic_library_browse.png");
}

TreeItemModel* RecordingFeature::getChildModel() {
    return &m_childModel;
}

bool RecordingFeature::dropAccept(QUrl url) {
    return false;
}

bool RecordingFeature::dropAcceptChild(const QModelIndex& index, QUrl url) {
    return false;
}

bool RecordingFeature::dragMoveAccept(QUrl url) {
    return false;
}

bool RecordingFeature::dragMoveAcceptChild(const QModelIndex& index, QUrl url) {
    return false;
}

void RecordingFeature::activate() {
    qDebug() << "RecordingFeature::activate;
    emit(showTrackModel(&m_proxyModel));
}

void RecordingFeature::activateChild(const QModelIndex& index) {

}

void RecordingFeature::onRightClick(const QPoint& globalPos) {
}

void RecordingFeature::onRightClickChild(const QPoint& globalPos, QModelIndex index) {
}
