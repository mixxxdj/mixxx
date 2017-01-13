// mixxxlibraryfeature.h
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#ifndef MIXXXLIBRARYFEATURE_H
#define MIXXXLIBRARYFEATURE_H

#include <QComboBox>
#include <QIcon>
#include <QList>
#include <QModelIndex>
#include <QPointer>
#include <QSharedPointer>
#include <QStackedWidget>
#include <QString>
#include <QStringListModel>
#include <QUrl>
#include <QVariant>

#include "library/libraryfeature.h"
#include "library/features/mixxxlibrary/mixxxlibrarytreemodel.h"
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
    QString getSettingsName() const override;

    bool dropAccept(QList<QUrl> urls, QObject* pSource);
    bool dragMoveAccept(QUrl url);
    TreeItemModel* getChildModel();
    QWidget* createInnerSidebarWidget(KeyboardEventFilter* pKeyboard) override;

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

  protected:
    void setChildModel(TreeItemModel* pChild);
    
    QPointer<TreeItemModel> m_pChildModel;
    QPointer<WLibrarySidebar> m_pSidebar;

  private:
    static const QString kLibraryTitle;
    static const QList<QStringList> kGroupingOptions;
    static const QStringList kGroupingText;
    
  private slots:
    void setTreeSettings(const QVariant &settings);
    
  private:
    QPointer<QComboBox> m_pGroupingCombo;
    QSharedPointer<BaseTrackCache> m_pBaseTrackCache;
    LibraryTableModel* m_pLibraryTableModel;
    TrackDAO& m_trackDao;
    QModelIndex m_lastClickedIndex;
    
};

#endif /* MIXXXLIBRARYFEATURE_H */
