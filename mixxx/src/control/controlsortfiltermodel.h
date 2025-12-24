#pragma once

#include <QSortFilterProxyModel>
#include <QString>
#ifdef MIXXX_USE_QML
#include <QQmlEngine>
#else
#define QML_ELEMENT
#endif

#include "control/controlmodel.h"

class ControlSortFilterModel : public QSortFilterProxyModel {
    Q_OBJECT
    Q_PROPERTY(int sortColumn READ sortColumn NOTIFY sortColumnChanged)
    Q_PROPERTY(bool sortDescending READ sortDescending NOTIFY sortDescendingChanged)
    QML_ELEMENT
  public:
    ControlSortFilterModel(QObject* pParent = nullptr);
    virtual ~ControlSortFilterModel();

    bool sortDescending() const;

    Q_INVOKABLE void sortByColumn(int sortColumn, bool sortDescending);

  signals:
    void sortColumnChanged(int sortColumn);
    void sortDescendingChanged(bool sortDescending);

  private:
    ControlModel* m_pModel;
};
