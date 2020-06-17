#include "library/trackset/basetracksetfeature.h"

BaseTrackSetFeature::BaseTrackSetFeature(
        Library* pLibrary,
        UserSettingsPointer pConfig,
        const QString& rootViewName)
        : LibraryFeature(pLibrary, pConfig),
          m_rootViewName(rootViewName) {
}

void BaseTrackSetFeature::activate() {
    emit switchToView(m_rootViewName);
    emit disableSearch();
    emit enableCoverArtDisplay(true);
}
