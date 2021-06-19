#include <QComboBox>

#include "controllers/delegates/midiopcodedelegate.h"
#include "controllers/midi/midimessage.h"
#include "controllers/midi/midiutils.h"

MidiOpCodeDelegate::MidiOpCodeDelegate(QObject* pParent)
        : QStyledItemDelegate(pParent) {
}

MidiOpCodeDelegate::~MidiOpCodeDelegate() {
}

QWidget* MidiOpCodeDelegate::createEditor(QWidget* parent,
                                          const QStyleOptionViewItem& option,
                                          const QModelIndex& index) const {
    Q_UNUSED(option);
    Q_UNUSED(index);
    QComboBox* pComboBox = new QComboBox(parent);

    QList<MidiOpCode> choices;
    choices.append(MidiOpCode::NoteOn);
    choices.append(MidiOpCode::NoteOff);
    choices.append(MidiOpCode::ControlChange);
    choices.append(MidiOpCode::PitchBendChange);

    for (const MidiOpCode choice : qAsConst(choices)) {
        pComboBox->addItem(MidiUtils::opCodeToTranslatedString(choice),
                static_cast<uint8_t>(choice));
    }
    return pComboBox;
}

QString MidiOpCodeDelegate::displayText(const QVariant& value,
                                        const QLocale& locale) const {
    Q_UNUSED(locale);
    MidiOpCode opCode = static_cast<MidiOpCode>(value.toInt());
    return MidiUtils::opCodeToTranslatedString(opCode);
}

void MidiOpCodeDelegate::setEditorData(QWidget* editor,
                                       const QModelIndex& index) const {
    int opCode = index.data(Qt::EditRole).toInt();
    QComboBox* pComboBox = qobject_cast<QComboBox*>(editor);
    if (pComboBox == nullptr) {
        return;
    }
    for (int i = 0; i < pComboBox->count(); ++i) {
        if (pComboBox->itemData(i).toInt() == opCode) {
            pComboBox->setCurrentIndex(i);
            break;
        }
    }
}

void MidiOpCodeDelegate::setModelData(QWidget* editor,
                                      QAbstractItemModel* model,
                                      const QModelIndex& index) const {
    QComboBox* pComboBox = qobject_cast<QComboBox*>(editor);
    if (pComboBox == nullptr) {
        return;
    }
    model->setData(index, pComboBox->itemData(pComboBox->currentIndex()),
                   Qt::EditRole);
}
