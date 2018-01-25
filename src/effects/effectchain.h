#ifndef EFFECTCHAIN_H
#define EFFECTCHAIN_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QDomDocument>

#include "effects/defs.h"
#include "effects/effect.h"
#include "engine/channelhandle.h"
#include "util/class.h"

class EffectsManager;
class EngineEffectRack;
class EngineEffectChain;
class EffectChain;
typedef QSharedPointer<EffectChain> EffectChainPointer;

// The main-thread representation of an effect chain. This class is NOT
// thread-safe and must only be used from the main thread.
class EffectChain : public QObject {
    Q_OBJECT
  public:
    EffectChain(EffectsManager* pEffectsManager, const QString& id,
                EffectChainPointer prototype=EffectChainPointer());
    virtual ~EffectChain();

    void addToEngine(EngineEffectRack* pRack, int iIndex);
    void removeFromEngine(EngineEffectRack* pRack, int iIndex);
    void updateEngineState();

    // The ID of an EffectChain is a unique ID given to it to help associate it
    // with the preset from which it was loaded.
    const QString& id() const;

    // Whether the chain is enabled (eligible for processing).
    bool enabled() const;
    void setEnabled(bool enabled);

    // Activates EffectChain processing for the provided channel.
    void enableForInputChannel(const ChannelHandleAndGroup& handle_group);
    bool enabledForChannel(const ChannelHandleAndGroup& handle_group) const;
    const QSet<ChannelHandleAndGroup>& enabledChannels() const;
    void disableForInputChannel(const ChannelHandleAndGroup& handle_group);

    EffectChainPointer prototype() const;

    // Get the human-readable name of the EffectChain
    const QString& name() const;
    void setName(const QString& name);

    // Get the human-readable description of the EffectChain
    QString description() const;
    void setDescription(const QString& description);

    double mix() const;
    void setMix(const double& dMix);

    static QString insertionTypeToString(EffectChainInsertionType type) {
        switch (type) {
            case EffectChainInsertionType::Insert:
                return "INSERT";
            case EffectChainInsertionType::Send:
                return "SEND";
            default:
                return "UNKNOWN";
        }
    }
    static EffectChainInsertionType insertionTypeFromString(const QString& typeStr) {
        if (typeStr == "INSERT") {
            return EffectChainInsertionType::Insert;
        } else if (typeStr == "SEND") {
            return EffectChainInsertionType::Send;
        } else {
            return EffectChainInsertionType::Num_Insertion_Types;
        }
    }

    EffectChainInsertionType insertionType() const;
    void setInsertionType(EffectChainInsertionType type);

    void addEffect(EffectPointer pEffect);
    void replaceEffect(unsigned int effectSlotNumber, EffectPointer pEffect);
    void removeEffect(unsigned int effectSlotNumber);
    const QList<EffectPointer>& effects() const;
    unsigned int numEffects() const;

    EngineEffectChain* getEngineEffectChain();

    static EffectChainPointer createFromXml(EffectsManager* pEffectsManager,
                                      const QDomElement& element);
    static EffectChainPointer clone(EffectChainPointer pChain);

  signals:
    // Signal that indicates that an effect has been added or removed.
    void effectChanged(unsigned int effectSlotNumber);
    void nameChanged(const QString& name);
    void descriptionChanged(const QString& name);
    void enabledChanged(bool enabled);
    void mixChanged(double v);
    void insertionTypeChanged(EffectChainInsertionType type);
    void channelStatusChanged(const QString& group, bool enabled);

  private:
    QString debugString() const {
        return QString("EffectChain(%1)").arg(m_id);
    }

    void sendParameterUpdate();

    EffectsManager* m_pEffectsManager;
    EffectChainPointer m_pPrototype;

    bool m_bEnabled;
    QString m_id;
    QString m_name;
    QString m_description;
    EffectChainInsertionType m_insertionType;
    double m_dMix;

    QSet<ChannelHandleAndGroup> m_enabledInputChannels;
    QList<EffectPointer> m_effects;
    EngineEffectChain* m_pEngineEffectChain;
    bool m_bAddedToEngine;

    DISALLOW_COPY_AND_ASSIGN(EffectChain);
};

#endif /* EFFECTCHAIN_H */
