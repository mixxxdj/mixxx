#ifndef EFFECTCHAINSLOT_H
#define EFFECTCHAINSLOT_H

#include <QObject>
#include <QMap>
#include <QMutex>
#include <QList>

#include "util.h"
#include "effects/effect.h"
#include "effects/effectslot.h"
#include "effects/effectchain.h"

class EffectChainSlot;
typedef QSharedPointer<EffectChainSlot> EffectChainSlotPointer;

class EffectChainSlot : public QObject {
    Q_OBJECT
  public:
    EffectChainSlot(QObject* pParent, const unsigned int iChainNumber);
    virtual ~EffectChainSlot();

    static QString formatGroupString(const unsigned int iChainNumber) {
        return QString("[EffectChain%1]").arg(iChainNumber+1);
    }

    // Get the ID of the loaded EffectChain
    QString id() const;

    // Get the human-readable name of the loaded EffectChain
    QString name() const;

    unsigned int numSlots() const;
    void addEffectSlot();
    EffectSlotPointer getEffectSlot(unsigned int slotNumber);

    void loadEffectChain(EffectChainPointer pEffectChain);
    EffectChainPointer getEffectChain() const;

    bool isEnabled() const;
    bool isEnabledForChannel(QString channelId) const;

    void registerChannel(const QString channelId);

  signals:
    // Indicates that the effect pEffect has been loaded into slotNumber of
    // EffectChainSlot chainNumber. pEffect may be an invalid pointer, which
    // indicates that a previously loaded effect was removed from the slot.
    void effectLoaded(EffectPointer pEffect, unsigned int chainNumber, unsigned int slotNumber);

    // Indicates that the given EffectChain was loaded into this
    // EffectChainSlot
    void effectChainLoaded(EffectChainPointer pEffectChain);

    // Signal that whoever is in charge of this EffectChainSlot should load the next
    // EffectChain into it.
    void nextChain(const unsigned int iChainSlotNumber, EffectChainPointer pEffectChain);

    // Signal that whoever is in charge of this EffectChainSlot should load the
    // previous EffectChain into it.
    void prevChain(const unsigned int iChainSlotNumber, EffectChainPointer pEffectChain);

    // Signal that indicates that the EffectChainSlot has been updated.
    void updated();

  private slots:
    void slotChainUpdated();
    void slotEffectLoaded(EffectPointer pEffect, unsigned int slotNumber);

    void slotControlNumEffects(double v);
    void slotControlChainEnabled(double v);
    void slotControlChainMix(double v);
    void slotControlChainParameter(double v);
    void slotControlChainNextPreset(double v);
    void slotControlChainPrevPreset(double v);

  private:
    QString debugString() const {
        return QString("EffectChainSlot(%1)").arg(m_iChainNumber);
    }

    void clear();
    bool privateIsEnabled() const;

    mutable QMutex m_mutex;
    const unsigned int m_iChainNumber;
    const QString m_group;

    EffectChainPointer m_pEffectChain;

    ControlObject* m_pControlNumEffects;
    ControlObject* m_pControlChainEnabled;
    ControlObject* m_pControlChainMix;
    ControlObject* m_pControlChainParameter;
    ControlObject* m_pControlChainNextPreset;
    ControlObject* m_pControlChainPrevPreset;

    QMap<QString, ControlObject*> m_channelEnableControls;

    QList<EffectSlotPointer> m_slots;

    DISALLOW_COPY_AND_ASSIGN(EffectChainSlot);
};


#endif /* EFFECTCHAINSLOT_H */
