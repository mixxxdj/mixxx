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
    QString getViewName() override;
    
    bool dropAccept(QList<QUrl> urls, QObject* pSource);
    bool dragMoveAccept(QUrl url);
    TreeItemModel* getChildModel();
    
    QWidget* createPaneWidget(KeyboardEventFilter*pKeyboard, int paneId);
    QWidget* createSidebarWidget(KeyboardEventFilter *pKeyboard);

  public slots:
    void activate();
    void activateChild(const QModelIndex& index);
    void refreshLibraryModels();
    
    void hiddenSelectionChanged(const QItemSelection&, const QItemSelection&);
    void missingSelectionChanged(const QItemSelection&, const QItemSelection&);
    
    void selectAllHidden();
    void selectAllMissing();
    
    
  signals:
    void unhideHidden();
    void purgeHidden();
    void purgeMissing();

  private:
    const QString kLibraryTitle;
    const QString kHiddenTitle;
    const QString kMissingTitle;
    QPointer<DlgHidden> m_pHiddenView;
    QPointer<DlgMissing> m_pMissingView;
    QHash<int, QPointer<WTrackTableView> > m_hiddenPane;
    QHash<int, QPointer<WTrackTableView> > m_missingPane;
    
    // This is needed to select the correct widget in the 2 size widget stack
    // for the hidden and missing widgets.
    QHash<int, int> m_hiddenPaneId;
    QHash<int, int> m_missingPaneId;
    
    // SidebarExpanded pane's ids
    int m_hiddenExpandedId;
    int m_missingExpandedId;
    
    QPointer<HiddenTableModel> m_pHiddenTableModel;
    QPointer<MissingTableModel> m_pMissingTableModel;
    
    QHash<int, QPointer<QStackedWidget> > m_paneStack;
    QPointer<QStackedWidget> m_pExpandedStack;
    
    QSharedPointer<BaseTrackCache> m_pBaseTrackCache;
    LibraryTableModel* m_pLibraryTableModel;
    TreeItemModel m_childModel;
    TrackDAO& m_trackDao;
    TrackCollection* m_pTrackCollection;
    static const QString m_sMixxxLibraryViewName;
};

#endif /* MIXXXLIBRARYFEATURE_H */
