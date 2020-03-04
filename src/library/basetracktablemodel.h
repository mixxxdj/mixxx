#pragma once

#include <QAbstractTableModel>
#include <QList>

#include "library/trackmodel.h"

class BaseTrackTableModel : public QAbstractTableModel, public TrackModel {
    Q_OBJECT
    DISALLOW_COPY_AND_ASSIGN(BaseTrackTableModel);

  public:
    explicit BaseTrackTableModel(
            QSqlDatabase db,
            const char* settingsNamespace,
            QObject* parent = nullptr);
    ~BaseTrackTableModel() override = default;

  protected:
    // Emit the dataChanged() signal for multiple rows in
    // a single column. The list of rows must be sorted in
    // ascending order without duplicates!
    void emitDataChangedForMultipleRowsSingleColumn(
            const QList<int>& rows,
            int column,
            const QVector<int>& roles = QVector<int>());
};
