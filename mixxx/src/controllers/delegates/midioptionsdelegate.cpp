#include "controllers/delegates/midioptionsdelegate.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QStandardItemModel>

#include "controllers/midi/midimessage.h"
#include "controllers/midi/midiutils.h"
#include "moc_midioptionsdelegate.cpp"
#include "util/parented_ptr.h"

namespace {

const QList<MidiOption> kMidiOptions = {
        // Don't add 'Normal' to the list because it's useless: it's exclusive,
        // meaning it's the implicit result of unchecking all other options, but
        // clicking it does not uncheck all other options. Also, showing it
        // checked is pointless (and it's not updated if others are checked).
        // Furthermore, the mapping list is cleaner without it, mappings that
        // have options set are much easier to spot.
        // MidiOption::None,
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

} // namespace

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

    // Create, populate and connect the box.
    QComboBox* pComboBox = make_parented<QComboBox>(parent);
    auto* pModel = static_cast<QStandardItemModel*>(pComboBox->model());
    DEBUG_ASSERT(pModel);
    for (const MidiOption opt : kMidiOptions) {
        QStandardItem* pItem =
                new QStandardItem(MidiUtils::midiOptionToTranslatedString(opt));
        pItem->setData(static_cast<uint16_t>(opt));
        pItem->setCheckable(true);
        pModel->appendRow(pItem);
    }
    // Add a special item to uncheck all. See commitAndCloseEditor()
    QStandardItem* pItem = new QStandardItem(tr("Unset all"));
    pItem->setCheckable(false); // doesn't hurt to set this explicitly
    pModel->appendRow(pItem);

    // Unsetting the index clears the display text which is visible when closing
    // the list view by clicking anywhere else. Default text is that of first
    // added item. It can be set to any combination of existing item texts, e.g.
    // all checked items, but it's not updated before the selection is committed
    // so let's simply clear the text to avoid confusion.
    pComboBox->setCurrentIndex(-1);

    // Default horizontal size policy is Preferred which results in center elide
    // as soon as items are checkable, regardless the elide mode.
    pComboBox->view()->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    pComboBox->view()->setTextElideMode(Qt::ElideNone);

    // * clicking an option or pressing Enter on a selected option toggles it,
    //   closes the list view and commits the updated data
    // * pressing Space on a selected option toggles it, list remains open
    // * clicking outside the listview closes it, and another click causing the
    //   combobox to lose focus closes that and commits pending changes
    connect(pComboBox,
            QOverload<int>::of(&QComboBox::activated),
            this,
            &MidiOptionsDelegate::commitAndCloseEditor);

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
    auto* pComboBox = qobject_cast<QComboBox*>(editor);
    if (!pComboBox) {
        return;
    }

    // Update checked states
    const MidiOptions options = index.data(Qt::EditRole).value<MidiOptions>();
    const auto* pModel = static_cast<QStandardItemModel*>(pComboBox->model());
    DEBUG_ASSERT(pModel);
    for (int row = 0; row < pModel->rowCount(); row++) {
        auto* pItem = pModel->item(row, 0);
        if (!pItem->isCheckable()) {
            continue;
        }
        auto opt = static_cast<MidiOption>(pItem->data().toInt());
        pItem->setCheckState(options.testFlag(opt) ? Qt::Checked : Qt::Unchecked);
    }

    // Show popup immediately, as with the other editors, no 'edit' click
    // required to open the list view.
    pComboBox->showPopup();
}

void MidiOptionsDelegate::setModelData(QWidget* editor,
                                       QAbstractItemModel* model,
                                       const QModelIndex& index) const {
    // Collect checked options and write them back to the model
    auto* pComboBox = qobject_cast<QComboBox*>(editor);
    if (!pComboBox) {
        return;
    }

    const auto* pModel = static_cast<QStandardItemModel*>(pComboBox->model());
    DEBUG_ASSERT(pModel);
    MidiOptions options;
    for (int i = 0; i < pModel->rowCount(); i++) {
        auto* pItem = pModel->item(i);
        // Only check for Qt::Checked or else, ignore Qt::PartiallyChecked
        options.setFlag(static_cast<MidiOption>(pItem->data().toUInt()),
                pItem->checkState() == Qt::Checked);
    }
    model->setData(index, QVariant::fromValue(options), Qt::EditRole);
}

void MidiOptionsDelegate::commitAndCloseEditor(int index) {
    QComboBox* pComboBox = qobject_cast<QComboBox*>(sender());
    if (!pComboBox) {
        return;
    }
    const auto* pModel = static_cast<QStandardItemModel*>(pComboBox->model());
    DEBUG_ASSERT(pModel);
    auto* pItem = pModel->item(index);
    DEBUG_ASSERT(pItem);
    if (pItem->isCheckable()) {
        pItem->setCheckState(pItem->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked);
        // TODO Concurrent option scan be selected. Implement a compatibility
        // matrix and uncheck all options that are incompatible with the last
        // checked option. Store initial check state for/in each item and
        // hook up to QStandardItemModel::itemChanged()
    } else {
        // Clear was selected. Uncheck all other items
        for (int row = 0; row < pModel->rowCount() - 1; row++) {
            if (row == index) { // Actually it's the last item, but this is safer.
                continue;
            }
            pItem = pModel->item(row, 0);
            pItem->setCheckState(Qt::Unchecked);
        }
    }

    emit commitData(pComboBox);
    emit closeEditor(pComboBox);
}
