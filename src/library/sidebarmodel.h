#pragma once

#include <QAbstractItemModel>
#include <QList>
#include <QModelIndex>
#include <QVariant>

class LibraryFeature;
class QTimer;
class TreeItem;

struct SidebarBookmark {
    SidebarBookmark(
            int row = -1,
            int cLevel = -1,
            int pRow = -1,
            const QVariant& datavar = QVariant(),
            const QString& sLabel = QString(),
            const QModelIndex& dIndex = QModelIndex())
            : featureRow(row),
              childLevel(cLevel),
              parentRow(pRow),
              data(datavar),
              label(sLabel),
              index(dIndex) {
    }
    bool isValid() {
        return featureRow >= 0 &&
                childLevel >= 0 &&
                parentRow >= 0 &&
                (data.isValid() || !label.isEmpty()) &&
                index.isValid();
    }
    bool operator==(const SidebarBookmark& other) const {
        if (featureRow != other.featureRow) {
            return false;
        }
        if (data.isValid() && other.data.isValid()) {
            return data == other.data;
        }
        return label == other.label;
    }
    bool operator<(const SidebarBookmark& other) const {
        if (featureRow == other.featureRow) {
            if (childLevel == other.childLevel) {
                return parentRow < other.parentRow;
            }
            return childLevel < other.childLevel;
        }
        return featureRow < other.featureRow;
    }

    // Store feature row, child level and row number relative to first parent.
    // This allows to sort bookmarks by their position in the tree. We need
    // this because QModelIndex operator<() doesn't work for our multi-level trees.
    // With the feature row we can also quickly ignore irrelevant items when
    // searching for a bookmark a rebuilt child model.
    int featureRow;
    int childLevel;
    int parentRow;
    // The TreeItem data. CrateId, int playlist id or directory path / special
    // node identifier in BrowseFeature.
    QVariant data;
    // For child items that have invalid data (Tracks -> Missing|Hidden and
    // AutoDJ Crates) or when the LibraryFeature says the data is not unique
    // (common playlist id of YEAR nodes).
    QString label;
    // The associated sidebar index. This is used to build the index lookup list
    // for getNextPrevBookmarkIndex().
    // Must be updated when the child model has changed.
    QModelIndex index;
};

inline QDebug operator<<(QDebug dbg, const SidebarBookmark& bm) {
    dbg << "SidebarBookmark" << bm.featureRow << bm.childLevel << bm.parentRow;
    if (bm.data.isValid()) {
        if (bm.data.canConvert<QString>()) {
            dbg << bm.data.toString();
        } else if (bm.data.canConvert<int>()) {
            dbg << bm.data.toInt();
        }
    }
    if (!bm.label.isEmpty()) {
        dbg << bm.label;
    }
    return dbg;
}

class SidebarModel : public QAbstractItemModel {
    Q_OBJECT
  public:
    // Keep object tree functions from QObject accessible
    // for parented_ptr
    using QObject::parent;

    enum Roles {
        IconNameRole = Qt::UserRole + 1,
        DataRole,
    };
    Q_ENUM(Roles);

    explicit SidebarModel(
            QObject* parent = nullptr);
    ~SidebarModel() override = default;

    void addLibraryFeature(LibraryFeature* feature);
    QModelIndex getDefaultSelection();
    void setDefaultSelection(unsigned int index);
    void activateDefaultSelection();

    // Required for QAbstractItemModel
    QModelIndex index(int row, int column,
                      const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index,
                  int role = Qt::DisplayRole) const override;
    bool dropAccept(const QModelIndex& index, const QList<QUrl>& urls, QObject* pSource);
    bool dragMoveAccept(const QModelIndex& index, const QUrl& url) const;
    bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;
    bool hasTrackTable(const QModelIndex& index) const;
    QModelIndex translateChildIndex(const QModelIndex& index) {
        return translateIndex(index, index.model());
    }
    QModelIndex getFeatureRootIndex(LibraryFeature* pFeature);

    void clear(const QModelIndex& index);
    void paste(const QModelIndex& index);

    void toggleBookmarkByIndex(const QModelIndex& selIndex);
    QModelIndex getNextPrevBookmarkIndex(const QModelIndex& selIndex, int direction);

  public slots:
    void pressed(const QModelIndex& index);
    void clicked(const QModelIndex& index);
    void doubleClicked(const QModelIndex& index);
    void rightClicked(const QPoint& globalPos, const QModelIndex& index);
    void renameItem(const QModelIndex& index);
    void deleteItem(const QModelIndex& index);
    void slotFeatureSelect(LibraryFeature* pFeature,
            const QModelIndex& index = QModelIndex(),
            bool scrollTo = true);

    // Slots for every single QAbstractItemModel signal
    // void slotColumnsAboutToBeInserted(const QModelIndex& parent, int start, int end);
    // void slotColumnsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
    // void slotColumnsInserted(const QModelIndex& parent, int start, int end);
    // void slotColumnsRemoved(const QModelIndex& parent, int start, int end);
    void slotDataChanged(const QModelIndex& topLeft, const QModelIndex & bottomRight);
    // void slotHeaderDataChanged(Qt::Orientation orientation, int first, int last);
    // void slotLayoutAboutToBeChanged();
    // void slotLayoutChanged();
    // void slotModelReset();
    void slotRowsAboutToBeInserted(const QModelIndex& parent, int start, int end);
    void slotRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
    void slotRowsInserted(const QModelIndex& parent, int start, int end);
    void slotRowsRemoved(const QModelIndex& parent, int start, int end);
    void slotModelAboutToBeReset();
    void slotModelReset();
    void slotFeatureIsLoading(LibraryFeature*, bool selectFeature);
    void slotFeatureLoadingFinished(LibraryFeature*);

  signals:
    void selectIndex(const QModelIndex& index, bool scrollTo);

  private slots:
    void slotPressedUntilClickedTimeout();

  protected:
    QList<LibraryFeature*> m_sFeatures;

  private:
    QModelIndex translateSourceIndex(const QModelIndex& parent);
    QModelIndex translateIndex(const QModelIndex& index, const QAbstractItemModel* model);
    void featureRenamed(LibraryFeature*);
    unsigned int m_iDefaultSelectedIndex; /** Index of the item in the sidebar model to select at startup. */

    QTimer* const m_pressedUntilClickedTimer;
    QModelIndex m_pressedIndex;
    QList<SidebarBookmark> m_bookmarks;
    QModelIndexList m_bookmarkIndices;

    void startPressedUntilClickedTimer(const QModelIndex& pressedIndex);
    void stopPressedUntilClickedTimer();

    QModelIndexList sortBookmarksUpdateIndices(QList<SidebarBookmark>* pBookmarks);
    QModelIndex getBookmarkIndexByPos(int pos);
    QModelIndex findBookmarkIndex(const SidebarBookmark& bookmark);
    SidebarBookmark createBookmarkFromIndex(const QModelIndex& index);
    void maybeUpdateBookmarkIndices(const QModelIndex& index);
};
