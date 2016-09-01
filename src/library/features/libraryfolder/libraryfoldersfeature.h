#ifndef LIBRARYFOLDERSFEATURE_H
#define LIBRARYFOLDERSFEATURE_H

#include "library/features/mixxxlibrary/mixxxlibraryfeature.h"

class LibraryFoldersModel;

class LibraryFoldersFeature : public MixxxLibraryFeature
{
  public:
    LibraryFoldersFeature(UserSettingsPointer pConfig,
                          Library* pLibrary,
                          QObject* parent,
                          TrackCollection* pTrackCollection);
    
    QVariant title() override;
    QString getIconPath() override;
    QString getSettingsName() const override;
    QWidget* createInnerSidebarWidget(KeyboardEventFilter* pKeyboard) override;
    
  public slots:
    void onRightClickChild(const QPoint& pos, const QModelIndex&) override;
};

#endif // LIBRARYFOLDERSFEATURE_H
