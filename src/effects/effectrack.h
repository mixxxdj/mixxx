#ifndef EFFECTRACK_H
#define EFFECTRACK_H

#include <QObject>
#include <QString>
#include <QSharedPointer>
#include <QHash>

#include "controlobject.h"
#include "effects/effectchainslot.h"

class EngineEffectRack;
class EffectsManager;
class EffectChainManager;

class EffectRack;
class StandardEffectRack;
class EqualizerRack;
class QuickEffectRack;
typedef QSharedPointer<EffectRack> EffectRackPointer;
typedef QSharedPointer<StandardEffectRack> StandardEffectRackPointer;
typedef QSharedPointer<EqualizerRack> EqualizerRackPointer;
typedef QSharedPointer<QuickEffectRack> QuickEffectRackPointer;

class EffectRack : public QObject {
    Q_OBJECT
  public:
    EffectRack(EffectsManager* pEffectsManager,
               EffectChainManager* pChainManager,
               const unsigned int iRackNumber,
               const QString& group);
    virtual ~EffectRack();

    void addToEngine();
    void removeFromEngine();
    EngineEffectRack* getEngineEffectRack();

    void registerGroup(const QString& group);
    int numEffectChainSlots() const;
    EffectChainSlotPointer getEffectChainSlot(int i);

    unsigned int getRackNumber() const {
        return m_iRackNumber;
    }

    const QString& getGroup() const {
        return m_group;
    }

  public slots:
    void slotClearRack(double v);
    void slotNumEffectChainSlots(double v);

  private slots:
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
    virtual EffectChainPointer makeEmptyChain();

    // We could make accessors for these for sub-classes. Doesn't really matter.
    EffectsManager* m_pEffectsManager;
    EffectChainManager* m_pEffectChainManager;

  private:
    const unsigned int m_iRackNumber;
    const QString m_group;
    QList<EffectChainSlotPointer> m_effectChainSlots;
    ControlObject m_controlNumEffectChainSlots;
    ControlObject m_controlClearRack;
    EngineEffectRack* m_pEngineEffectRack;
};

class StandardEffectRack : public EffectRack {
    Q_OBJECT
  public:
    StandardEffectRack(EffectsManager* pEffectsManager,
                       EffectChainManager* pChainManager,
                       const unsigned int iRackNumber);
    virtual ~StandardEffectRack() {}

    static QString formatGroupString(const unsigned int iRackNumber) {
        return QString("[EffectRack%1]")
                .arg(QString::number(iRackNumber + 1));
    }

    static QString formatEffectChainSlotGroupString(const unsigned int iRackNumber,
                                                    const unsigned int iChainSlotNumber) {
        return QString("[EffectRack%1_EffectUnit%2]")
                .arg(QString::number(iRackNumber + 1))
                .arg(QString::number(iChainSlotNumber + 1));
    }

    static QString formatEffectSlotGroupString(const unsigned int iRackNumber,
                                               const unsigned int iChainSlotNumber,
                                               const unsigned int iEffectSlotNumber) {
        return QString("[EffectRack%1_EffectUnit%2_Effect%3]")
                .arg(QString::number(iRackNumber + 1))
                .arg(QString::number(iChainSlotNumber + 1))
                .arg(QString::number(iEffectSlotNumber + 1));
    }

    EffectChainSlotPointer addEffectChainSlot();
};

class PerGroupRack : public EffectRack {
    Q_OBJECT
  public:
    PerGroupRack(EffectsManager* pEffectsManager,
                 EffectChainManager* pChainManager,
                 const unsigned int iRackNumber,
                 const QString& group);
    virtual ~PerGroupRack() {}

    EffectChainSlotPointer addEffectChainSlotForGroup(const QString& group);
    EffectChainSlotPointer getGroupEffectChainSlot(const QString& group);

  protected:
    virtual void configureEffectChainSlotForGroup(EffectChainSlotPointer pSlot,
                                                  const QString& group) = 0;
    virtual QString formatEffectChainSlotGroupForGroup(const unsigned int iRackNumber,
                                                       const unsigned int iChainSlotNumber,
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

    bool loadEffectToGroup(const QString& group, EffectPointer pEffect);

    static QString formatGroupString(const unsigned int iRackNumber) {
        return QString("[QuickEffectRack%1]")
                .arg(QString::number(iRackNumber + 1));
    }

    static QString formatEffectChainSlotGroupString(const unsigned int iRackNumber,
                                                    const QString& group) {
        return QString("[QuickEffectRack%1_%2]")
                .arg(QString::number(iRackNumber + 1))
                .arg(group);
    }

    static QString formatEffectSlotGroupString(const unsigned int iRackNumber,
                                               const unsigned int iEffectSlotNumber,
                                               const QString& group) {
        return QString("[QuickEffectRack%1_%2_Effect%3]")
                .arg(QString::number(iRackNumber + 1))
                .arg(group)
                .arg(QString::number(iEffectSlotNumber + 1));
    }

    QString formatEffectSlotGroupString(const unsigned int iEffectSlotNumber,
                                        const QString& group) const {
        return formatEffectSlotGroupString(getRackNumber(), iEffectSlotNumber,
                                           group);
    }

  protected:
    virtual void configureEffectChainSlotForGroup(EffectChainSlotPointer pSlot,
                                                  const QString& group);
    virtual QString formatEffectChainSlotGroupForGroup(const unsigned int iRackNumber,
                                                       const unsigned int iChainSlotNumber,
                                                       const QString& group) const {
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
        return QString("[EqualizerRack%1]")
                .arg(QString::number(iRackNumber + 1));
    }

    static QString formatEffectChainSlotGroupString(const unsigned int iRackNumber,
                                                    const QString& group) {
        return QString("[EqualizerRack%1_%2]")
                .arg(QString::number(iRackNumber + 1))
                .arg(group);
    }

    static QString formatEffectSlotGroupString(const unsigned int iRackNumber,
                                               const unsigned int iEffectSlotNumber,
                                               const QString& group) {
        return QString("[EqualizerRack%1_%2_Effect%3]")
                .arg(QString::number(iRackNumber + 1))
                .arg(group)
                .arg(QString::number(iEffectSlotNumber + 1));
    }

    QString formatEffectSlotGroupString(const unsigned int iEffectSlotNumber,
                                        const QString& group) const {
        return formatEffectSlotGroupString(getRackNumber(), iEffectSlotNumber,
                                           group);
    }

  protected:
    virtual void configureEffectChainSlotForGroup(EffectChainSlotPointer pSlot,
                                                  const QString& group);
    virtual QString formatEffectChainSlotGroupForGroup(const unsigned int iRackNumber,
                                                       const unsigned int iChainSlotNumber,
                                                       const QString& group) const {
        Q_UNUSED(iChainSlotNumber);
        return formatEffectChainSlotGroupString(iRackNumber, group);
    }
};

#endif /* EFFECTRACK_H */
