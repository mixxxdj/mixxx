// sidebarmodel.h
// Created 8/21/09 by RJ Ryan (rryan@mit.edu)

#ifndef SIDEBARMODEL_H
#define SIDEBARMODEL_H

#include <QAbstractItemModel>
#include <QList>
#include <QModelIndex>
#include <QTimer>
#include <QVariant>

class LibraryFeature;

class SidebarModel : public QAbstractItemModel {
    Q_OBJECT
  public:
    // Keep object tree functions from QObject accessible
    // for parented_ptr
    using QObject::parent;

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
    bool dropAccept(const QModelIndex& index, QList<QUrl> urls, QObject* pSource);
    bool dragMoveAccept(const QModelIndex& index, QUrl url);
    bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;
    bool hasTrackTable(const QModelIndex& index) const;

  public slots:
    void pressed(const QModelIndex& index);
    void clicked(const QModelIndex& index);
    void doubleClicked(const QModelIndex& index);
    void rightClicked(const QPoint& globalPos, const QModelIndex& index);
    void slotFeatureSelect(LibraryFeature* pFeature, const QModelIndex& index = QModelIndex());

    // Slots for every single QAbstractItemModel signal
    // void slotColumnsAboutToBeInserted(const QModelIndex& parent, int start, int end);
    // void slotColumnsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
    // void slotColumnsInserted(const QModelIndex& parent, int start, int end);
    // void slotColumnsRemoved(const QModelIndex& parent, int start, int end);
    void slotDataChanged(const QModelIndex& topLeft, const QModelIndex & bottomRight);
    //void slotHeaderDataChanged(Qt::Orientation orientation, int first, int last);
    // void slotLayoutAboutToBeChanged();
    // void slotLayoutChanged();
    // void slotModelAboutToBeReset();
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
    void featureRenamed(LibraryFeature*);
    QList<LibraryFeature*> m_sFeatures;
    unsigned int m_iDefaultSelectedIndex; /** Index of the item in the sidebar model to select at startup. */

    QTimer* const m_pressedUntilClickedTimer;
    QModelIndex m_pressedIndex;

    void startPressedUntilClickedTimer(QModelIndex pressedIndex);
    void stopPressedUntilClickedTimer();
};

#endif /* SIDEBARMODEL_H */
