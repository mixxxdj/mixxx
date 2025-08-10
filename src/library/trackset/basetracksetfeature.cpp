#include "library/trackset/basetracksetfeature.h"

#include "analyzer/analyzerscheduledtrack.h"
#include "library/trackset/searchcrate/dlggroupedsearchcratesinfo.h"
#include "library/trackset/searchcrate/dlgsearchcrateinfo.h"
#include "moc_basetracksetfeature.cpp"

BaseTrackSetFeature::BaseTrackSetFeature(
        Library* pLibrary,
        UserSettingsPointer pConfig,
        const QString& rootViewName,
        const QString& iconName)
        : LibraryFeature(pLibrary, pConfig, iconName),
          m_rootViewName(rootViewName),
          m_pSidebarModel(make_parented<TreeItemModel>(this)) {
}

void BaseTrackSetFeature::pasteChild(const QModelIndex&) {
    emit pasteFromSidebar();
}

void BaseTrackSetFeature::activate() {
    emit switchToView(m_rootViewName);
    emit disableSearch();
    emit enableCoverArtDisplay(true);
}
