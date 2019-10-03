// recordingfeature.cpp
// Created 03/26/2010 by Tobias Rafreider

#include "library/recording/dlgrecording.h"
#include "track/track.h"
#include "library/treeitem.h"
#include "library/recording/recordingfeature.h"
#include "library/library.h"
#include "library/trackcollection.h"
#include "widget/wlibrary.h"
#include "controllers/keyboard/keyboardeventfilter.h"

const QString RecordingFeature::m_sRecordingViewName = QString("Recording");

RecordingFeature::RecordingFeature(Library* pLibrary,
                                   UserSettingsPointer pConfig,
                                   TrackCollection* pTrackCollection,
                                   RecordingManager* pRecordingManager)
        : LibraryFeature(pLibrary),
          m_pConfig(pConfig),
          m_pLibrary(pLibrary),
          m_pTrackCollection(pTrackCollection),
          m_pRecordingManager(pRecordingManager),
          m_icon(":/images/library/ic_library_recordings.svg") {
}

RecordingFeature::~RecordingFeature() {

}

QVariant RecordingFeature::title() {
    return QVariant(tr("Recordings"));
}

QIcon RecordingFeature::getIcon() {
    return m_icon;
}

TreeItemModel* RecordingFeature::getChildModel() {
    return &m_childModel;
}
void RecordingFeature::bindWidget(WLibrary* pLibraryWidget,
                                  KeyboardEventFilter *keyboard) {
    //The view will be deleted by LibraryWidget
    DlgRecording* pRecordingView = new DlgRecording(pLibraryWidget,
                                                    m_pConfig,
                                                    m_pLibrary,
                                                    m_pTrackCollection,
                                                    m_pRecordingManager,
                                                    keyboard);

    pRecordingView->installEventFilter(keyboard);
    pLibraryWidget->registerView(m_sRecordingViewName, pRecordingView);
    connect(pRecordingView,
            &DlgRecording::loadTrack,
            this,
            &RecordingFeature::loadTrack);
    connect(pRecordingView,
            &DlgRecording::loadTrackToPlayer,
            this,
            &RecordingFeature::loadTrackToPlayer);
    connect(this,
            &RecordingFeature::refreshBrowseModel,
            pRecordingView,
            &DlgRecording::refreshBrowseModel);
    connect(this,
            &RecordingFeature::requestRestoreSearch,
            pRecordingView,
            &DlgRecording::slotRestoreSearch);
    connect(pRecordingView,
            &DlgRecording::restoreSearch,
            this,
            &RecordingFeature::restoreSearch);
}


void RecordingFeature::activate() {
    emit(refreshBrowseModel());
    emit(switchToView(m_sRecordingViewName));
    // Ask the view to emit a restoreSearch signal.
    emit(requestRestoreSearch());
    emit(enableCoverArtDisplay(false));
}
