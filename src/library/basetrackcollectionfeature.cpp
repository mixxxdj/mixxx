#include "basetrackcollectionfeature.h"

BaseTrackCollectionFeature::BaseTrackCollectionFeature(
        Library* pLibrary,
        UserSettingsPointer pConfig,
        const QString& rootViewName)
        : LibraryFeature(pLibrary, pConfig),
        m_rootViewName(rootViewName){
}

void BaseTrackCollectionFeature::activate() {
    emit switchToView(m_rootViewName);
    emit disableSearch();
    emit enableCoverArtDisplay(true);
}
