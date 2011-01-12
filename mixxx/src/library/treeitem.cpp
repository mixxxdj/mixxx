// TreeItem.cpp
// Created 10/02/2010 by Tobias Rafreider

#include <QStringList>
#include "treeitem.h"

TreeItem::TreeItem(const QString &data, const QString &data_path, LibraryFeature* feature, TreeItem* parent)
 {
     m_data = data;
     m_dataPath = data_path;
     m_parentItem = parent;
     m_feature = feature;
 }

 TreeItem::~TreeItem()
 {
     qDeleteAll(m_childItems);
 }

 void TreeItem::appendChild(TreeItem *item)
 {
     m_childItems.append(item);
 }

 TreeItem *TreeItem::child(int row)
 {
     return m_childItems.value(row);
 }

 int TreeItem::childCount() const
 {
     return m_childItems.count();
 }


 QVariant TreeItem::data() const
 {
     return m_data;
 }
 QVariant TreeItem::dataPath() const
 {
     return m_dataPath;
 }
 bool TreeItem::isPlaylist() const
 {
     return (m_childItems.count() == 0);
 }
bool TreeItem::isFolder() const
 {
     return (m_childItems.count() != 0);
 }

 TreeItem *TreeItem::parent()
 {
     return m_parentItem;
 }

 int TreeItem::row() const
 {
     if (m_parentItem)
         return m_parentItem->m_childItems.indexOf(const_cast<TreeItem*>(this));

     return 0;
 }
 LibraryFeature* TreeItem::getFeature()
 {
     return m_feature;
 }
bool TreeItem::insertChildren(QList<TreeItem*> &data, int position, int count)
 {
     if (position < 0 || position > m_childItems.size())
         return false;

     for (int row = 0; row < count; ++row) {
         TreeItem* item = data.at(row);
         m_childItems.insert(row, item);
     }

     return true;
 }
bool TreeItem::removeChildren(int position, int count)
 {
    if (position < 0 || position + count > m_childItems.size())
        return false;

    for (int row = 0; row < count; ++row){
        //Remove from list to avoid invalid pointers
        TreeItem* item = m_childItems.takeAt(position);
        if(item) delete item;
    }
    return true;
 }

bool TreeItem::setData(const QVariant &data, const QVariant &data_path)
 {
    m_data = data.toString();
    m_dataPath = data_path.toString();
    return true;
 }
