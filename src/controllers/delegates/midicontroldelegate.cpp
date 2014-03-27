#include "controllers/delegates/midicontroldelegate.h"
#include "controllers/midi/midimessage.h"
#include "controllers/midi/midiutils.h"
#include "widget/hexspinbox.h"

MidiControlDelegate::MidiControlDelegate(QObject* pParent)
        : QStyledItemDelegate(pParent) {
}

MidiControlDelegate::~MidiControlDelegate() {
}

QWidget* MidiControlDelegate::createEditor(QWidget* parent,
                                          const QStyleOptionViewItem& option,
                                          const QModelIndex& index) const {
    HexSpinBox* pSpinBox = new HexSpinBox(parent);
    pSpinBox->setRange(0x00, 0x7F);
    return pSpinBox;
}

QString MidiControlDelegate::displayText(const QVariant& value,
                                        const QLocale& locale) const {
    unsigned char control = static_cast<unsigned char>(value.toInt());
    return MidiUtils::formatByte(control);
}

void MidiControlDelegate::setEditorData(QWidget* editor,
                                        const QModelIndex& index) const {
    int control = index.data(Qt::EditRole).toInt();
    HexSpinBox* pSpinBox = dynamic_cast<HexSpinBox*>(editor);
    if (pSpinBox == NULL) {
        return;
    }
    pSpinBox->setValue(control);
}

void MidiControlDelegate::setModelData(QWidget* editor,
                                      QAbstractItemModel* model,
                                      const QModelIndex& index) const {
    HexSpinBox* pSpinBox = dynamic_cast<HexSpinBox*>(editor);
    if (pSpinBox == NULL) {
        return;
    }
    model->setData(index, pSpinBox->value(), Qt::EditRole);
}
