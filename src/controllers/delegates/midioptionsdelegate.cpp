#include "controllers/delegates/midioptionsdelegate.h"

#include <QComboBox>
#include <QTableView>
#include <QtDebug>

#include "controllers/midi/midimessage.h"
#include "controllers/midi/midiutils.h"
#include "moc_midioptionsdelegate.cpp"

MidiOptionsDelegate::MidiOptionsDelegate(QObject* pParent)
        : QStyledItemDelegate(pParent) {
}

MidiOptionsDelegate::~MidiOptionsDelegate() {
}


QWidget* MidiOptionsDelegate::createEditor(QWidget* parent,
                                           const QStyleOptionViewItem& option,
                                           const QModelIndex& index) const {
    Q_UNUSED(option);
    Q_UNUSED(index);
    QComboBox* pComboBox = new QComboBox(parent);

    QList<MidiOption> choices;
    choices.append(MIDI_OPTION_NONE);
    choices.append(MIDI_OPTION_INVERT);
    choices.append(MIDI_OPTION_ROT64);
    choices.append(MIDI_OPTION_ROT64_INV);
    choices.append(MIDI_OPTION_ROT64_FAST);
    choices.append(MIDI_OPTION_DIFF);
    choices.append(MIDI_OPTION_BUTTON);
    choices.append(MIDI_OPTION_SWITCH);
    choices.append(MIDI_OPTION_SPREAD64);
    choices.append(MIDI_OPTION_HERC_JOG);
    choices.append(MIDI_OPTION_SELECTKNOB);
    choices.append(MIDI_OPTION_SOFT_TAKEOVER);
    choices.append(MIDI_OPTION_SCRIPT);
    choices.append(MIDI_OPTION_14BIT_MSB);
    choices.append(MIDI_OPTION_14BIT_LSB);

    for (int i = 0; i < choices.size(); ++i) {
        MidiOption choice = choices.at(i);
        pComboBox->addItem(MidiUtils::midiOptionToTranslatedString(choice),
                           choice);
    }

    return pComboBox;
}

QString MidiOptionsDelegate::displayText(const QVariant& value,
                                         const QLocale& locale) const {
    Q_UNUSED(locale);
    MidiOptions options = value.value<MidiOptions>();
    QStringList optionStrs;
    MidiOption option = static_cast<MidiOption>(1);
    while (option < MIDI_OPTION_MASK) {
        if (options.all & option) {
            optionStrs.append(MidiUtils::midiOptionToTranslatedString(option));
        }
        option = static_cast<MidiOption>(option << 1);
    }
    return optionStrs.join(", ");
}

void MidiOptionsDelegate::setEditorData(QWidget* editor,
                                        const QModelIndex& index) const {
    MidiOptions options = index.data(Qt::EditRole).value<MidiOptions>();

    QComboBox* pComboBox = qobject_cast<QComboBox*>(editor);
    if (pComboBox == nullptr) {
        return;
    }
    for (int i = 0; i < pComboBox->count(); ++i) {
        if (pComboBox->itemData(i).toInt() & options.all) {
            pComboBox->setCurrentIndex(i);
            return;
        }
    }
}

void MidiOptionsDelegate::setModelData(QWidget* editor,
                                       QAbstractItemModel* model,
                                       const QModelIndex& index) const {
    MidiOptions options;
    QComboBox* pComboBox = qobject_cast<QComboBox*>(editor);
    if (pComboBox == nullptr) {
        return;
    }
    options.all = pComboBox->itemData(pComboBox->currentIndex()).toInt();
    model->setData(index, QVariant::fromValue(options), Qt::EditRole);
}
