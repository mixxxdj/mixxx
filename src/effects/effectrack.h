#pragma once

#include <QObject>
#include <QString>
#include <QSharedPointer>
#include <QHash>

#include "control/controlobject.h"
#include "engine/channelhandle.h"
#include "effects/defs.h"

class EngineEffectRack;
class EffectsManager;
class EffectChainManager;

#include "effects/effectchainslot.h"

//TODO(Be): Remove these superfluous classes.
class EffectRack : public QObject {
    Q_OBJECT
  public:
    EffectRack(EffectsManager* pEffectsManager,
               EffectChainManager* pChainManager,
               const unsigned int iRackNumber,
               const QString& group, SignalProcessingStage stage);
    virtual ~EffectRack();

    void addToEngine();
    void removeFromEngine();
    EngineEffectRack* getEngineEffectRack();

    void registerInputChannel(const ChannelHandleAndGroup& handleGroup);
    int numEffectChainSlots() const;
    EffectChainSlotPointer getEffectChainSlot(int i);

    void maybeLoadEffect(const unsigned int iChainSlotNumber,
                         const unsigned int iEffectSlotNumber,
                         const QString& id);

    unsigned int getRackNumber() const {
        return m_iRackNumber;
    }

    const QString& getGroup() const {
        return m_group;
    }

    void refresh();

    QDomElement toXml(QDomDocument* doc) const;

    virtual bool isAdoptMetaknobValueEnabled() const;

  public slots:
    void slotClearRack(double v);

    void loadNextChain(const unsigned int iChainSlotNumber,
                       EffectChainPointer pLoadedChain);
    void loadPrevChain(const unsigned int iChainSlotNumber,
                       EffectChainPointer pLoadedChain);

    void loadNextEffect(const unsigned int iChainSlotNumber,
                        const unsigned int iEffectSlotNumber,
                        EffectPointer pEffect);
    void loadPrevEffect(const unsigned int iChainSlotNumber,
                        const unsigned int iEffectSlotNumber,
                        EffectPointer pEffect);

  protected:
    void addEffectChainSlotInternal(EffectChainSlotPointer pChainSlot);

    EngineEffectRack* m_pEngineEffectRack;

    // We could make accessors for these for sub-classes. Doesn't really matter.
    EffectsManager* m_pEffectsManager;
    EffectChainManager* m_pEffectChainManager;

  private:
    SignalProcessingStage m_signalProcessingStage;
    const unsigned int m_iRackNumber;
    const QString m_group;
    QList<EffectChainSlotPointer> m_effectChainSlots;
    ControlObject m_controlNumEffectChainSlots;
    ControlObject m_controlClearRack;
};

class StandardEffectRack : public EffectRack {
    Q_OBJECT
  public:
    StandardEffectRack(EffectsManager* pEffectsManager,
                       EffectChainManager* pChainManager,
                       const unsigned int iRackNumber);
    virtual ~StandardEffectRack() {}

    static QString formatGroupString(const unsigned int iRackNumber) {
        return QString("[EffectRack%1]").arg(QString::number(iRackNumber + 1));
    }

    static QString formatEffectChainSlotGroupString(const unsigned int iRackNumber,
                                                    const unsigned int iChainSlotNumber) {
        return QString("[EffectRack%1_EffectUnit%2]")
                .arg(QString::number(iRackNumber + 1),
                        QString::number(iChainSlotNumber + 1));
    }

    static QString formatEffectSlotGroupString(const unsigned int iRackNumber,
                                               const unsigned int iChainSlotNumber,
                                               const unsigned int iEffectSlotNumber) {
        return QString("[EffectRack%1_EffectUnit%2_Effect%3]")
                .arg(QString::number(iRackNumber + 1),
                        QString::number(iChainSlotNumber + 1),
                        QString::number(iEffectSlotNumber + 1));
    }

    EffectChainSlotPointer addEffectChainSlot();
};

class OutputEffectRack : public EffectRack {
    Q_OBJECT
  public:
    OutputEffectRack(EffectsManager* pEffectsManager,
                     EffectChainManager* pChainManager);
    virtual ~OutputEffectRack() {};
};

class PerGroupRack : public EffectRack {
    Q_OBJECT
  public:
    PerGroupRack(EffectsManager* pEffectsManager,
                 EffectChainManager* pChainManager,
                 const unsigned int iRackNumber,
                 const QString& group);
    virtual ~PerGroupRack() {}


    void setupForGroup(const QString& group);
    EffectChainSlotPointer getGroupEffectChainSlot(const QString& group);
    virtual bool loadEffectToGroup(const QString& group, EffectPointer pEffect);

  protected:
    virtual void configureEffectChainSlotForGroup(EffectChainSlotPointer pSlot,
                                                  const QString& group) = 0;

    virtual QString formatEffectChainSlotGroupForGroup(const unsigned int iRackNumber,
                                                       const unsigned int iChainSlotNumber,
                                                       const QString& group) const = 0;

    virtual QString formatEffectSlotGroupString(const unsigned int iEffectSlotNumber,
                                                const QString& group) const = 0;

  private:
    QHash<QString, EffectChainSlotPointer> m_groupToChainSlot;
};

class QuickEffectRack : public PerGroupRack {
    Q_OBJECT
  public:
    QuickEffectRack(EffectsManager* pEffectsManager,
                    EffectChainManager* pChainManager,
                    const unsigned int iRackNumber);
    virtual ~QuickEffectRack() {}

    bool loadEffectToGroup(const QString& group, EffectPointer pEffect) override;

    static QString formatGroupString(const unsigned int iRackNumber) {
        return QString("[QuickEffectRack%1]").arg(QString::number(iRackNumber + 1));
    }

    static QString formatEffectChainSlotGroupString(const unsigned int iRackNumber,
                                                    const QString& group) {
        return QString("[QuickEffectRack%1_%2]").arg(QString::number(iRackNumber + 1), group);
    }

    static QString formatEffectSlotGroupString(const unsigned int iRackNumber,
                                               const unsigned int iEffectSlotNumber,
                                               const QString& group) {
        return QString("[QuickEffectRack%1_%2_Effect%3]")
                .arg(QString::number(iRackNumber + 1),
                        group,
                        QString::number(iEffectSlotNumber + 1));
    }

    QString formatEffectSlotGroupString(const unsigned int iEffectSlotNumber,
                                        const QString& group) const override {
        return formatEffectSlotGroupString(getRackNumber(), iEffectSlotNumber,
                                           group);
    }

    bool isAdoptMetaknobValueEnabled() const override {
        // No visible Metaknobs to adopt
        return false;
    }

  protected:
    void configureEffectChainSlotForGroup(EffectChainSlotPointer pSlot,
                                          const QString& group) override;

    QString formatEffectChainSlotGroupForGroup(const unsigned int iRackNumber,
                                               const unsigned int iChainSlotNumber,
                                               const QString& group) const override {
        Q_UNUSED(iChainSlotNumber);
        return formatEffectChainSlotGroupString(iRackNumber, group);
    }
};

class EqualizerRack : public PerGroupRack {
    Q_OBJECT
  public:
    EqualizerRack(EffectsManager* pEffectsManager,
                  EffectChainManager* pChainManager,
                  const unsigned int iRackNumber);
    virtual ~EqualizerRack() {}

    static QString formatGroupString(const unsigned int iRackNumber) {
        return QString("[EqualizerRack%1]").arg(QString::number(iRackNumber + 1));
    }

    static QString formatEffectChainSlotGroupString(const unsigned int iRackNumber,
                                                    const QString& group) {
        return QString("[EqualizerRack%1_%2]").arg(QString::number(iRackNumber + 1), group);
    }

    static QString formatEffectSlotGroupString(const unsigned int iRackNumber,
                                               const unsigned int iEffectSlotNumber,
                                               const QString& group) {
        return QString("[EqualizerRack%1_%2_Effect%3]")
                .arg(QString::number(iRackNumber + 1),
                        group,
                        QString::number(iEffectSlotNumber + 1));
    }

    QString formatEffectSlotGroupString(const unsigned int iEffectSlotNumber,
                                        const QString& group) const override {
        return formatEffectSlotGroupString(getRackNumber(), iEffectSlotNumber,
                                           group);
    }

    bool isAdoptMetaknobValueEnabled() const override {
        // No visible Metaknobs to adopt
        return false;
    }

  protected:
    void configureEffectChainSlotForGroup(EffectChainSlotPointer pSlot,
                                          const QString& group) override;
    QString formatEffectChainSlotGroupForGroup(const unsigned int iRackNumber,
                                               const unsigned int iChainSlotNumber,
                                               const QString& group) const override {
        Q_UNUSED(iChainSlotNumber);
        return formatEffectChainSlotGroupString(iRackNumber, group);
    }
};
