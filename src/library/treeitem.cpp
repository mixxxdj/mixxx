#include "library/treeitem.h"

/*
 * Just a word about how the TreeItem objects and TreeItemModels are used in general:
 * TreeItems are used by the TreeItemModel class to display tree
 * structures in the sidebar.
 *
 * The constructor has 4 arguments:
 * 1. argument represents a name shown in the sidebar view later on
 * 2. argument represents the absolute path of this tree item
 * 3. argument is a library feature object.
 *    This is necessary because in sidebar.cpp we handle 'activateChid' events
 * 4. the parent TreeItem object
 *    The constructor does not add this TreeItem object to the parent's child list
 *
 * In case of no arguments, the standard constructor creates a
 * root item that is not visible in the sidebar.
 *
 * Once the TreeItem objects are inserted to models, the models take care of their
 * deletion.
 *
 * Examples on how to use TreeItem and TreeItemModels can be found in
 * - playlistfeature.cpp
 * - cratefeature.cpp
 * - *feature.cpp
 */

TreeItem::TreeItem(
        LibraryFeature* pFeature,
        QString label,
        QVariant data)
    : m_pFeature(pFeature),
      m_pParent(nullptr),
      m_label(std::move(label)),
      m_data(std::move(data)),
      m_bold(false) {
}

TreeItem::~TreeItem() {
    qDeleteAll(m_children);
}

int TreeItem::parentRow() const {
    if (m_pParent) {
        return m_pParent->m_children.indexOf(const_cast<TreeItem*>(this));
    } else {
        return kInvalidRow;
    }
}

TreeItem* TreeItem::child(int row) const {
    DEBUG_ASSERT(row >= 0);
    VERIFY_OR_DEBUG_ASSERT(row < m_children.size()) {
        return nullptr;
    }
    return m_children[row];
}

void TreeItem::insertChild(int row, TreeItem* pChild) {
    DEBUG_ASSERT(pChild);
    DEBUG_ASSERT(!pChild->m_pParent);
    DEBUG_ASSERT(!pChild->m_pFeature ||
            pChild->m_pFeature == m_pFeature);
    DEBUG_ASSERT(row >= 0);
    DEBUG_ASSERT(row <= m_children.size());
    m_children.insert(row, pChild);
    pChild->m_pParent = this;
    pChild->initFeatureRecursively(m_pFeature);
}

void TreeItem::initFeatureRecursively(LibraryFeature* pFeature) {
    DEBUG_ASSERT(!m_pFeature ||
            m_pFeature == pFeature);
    DEBUG_ASSERT(!m_pParent ||
            m_pParent->m_pFeature == pFeature);
    if (m_pFeature == pFeature) {
        return;
    }
    m_pFeature = pFeature;
    for (auto* pChild : qAsConst(m_children)) {
        pChild->initFeatureRecursively(pFeature);
    }
}

TreeItem* TreeItem::appendChild(
        std::unique_ptr<TreeItem> pChild) {
    insertChild(m_children.size(), pChild.get()); // transfer ownership
    return pChild.release();
}

TreeItem* TreeItem::appendChild(
        QString label,
        QVariant data) {
    auto pNewChild = std::make_unique<TreeItem>(
            std::move(label),
            std::move(data));
    return appendChild(std::move(pNewChild));
}

void TreeItem::removeChild(int row) {
    DEBUG_ASSERT(row >= 0);
    DEBUG_ASSERT(row < m_children.size());
    delete m_children.takeAt(row);
}

void TreeItem::insertChildren(int row, QList<TreeItem*>& children) {
    DEBUG_ASSERT(row >= 0);
    DEBUG_ASSERT(row <= m_children.size());
    while (!children.isEmpty()) {
        TreeItem* pChild = children.front();
        insertChild(row++, pChild);
        children.pop_front();
    }
}

void TreeItem::removeChildren(int row, int count) {
    DEBUG_ASSERT(count >= 0);
    DEBUG_ASSERT(count <= m_children.size());
    DEBUG_ASSERT(row >= 0);
    DEBUG_ASSERT(row <= (m_children.size() - count));
    qDeleteAll(m_children.begin() + row, m_children.begin() + (row + count));
    m_children.erase(m_children.begin() + row, m_children.begin() + (row + count));
}
