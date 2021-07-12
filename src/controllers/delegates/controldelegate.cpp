#include "controllers/delegates/controldelegate.h"

#include <QLineEdit>
#include <QStringList>
#include <QtDebug>

#include "controllers/midi/midimessage.h"
#include "moc_controldelegate.cpp"

ControlDelegate::ControlDelegate(QObject* pParent)
        : QStyledItemDelegate(pParent),
          m_pPicker(new ControlPickerMenu(nullptr)),
          m_iMidiOptionsColumn(-1),
          m_bIsIndexScript(false) {
}

ControlDelegate::~ControlDelegate() {
}

QWidget* ControlDelegate::createEditor(QWidget* parent,
                                       const QStyleOptionViewItem& option,
                                       const QModelIndex& index) const {
    Q_UNUSED(option);
    Q_UNUSED(index);
    QLineEdit* pLineEdit = new QLineEdit(parent);
    return pLineEdit;
}

void ControlDelegate::paint(QPainter* painter,
                            const QStyleOptionViewItem& option,
                            const QModelIndex& index) const {
    // Custom logic for MIDI. If we are enabled for script then say so.
    if (m_iMidiOptionsColumn != -1) {
        QModelIndex optionsColumn = index.sibling(index.row(),
                                                  m_iMidiOptionsColumn);
        MidiOptions options = optionsColumn.data().value<MidiOptions>();
        m_bIsIndexScript = options.testFlag(MidiOption::Script);
    }

    QStyledItemDelegate::paint(painter, option, index);
}

QString ControlDelegate::displayText(const QVariant& value,
                                     const QLocale& locale) const {
    Q_UNUSED(locale);
    ConfigKey key = value.value<ConfigKey>();

    if (key.group.isEmpty() && key.item.isEmpty()) {
        return tr("No control chosen.");
    }

    if (m_bIsIndexScript) {
        return tr("%1 %2").arg(key.group, key.item);
    }

    QString description = m_pPicker->descriptionForConfigKey(key);
    if (!description.isEmpty()) {
        return description;
    }

    return key.group + "," + key.item;
}

void ControlDelegate::setEditorData(QWidget* editor,
                                    const QModelIndex& index) const {
    ConfigKey key = index.data(Qt::EditRole).value<ConfigKey>();

    QLineEdit* pLineEdit = qobject_cast<QLineEdit*>(editor);
    if (pLineEdit == nullptr) {
        return;
    }

    if (key.group.isEmpty() && key.item.isEmpty()) {
        return;
    }

    pLineEdit->setText(key.group + "," + key.item);
}

void ControlDelegate::setModelData(QWidget* editor,
                                   QAbstractItemModel* model,
                                   const QModelIndex& index) const {
    QLineEdit* pLineEdit = qobject_cast<QLineEdit*>(editor);
    if (pLineEdit == nullptr) {
        return;
    }

    QStringList keyStrs = pLineEdit->text().split(",");
    if (keyStrs.size() == 2) {
        model->setData(index, QVariant::fromValue(
            ConfigKey(keyStrs.at(0), keyStrs.at(1))), Qt::EditRole);
    }
}
