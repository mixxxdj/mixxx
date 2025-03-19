#pragma once

#include <QObject>
#include <QPointer>
#include <QSortFilterProxyModel>
#include <QString>
#include <QStringListModel>
#include <QVariant>

#include "library/trackset/basetracksetfeature.h"

#define ALL_RELATIONS_NODE "::mixxx_all_relations_node::"
#define DECK_1_NODE "::mixxx_deck1_relations_node::"
#define DECK_2_NODE "::mixxx_deck2_relations_node::"
#define DECK_3_NODE "::mixxx_deck3_relations_node::"
#define DECK_4_NODE "::mixxx_deck4_relations_node::"

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

  private:
    QString getRootViewHtml() const;
    TreeItem* m_pAllRelationsItem;
    TreeItem* m_pDeck1RelationsItem;
    TreeItem* m_pDeck2RelationsItem;
    TreeItem* m_pDeck3RelationsItem;
    TreeItem* m_pDeck4RelationsItem;
    QPointer<WLibrarySidebar> m_pSidebarWidget;
};
