#pragma once

#include <QStyledItemDelegate>
#include <QTableView>

#include "controllers/midi/midimessage.h"

class MidiOptionsDelegate : public QStyledItemDelegate {
    Q_OBJECT
  public:
    MidiOptionsDelegate(QObject* pParent);
    virtual ~MidiOptionsDelegate();

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const;

    QString displayText(const QVariant& value, const QLocale& locale) const;

    void setEditorData(QWidget* editor, const QModelIndex& index) const;

    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const;
};
