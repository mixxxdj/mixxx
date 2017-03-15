#ifndef MIXXX_TREEITEM_H
#define MIXXX_TREEITEM_H

#include <QList>
#include <QString>
#include <QVariant>

#include "library/libraryfeature.h"

#include "util/memory.h"


class TreeItem final {
  public:
    static const int kInvalidRow = -1;

    TreeItem();
    explicit TreeItem(
            LibraryFeature* pFeature,
            const QString& label = QString(),
            const QVariant& data = QVariant());
    ~TreeItem();


    /////////////////////////////////////////////////////////////////////////
    // Feature
    /////////////////////////////////////////////////////////////////////////

    LibraryFeature* feature() const {
        return m_pFeature;
    }


    /////////////////////////////////////////////////////////////////////////
    // Parent
    /////////////////////////////////////////////////////////////////////////

    TreeItem* parent() const {
        return m_pParent;
    }
    bool hasParent() const {
        return m_pParent != nullptr;
    }
    bool isRoot() const {
        return !hasParent();
    }
    // Returns the position of this object within its parent
    // or kInvalidRow if this is a root item without a parent.
    int parentRow() const;


    /////////////////////////////////////////////////////////////////////////
    // Children
    /////////////////////////////////////////////////////////////////////////

    bool hasChildren() const {
        return !m_children.empty();
    }
    int childRows() const {
        return m_children.size();
    }
    TreeItem* child(int row) const;
    const QList<TreeItem*>& children() const {
        return m_children;
    }

    // single child items
    TreeItem* appendChild(
            std::unique_ptr<TreeItem> pChild);
    TreeItem* appendChild(
            const QString& label,
            const QVariant& data = QVariant());
    void removeChild(int row);

    // multiple child items
    void insertChildren(QList<TreeItem*>& children, int row, int count); // take ownership
    void removeChildren(int row, int count);


    /////////////////////////////////////////////////////////////////////////
    // Payload
    /////////////////////////////////////////////////////////////////////////

    void setLabel(const QString& label) {
        m_label = label;
    }
    const QString& getLabel() const {
        return m_label;
    }

    void setData(const QVariant& data) {
        m_data = data;
    }
    const QVariant& getData() const {
        return m_data;
    }

    void setIcon(const QIcon& icon) {
        m_icon = icon;
    }
    const QIcon& getIcon() {
        return m_icon;
    }

    void setBold(bool bold) {
        m_bold = bold;
    }
    bool isBold() const {
        return m_bold;
    }

  private:
    void appendChild(TreeItem* pChild);

    LibraryFeature* m_pFeature;

    TreeItem* m_pParent;
    QList<TreeItem*> m_children; // owned child items

    QString m_label;
    QVariant m_data;
    QIcon m_icon;
    bool m_bold;
};

#endif // MIXXX_TREEITEM_H
