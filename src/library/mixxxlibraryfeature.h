// mixxxlibraryfeature.h
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#ifndef MIXXXLIBRARYFEATURE_H
#define MIXXXLIBRARYFEATURE_H

#include <QAction>
#include <QIcon>
#include <QList>
#include <QModelIndex>
#include <QObject>
#include <QPointer>
#include <QSharedPointer>
#include <QString>
#include <QStringListModel>
#include <QUrl>
#include <QVariant>

#include "library/libraryfeature.h"
#include "library/dao/trackdao.h"
#include "library/treeitemmodel.h"
#include "preferences/usersettings.h"
#include "util/parented_ptr.h"

class DlgHidden;
class DlgMissing;
class BaseTrackCache;
class LibraryTableModel;
class TrackCollection;

class MixxxLibraryFeature final : public LibraryFeature {
    Q_OBJECT
  public:
    MixxxLibraryFeature(Library* pLibrary,
                        UserSettingsPointer pConfig);
    ~MixxxLibraryFeature() override = default;

    QVariant title() override;
    QIcon getIcon() override;
    bool dropAccept(QList<QUrl> urls, QObject* pSource) override;
    bool dragMoveAccept(QUrl url) override;
    TreeItemModel* getChildModel() override;
    void bindLibraryWidget(WLibrary* pLibrary,
                    KeyboardEventFilter* pKeyboard) override;
    void bindSidebarWidget(WLibrarySidebar* pSidebarWidget) override;

    bool hasTrackTable() override {
        return true;
    }

  public slots:
    void activate() override;
    void activateChild(const QModelIndex& index) override;
    void onRightClick(const QPoint& globalPos) override;
    void refreshLibraryModels();

  signals:
    void exportLibrary();

  private slots:
    void slotExportLibrary();

  private:
    const QString kMissingTitle;
    const QString kHiddenTitle;
    const QIcon m_icon;
    TrackCollection* const m_pTrackCollection;

    QSharedPointer<BaseTrackCache> m_pBaseTrackCache;
    LibraryTableModel* m_pLibraryTableModel;

    TreeItemModel m_childModel;

    DlgMissing* m_pMissingView;
    DlgHidden* m_pHiddenView;

    parented_ptr<QAction> m_pExportLibraryAction;

    QPointer<WLibrarySidebar> m_pSidebarWidget;
};

#endif /* MIXXXLIBRARYFEATURE_H */
