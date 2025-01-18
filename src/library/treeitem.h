#pragma once

#include <QIcon>
#include <QList>
#include <QString>
#include <QVariant>
#include <memory>

#include "util/assert.h"

class LibraryFeature;

class TreeItem final {
    struct PrivateRootTag {};

  public:
    static constexpr int kInvalidRow = -1;

    static std::unique_ptr<TreeItem> newRoot(
            LibraryFeature* pFeature) {
        DEBUG_ASSERT(pFeature);
        return std::make_unique<TreeItem>(pFeature, PrivateRootTag{});
    }

    explicit TreeItem(
            QString label = QString(),
            QVariant data = QVariant())
            : TreeItem(nullptr, std::move(label), std::move(data)) {
    }
    // This constructor should actually be private. But that wouldn't
    // work for std::make_unique(). The private, nested tag essentially
    // makes this constructor unavailable for everyone else.
    TreeItem(
            LibraryFeature* pFeature,
            PrivateRootTag)
            : TreeItem(pFeature) {
    }

    ~TreeItem();

    /////////////////////////////////////////////////////////////////////////
    // Full Path Accessors
    /////////////////////////////////////////////////////////////////////////

    // Set the full path
    void setFullPath(const QString& fullPath) {
        m_fullPath = fullPath;
    }

    // Get the full path
    QString fullPath() const {
        return m_fullPath;
    }

    /////////////////////////////////////////////////////////////////////////
    // Feature
    /////////////////////////////////////////////////////////////////////////

    LibraryFeature* feature() const {
        DEBUG_ASSERT(
                !m_pParent ||
                m_pParent->m_pFeature == m_pFeature);
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

    // Eve
    int getRow() const;
    int childCount() const;
    // EVE

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

    TreeItem* appendChild(
            QString label,
            QVariant data = QVariant());

    // multiple child items
    // take ownership of children items
    void insertChildren(int row, std::vector<std::unique_ptr<TreeItem>>&& children);
    void removeChildren(int row, int count);
    void insertChild(int row, std::unique_ptr<TreeItem> pChild);

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
    explicit TreeItem(
            LibraryFeature* pFeature,
            QString label = QString(),
            QVariant data = QVariant());

    void initFeatureRecursively(LibraryFeature* pFeature);

    // The library feature is inherited from the parent.
    // For all child items this is just a shortcut to the
    // library feature of the root item!
    LibraryFeature* m_pFeature;

    TreeItem* m_pParent;

    QList<TreeItem*> m_children; // owned child items

    QString m_label;
    QVariant m_data;
    QIcon m_icon;
    bool m_bold;
    QString m_fullPath;
};
