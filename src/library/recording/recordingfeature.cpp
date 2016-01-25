// recordingfeature.cpp
// Created 03/26/2010 by Tobias Rafreider

#include "library/recording/dlgrecording.h"
#include "trackinfoobject.h"
#include "library/treeitem.h"
#include "library/recording/recordingfeature.h"
#include "library/library.h"
#include "library/trackcollection.h"
#include "widget/wlibrary.h"
#include "mixxxkeyboard.h"

const QString RecordingFeature::m_sRecordingViewName = QString("Recording");

RecordingFeature::RecordingFeature(Library* pLibrary,
                                   UserSettingsPointer pConfig,
                                   TrackCollection* pTrackCollection,
                                   RecordingManager* pRecordingManager)
        : LibraryFeature(pLibrary),
          m_pConfig(pConfig),
          m_pLibrary(pLibrary),
          m_pTrackCollection(pTrackCollection),
          m_pRecordingManager(pRecordingManager) {
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
void RecordingFeature::bindWidget(WLibrary* pLibraryWidget,
                                  MixxxKeyboard *keyboard) {
    //The view will be deleted by LibraryWidget
    DlgRecording* pRecordingView = new DlgRecording(pLibraryWidget,
                                                    m_pConfig,
                                                    m_pLibrary,
                                                    m_pTrackCollection,
                                                    m_pRecordingManager,
                                                    keyboard);

    pRecordingView->installEventFilter(keyboard);
    pLibraryWidget->registerView(m_sRecordingViewName, pRecordingView);
    connect(pRecordingView, SIGNAL(loadTrack(TrackPointer)),
            this, SIGNAL(loadTrack(TrackPointer)));
    connect(pRecordingView, SIGNAL(loadTrackToPlayer(TrackPointer, QString, bool)),
            this, SIGNAL(loadTrackToPlayer(TrackPointer, QString, bool)));
    connect(this, SIGNAL(refreshBrowseModel()),
            pRecordingView, SLOT(refreshBrowseModel()));
    connect(this, SIGNAL(requestRestoreSearch()),
            pRecordingView, SLOT(slotRestoreSearch()));
    connect(pRecordingView, SIGNAL(restoreSearch(QString)),
            this, SIGNAL(restoreSearch(QString)));
}


void RecordingFeature::activate() {
    emit(refreshBrowseModel());
    emit(switchToView(m_sRecordingViewName));
    // Ask the view to emit a restoreSearch signal.
    emit(requestRestoreSearch());
    emit(enableCoverArtDisplay(false));
}
