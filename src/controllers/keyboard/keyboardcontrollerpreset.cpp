#include "controllers/keyboard/keyboardcontrollerpreset.h"

QString KeyboardControllerPreset::getKeySequencesToString(ConfigKey configKey, QString separator) {
    QList<QKeySequence> keySeqs = getKeySequences(configKey);
    QString keySeqsString = "";

        for (QKeySequence keySeq: keySeqs) {
            keySeqsString += keySeq.toString() + separator;
        }

    return keySeqsString;
}

QList<QKeySequence> KeyboardControllerPreset::getKeySequences(ConfigKey configKey) {
    QList<QKeySequence> keySeqs;
    QMultiHash<ConfigValueKbd, ConfigKey>::iterator it;
    for (it = m_mapping.begin(); it != m_mapping.end(); ++it) {
        const ConfigKey& currentConfigKey = it.value();
        if (currentConfigKey == configKey) {
            ConfigValueKbd configValueKbd = it.key();
            keySeqs.append(configValueKbd.m_qKey);
        }
    }
    return keySeqs;
}

QMultiHash<ConfigValueKbd, ConfigKey> KeyboardControllerPreset::getMappingByGroup(QString targetGroup) {
    QMultiHash<ConfigValueKbd, ConfigKey> filteredKeySequenceHash;
    QMultiHash<ConfigValueKbd, ConfigKey>::iterator it;
    for (it = m_mapping.begin(); it != m_mapping.end(); ++it) {
        QString currentGroup = it.value().group;
        if (currentGroup == targetGroup) {
            filteredKeySequenceHash.insert(it.key(), it.value());
        }
    }
    return filteredKeySequenceHash;
}



