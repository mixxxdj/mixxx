#pragma once

#include <QObject>
#include <QPointer>
#include <QSortFilterProxyModel>
#include <QString>
#include <QStringListModel>
#include <QVariant>

#include "control/pollingcontrolproxy.h"
#include "library/relations/relationstablemodel.h"
#include "library/trackset/basetracksetfeature.h"

#define ALL_RELATIONS_NODE "::mixxx_all_relations_node::"

class DlgRelations;
class Library;
class WLibrarySidebar;
class TreeItemModel;

class RelationsFeature : public BaseTrackSetFeature {
    Q_OBJECT

  public:
    RelationsFeature(Library* pLibrary, UserSettingsPointer pConfig);
    ~RelationsFeature() override = default;

    QVariant title() override;

    void bindLibraryWidget(WLibrary* libraryWidget, KeyboardEventFilter* keyboard) override;
    void bindSidebarWidget(WLibrarySidebar* pSidebarWidget) override;

    TreeItemModel* sidebarModel() const override;

  public slots:
    void activate() override;
    void activateChild(const QModelIndex& index) override;

  private:
    QString getEmptyDeckViewHtml() const;
    QString deckGroupFromIndex(const QModelIndex& index) const;

    void setConnections(DlgRelations* pView);

    PollingControlProxy m_pNumDecks;
    QString m_deckGroup;

    DlgRelations* m_pAllRelationView;
    DlgRelations* m_pDeckRelationView;

    QModelIndex m_lastClickedIndex;
    QModelIndex m_lastRightClickedIndex;

    QList<TreeItem*> m_DeckRelationItemList;
    QPointer<WLibrarySidebar> m_pSidebarWidget;
};
