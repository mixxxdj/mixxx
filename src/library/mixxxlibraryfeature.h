// mixxxlibraryfeature.h
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#ifndef MIXXXLIBRARYFEATURE_H
#define MIXXXLIBRARYFEATURE_H

#include <QStringListModel>
#include <QUrl>
#include <QVariant>
#include <QIcon>
#include <QModelIndex>
#include <QList>
#include <QSharedPointer>
#include <QString>
#include <QStackedWidget>
#include <QPointer>

#include "library/libraryfeature.h"
#include "library/dao/trackdao.h"
#include "treeitemmodel.h"
#include "preferences/usersettings.h"

class DlgHidden;
class DlgMissing;
class Library;
class BaseTrackCache;
class LibraryTableModel;
class TrackCollection;
class WTrackTableView;
class HiddenTableModel;
class MissingTableModel;

class MixxxLibraryFeature : public LibraryFeature {
    Q_OBJECT
    
  public:
    MixxxLibraryFeature(UserSettingsPointer pConfig,
                        Library* pLibrary,
                        QObject* parent,
                        TrackCollection* pTrackCollection);
    virtual ~MixxxLibraryFeature();

    QVariant title();
    QIcon getIcon();
    
    bool dropAccept(QList<QUrl> urls, QObject* pSource);
    bool dragMoveAccept(QUrl url);
    TreeItemModel* getChildModel();
    
    QWidget* createPaneWidget(KeyboardEventFilter*pKeyboard, int paneId) override;
    QWidget* createInnerSidebarWidget(KeyboardEventFilter* pKeyboard) override;

  public slots:
    void activate();
    void activateChild(const QModelIndex& index);
    void refreshLibraryModels();
    
    void selectionChanged(const QItemSelection&, const QItemSelection&);
    
    void selectAll();
    
    
  signals:
    void unhideHidden();
    void purgeHidden();
    void purgeMissing();

  private:
    enum Panes {
        MixxxLibrary = 1,
        Hidden = 2,
        Missing = 3
    };
    
    HiddenTableModel* getHiddenTableModel();
    MissingTableModel* getMissingTableModel();
    
    const QString kLibraryTitle;
    const QString kHiddenTitle;
    const QString kMissingTitle;
    QPointer<DlgHidden> m_pHiddenView;
    QPointer<DlgMissing> m_pMissingView;
    QHash<int, Panes> m_idPaneCurrent;
    
    // SidebarExpanded pane's ids
    int m_idExpandedHidden;
    int m_idExpandedMissing;
    int m_idExpandedControls;
    int m_idExpandedTree;
    
    QPointer<HiddenTableModel> m_pHiddenTableModel;
    QPointer<MissingTableModel> m_pMissingTableModel;
    
    QPointer<QStackedWidget> m_pExpandedStack;
    QPointer<QTabWidget> m_pSidebarTab;
    
    QSharedPointer<BaseTrackCache> m_pBaseTrackCache;
    LibraryTableModel* m_pLibraryTableModel;
    TreeItemModel m_childModel;
    TrackDAO& m_trackDao;
    TrackCollection* m_pTrackCollection;
};

#endif /* MIXXXLIBRARYFEATURE_H */
