// treeitem.h
// Created 10/02/2010 by Tobias Rafreider

#ifndef TREEITEM_H
#define TREEITEM_H

#include <QList>
#include <QString>
#include <QVariant>

#include "library/libraryfeature.h"

class TreeItem {
  public:
    TreeItem(); //creates an invisible root item for the tree
    TreeItem(const QString &data,
             const QString &data_path,
             LibraryFeature* feature,
             TreeItem* parent);
    ~TreeItem();
    /** appends a child item to this object **/
    void appendChild(TreeItem *child);
    /** remove a child item at the given index **/
    void removeChild(int index);
    /** returns the tree item at position 'row' in the childlist **/
    TreeItem *child(int row);
    /** returns the number of childs of this tree item **/
    int childCount() const;
    /** Returns the position of this object within its parent **/
    int row() const;
    /** returns the parent **/
    TreeItem *parent();

    /** for dynamic resizing models **/
    bool insertChildren(QList<TreeItem*> &data, int position, int count);
    /** Removes <count> children from the child list starting at index <position> **/
    bool removeChildren(int position, int count);

    /** sets data **/
    bool setData(const QVariant &data, const QVariant &data_path);
    /** simple name of the playlist **/
    QVariant data() const;
    /** Full path of the playlist **/
    QVariant dataPath() const;
    /** Returns true if we have a leaf node **/
    bool isPlaylist() const;
    /** returns true if we have an inner node **/
    bool isFolder() const;
    /* Returns the Library feature object to which an item belongs to */
    LibraryFeature* getFeature();

    void setIcon(const QIcon& icon);
    QIcon getIcon();

  private:
    QList<TreeItem*> m_childItems;
    QString m_dataPath;
    QString m_data;
    LibraryFeature* m_feature;

    TreeItem *m_parentItem;
    QIcon m_icon;
};

#endif
