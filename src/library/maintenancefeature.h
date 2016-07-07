#ifndef MAINTENANCEFEATURE_H
#define MAINTENANCEFEATURE_H
#include "library/libraryfeature.h"

class MaintenanceFeature : public LibraryFeature
{
  public:
    MaintenanceFeature(UserSettingsPointer pConfig,
                       Library* pLibrary, TrackCollection* pTrackCollection,
                       QObject* parent);

    QVariant title();
    QIcon getIcon();
    
  public slots:
    void activate();

  private:
    
    QString kMaintenanceTitle;
};

#endif // MAINTENANCEFEATURE_H
