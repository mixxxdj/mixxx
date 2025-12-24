#include "controllers/delegates/controldelegate.h"

#include <QLineEdit>
#include <QRegExp>
#include <QStringList>

#include "controllers/controlpickermenu.h"
#include "controllers/midi/midimessage.h"
#include "moc_controldelegate.cpp"

ControlDelegate::ControlDelegate(QObject* pParent, ControlPickerMenu* pControlPickerMenu)
        : QStyledItemDelegate(pParent),
          m_pPicker(pControlPickerMenu),
          m_iMidiOptionsColumn(-1),
          m_bIsIndexScript(false) {
    m_numGroupsTrMap = m_pPicker->getNumGroupsTrMap();
    m_otherGroupsTrMap = m_pPicker->getOtherGroupsTrMap();
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
    if (value.canConvert<ConfigKey>()) {
        ConfigKey key = value.value<ConfigKey>();

        QString description = m_pPicker->descriptionForConfigKey(key);
        if (!description.isEmpty()) {
            return description;
        }

        if (m_bIsIndexScript || description.isEmpty()) {
            return QString("%1: %2").arg(translateConfigKeyGroup(key.group), key.item);
        }

        return key.group + "," + key.item;
    } else if (value.canConvert<QString>()) {
        return value.value<QString>();
    } else {
        return tr("No control chosen.");
    }
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

// return more readable group names like "Deck 1", "Sampler 1" etc.
QString ControlDelegate::translateConfigKeyGroup(const QString& group) const {
    QMapIterator<QString, QString> numIt(m_numGroupsTrMap);
    while (numIt.hasNext()) {
        numIt.next();
        QString regExpStr = QString("\\[%1([1-9]\\d*|)\\]").arg(numIt.key());
        QRegExp numGroupMatcher(regExpStr);
        if (numGroupMatcher.exactMatch(group)) {
            // special case for legacy group [Microphone] > "Microphone 1"
            if (numIt.key() == "Microphone" && numGroupMatcher.cap(1).isEmpty()) {
                return QString("%1 1").arg(numIt.value());
            }
            bool ok = false;
            int num = numGroupMatcher.cap(1).toInt(&ok);
            if (ok) {
                return numIt.value().arg(QString::number(num));
            }
            return group;
        }
    }

    QMapIterator<QString, QString> oIt(m_otherGroupsTrMap);
    while (oIt.hasNext()) {
        oIt.next();
        QString regExpStr = QString("\\[%1\\]").arg(oIt.key());
        QRegExp otherGroupMatcher(regExpStr);
        if (otherGroupMatcher.exactMatch(group)) {
            return oIt.value();
        }
    }

    // no match (custom group maybe), return raw [Group]
    return group;
}
