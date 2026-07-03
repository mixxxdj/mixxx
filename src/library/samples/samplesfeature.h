#pragma once

#include <QString>

#include "library/libraryfeature.h"
#include "preferences/usersettings.h"

class Library;
class TreeItemModel;
class WLibrary;
class KeyboardEventFilter;

class SamplesFeature final : public LibraryFeature {
    Q_OBJECT
  public:
    SamplesFeature(Library* parent,
            UserSettingsPointer pConfig);
    ~SamplesFeature() override = default;

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
    TreeItemModel* m_pSidebarModel;
};
