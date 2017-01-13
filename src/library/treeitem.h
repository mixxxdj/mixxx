#ifndef MIXXX_TREEITEM_H
#define MIXXX_TREEITEM_H

#include <QList>
#include <QString>
#include <QVariant>
#include <QIcon>

#include "library/libraryfeature.h"
#include "util/memory.h"

class CoverInfo;

class TreeItem final {
  public:
    static const int kInvalidRow = -1;
    
	TreeItem(); //creates an invisible root item for the tree
    explicit TreeItem(
            LibraryFeature* pFeature,
            const QString& label = QString(),
            const QVariant& data = QVariant());
    ~TreeItem();
    
	/////////////////////////////////////////////////////////////////////////
    // Feature
    /////////////////////////////////////////////////////////////////////////

    inline LibraryFeature* feature() const {
        return m_pFeature;
    }


    /////////////////////////////////////////////////////////////////////////
    // Parent
    /////////////////////////////////////////////////////////////////////////

    inline TreeItem* parent() const {
        return m_pParent;
    }
    inline bool hasParent() const {
        return m_pParent != nullptr;
    }
    inline bool isRoot() const {
        return !hasParent();
    }
    // Returns the position of this object within its parent
    // or kInvalidRow if this is a root item without a parent.
    int parentRow() const;


    /////////////////////////////////////////////////////////////////////////
    // Children
    /////////////////////////////////////////////////////////////////////////

    inline bool hasChildren() const {
        return !m_children.empty();
    }
    inline int childRows() const {
        return m_children.size();
    }
    TreeItem* child(int row) const;

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

    inline void setLabel(const QString& label) {
        m_label = label;
    }

    inline const QString& getLabel() const {
		if (m_trackCount >= 0) {
			return m_labelNumbered;
		}
        return m_label;
    }

    inline void setData(const QVariant& data) {
        m_data = data;
    }
    inline const QVariant& getData() const {
        return m_data;
    }

    inline void setIcon(const QIcon& icon) {
        m_icon = icon;
    }
    inline const QIcon& getIcon() {
        return m_icon;
    }
    
	// Returns true if we have a leaf node
    bool isPlaylist() const;
    // Returns true if we have an inner node
    bool isFolder() const;

	inline void setBold(bool bold) {
		m_bold = bold;
	}
	inline bool isBold() const {
		return m_bold;
	}

	inline void setDivider(bool divider) {
		m_divider = divider;
	}
	inline bool isDivider() const {
		return m_divider;
	}
    
	inline void setCoverInfo(const CoverInfo& cover) {
		m_cover = cover;
	}
	inline const CoverInfo& getCoverInfo() const {
		return m_cover;
	}
    
	inline void setTrackCount(int count) {
		m_trackCount = count;
		m_labelNumbered = m_label + " (" + QString::number(m_trackCount) + ")";
	}

	inline int getTrackCount() {
		return m_trackCount;
	}

  private:
    void appendChild(TreeItem* pChild);
	void insertChild(TreeItem* pChild, int row);

    TreeItem* m_pParent;
    QList<TreeItem*> m_children; // owned child items
    
	LibraryFeature* m_pFeature;
    
    QString m_label;
	QString m_labelNumbered;
    QVariant m_data;
    QIcon m_icon;
    CoverInfo m_cover;

	bool m_divider;
    bool m_bold;
    int m_trackCount;
};

#endif // MIXXX_TREEITEM_H
