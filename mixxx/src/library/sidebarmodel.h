// sidebarmodel.h
// Created 8/21/09 by RJ Ryan (rryan@mit.edu)

#ifndef SIDEBARMODEL_H
#define SIDEBARMODEL_H

#include <QAbstractItemModel>
#include <QList>
#include <QModelIndex>
#include <QVariant>

class LibraryFeature;

class SidebarModel : public QAbstractItemModel {
    Q_OBJECT
  public:
    explicit SidebarModel(QObject* parent = 0);
    virtual ~SidebarModel();

    void addLibraryFeature(LibraryFeature* feature);
    QModelIndex getDefaultSelection();
    void setDefaultSelection(unsigned int index);
    void activateDefaultSelection();

    // Required for QAbstractItemModel
    QModelIndex index(int row, int column,
                      const QModelIndex& parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex& index) const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index,
                  int role = Qt::DisplayRole ) const;
    bool dropAccept(const QModelIndex& index, QList<QUrl> urls);
    bool dragMoveAccept(const QModelIndex& index, QUrl url);
    virtual bool hasChildren ( const QModelIndex & parent = QModelIndex() ) const;

  public slots:
    void clicked(const QModelIndex& index);
    void doubleClicked(const QModelIndex& index);
    void rightClicked(const QPoint& globalPos, const QModelIndex& index);
    void slotFeatureSelect(LibraryFeature* pFeature, const QModelIndex& index = QModelIndex());

    // Slots for every single QAbstractItemModel signal
    // void slotColumnsAboutToBeInserted(const QModelIndex& parent, int start, int end);
    // void slotColumnsAboutToBeRemoved(const QModelIndex & parent, int start, int end);
    // void slotColumnsInserted(const QModelIndex & parent, int start, int end);
    // void slotColumnsRemoved(const QModelIndex & parent, int start, int end);
    void slotDataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight);
    //void slotHeaderDataChanged(Qt::Orientation orientation, int first, int last);
    // void slotLayoutAboutToBeChanged();
    // void slotLayoutChanged();
    // void slotModelAboutToBeReset();
    // void slotModelReset();
    void slotRowsAboutToBeInserted(const QModelIndex& parent, int start, int end);
    void slotRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
    void slotRowsInserted(const QModelIndex& parent, int start, int end);
    void slotRowsRemoved(const QModelIndex& parent, int start, int end);
    void slotModelReset();
    void slotFeatureIsLoading(LibraryFeature*);
    void slotFeatureLoadingFinished(LibraryFeature*);

  signals:
    void selectIndex(const QModelIndex& index);

  private:
    QModelIndex translateSourceIndex(const QModelIndex& parent);
    void featureRenamed(LibraryFeature*);
    QList<LibraryFeature*> m_sFeatures;
    unsigned int m_iDefaultSelectedIndex; /** Index of the item in the sidebar model to select at startup. */
};

#endif /* SIDEBARMODEL_H */
