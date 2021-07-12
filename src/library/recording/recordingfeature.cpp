#include "library/recording/recordingfeature.h"

#include "controllers/keyboard/keyboardeventfilter.h"
#include "library/library.h"
#include "library/recording/dlgrecording.h"
#include "library/treeitem.h"
#include "moc_recordingfeature.cpp"
#include "recording/recordingmanager.h"
#include "track/track.h"
#include "widget/wlibrary.h"

namespace {

const QString kViewName = QStringLiteral("Recording");

} // anonymous namespace

RecordingFeature::RecordingFeature(Library* pLibrary,
        UserSettingsPointer pConfig,
        RecordingManager* pRecordingManager)
        : LibraryFeature(pLibrary, pConfig, QStringLiteral("recordings")),
          m_pRecordingManager(pRecordingManager),
          m_pSidebarModel(new FolderTreeModel(this)) {
}

QVariant RecordingFeature::title() {
    return QVariant(tr("Recordings"));
}

TreeItemModel* RecordingFeature::sidebarModel() const {
    return m_pSidebarModel;
}
void RecordingFeature::bindLibraryWidget(WLibrary* pLibraryWidget,
                                  KeyboardEventFilter *keyboard) {
    //The view will be deleted by LibraryWidget
    DlgRecording* pRecordingView = new DlgRecording(pLibraryWidget,
                                                    m_pConfig,
                                                    m_pLibrary,
                                                    m_pRecordingManager,
                                                    keyboard);

    pRecordingView->installEventFilter(keyboard);
    pLibraryWidget->registerView(kViewName, pRecordingView);
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
    emit refreshBrowseModel();
    emit switchToView(kViewName);
    // Ask the view to emit a restoreSearch signal.
    emit requestRestoreSearch();
    emit enableCoverArtDisplay(false);
}
