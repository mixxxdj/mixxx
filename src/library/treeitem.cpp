#include "library/treeitem.h"

#include "util/make_const_iterator.h"

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

void TreeItem::insertChild(int row, std::unique_ptr<TreeItem> pChild) {
    DEBUG_ASSERT(pChild);
    DEBUG_ASSERT(!pChild->m_pParent);
    DEBUG_ASSERT(!pChild->m_pFeature ||
            pChild->m_pFeature == m_pFeature);
    DEBUG_ASSERT(row >= 0);
    DEBUG_ASSERT(row <= m_children.size());
    pChild->m_pParent = this;
    pChild->initFeatureRecursively(m_pFeature);
    m_children.insert(row, pChild.release()); // transfer ownership
}

void TreeItem::initFeatureRecursively(LibraryFeature* pFeature) {
    DEBUG_ASSERT(!m_pParent ||
            m_pParent->m_pFeature == pFeature);
    if (m_pFeature == pFeature) {
        return;
    }
    DEBUG_ASSERT(!m_pFeature);
    m_pFeature = pFeature;
    for (auto* pChild : std::as_const(m_children)) {
        pChild->initFeatureRecursively(pFeature);
    }
}

TreeItem* TreeItem::appendChild(
        QString label,
        QVariant data) {
    auto pNewChild = std::make_unique<TreeItem>(
            std::move(label),
            std::move(data));
    TreeItem* pRet = pNewChild.get();
    insertChild(m_children.size(), std::move(pNewChild));
    return pRet;
}

void TreeItem::insertChildren(int row, std::vector<std::unique_ptr<TreeItem>>&& children) {
    DEBUG_ASSERT(row >= 0);
    DEBUG_ASSERT(row <= m_children.size());
    for (auto&& pChild : children) {
        insertChild(row++, std::move(pChild));
    }
}

void TreeItem::removeChildren(int row, int count) {
    DEBUG_ASSERT(count >= 0);
    DEBUG_ASSERT(count <= m_children.size());
    DEBUG_ASSERT(row >= 0);
    DEBUG_ASSERT(row <= (m_children.size() - count));
    qDeleteAll(m_children.constBegin() + row, m_children.constBegin() + (row + count));
    constErase(&m_children, m_children.constBegin() + row, m_children.constBegin() + (row + count));
}

int TreeItem::getRow() const {
    if (m_pParent) {
        return m_pParent->m_children.indexOf(const_cast<TreeItem*>(this));
    }
    return kInvalidRow; // If this is a root item or parent is missing
}

int TreeItem::childCount() const {
    return m_children.size();
}
