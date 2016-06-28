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
#include <QString>
#include <QSharedPointer>
#include <QObject>
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
    void bindPaneWidget(WLibrary* pLibrary,
                        KeyboardEventFilter* pKeyboard, int);
    QWidget* createPaneWidget(KeyboardEventFilter*pKeyboard, int paneId);
    void bindSidebarWidget(WBaseLibrary* pLibraryWidget, 
                           KeyboardEventFilter*,
                           int paneId);
    
    inline QString getViewName() {
        return m_sMixxxLibraryViewName;
    }

  public slots:
    void activate();
    void activateChild(const QModelIndex& index);
    void refreshLibraryModels();

  private:
    const QString kMissingTitle;
    const QString kHiddenTitle;
    const QString kLibraryTitle;
    QPointer<DlgHidden> m_pHiddenView;
    QPointer<DlgMissing> m_pMissingView;
    QHash<int, int> m_hiddenPaneId;
    QHash<int, int> m_missingPaneId;
    QHash<int, WTrackTableView*> m_hiddenPane;
    QHash<int, WTrackTableView*> m_missingPane;
    
    Library* m_pLibrary;
    QSharedPointer<BaseTrackCache> m_pBaseTrackCache;
    LibraryTableModel* m_pLibraryTableModel;
    TreeItemModel m_childModel;
    TrackDAO& m_trackDao;
    UserSettingsPointer m_pConfig;
    TrackCollection* m_pTrackCollection;
    static const QString m_sMixxxLibraryViewName;
};

#endif /* MIXXXLIBRARYFEATURE_H */
