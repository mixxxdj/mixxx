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
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "mixxxkeyboard.h"

const QString RecordingFeature::m_sRecordingViewName = QString("Recording");

RecordingFeature::RecordingFeature(QObject* parent, ConfigObject<ConfigValue>* pConfig, TrackCollection* pTrackCollection)
        : LibraryFeature(parent),
          m_pConfig(pConfig), m_pRecordingView(0),
          m_pTrackCollection(pTrackCollection) {

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
void RecordingFeature::bindWidget(WLibrarySidebar *sidebarWidget,
                             WLibrary *libraryWidget,
                             MixxxKeyboard *keyboard)
{
    //The view will be deleted by LibraryWidget
    m_pRecordingView = new DlgRecording(libraryWidget,
                                           m_pConfig,
                                           m_pTrackCollection);

    m_pRecordingView->installEventFilter(keyboard);
    libraryWidget->registerView(m_sRecordingViewName, m_pRecordingView);
    connect(m_pRecordingView, SIGNAL(loadTrack(TrackPointer)),
            this, SIGNAL(loadTrack(TrackPointer)));
    connect(m_pRecordingView, SIGNAL(loadTrackToPlayer(TrackPointer, QString)),
            this, SIGNAL(loadTrackToPlayer(TrackPointer, QString)));
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
    qDebug() << "RecordingFeature::activate";

    m_pRecordingView->refreshBrowseModel();
    emit(switchToView("Recording"));
}

void RecordingFeature::activateChild(const QModelIndex& index) {

}

void RecordingFeature::onRightClick(const QPoint& globalPos) {
}

void RecordingFeature::onRightClickChild(const QPoint& globalPos, QModelIndex index) {
}
void RecordingFeature::onLazyChildExpandation(const QModelIndex &index){
    //Nothing to do here since we have no child models
}
