// mixxxlibraryfeature.h
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#ifndef MIXXXLIBRARYFEATURE_H
#define MIXXXLIBRARYFEATURE_H

#include <QComboBox>
#include <QIcon>
#include <QList>
#include <QPersistentModelIndex>
#include <QPointer>
#include <QSharedPointer>
#include <QStackedWidget>
#include <QString>
#include <QStringListModel>
#include <QUrl>
#include <QVariant>

#include "library/libraryfeature.h"
#include "library/features/tracks/trackstreemodel.h"
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

class TracksFeature : public LibraryFeature {
    Q_OBJECT

  public:
    TracksFeature(UserSettingsPointer pConfig,
                        Library* pLibrary,
                        QObject* parent,
                        TrackCollection* pTrackCollection);
    virtual ~TracksFeature();

    QVariant title() override;
    QString getIconPath() override;
    QString getSettingsName() const override;

    bool dropAccept(QList<QUrl> urls, QObject* pSource);
    bool dragMoveAccept(QUrl url);
    QPointer<TreeItemModel> getChildModel();
    parented_ptr<QWidget> createInnerSidebarWidget(KeyboardEventFilter*,
                                                   QWidget* parent) override;

    bool hasTrackTable() override {
        return true;
    }

  public slots:
    void activate() override;
    void activateChild(const QModelIndex& index) override;
    void onRightClickChild(const QPoint& pos, const QModelIndex&) override;
    void invalidateChild() override;
    void refreshLibraryModels();

    void onSearch(const QString&) override;

  signals:
    void unhideHidden();
    void purgeHidden();
    void purgeMissing();

  private:
    static const QString kLibraryTitle;
    static const QList<QStringList> kGroupingOptions;
    static const QStringList kGroupingText;
    static const QString kLibraryFolder;

  private slots:
    void setTreeSettings(const QVariant &settings,
                         AbstractRole role = AbstractRole::RoleSorting);

  private:
    std::unique_ptr<TreeItemModel> m_pChildModel;
    QPointer<WLibrarySidebar> m_pSidebar;
    QSharedPointer<BaseTrackCache> m_pBaseTrackCache;
    parented_ptr<LibraryTableModel> m_pLibraryTableModel;
    TrackDAO& m_trackDao;
    QPersistentModelIndex m_lastClickedIndex;
    bool m_foldersShown;

};

#endif /* MIXXXLIBRARYFEATURE_H */
