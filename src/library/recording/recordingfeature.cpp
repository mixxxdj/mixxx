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
          m_pSidebarModel(new FolderTreeModel(this)),
          m_pRecordingView(nullptr) {
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
    m_pRecordingView = new DlgRecording(pLibraryWidget,
            m_pConfig,
            m_pLibrary,
            m_pRecordingManager,
            keyboard);

    m_pRecordingView->installEventFilter(keyboard);
    pLibraryWidget->registerView(kViewName, m_pRecordingView);
    connect(m_pRecordingView,
            &DlgRecording::loadTrack,
            this,
            &RecordingFeature::loadTrack);
    connect(m_pRecordingView,
            &DlgRecording::loadTrackToPlayer,
            this,
            &RecordingFeature::loadTrackToPlayer);
    connect(this,
            &RecordingFeature::refreshBrowseModel,
            m_pRecordingView,
            &DlgRecording::refreshBrowseModel);
    connect(this,
            &RecordingFeature::requestRestoreSearch,
            m_pRecordingView,
            &DlgRecording::slotRestoreSearch);
    connect(m_pRecordingView,
            &DlgRecording::restoreSearch,
            this,
            &RecordingFeature::restoreSearch);
    connect(m_pRecordingView,
            &DlgRecording::restoreModelState,
            this,
            &RecordingFeature::restoreModelState);
}

void RecordingFeature::activate() {
    emit refreshBrowseModel();
    emit switchToView(kViewName);
    emit disableSearch();
    emit showTrackModel(m_pRecordingView->getTableModel());
    emit enableCoverArtDisplay(false);
}
