#include <QtDebug>
#include <QLineEdit>
#include <QStringList>

#include "controllers/delegates/controldelegate.h"
#include "controllers/midi/midimessage.h"

ControlDelegate::ControlDelegate(QObject* pParent)
        : QStyledItemDelegate(pParent),
          m_pPicker(new ControlPickerMenu(NULL)),
          m_iMidiOptionsColumn(-1),
          m_bIsIndexScript(false) {
}

ControlDelegate::~ControlDelegate() {
}

QWidget* ControlDelegate::createEditor(QWidget* parent,
                                       const QStyleOptionViewItem& option,
                                       const QModelIndex& index) const {
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
        MidiOptions options = qVariantValue<MidiOptions>(optionsColumn.data());
        m_bIsIndexScript = options.script;
    }

    QStyledItemDelegate::paint(painter, option, index);
}

QString ControlDelegate::displayText(const QVariant& value,
                                     const QLocale& locale) const {
    ConfigKey key = qVariantValue<ConfigKey>(value);

    if (key.group.isEmpty() && key.item.isEmpty()) {
        return tr("No control chosen.");
    }

    if (m_bIsIndexScript) {
        return tr("Script: %1(%2)").arg(key.item, key.group);
    }

    QString description = m_pPicker->descriptionForConfigKey(key);
    if (!description.isEmpty()) {
        return description;
    }

    return key.group + "," + key.item;
}

void ControlDelegate::setEditorData(QWidget* editor,
                                    const QModelIndex& index) const {
    ConfigKey key = qVariantValue<ConfigKey>(index.data(Qt::EditRole));

    QLineEdit* pLineEdit = dynamic_cast<QLineEdit*>(editor);
    if (pLineEdit == NULL) {
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
    if (pLineEdit == NULL) {
        return;
    }

    QStringList keyStrs = pLineEdit->text().split(",");
    if (keyStrs.size() == 2) {
        model->setData(index, qVariantFromValue(
            ConfigKey(keyStrs.at(0), keyStrs.at(1))), Qt::EditRole);
    }
}
