#pragma once

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

#include "library/dao/trackdao.h"
#include "library/libraryfeature.h"
#include "library/treeitemmodel.h"
#include "preferences/usersettings.h"
#ifdef __ENGINEPRIME__
#include "util/parented_ptr.h"
#endif

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
    bool dropAccept(const QList<QUrl>& urls, QObject* pSource) override;
    bool dragMoveAccept(const QUrl& url) override;
    TreeItemModel* sidebarModel() const override;
    void bindLibraryWidget(WLibrary* pLibrary,
                    KeyboardEventFilter* pKeyboard) override;
#ifdef __ENGINEPRIME__
    void bindSidebarWidget(WLibrarySidebar* pSidebarWidget) override;
#endif

    bool hasTrackTable() override {
        return true;
    }

    LibraryTableModel* trackTableModel() const {
        return m_pLibraryTableModel;
    }

    void searchAndActivate(const QString& query);

  public slots:
    void activate() override;
    void activateChild(const QModelIndex& index) override;
#ifdef __ENGINEPRIME__
    void onRightClick(const QPoint& globalPos) override;
#endif
    void refreshLibraryModels();

#ifdef __ENGINEPRIME__
  signals:
    /// Inform that a request has been made to export the whole Mixxx library.
    void exportLibrary();
#endif

  private:
    const QString kMissingTitle;
    const QString kHiddenTitle;
    TrackCollection* const m_pTrackCollection;

    QSharedPointer<BaseTrackCache> m_pBaseTrackCache;
    LibraryTableModel* m_pLibraryTableModel;

    parented_ptr<TreeItemModel> m_pSidebarModel;

    DlgMissing* m_pMissingView;
    DlgHidden* m_pHiddenView;

#ifdef __ENGINEPRIME__
    parented_ptr<QAction> m_pExportLibraryAction;

    QPointer<WLibrarySidebar> m_pSidebarWidget;
#endif
};
