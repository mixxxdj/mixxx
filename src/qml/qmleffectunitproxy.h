#pragma once

#include <QObject>
#include <QQmlEngine>
#include <memory>

#include "effects/effectsmanager.h"

namespace mixxx {
namespace qml {

class QmlEffectUnitProxy : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(QmlEffectUnitProxy)
    Q_PROPERTY(int unitNumber READ getUnitNumber CONSTANT)
    Q_PROPERTY(QString group READ getGroup CONSTANT)
    Q_PROPERTY(QString presetName READ getPresetName NOTIFY presetChanged)
    Q_PROPERTY(int presetIndex READ getPresetIndex NOTIFY presetChanged)
    Q_PROPERTY(bool presetReadOnly READ isPresetReadOnly NOTIFY presetChanged)
    Q_PROPERTY(bool canUpdatePreset READ canUpdatePreset NOTIFY presetChanged)
    Q_PROPERTY(bool canRenamePreset READ canRenamePreset NOTIFY presetChanged)
    QML_NAMED_ELEMENT(EffectUnitProxy)
    QML_UNCREATABLE("Only accessible via Mixxx.EffectsManager.getEffectUnit(unitNumber)")

  public:
    explicit QmlEffectUnitProxy(std::shared_ptr<EffectsManager> pEffectsManager,
            int unitNumber,
            EffectChainPointer pEffectUnit,
            QObject* parent = nullptr);

    int getUnitNumber() const;
    QString getGroup() const;
    QString getPresetName() const;
    int getPresetIndex() const;
    bool isPresetReadOnly() const;
    bool canUpdatePreset() const;
    bool canRenamePreset() const;

    Q_INVOKABLE void loadPreset(int index);
    Q_INVOKABLE void updatePreset();
    Q_INVOKABLE bool renamePreset();
    Q_INVOKABLE void savePresetAs();

  signals:
    void presetChanged();

  private:
    EffectChainPresetPointer currentPreset() const;

    const std::shared_ptr<EffectsManager> m_pEffectsManager;
    const EffectChainPresetManagerPointer m_pPresetManager;
    const int m_unitNumber;
    const EffectChainPointer m_pEffectUnit;
};

} // namespace qml
} // namespace mixxx
