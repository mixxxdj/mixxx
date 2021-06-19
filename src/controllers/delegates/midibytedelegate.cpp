#include "controllers/delegates/midibytedelegate.h"
#include "controllers/midi/midimessage.h"
#include "controllers/midi/midiutils.h"
#include "widget/hexspinbox.h"

MidiByteDelegate::MidiByteDelegate(QObject* pParent)
        : QStyledItemDelegate(pParent) {
}

MidiByteDelegate::~MidiByteDelegate() {
}

QWidget* MidiByteDelegate::createEditor(QWidget* parent,
                                        const QStyleOptionViewItem& option,
                                        const QModelIndex& index) const {
    Q_UNUSED(option);
    Q_UNUSED(index);
    HexSpinBox* pSpinBox = new HexSpinBox(parent);
    pSpinBox->setRange(0x00, 0x7F);
    return pSpinBox;
}

QString MidiByteDelegate::displayText(const QVariant& value,
                                      const QLocale& locale) const {
    Q_UNUSED(locale);
    unsigned char control = static_cast<unsigned char>(value.toInt());
    return MidiUtils::formatByteAsHex(control);
}

void MidiByteDelegate::setEditorData(QWidget* editor,
                                     const QModelIndex& index) const {
    int control = index.data(Qt::EditRole).toInt();
    HexSpinBox* pSpinBox = qobject_cast<HexSpinBox*>(editor);
    if (pSpinBox == nullptr) {
        return;
    }
    pSpinBox->setValue(control);
}

void MidiByteDelegate::setModelData(QWidget* editor,
                                    QAbstractItemModel* model,
                                    const QModelIndex& index) const {
    HexSpinBox* pSpinBox = qobject_cast<HexSpinBox*>(editor);
    if (pSpinBox == nullptr) {
        return;
    }
    model->setData(index, pSpinBox->value(), Qt::EditRole);
}
