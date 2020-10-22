#pragma once

#include <QAbstractTableModel>
#include <QList>
#include <QPointer>
#include <QTableView>

#include "library/columncache.h"
#include "library/trackmodel.h"
#include "track/track_decl.h"

class BaseCoverArtDelegate;
class TrackCollectionManager;

class BaseTrackTableModel : public QAbstractTableModel, public TrackModel {
    Q_OBJECT
    DISALLOW_COPY_AND_ASSIGN(BaseTrackTableModel);

  public:
    explicit BaseTrackTableModel(
            QObject* parent,
            TrackCollectionManager* const pTrackCollectionManager,
            const char* settingsNamespace);
    ~BaseTrackTableModel() override = default;

    ///////////////////////////////////////////////////////
    //  Overridable functions
    ///////////////////////////////////////////////////////

    virtual int fieldIndex(
            ColumnCache::Column column) const {
        return m_columnCache.fieldIndex(column);
    }

    ///////////////////////////////////////////////////////
    // Inherited from QAbstractItemModel
    ///////////////////////////////////////////////////////

    QVariant headerData(
            int section,
            Qt::Orientation orientation,
            int role = Qt::DisplayRole) const final;
    bool setHeaderData(
            int section,
            Qt::Orientation orientation,
            const QVariant& value,
            int role = Qt::DisplayRole) final;

    QMimeData* mimeData(
            const QModelIndexList& indexes) const final;

    QVariant data(
            const QModelIndex& index,
            int role = Qt::DisplayRole) const final;
    bool setData(
            const QModelIndex& index,
            const QVariant& value,
            int role = Qt::EditRole) final;

    // Calculate the number of columns from all valid
    // column headers.
    // Reimplement in derived classes if a more efficient
    // implementation is available.
    int columnCount(
            const QModelIndex& parent = QModelIndex()) const override;

    // Calls readWriteFlags() by default
    // Reimplement in derived classes if the table model
    // should be readOnly
    Qt::ItemFlags flags(
            const QModelIndex& index) const override;

    ///////////////////////////////////////////////////////
    // Inherited from TrackModel
    ///////////////////////////////////////////////////////

    QAbstractItemDelegate* delegateForColumn(
            const int column,
            QObject* pParent) final;

    int fieldIndex(
            const QString& fieldName) const override {
        return m_columnCache.fieldIndex(fieldName);
    }

    bool isColumnHiddenByDefault(
            int column) override;

    TrackPointer getTrackByRef(
            const TrackRef& trackRef) const override;

  protected:
    static constexpr int defaultColumnWidth() {
        return 50;
    }
    static QStringList defaultTableColumns();

    // Build a map from the column names to their indices
    // used by fieldIndex(). This function has to be called
    void initTableColumnsAndHeaderProperties(
            const QStringList& tableColumns = defaultTableColumns());

    QString columnNameForFieldIndex(int index) const {
        return m_columnCache.columnNameForFieldIndex(index);
    }

    // A simple helper function for initializing header title and width.
    // Note that the ideal width of a column is based on the width of
    // its data, not the title string itself.
    void setHeaderProperties(
            ColumnCache::Column column,
            QString title,
            int defaultWidth = 0);

    ColumnCache::Column mapColumn(int column) const {
        if (column >= 0 && column < m_columnHeaders.size()) {
            return m_columnHeaders[column].column;
        } else {
            return ColumnCache::COLUMN_LIBRARYTABLE_INVALID;
        }
    }

    // Emit the dataChanged() signal for multiple rows in
    // a single column. The list of rows must be sorted in
    // ascending order without duplicates!
    void emitDataChangedForMultipleRowsInColumn(
            const QList<int>& rows,
            int column,
            const QVector<int>& roles = QVector<int>());

    const TrackId previewDeckTrackId() const {
        return m_previewDeckTrackId;
    }

    bool isBpmLocked(
            const QModelIndex& index) const;

    const QPointer<TrackCollectionManager> m_pTrackCollectionManager;

    ///////////////////////////////////////////////////////
    //  Overridable functions
    ///////////////////////////////////////////////////////

    virtual void initHeaderProperties();

    // Use this if you want a model that is read-only.
    virtual Qt::ItemFlags readOnlyFlags(
            const QModelIndex& index) const;
    // Use this if you want a model that can be changed
    virtual Qt::ItemFlags readWriteFlags(
            const QModelIndex& index) const;

    virtual QVariant rawValue(
            const QModelIndex& index) const = 0;

    // Reimplement in derived classes to handle columns other
    // then COLUMN_LIBRARYTABLE
    virtual QVariant roleValue(
            const QModelIndex& index,
            QVariant&& rawValue,
            int role) const;

    virtual bool setTrackValueForColumn(
            const TrackPointer& pTrack,
            int column,
            const QVariant& value,
            int role) {
        Q_UNUSED(pTrack);
        Q_UNUSED(column);
        Q_UNUSED(value);
        Q_UNUSED(role);
        return false;
    }

  private slots:
    void slotTrackLoaded(
            QString group,
            TrackPointer pTrack);

    void slotRefreshCoverRows(
            QList<int> rows);

    void slotRefreshAllRows();

  private:
    // Track models may reference tracks by an external id
    // TODO: TrackId should only be used for tracks from
    // the internal database.
    virtual TrackId doGetTrackId(
            const TrackPointer& pTrack) const;
    virtual BaseCoverArtDelegate* doCreateCoverArtDelegate(
            QTableView* pTableView) const = 0;

    Qt::ItemFlags defaultItemFlags(
            const QModelIndex& index) const;

    QList<QUrl> collectUrls(
            const QModelIndexList& indexes) const;

    const QString m_previewDeckGroup;

    double m_backgroundColorOpacity;

    ColumnCache m_columnCache;

    struct ColumnHeader {
        ColumnCache::Column column = ColumnCache::COLUMN_LIBRARYTABLE_INVALID;
        QHash</*role*/ int, QVariant> header;
    };
    QVector<ColumnHeader> m_columnHeaders;

    int countValidColumnHeaders() const;

    TrackId m_previewDeckTrackId;
};
