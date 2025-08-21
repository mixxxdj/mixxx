#pragma once

#include <QStyledItemDelegate>

#include "library/tabledelegates/genrelineeditor.h"

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

    QWidget* createEditor(QWidget* parent,
            const QStyleOptionViewItem&,
            const QModelIndex&) const override;

    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor,
            QAbstractItemModel* model,
            const QModelIndex& index) const override;

  private:
    GenreDao* m_pGenreDao;
};
