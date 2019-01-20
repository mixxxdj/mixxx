#include <QKeySequence>
#include "controllers/keyboard/keyboardcontrollerpreset.h"
#include "controllers/keyboard/layouts.h"

QString KeyboardControllerPreset::getKeySequencesToString(
        ConfigKey configKey, QString separator) const {
    QStringList keyseqs = getKeySequences(configKey);
    QString keySeqsString = "";

    for (const auto& keyseq: keyseqs) {
        keySeqsString += keyseq + separator;
    }

    return keySeqsString;
}

QStringList KeyboardControllerPreset::getKeySequences(ConfigKey configKey) const {
    QStringList keyseqs;
    QMultiHash<QString, ConfigKey>::const_iterator it;

    for (it = m_mapping.begin(); it != m_mapping.end(); ++it) {
        const ConfigKey& currentConfigKey = it.value();
        if (currentConfigKey == configKey) {
            QString keyseq = it.key();
            keyseqs.append(keyseq);
        }
    }
    return keyseqs;
}

QMultiHash<QString, ConfigKey> KeyboardControllerPreset::getMappingByGroup(
        const QString& targetGroup) const {
    QMultiHash<QString, ConfigKey> filteredKeySequenceHash;

    for (auto it = m_mapping.begin(); it != m_mapping.end(); ++it) {
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

    // Iterate through all KbdControllerPresetControl objects
    QList<KbdControllerPresetControl>::const_iterator ctrlI;
    for (ctrlI = m_mappingRaw.constBegin(); ctrlI != m_mappingRaw.constEnd(); ++ctrlI) {

        // True if there is no key sequence object which has the same
        // keyboard layout as the user's layout. False if there is.
        bool keyseqNeedsTranslate = true;

        // All keyseq elements (also the overloaded elements, that may
        // not be used for the user's current keyboard layout)
        const QList<KbdControllerPresetKeyseq>& keyseqsRaw = ctrlI->keyseqs;

        // Retrieve config key for this current control
        ConfigKey configKey = !keyseqsRaw.isEmpty() ? ctrlI->configKey : ConfigKey();

        // Try to find a key sequence that targets the current
        // keyboard layout, if not found set to constEnd()
        auto keyseqsRawIterator = keyseqsRaw.constEnd();
        while (keyseqsRawIterator != keyseqsRaw.constBegin()) {
            --keyseqsRawIterator;

            // Check if this key sequence's target layout
            // is the same as the user's language
            if (keyseqsRawIterator->lang == layoutName) {
                break;
            }

            // If none found, set keyseqsRawIterator to end(), which is
            // kind of setting it to null
            if (keyseqsRawIterator == keyseqsRaw.constBegin()) {
                keyseqsRawIterator = keyseqsRaw.constEnd();
                break;
            }
        }

        // If no keyseq objects found for this layout, check if there
        // is one that targets no specific layout. One whose 'lang'
        // attribute was not given in the XML.
        //
        // NOTE: If there is more than one keyseq whose 'lang' attribute
        //       is null, the last one will be chosen.
        if (keyseqsRawIterator == keyseqsRaw.constEnd()) {
            while (keyseqsRawIterator != keyseqsRaw.constBegin()) {
                --keyseqsRawIterator;

                // If lang is not given, that means that it is
                // targeted at all not explicitly listed layouts
                if (keyseqsRawIterator->lang.isEmpty()) {
                    break;
                }

                // Same as in the first loop, setting to "null"-ish
                // if none was found
                if (keyseqsRawIterator == keyseqsRaw.constBegin()) {
                    keyseqsRawIterator = keyseqsRaw.constEnd();
                    break;
                }
            }
        }

        // If no keyseq was found for current layout, nor a global layout targeted at no
        // layout in particular, Mixxx can't load in any mapping for this current action
        if (keyseqsRawIterator == keyseqsRaw.constEnd()) {
            qWarning() << "No keyseq found for" << layoutName
                       << "nor any translatable keyseq. Skipping"
                       << "mapping for ConfigKey" << configKey;
            continue;
        }

        // Check if this keyseq is position based (if 'byPositionOf' attribute was given)
        bool positionBased = !keyseqsRawIterator->byPositionOf.isEmpty();

        // Check if this keyseq is position based, based on the current layout
        bool byPositionOfCurrentLayout = keyseqsRawIterator->byPositionOf == layoutName;

        // If 'byPositionOf' attribute not given, or given but where the
        // layout is matches the current local, no need to translate
        if (!positionBased || byPositionOfCurrentLayout) {
            keyseqNeedsTranslate = false;
        }

        // Key sequences for control. If key sequence does not need any translation,
        // this array will always contain just one key sequence. However, after
        // translation it could potentially contain multiple key sequences.
        QStringList keyseqs;

        if (!keyseqsRaw.isEmpty()) {
            keyseqs += keyseqsRawIterator->keysequence;
        } else {
            keyseqs += "";
        }

        if (keyseqNeedsTranslate) {
            // Can't happen. Otherwise keyseqNeedsTranslate would be false.
            DEBUG_ASSERT(!keyseqsRawIterator->byPositionOf.isEmpty());

            QString key = layoutUtils::keyseqGetKey(keyseqs[0]);
            if (key.size() > 1) {
                qWarning() << "Can't translate keyseq" << keyseqs[0]
                           << "because key" << key << "is not a character.";
            }

            // Get original layout
            KeyboardLayoutPointer originalKeyseqLayout = getLayout(keyseqsRawIterator->byPositionOf.toStdString());
            if (originalKeyseqLayout == nullptr) {
                qWarning() << "Can't translate keyseq" << keyseqs[0]
                           << "because Mixxx can't retrieve it's scancode."
                           << "'byPositionOf' layout" << keyseqsRawIterator->byPositionOf
                           << "not found. Skipping mapping for" << configKey;
                continue;
            }

            // Check whether we need no modifier or shift modifier for the scancode lookup
            QStringList modifiers = layoutUtils::keyseqGetModifiers(keyseqs[0]);
            bool onlyShift = modifiers.size() == 1 && modifiers[0] == "Shift";
            Qt::KeyboardModifier modifier = onlyShift ? Qt::ShiftModifier : Qt::NoModifier;

            // Retrieve scancodes (usually just one, sometimes two,
            // due to multiple keys sharing the same character)
            QList<Scancode> scancodes = layoutUtils::findScancodesForCharacter(
                    originalKeyseqLayout, key.at(0), modifier);

            // Warn user if any scancodes where found for this character
            if (scancodes.isEmpty()) {
                qWarning() << "Couldn't find any scancodes for character"
                           << key.at(0) << "on" << keyseqsRawIterator->byPositionOf
                           << (onlyShift ? "with" : "without") << "shift."
                           << "Skipping mapping for" << configKey;
                continue;
            }

            // Remove original key sequence
            keyseqs.clear();

            // Iterate through all found scancodes
            for (const Scancode scancode : scancodes) {
                // Get KbdKeyChar
                const KbdKeyChar* keyChar = layoutUtils::getKbdKeyChar(layout, scancode, modifier);
                QChar character(keyChar->character);

                // If key is not dead, reconstruct key sequence with translated character. Otherwise,
                // warn user about key that won't work.
                if (!keyChar->isDead) {
                    QStringList keyseq(modifiers);
                    keyseq.append(character);
                    keyseqs.append(keyseq.join("+"));
                } else {
                    qWarning() << "Can't use key with scancode " << scancode
                               << " because it's a dead key on layout '" << layoutName
                               << "'. Please specify <keyseq lang=\"" << layoutName
                               << "\" scancode=\"" << scancode << "\"> in the keyboard preset file (group: "
                               << configKey.group << ").";
                    keyseqs[0] = "";
                }
            }
        }

        for (QString& keyseq : keyseqs) {
            QString key = layoutUtils::keyseqGetKey(keyseq);
            QStringList mods = layoutUtils::keyseqGetModifiers(keyseq);

            // Clear keyseq
            keyseq = "";

            // NOTE: This order must be the same as in
            // ...   KeyboardEventFilter::getKeySeq()
            if (mods.contains("Shift")) {
                keyseq += "Shift+";
            }
            if (mods.contains("Ctrl")) {
                keyseq += "Ctrl+";
            }
            if (mods.contains("Alt")) {
                keyseq += "Alt+";
            }
            if (mods.contains("Meta")) {
                keyseq += "Meta+";
            }

            // If the key is modified, the comparisons in KeyboardEventFilter
            // are based on QKeySequence::toString()
            if (!keyseq.isEmpty()) {
              key = QKeySequence(key).toString();
            }

            // Reconstruct key sequence with ordened modifiers
            keyseq += key;

            // Load action into preset
            m_mapping.insert(keyseq, configKey);
        }
    }
}
