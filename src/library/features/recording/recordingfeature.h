// recordingfeature.h
// Created 03/26/2010 by Tobias Rafreider

#ifndef RECORDING_FEATURE_H
#define RECORDING_FEATURE_H

#include <QStringListModel>
#include <QSortFilterProxyModel>

#include "preferences/usersettings.h"
#include "library/features/browse/browsetablemodel.h"
#include "library/features/browse/foldertreemodel.h"
#include "library/libraryfeature.h"
#include "library/proxytrackmodel.h"
#include "recording/recordingmanager.h"
#include "util/memory.h"

class TrackCollection;
class WTrackTableView;
class DlgRecording;

class RecordingFeature : public LibraryFeature {
    Q_OBJECT
  public:
    RecordingFeature(UserSettingsPointer pConfig,
                     Library* pLibrary,
                     QObject* parent,
                     TrackCollection* pTrackCollection,
                     RecordingManager* pRecordingManager);
    virtual ~RecordingFeature();

    QVariant title() override;
    QString getIconPath() override;
    QString getSettingsName() const override;

    parented_ptr<QWidget> createPaneWidget(KeyboardEventFilter*, int paneId,
                                           QWidget* parent) override;
    parented_ptr<QWidget> createInnerSidebarWidget(KeyboardEventFilter* pKeyboard,
                                                   QWidget* parent) override;

    QPointer<TreeItemModel> getChildModel();

  public slots:
    void activate();

  signals:
    void setRootIndex(const QModelIndex&);

  private:

    BrowseTableModel* getBrowseTableModel();
    ProxyTrackModel* getProxyTrackModel();

    TrackCollection* m_pTrackCollection;
    FolderTreeModel m_childModel;
    RecordingManager* m_pRecordingManager;

    QPointer<DlgRecording> m_pRecordingView;
    parented_ptr<BrowseTableModel> m_pBrowseModel;
    std::unique_ptr<ProxyTrackModel> m_pProxyModel;
};

#endif
