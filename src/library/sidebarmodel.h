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
            const QVariant& datavar = QVariant(),
            const QString& sLabel = QString(),
            int cLevel = -1,
            int pRow = -1,
            const QModelIndex& dIndex = QModelIndex())
            : featureRow(row),
              data(datavar),
              label(sLabel),
              childLevel(cLevel),
              parentRow(pRow),
              index(dIndex) {
    }
    bool isValid() {
        // qWarning() << "SidebarBookmark.isValid? fRrow" << featureRow
        //           << " level:" << childLevel << "pRow:" << parentRow;
        return featureRow >= 0 &&
                (data.isValid() || !label.isEmpty()) &&
                childLevel >= 0 &&
                parentRow >= 0 &&
                index.isValid();
        // There is no point in validating data for levels > 0 because
        // Missing/Hidden and 'AutoDJ Crates' items can also have no data.
    }
    bool operator==(const SidebarBookmark& other) const {
        if (featureRow != other.featureRow) {
            return false;
        }
        if (data.isValid() && other.data.isValid()) {
            return data == other.data;
        }
        // Invalid data only happens with Tracks: Missing|Hidden and AutoDJ: Crates,
        // and for those we compare the labels
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

    int featureRow;
    QVariant data;
    // We store the label only for child items that have invalid data. Currently
    // the only dataless children are Tracks: Missing|Hidden and AutoDJ: Crates
    QString label;
    // Store child level and row number relative to first parent in order to allow
    // sorting bookmarks by their position in the tree,
    // Note: this might break when a BrowseFeature path's tree is rebuilt anddirectories
    // have been moved in the hierarchy, and when features with customizable
    // nesting are added, eg. nested crates.
    int childLevel;
    int parentRow;
    // The associated sidebar index. Must be updated when the child model is changed.
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

    bool indexIsBookmark(const QModelIndex& index) const;

  public slots:
    void pressed(const QModelIndex& index);
    void clicked(const QModelIndex& index);
    void doubleClicked(const QModelIndex& index);
    void rightClicked(const QPoint& globalPos, const QModelIndex& index);
    void renameItem(const QModelIndex& index);
    void deleteItem(const QModelIndex& index);
    void slotFeatureSelect(LibraryFeature* pFeature, const QModelIndex& index = QModelIndex());

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
    void selectIndex(const QModelIndex& index);

  private slots:
    void slotPressedUntilClickedTimeout();

  private:
    QModelIndex translateSourceIndex(const QModelIndex& parent);
    QModelIndex translateIndex(const QModelIndex& index, const QAbstractItemModel* model);
    void featureRenamed(LibraryFeature*);
    QList<LibraryFeature*> m_sFeatures;
    unsigned int m_iDefaultSelectedIndex; /** Index of the item in the sidebar model to select at startup. */

    QTimer* const m_pressedUntilClickedTimer;
    QModelIndex m_pressedIndex;
    QList<SidebarBookmark> m_bookmarks;
    QModelIndexList m_bookmarkIndices;

    void startPressedUntilClickedTimer(const QModelIndex& pressedIndex);
    void stopPressedUntilClickedTimer();

    void sortBookmarksUpdateIndices();
    QModelIndex getBookmarkIndexByPos(int pos);
    QModelIndex findBookmarkIndex(const SidebarBookmark& bookmark, const TreeItem* pItem);
    SidebarBookmark createBookmarkFromIndex(const QModelIndex& index);
    void maybeUpdateBookmarkIndices(const QModelIndex& index);
};
