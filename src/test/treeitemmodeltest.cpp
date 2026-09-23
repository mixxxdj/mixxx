#include <gtest/gtest.h>

#include <QModelIndex>

#include "library/treeitem.h"
#include "library/treeitemmodel.h"

namespace {

// Reproduces the sidebar tree of the History feature with enough playlists
// to trigger year grouping (> kNumToplevelHistoryEntries), i.e. a subtree
// nested one level below the top-level nodes.
struct SidebarTree {
    TreeItemModel model;

    explicit SidebarTree() {
        // newRoot() DEBUG_ASSERTs a non-null feature; a plain TreeItem
        // (no parent) is all the model lookup needs here.
        auto pRoot = std::make_unique<TreeItem>();

        // 5 recent playlists at the top level
        for (int i = 1; i <= 5; ++i) {
            pRoot->appendChild(QString("playlist %1").arg(i), i);
        }
        // An older playlist grouped under a year node
        auto pYear = pRoot->appendChild("2026", -1 /*placeholder*/);
        pYear->appendChild("old playlist", 6);

        model.setRootItem(std::move(pRoot));
    }
};

} // namespace

class TreeItemModelTest : public testing::Test {};

TEST_F(TreeItemModelTest, FindItemByDataFindsTopLevelItem) {
    SidebarTree tree;
    TreeItem* pFound = tree.model.findItemByData(QVariant(3));
    ASSERT_NE(pFound, nullptr);
    EXPECT_EQ(pFound->getData().toInt(), 3);
    EXPECT_EQ(pFound, tree.model.getRootItem()->child(2));
}

TEST_F(TreeItemModelTest, FindItemByDataFindsYearGroupedItem) {
    // The regression scenario: a playlist nested under a year group node,
    // as produced by SetlogFeature once there are more history playlists
    // than kNumToplevelHistoryEntries. indexFromPlaylistId() must still find
    // it (previously the Qt::MatchRecursive based lookup crashed here).
    SidebarTree tree;
    TreeItem* pYear = tree.model.getRootItem()->child(5);
    ASSERT_NE(pYear, nullptr);
    ASSERT_EQ(pYear->childRows(), 1);
    TreeItem* pOld = pYear->child(0);
    ASSERT_NE(pOld, nullptr);

    TreeItem* pFound = tree.model.findItemByData(QVariant(6));
    ASSERT_NE(pFound, nullptr);
    EXPECT_EQ(pFound, pOld);
}

TEST_F(TreeItemModelTest, FindItemByDataReturnsNullForMissing) {
    SidebarTree tree;
    EXPECT_EQ(tree.model.findItemByData(QVariant(999)), nullptr);
    // Root has no payload, so looking up an empty QVariant must not
    // accidentally match the root item.
    EXPECT_EQ(tree.model.findItemByData(QVariant()), nullptr);
}

TEST_F(TreeItemModelTest, IndexFromItemRecoversTopLevelItem) {
    SidebarTree tree;
    TreeItem* pRootItem = tree.model.getRootItem();
    ASSERT_NE(pRootItem, nullptr);
    ASSERT_EQ(pRootItem->childRows(), 6);

    TreeItem* pFirst = pRootItem->child(0);
    ASSERT_NE(pFirst, nullptr);

    QModelIndex idx = tree.model.indexFromItem(pFirst);
    EXPECT_TRUE(idx.isValid());
    EXPECT_EQ(idx.row(), 0);
    EXPECT_EQ(idx.internalPointer(), pFirst);
}

TEST_F(TreeItemModelTest, IndexFromItemRecoversNestedItem) {
    SidebarTree tree;
    TreeItem* pRootItem = tree.model.getRootItem();
    TreeItem* pYear = pRootItem->child(5);
    ASSERT_NE(pYear, nullptr);
    ASSERT_EQ(pYear->childRows(), 1);
    TreeItem* pOld = pYear->child(0);
    ASSERT_NE(pOld, nullptr);

    QModelIndex idx = tree.model.indexFromItem(pOld);
    EXPECT_TRUE(idx.isValid());
    EXPECT_EQ(idx.internalPointer(), pOld);
    // The parent must resolve back to the year group node.
    QModelIndex parentIdx = idx.parent();
    EXPECT_TRUE(parentIdx.isValid());
    EXPECT_EQ(parentIdx.internalPointer(), pYear);
}

TEST_F(TreeItemModelTest, IndexFromItemRejectsRoot) {
    SidebarTree tree;
    EXPECT_FALSE(tree.model.indexFromItem(tree.model.getRootItem()).isValid());
    EXPECT_FALSE(tree.model.indexFromItem(nullptr).isValid());
}
