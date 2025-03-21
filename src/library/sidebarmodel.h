#pragma once

#include <QAbstractItemModel>
#include <QList>
#include <QModelIndex>
#include <QPoint>
#include <QVariant>

class LibraryFeature;
class QTimer;
class TreeItem;
class TreeItemModel;

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

    // TODO Make class. move to own file?
    struct Bookmark {
        Bookmark(
                int row = -1,
                const QVariant& datavar = QVariant(),
                int cLevel = -1,
                int pRow = -1)
                : featureRow(row),
                  data(datavar),
                  childLevel(cLevel),
                  parentRow(pRow) {
        }
        bool isValid() {
            // qWarning() << "Bookmark.isValid? fRrow" << featureRow
            //           << " level:" << childLevel << "pRow:" << parentRow;
            return featureRow >= 0 &&
                    childLevel >= 0 &&
                    parentRow >= 0;
            // No need to compare data for level > 0, Missing/Hidden and AutoDJ
            // crates items can also have no data.
        }
        bool operator==(const Bookmark& other) const {
            return featureRow == other.featureRow &&
                    data == other.data &&
                    childLevel == other.childLevel;
        }
        bool operator<(Bookmark& other) const {
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
        // Store child level and row number relative to first parent.
        // This allows to sort bookmarks by their position in the tree.
        // Note: this might break when a Browse path's tree is rebuilt, and when
        // a new YEAR node is added.
        // We might listen to all TreeItemModels' signals rowsRemoved(),
        // rowsInserted() and modelReset() and re-evaluate the bookmark's level,
        // but this seems overkill.
        int childLevel;
        int parentRow;
    };

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

    void bookmarkSelectedItem(const QModelIndex& selIndex);
    QModelIndex selectNextPrevBookmark(const QModelIndex& selIndex, int direction);

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
    QList<Bookmark> m_bookmarks;
    QModelIndex m_bookmarkIndex;

    void startPressedUntilClickedTimer(const QModelIndex& pressedIndex);
    void stopPressedUntilClickedTimer();

    QModelIndex selectBookmarkByPos(int pos);
    QModelIndex getBookmarkIndex(
            const TreeItem* pItem,
            const QModelIndex& parent,
            const SidebarModel::Bookmark& bookmark);
    Bookmark createBookmarkFromIndex(const QModelIndex& index);
    bool treeItemIsBookmark(const TreeItem* pTreeItem) const;
    bool featureRootIsBookmark(int featureRow) const;
};
