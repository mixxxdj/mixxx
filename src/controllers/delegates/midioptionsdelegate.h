#ifndef MIDIOPTIONSDELEGATE_H
#define MIDIOPTIONSDELEGATE_H

#include <QByteArrayData>
#include <QLocale>
#include <QString>
#include <QStyleOptionViewItem>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QVariant>

#include "controllers/midi/midimessage.h"

class QAbstractItemModel;
class QModelIndex;
class QObject;
class QWidget;

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

#endif /* MIDIOPTIONSDELEGATE_H */
