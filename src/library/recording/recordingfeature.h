// recordingfeature.h
// Created 03/26/2010 by Tobias Rafreider

#ifndef RECORDING_FEATURE_H
#define RECORDING_FEATURE_H

#include <QByteArrayData>
#include <QIcon>
#include <QSortFilterProxyModel>
#include <QString>
#include <QStringListModel>
#include <QVariant>

#include "library/browse/browsetablemodel.h"
#include "library/browse/foldertreemodel.h"
#include "library/libraryfeature.h"
#include "library/proxytrackmodel.h"
#include "preferences/usersettings.h"

class RecordingManager;
class KeyboardEventFilter;
class Library;
class QModelIndex;
class QObject;
class TreeItemModel;
class WLibrary;

class RecordingFeature final : public LibraryFeature {
    Q_OBJECT
  public:
    RecordingFeature(Library* parent,
                     UserSettingsPointer pConfig,
                     RecordingManager* pRecordingManager);
    ~RecordingFeature() override = default;

    QVariant title() override;
    QIcon getIcon() override;

    void bindLibraryWidget(WLibrary* libraryWidget,
                    KeyboardEventFilter* keyboard) override;

    TreeItemModel* getChildModel() override;

  public slots:
    void activate() override;

  signals:
    void setRootIndex(const QModelIndex&);
    void requestRestoreSearch();
    void refreshBrowseModel();

  private:
    RecordingManager* const m_pRecordingManager;
    const QIcon m_icon;

    FolderTreeModel m_childModel;
};

#endif
