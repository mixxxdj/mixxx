#pragma once

#include <QStyledItemDelegate>

class BaseSqlTableModel;
class GenreDao;

class GenreDelegate : public QStyledItemDelegate {
    Q_OBJECT
  public:
    explicit GenreDelegate(GenreDao* pGenreDao, QObject* parent = nullptr);

    QString displayText(const QVariant& value, const QLocale& locale) const override;
    bool helpEvent(QHelpEvent* event,
            QAbstractItemView* view,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) override;

  private:
    GenreDao* m_pGenreDao;
};
