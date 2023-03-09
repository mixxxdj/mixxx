#pragma once

#include <QStringListModel>
#include <QSortFilterProxyModel>

#include "preferences/usersettings.h"
#include "preferences/keyboardconfig.h"
#include "library/browse/browsetablemodel.h"
#include "library/browse/foldertreemodel.h"
#include "library/libraryfeature.h"

class RecordingManager;

class RecordingFeature final : public LibraryFeature {
    Q_OBJECT
  public:
    RecordingFeature(Library* parent,
                     UserSettingsPointer pConfig,
                     KeyboardConfigPointer pKbdConfig,
                     RecordingManager* pRecordingManager);
    ~RecordingFeature() override = default;

    QVariant title() override;

    void bindLibraryWidget(WLibrary* libraryWidget,
                    KeyboardEventFilter* keyboard) override;

    TreeItemModel* sidebarModel() const override;

  public slots:
    void activate() override;

  signals:
    void setRootIndex(const QModelIndex&);
    void requestRestoreSearch();
    void refreshBrowseModel();

  private:
    RecordingManager* const m_pRecordingManager;

    FolderTreeModel* m_pSidebarModel;
};
