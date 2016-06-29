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
    inline QString getViewName() {
        return m_sRecordingViewName;
    }

    void bindPaneWidget(WLibrary* libraryWidget,
                        KeyboardEventFilter* pKeyboard, int);
    QWidget* createPaneWidget(KeyboardEventFilter *pKeyboard, int);
    void bindSidebarWidget(WBaseLibrary* pBaseLibrary, 
                           KeyboardEventFilter* pKeyboard);

    TreeItemModel* getChildModel();

  public slots:
    void activate();

  signals:
    void setRootIndex(const QModelIndex&);

  private:
    TrackCollection* m_pTrackCollection;
    FolderTreeModel m_childModel;
    const static QString m_sRecordingViewName;
    RecordingManager* m_pRecordingManager;
    
    QList<WTrackTableView*> m_trackTables;
    QPointer<DlgRecording> m_pRecordingView;
};

#endif
