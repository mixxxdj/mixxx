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

RecordingFeature::RecordingFeature(QObject* parent, ConfigObject<ConfigValue>* pConfig,
                                   TrackCollection* pTrackCollection,
                                   RecordingManager* pRecordingManager)
        : LibraryFeature(parent),
          m_pConfig(pConfig),
          m_pTrackCollection(pTrackCollection), 
          m_pRecordingView(0),
          m_pRecordingManager(pRecordingManager){

}

RecordingFeature::~RecordingFeature() {

}

QVariant RecordingFeature::title() {
    return QVariant(tr("Recordings"));
}

QIcon RecordingFeature::getIcon() {
    return QIcon(":/images/library/ic_library_recordings.png");
}

TreeItemModel* RecordingFeature::getChildModel() {
    return &m_childModel;
}
void RecordingFeature::bindWidget(WLibrarySidebar *sidebarWidget,
                             WLibrary *libraryWidget,
                             MixxxKeyboard *keyboard)
{
    Q_UNUSED(sidebarWidget);

    //The view will be deleted by LibraryWidget
    m_pRecordingView = new DlgRecording(libraryWidget,
                                           m_pConfig,
                                           m_pTrackCollection,
                                           m_pRecordingManager,
                                           keyboard);

    m_pRecordingView->installEventFilter(keyboard);
    libraryWidget->registerView(m_sRecordingViewName, m_pRecordingView);
    connect(m_pRecordingView, SIGNAL(loadTrack(TrackPointer)),
            this, SIGNAL(loadTrack(TrackPointer)));
    connect(m_pRecordingView, SIGNAL(loadTrackToPlayer(TrackPointer, QString)),
            this, SIGNAL(loadTrackToPlayer(TrackPointer, QString)));
}

bool RecordingFeature::dropAccept(QList<QUrl> urls) {
    Q_UNUSED(urls);
    return false;
}

bool RecordingFeature::dropAcceptChild(const QModelIndex& index, QList<QUrl> urls) {
    Q_UNUSED(index);
    Q_UNUSED(urls);
    return false;
}

bool RecordingFeature::dragMoveAccept(QUrl url) {
    Q_UNUSED(url);
    return false;
}

bool RecordingFeature::dragMoveAcceptChild(const QModelIndex& index, QUrl url) {
    Q_UNUSED(index);
    Q_UNUSED(url);
    return false;
}

void RecordingFeature::activate() {
    m_pRecordingView->refreshBrowseModel();
    emit(switchToView(m_sRecordingViewName));
    emit(restoreSearch(m_pRecordingView->currentSearch()));
}

void RecordingFeature::activateChild(const QModelIndex& index) {
    Q_UNUSED(index);
}

void RecordingFeature::onRightClick(const QPoint& globalPos) {
    Q_UNUSED(globalPos);
}

void RecordingFeature::onRightClickChild(const QPoint& globalPos, QModelIndex index) {
    Q_UNUSED(globalPos);
    Q_UNUSED(index);
}

void RecordingFeature::onLazyChildExpandation(const QModelIndex &index){
    Q_UNUSED(index);    
    // Nothing to do here since we have no child models
}
