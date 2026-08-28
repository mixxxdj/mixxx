#pragma once
#include <QObject>
#include <QQmlEngine>

#include "effects/effectsmanager.h"
#include "qml/qmleffectslotparametersmodel.h"

namespace mixxx {
namespace qml {

class QmlEffectSlotProxy : public QObject {
    Q_OBJECT
    Q_PROPERTY(int chainSlotNumber READ getChainSlotNumber CONSTANT)
    Q_PROPERTY(QString chainSlotGroup READ getChainSlotGroup CONSTANT)
    Q_PROPERTY(int number READ getNumber CONSTANT)
    Q_PROPERTY(QString group READ getGroup CONSTANT)
    Q_PROPERTY(bool loaded READ isLoaded NOTIFY effectIdChanged)
    Q_PROPERTY(QString effectName READ getEffectName NOTIFY effectIdChanged)
    Q_PROPERTY(QString effectDescription READ getEffectDescription NOTIFY effectIdChanged)
    Q_PROPERTY(QString effectId READ getEffectId WRITE setEffectId NOTIFY effectIdChanged)
    Q_PROPERTY(mixxx::qml::QmlEffectSlotParametersModel* parametersModel
                    READ getParametersModel CONSTANT)
    QML_NAMED_ELEMENT(EffectSlotProxy)
    QML_UNCREATABLE(
            "Only accessible via "
            "Mixxx.EffectsManager.getEffectSlot(rackNumber, unitNumber, "
            "effectNumber)");

  public:
    explicit QmlEffectSlotProxy(
            std::shared_ptr<EffectsManager> pEffectsManager,
            int unitNumber,
            EffectChainPointer pChainSlot,
            EffectSlotPointer pEffectSlot,
            QObject* parent = nullptr);

    int getChainSlotNumber() const;
    QString getChainSlotGroup() const;
    int getNumber() const;
    QString getGroup() const;
    QString getEffectId() const;
    bool isLoaded() const;
    QString getEffectName() const;
    QString getEffectDescription() const;
    QmlEffectSlotParametersModel* getParametersModel() const;

  public slots:
    void setEffectId(const QString& effectId);

  public:
    Q_INVOKABLE void setParameterVisible(const QString& parameterId, bool visible);
    Q_INVOKABLE void saveDefaultSnapshot();

  signals:
    void effectIdChanged();

  private:
    /// FIXME: The reference to EffectManager is needed for loading effects.
    /// Unfortunately, the EffectSlot class doesn't provide a method to load an
    /// effect directly.
    const std::shared_ptr<EffectsManager> m_pEffectsManager;
    /// FIXME: This is a workaround for the missing getChainSlotNumber()
    /// method, so we need to store it separately. Fortunately, the EffectSlot
    /// class still has this method, so we don't need to store that, too.
    const int m_unitNumber;
    const EffectChainPointer m_pChainSlot;
    const EffectSlotPointer m_pEffectSlot;
    QmlEffectSlotParametersModel* const m_pParametersModel;
};

} // namespace qml
} // namespace mixxx
