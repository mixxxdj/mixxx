#include <QSpinBox>

#include "controllers/delegates/midichanneldelegate.h"
#include "controllers/midi/midimessage.h"
#include "controllers/midi/midiutils.h"

MidiChannelDelegate::MidiChannelDelegate(QObject* pParent)
        : QStyledItemDelegate(pParent) {
}

MidiChannelDelegate::~MidiChannelDelegate() {
}

QWidget* MidiChannelDelegate::createEditor(QWidget* parent,
                                          const QStyleOptionViewItem& option,
                                          const QModelIndex& index) const {
    Q_UNUSED(option);
    Q_UNUSED(index);
    QSpinBox* pSpinBox = new QSpinBox(parent);
    // The range is 0x0 through 0xF but it's common to display channels as
    // 1-indexed instead of 0-indexed.
    pSpinBox->setRange(1, 16);
    return pSpinBox;
}

QString MidiChannelDelegate::displayText(const QVariant& value,
                                        const QLocale& locale) const {
    Q_UNUSED(locale);
    unsigned char channel = static_cast<unsigned char>(value.toInt());
    // It's common to display channels as 1-indexed instead of 0-indexed.
    return QString::number(channel + 1);
}

void MidiChannelDelegate::setEditorData(QWidget* editor,
                                        const QModelIndex& index) const {
    int channel = index.data(Qt::EditRole).toInt();
    QSpinBox* pSpinBox = qobject_cast<QSpinBox*>(editor);
    if (pSpinBox == nullptr) {
        return;
    }
    // It's common to display channels as 1-indexed instead of 0-indexed.
    pSpinBox->setValue(channel + 1);
}

void MidiChannelDelegate::setModelData(QWidget* editor,
                                      QAbstractItemModel* model,
                                      const QModelIndex& index) const {
    QSpinBox* pSpinBox = qobject_cast<QSpinBox*>(editor);
    if (pSpinBox == nullptr) {
        return;
    }
    model->setData(index, pSpinBox->value() - 1, Qt::EditRole);
}
