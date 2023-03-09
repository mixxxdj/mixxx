#include "library/trackset/basetracksetfeature.h"
#include "preferences/configobject.h"

#include "moc_basetracksetfeature.cpp"

BaseTrackSetFeature::BaseTrackSetFeature(
        Library* pLibrary,
        UserSettingsPointer pConfig,
        ConfigObject<ConfigValueKbd>* pKbdConfig,
        const QString& rootViewName,
        const QString& iconName)
        : LibraryFeature(pLibrary, pConfig, pKbdConfig, iconName),
          m_rootViewName(rootViewName),
          m_pSidebarModel(make_parented<TreeItemModel>(this)) {
}

void BaseTrackSetFeature::activate() {
    emit switchToView(m_rootViewName);
    emit disableSearch();
    emit enableCoverArtDisplay(true);
}
