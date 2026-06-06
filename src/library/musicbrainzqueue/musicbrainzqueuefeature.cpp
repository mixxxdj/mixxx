#include "library/musicbrainzqueue/musicbrainzqueuefeature.h"

#include "library/musicbrainzqueue/dlgmusicbrainzqueue.h"
#include "moc_musicbrainzqueuefeature.cpp"
#include "widget/wlibrary.h"

// static
const QString MusicBrainzQueueFeature::kViewName =
        QStringLiteral("MusicBrainzQueue");

MusicBrainzQueueFeature::MusicBrainzQueueFeature(
        Library* pLibrary,
        UserSettingsPointer pConfig)
        : LibraryFeature(pLibrary, pConfig, QStringLiteral("prepare")),
          m_pSidebarModel(make_parented<TreeItemModel>(this)),
          m_pView(nullptr) {
}

QVariant MusicBrainzQueueFeature::title() {
    return tr("Fingerprint Queue");
}

void MusicBrainzQueueFeature::bindLibraryWidget(
        WLibrary* libraryWidget,
        KeyboardEventFilter* keyboard) {
    m_pView = new DlgMusicBrainzQueue(
            libraryWidget, m_pConfig, m_pLibrary, keyboard);
    connect(m_pView,
            &DlgMusicBrainzQueue::trackSelected,
            this,
            &MusicBrainzQueueFeature::trackSelected);
    libraryWidget->registerView(kViewName, m_pView);
}

TreeItemModel* MusicBrainzQueueFeature::sidebarModel() const {
    return m_pSidebarModel;
}

void MusicBrainzQueueFeature::activate() {
    emit switchToView(kViewName);
    if (m_pView) {
        emit restoreSearch(m_pView->currentSearch());
    }
    emit enableCoverArtDisplay(false);
}
