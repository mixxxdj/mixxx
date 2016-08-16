// TreeItem.cpp
// Created 10/02/2010 by Tobias Rafreider

#include <QString>
#include <QStringList>

#include "library/coverart.h"
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
 *    This is necessary because in sidebar.cpp we hanlde 'activateChid' events
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
TreeItem::TreeItem(const QVariant& data, const QVariant& dataPath,
                   LibraryFeature* pFeature, TreeItem* parent)
        : m_data(data),
          m_dataPath(dataPath),
          m_pFeature(pFeature),
          m_bold(false),
          m_divider(false),
          m_trackCount(-1),
          m_pParent(parent) {
}

TreeItem::TreeItem(LibraryFeature* pFeature)
        : m_data("$root"),
          m_dataPath("$root"),
          m_pFeature(pFeature),
          m_bold(false),
          m_divider(false),
          m_trackCount(-1),
          m_pParent(nullptr) {
    
}

TreeItem::TreeItem(TreeItem* parent)
        : m_data("$root"),
          m_dataPath("$root"),
          m_pFeature(nullptr),
          m_bold(false),
          m_divider(false),
          m_trackCount(-1),
          m_pParent(parent) {

}

TreeItem::TreeItem()
        : m_data("$root"),
          m_dataPath("$root"),
          m_pFeature(nullptr),
          m_bold(false),
          m_divider(false),
          m_trackCount(-1),
          m_pParent(nullptr) {
}

TreeItem::~TreeItem() {
    qDeleteAll(m_childItems);
}

void TreeItem::appendChild(TreeItem* item) {
    m_childItems.append(item);
}

void TreeItem::removeChild(int index) {
    m_childItems.removeAt(index);
}

TreeItem* TreeItem::child(int row) {
    return m_childItems.value(row);
}

int TreeItem::childCount() const {
    return m_childItems.count();
}

QVariant TreeItem::data() const {
    if (m_trackCount >= 0) {
        return m_data.toString() + " (" + QString::number(m_trackCount) + ")";
    }
    
    return m_data;
}

QVariant TreeItem::dataPath() const {
    return m_dataPath;
}

bool TreeItem::isPlaylist() const {
    return (m_childItems.count() == 0);
}

bool TreeItem::isFolder() const {
    return (m_childItems.count() != 0);
}

TreeItem* TreeItem::parent() {
    return m_pParent;
}

void TreeItem::setParent(TreeItem* parent) {
    m_pParent = parent;
}

int TreeItem::row() const {
    if (m_pParent) {
        return m_pParent->m_childItems.indexOf(const_cast<TreeItem*>(this));
    }

    return 0;
}

LibraryFeature* TreeItem::getFeature() {
    return m_pFeature;
}

void TreeItem::setLibraryFeature(LibraryFeature* pFeature) {
    m_pFeature = pFeature;
}

void TreeItem::setBold(bool bold) {
    m_bold = bold;
}

bool TreeItem::isBold() const {
    return m_bold;
}

void TreeItem::setDivider(bool divider) {
    m_divider = divider;
}

bool TreeItem::isDivider() const {
    return m_divider;
}

bool TreeItem::insertChildren(QList<TreeItem*>& data, int position, int count) {
    if (position < 0 || position > m_childItems.size()) {
        return false;
    }

    for (int row = 0; row < count; ++row) {
        TreeItem* item = data.at(row);
        m_childItems.insert(position + row, item);
    }

    return true;
}

bool TreeItem::removeChildren(int position, int count) {
    if (position < 0 || position + count > m_childItems.size()) {
        return false;
    }

    for (int row = 0; row < count; ++row) {
        //Remove from list to avoid invalid pointers
        TreeItem* item = m_childItems.takeAt(position);
        if (item) {
            delete item;
        }
    }
    return true;
}

void TreeItem::setData(const QVariant& data) {
    m_data = data;
}

void TreeItem::setData(const QVariant& data, const QVariant& dataPath) {
    m_data = data;
    m_dataPath = dataPath;
}

void TreeItem::setDataPath(const QVariant& dataPath) {
    m_dataPath = dataPath;
}

QIcon TreeItem::getIcon() const {
    return m_icon;
}

void TreeItem::setIcon(const QIcon& icon) {
    m_icon = icon;
}

void TreeItem::setCoverInfo(const CoverInfo &cover) {
    m_cover = cover;
}

const CoverInfo& TreeItem::getCoverInfo() const {
    return m_cover;
}

void TreeItem::setTrackCount(int count) {
    m_trackCount = count;
}

int TreeItem::getTrackCount() {
    return m_trackCount;
}
