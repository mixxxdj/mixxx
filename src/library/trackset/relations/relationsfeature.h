#pragma once

#include <QObject>
#include <QPointer>
#include <QSortFilterProxyModel>
#include <QString>
#include <QStringListModel>
#include <QVariant>

#include "control/pollingcontrolproxy.h"
#include "library/trackset/basetracksetfeature.h"
#include "library/trackset/relations/relationstablemodel.h"

#define ALL_RELATIONS_NODE "::mixxx_all_relations_node::"

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
    bool activateDeckRelations(const QString& deckGroup);

    QString getRootViewHtml() const;
    QString getEmptyDeckViewHtml() const;

    QString deckGroupFromIndex(const QModelIndex& index) const;
    QModelIndex indexFromDeckGroup(const QString& deckGroup) const;

    PollingControlProxy m_pNumDecks;
    QString m_deckGroup;

    RelationsTableModel m_relationsTableModel;

    QModelIndex m_lastClickedIndex;
    QModelIndex m_lastRightClickedIndex;

    QList<TreeItem*> m_DeckRelationItemList;
    QPointer<WLibrarySidebar> m_pSidebarWidget;
};
