// treeitem.h
// Created 10/02/2010 by Tobias Rafreider

#ifndef TREEITEM_H
#define TREEITEM_H

#include <QList>
#include <QString>
#include <QVariant>
#include <QIcon>

#include "library/libraryfeature.h"

class CoverInfo;

class TreeItem {
  public:
    TreeItem(); //creates an invisible root item for the tree
    TreeItem(const QVariant &data,
             const QVariant &dataPath,
             LibraryFeature* pFeature,
             TreeItem* parent);
    TreeItem(LibraryFeature* pFeature);
    TreeItem(TreeItem* parent);
    ~TreeItem();
    /** appends a child item to this object **/
    void appendChild(TreeItem* child);
    /** remove a child item at the given index **/
    void removeChild(int index);
    /** returns the tree item at position 'row' in the child list **/
    TreeItem *child(int row);
    /** returns the number of childs of this tree item **/
    int childCount() const;
    /** Returns the position of this object within its parent **/
    int row() const;
    /** returns the parent **/
    TreeItem* parent();
    /** sets the parent **/
    void setParent(TreeItem* parent);

    /** for dynamic resizing models **/
    bool insertChildren(QList<TreeItem*>& data, int position, int count);
    /** Removes <count> children from the child list starting at index <position> **/
    bool removeChildren(int position, int count);

    /** sets data **/
    void setData(const QVariant& data);
    void setData(const QVariant& data, const QVariant& dataPath);
    void setDataPath(const QVariant& dataPath);
    /** simple name of the playlist **/
    QVariant data() const;
    /** Full path of the playlist **/
    QVariant dataPath() const;
    /** Returns true if we have a leaf node **/
    bool isPlaylist() const;
    /** returns true if we have an inner node **/
    bool isFolder() const;
    // Returns the Library feature object to which an item belongs to
    LibraryFeature* getFeature();
    
    void setLibraryFeature(LibraryFeature* pFeature);

    void setBold(bool bold);
    bool isBold() const;

    void setDivider(bool divider);
    bool isDivider() const;

    void setIcon(const QIcon& icon);
    QIcon getIcon() const;
    
    void setCoverInfo(const CoverInfo& cover);
    const CoverInfo& getCoverInfo() const;
    
    void setTrackCount(int count);
    int getTrackCount();

  private:
    QList<TreeItem*> m_childItems;
    QVariant m_data;
    QVariant m_dataPath;
    LibraryFeature* m_pFeature;
    bool m_bold;
    bool m_divider;
    int m_trackCount;

    TreeItem* m_pParent;
    QIcon m_icon;
    CoverInfo m_cover;
};

#endif
