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
#include "library/librarytreemodel.h"
#include "library/dao/trackdao.h"
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

    QVariant title() override;
    QString getIconPath() override;
    
    bool dropAccept(QList<QUrl> urls, QObject* pSource);
    bool dragMoveAccept(QUrl url);
    TreeItemModel* getChildModel();
    virtual QWidget* createInnerSidebarWidget(KeyboardEventFilter *pKeyboard);

  public slots:
    void activate();
    void activateChild(const QModelIndex& index);
    void onRightClickChild(const QPoint& pos, const QModelIndex&);
    void refreshLibraryModels();
    
    void selectAll();
    void onSearch(const QString&) override;
    
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
    const QString kLibraryTitle;
    
    QSharedPointer<BaseTrackCache> m_pBaseTrackCache;
    QPointer<WLibrarySidebar> m_pSidebar;
    LibraryTableModel* m_pLibraryTableModel;
    LibraryTreeModel m_childModel;
    TrackDAO& m_trackDao;
};

#endif /* MIXXXLIBRARYFEATURE_H */
