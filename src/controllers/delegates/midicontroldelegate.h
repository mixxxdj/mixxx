#ifndef MIDICONTROLDELEGATE_H
#define MIDICONTROLDELEGATE_H

#include <QStyledItemDelegate>

class MidiControlDelegate : public QStyledItemDelegate {
  public:
    MidiControlDelegate(QObject* pParent);
    virtual ~MidiControlDelegate();

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const;

    QString displayText(const QVariant& value, const QLocale& locale) const;

    void setEditorData(QWidget* editor, const QModelIndex& index) const;

    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const;
};

#endif /* MIDICONTROLDELEGATE_H */
