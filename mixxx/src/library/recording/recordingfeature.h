// recordingfeature.h
// Created 03/26/2010 by Tobias Rafreider

#ifndef RECORDING_FEATURE_H
#define RECORDING_FEATURE_H

#include <QStringListModel>
#include <QSortFilterProxyModel>

#include "configobject.h"
#include "library/browse/browsetablemodel.h"
#include "library/browse/foldertreemodel.h"
#include "library/libraryfeature.h"
#include "library/proxytrackmodel.h"
#include "dlgrecording.h"
#include "recording/recordingmanager.h"

class TrackCollection;

class RecordingFeature : public LibraryFeature {
    Q_OBJECT
  public:
    RecordingFeature(QObject* parent,
                  ConfigObject<ConfigValue>* pConfig,
                  TrackCollection* pTrackCollection,
                  RecordingManager* pRecordingManager);
    virtual ~RecordingFeature();

    QVariant title();
    QIcon getIcon();

    bool dropAccept(QList<QUrl> urls);
    bool dropAcceptChild(const QModelIndex& index, QList<QUrl> urls);
    bool dragMoveAccept(QUrl url);
    bool dragMoveAcceptChild(const QModelIndex& index, QUrl url);
    void bindWidget(WLibrarySidebar* sidebarWidget,
                    WLibrary* libraryWidget,
                    MixxxKeyboard* keyboard);

    TreeItemModel* getChildModel();

  public slots:
    void activate();
    void activateChild(const QModelIndex& index);
    void onRightClick(const QPoint& globalPos);
    void onRightClickChild(const QPoint& globalPos, QModelIndex index);
    void onLazyChildExpandation(const QModelIndex& index);

  signals:
    void setRootIndex(const QModelIndex&);

  private:
    ConfigObject<ConfigValue>* m_pConfig;
    TrackCollection* m_pTrackCollection;
    FolderTreeModel m_childModel;
    const static QString m_sRecordingViewName;
    DlgRecording* m_pRecordingView;
    RecordingManager* m_pRecordingManager;
};

#endif
