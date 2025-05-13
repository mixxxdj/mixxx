#pragma once

#include "library/libraryfeature.h"
#include "preferences/usersettings.h"
#include "util/parented_ptr.h"

class DlgRecording;
class FolderTreeModel;
class RecordingManager;

class RecordingFeature final : public LibraryFeature {
    Q_OBJECT
  public:
    RecordingFeature(Library* parent,
                     UserSettingsPointer pConfig,
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
    parented_ptr<DlgRecording> m_pRecordingView;
    FolderTreeModel* m_pSidebarModel;
};
