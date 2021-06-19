#pragma once

#include <QStyledItemDelegate>

class MidiOpCodeDelegate : public QStyledItemDelegate {
  public:
    MidiOpCodeDelegate(QObject* pParent);
    virtual ~MidiOpCodeDelegate();

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const;

    QString displayText(const QVariant& value, const QLocale& locale) const;

    void setEditorData(QWidget* editor, const QModelIndex& index) const;

    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const;
};
