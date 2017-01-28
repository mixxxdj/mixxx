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

class MixxxLibraryFeature : public LibraryFeature {
    Q_OBJECT
    public:
    MixxxLibraryFeature(Library* pLibrary,
                        TrackCollection* pTrackCollection,
                        UserSettingsPointer pConfig);
    virtual ~MixxxLibraryFeature();

    QVariant title();
    QIcon getIcon();
    bool dropAccept(QList<QUrl> urls, QObject* pSource);
    bool dragMoveAccept(QUrl url);
    TreeItemModel* getChildModel();
    void bindWidget(WLibrary* pLibrary,
                    KeyboardEventFilter* pKeyboard);

  public slots:
    void activate();
    void activateChild(const QModelIndex& index);
    void refreshLibraryModels();

  private:
    const QString kMissingTitle;
    const QString kHiddenTitle;
    Library* m_pLibrary;
    QSharedPointer<BaseTrackCache> m_pBaseTrackCache;
    LibraryTableModel* m_pLibraryTableModel;
    DlgMissing* m_pMissingView;
    DlgHidden* m_pHiddenView;
    TreeItemModel m_childModel;
    TrackDAO& m_trackDao;
    UserSettingsPointer m_pConfig;
    TrackCollection* m_pTrackCollection;
};

#endif /* MIXXXLIBRARYFEATURE_H */
