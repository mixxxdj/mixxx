#include "controllers/keyboard/keyboardcontrollerpreset.h"

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

QMultiHash<QString, ConfigKey> KeyboardControllerPreset::getMappingByGroup(QString targetGroup) {
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



