#include "controllers/keyboard/keyboardcontrollerpreset.h"
#include "controllers/keyboard/layouts.h"

QString KeyboardControllerPreset::getKeySequencesToString(ConfigKey configKey, QString separator) {
    QStringList keyseqs = getKeySequences(configKey);
    QString keySeqsString = "";

    for (const auto& keyseq: keyseqs) {
        keySeqsString += keyseq + separator;
    }

    return keySeqsString;
}

QStringList KeyboardControllerPreset::getKeySequences(ConfigKey configKey) {
    QStringList keyseqs;
    QMultiHash<QString, ConfigKey>::iterator it;

    for (it = m_mapping.begin(); it != m_mapping.end(); ++it) {
        const ConfigKey& currentConfigKey = it.value();
        if (currentConfigKey == configKey) {
            QString keyseq = it.key();
            keyseqs.append(keyseq);
        }
    }
    return keyseqs;
}

QMultiHash<QString, ConfigKey> KeyboardControllerPreset::getMappingByGroup(const QString& targetGroup) {
    QMultiHash<QString, ConfigKey> filteredKeySequenceHash;
    QMultiHash<QString, ConfigKey>::iterator it;

    for (it = m_mapping.begin(); it != m_mapping.end(); ++it) {
        QString currentGroup = it.value().group;
        if (currentGroup == targetGroup) {
            filteredKeySequenceHash.insert(it.key(), it.value());
        }
    }
    return filteredKeySequenceHash;
}

void KeyboardControllerPreset::translate(QString layoutName) {
    qDebug() << "KeyboardControllerPreset::translate() " << layoutName;
    KeyboardLayoutPointer layout = getLayout(layoutName.toStdString());

    // Default to American English layout if no layout
    // found with given layout name
    if (layout == nullptr) {
        qDebug() << "Couldn't find layout translation tables for "
                 << layoutName << ", falling back to \"en_US\"";
        layoutName = "en_US";
        layout = getLayout("en_US");

        // TODO(Tomasito) - If previous layout was also en_US, return
    }

    // Reset mapping
    m_mapping.clear();

    // Iterate through all KbdControllerPresetControl
    QList<KbdControllerPresetControl>::const_iterator ctrlI;
    for (ctrlI = m_mappingRaw.constBegin(); ctrlI != m_mappingRaw.constEnd(); ++ctrlI) {

        // True if there is no key sequence object which has the same
        // keyboardlayout as the user's layout. False if there is.
        bool keyseqNeedsTranslate = true;

        // All keyseq elements (also the overloaded elements, that may
        // not be used for the user's current keyboard layout)
        const QList<KbdControllerPresetKeyseq>& keyseqsRaw = ctrlI->keyseqs;

        // Try to find a key sequence that targets the current keyboard layout. If not found,
        // use the first key sequence of the list and indicate that it needs a translation.
        QList<KbdControllerPresetKeyseq>::const_iterator keyseqsRawI = keyseqsRaw.constEnd();
        while (keyseqsRawI != keyseqsRaw.constBegin()) {
            --keyseqsRawI;

            // Check if this key sequence's target layout is the same as the user's language
            bool keyboardLayoutIsSame = keyseqsRawI->lang == layoutName;

            // Check if this key sequence is final and thus does not need any translation
            bool isFinal = keyseqsRawI->final;

            if (keyboardLayoutIsSame || isFinal) {
                keyseqNeedsTranslate = false;
                break;
            }
        }

        ConfigKey configKey = !keyseqsRaw.isEmpty() ? ctrlI->configKey : ConfigKey();
        QString keyseq = !keyseqsRaw.isEmpty() ? keyseqsRawI->keysequence : "";

        if (keyseqNeedsTranslate && !keyseqsRaw.isEmpty()) {
            QString scancode_string = keyseqsRawI->scancode;
            unsigned char scancode = (unsigned char) scancode_string.toInt();

            if (!scancode_string.isEmpty()) {
                // Find out if the character is regular or shifted
                QStringList modifiers = layoutUtils::getModifiersFromKeysequence(keyseq);
                bool onlyShift = modifiers.size() == 1 && modifiers.contains("Shift");
                Qt::KeyboardModifier modifier = onlyShift ? Qt::ShiftModifier : Qt::NoModifier;

                // Get KbdKeyChar
                const KbdKeyChar* keyChar = layoutUtils::getKbdKeyChar(layout, scancode, modifier);
                QChar character = QChar(keyChar->character);

                // If key is not dead, reconstruct key sequence with translated character.
                // Otherwise, warn the user about the key that is not going to work.
                if (!keyChar->isDead) {
                    QString modifiersString = modifiers.join("+");
                    if (!modifiersString.isEmpty()) {
                        modifiersString += "+";
                    }
                    keyseq = modifiersString + character;
                } else {
                    qWarning() << "Can't use key with scancode " << scancode
                               << " because it's a dead key on layout '" << layoutName
                               << "'. Please specify <keyseq lang=\"" << layoutName
                               << "\" scancode=\"" << scancode << "\"> in the keyboard preset file (group: "
                               << configKey.group << ").";
                    keyseq = "";
                }

            }
        }

        // Load action into preset
        m_mapping.insert(keyseq, configKey);
    }
}
