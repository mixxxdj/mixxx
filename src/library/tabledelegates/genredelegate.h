#pragma once

#include <QStyledItemDelegate>
// #include "library/dao/playlistdao.h"
// #include "library/basesqltablemodel.h"

class BaseSqlTableModel;

class GenreDelegate : public QStyledItemDelegate {
    Q_OBJECT
  public:
    explicit GenreDelegate(QObject* parent = nullptr);

    QString displayText(const QVariant& value, const QLocale& locale) const override;
};
