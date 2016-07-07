#include <QIcon>

#include "maintenancefeature.h"

MaintenanceFeature::MaintenanceFeature(UserSettingsPointer pConfig, Library* pLibrary,
                                       TrackCollection* pTrackCollection,
                                       QObject* parent)
        : LibraryFeature(pConfig, pLibrary, pTrackCollection, parent),
          kMaintenanceTitle(tr("Library Maintenance")) {

}

QVariant MaintenanceFeature::title() {
    return kMaintenanceTitle;
}

QIcon MaintenanceFeature::getIcon() {
    return QIcon(":/images/library/ic_library_maintenance.png");
}

void MaintenanceFeature::activate() {
    switchToFeature();
}

