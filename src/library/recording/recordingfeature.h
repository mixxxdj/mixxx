// recordingfeature.h
// Created 03/26/2010 by Tobias Rafreider

#ifndef RECORDING_FEATURE_H
#define RECORDING_FEATURE_H

#include <QStringListModel>
#include <QSortFilterProxyModel>

#include "preferences/usersettings.h"
#include "library/browse/browsetablemodel.h"
#include "library/browse/foldertreemodel.h"
#include "library/libraryfeature.h"
#include "library/proxytrackmodel.h"
#include "recording/recordingmanager.h"

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

    QVariant title();
    QIcon getIcon();

    QWidget* createPaneWidget(KeyboardEventFilter *pKeyboard, int) override;
    QWidget* createInnerSidebarWidget(KeyboardEventFilter* pKeyboard) override;

    TreeItemModel* getChildModel();

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
    QPointer<BrowseTableModel> m_pBrowseModel;
    QPointer<ProxyTrackModel> m_pProxyModel;
};

#endif
