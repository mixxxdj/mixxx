#include "controllers/delegates/midioptionsdelegate.h"

#include <QComboBox>
#include <QTableView>
#include <QtDebug>

#include "controllers/midi/midimessage.h"
#include "controllers/midi/midiutils.h"
#include "moc_midioptionsdelegate.cpp"

namespace {

const QList<MidiOption> kMidiOptions = {
        MidiOption::None,
        MidiOption::Invert,
        MidiOption::Rot64,
        MidiOption::Rot64Invert,
        MidiOption::Rot64Fast,
        MidiOption::Diff,
        MidiOption::Button,
        MidiOption::Switch,
        MidiOption::Spread64,
        MidiOption::HercJog,
        MidiOption::SelectKnob,
        MidiOption::SoftTakeover,
        MidiOption::Script,
        MidiOption::FourteenBitMSB,
        MidiOption::FourteenBitLSB,
};

}

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

    for (const MidiOption choice : kMidiOptions) {
        pComboBox->addItem(MidiUtils::midiOptionToTranslatedString(choice),
                static_cast<uint16_t>(choice));
    }

    return pComboBox;
}

QString MidiOptionsDelegate::displayText(const QVariant& value,
                                         const QLocale& locale) const {
    Q_UNUSED(locale);
    MidiOptions options = value.value<MidiOptions>();
    QStringList optionStrs;
    for (const MidiOption option : kMidiOptions) {
        if (options.testFlag(option)) {
            optionStrs.append(MidiUtils::midiOptionToTranslatedString(option));
        }
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
        if (MidiOptions(pComboBox->itemData(i).toInt()) & options) {
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
    options = MidiOptions(pComboBox->itemData(pComboBox->currentIndex()).toInt());
    model->setData(index, QVariant::fromValue(options), Qt::EditRole);
}
