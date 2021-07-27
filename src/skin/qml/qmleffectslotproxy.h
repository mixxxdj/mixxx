#pragma once
#include <QObject>

#include "effects/effectsmanager.h"

namespace mixxx {
namespace skin {
namespace qml {

class QmlEffectManifestParametersModel;

class QmlEffectSlotProxy : public QObject {
    Q_OBJECT
    Q_PROPERTY(int rackNumber READ getRackNumber CONSTANT)
    Q_PROPERTY(QString rackGroup READ getRackGroup CONSTANT)
    Q_PROPERTY(int chainSlotNumber READ getChainSlotNumber CONSTANT)
    Q_PROPERTY(QString chainSlotGroup READ getChainSlotGroup CONSTANT)
    Q_PROPERTY(int number READ getNumber CONSTANT)
    Q_PROPERTY(QString group READ getGroup CONSTANT)
    Q_PROPERTY(QString effectId READ getEffectId WRITE setEffectId NOTIFY effectIdChanged)
    Q_PROPERTY(mixxx::skin::qml::QmlEffectManifestParametersModel* parametersModel
                    READ getParametersModel NOTIFY parametersModelChanged)

  public:
    explicit QmlEffectSlotProxy(EffectRackPointer pEffectRack,
            EffectChainSlotPointer pChainSlot,
            EffectSlotPointer pEffectSlot,
            QObject* parent = nullptr);

    int getRackNumber() const;
    QString getRackGroup() const;
    int getChainSlotNumber() const;
    QString getChainSlotGroup() const;
    int getNumber() const;
    QString getGroup() const;
    QString getEffectId() const;
    QmlEffectManifestParametersModel* getParametersModel() const;

  public slots:
    void setEffectId(const QString& effectId);

  signals:
    void effectIdChanged();
    void parametersModelChanged();

  private:
    const EffectRackPointer m_pRack;
    const EffectChainSlotPointer m_pChainSlot;
    const EffectSlotPointer m_pEffectSlot;
};

} // namespace qml
} // namespace skin
} // namespace mixxx
