#pragma once

#include <QObject>
#include <QList>
#include <QHash>

#include "preferences/usersettings.h"
#include "effects/effectchain.h"
#include "effects/effectrack.h"
#include "engine/channelhandle.h"
#include "util/class.h"
#include "util/xml.h"

class EffectsManager;

// A class for keeping track of all the user's EffectChains. Eventually will
// serialize/deserialize the EffectChains from storage but for Effects v1 we are
// hard-coding the available chains.
class EffectChainManager : public QObject {
    Q_OBJECT
  public:
    EffectChainManager(UserSettingsPointer pConfig,
                       EffectsManager* pEffectsManager);
    virtual ~EffectChainManager();

    void registerInputChannel(const ChannelHandleAndGroup& handle_group);
    const QSet<ChannelHandleAndGroup>& registeredInputChannels() const {
        return m_registeredInputChannels;
    }

    void registerOutputChannel(const ChannelHandleAndGroup& handle_group);
    const QSet<ChannelHandleAndGroup>& registeredOutputChannels() const {
        return m_registeredOutputChannels;
    }

    StandardEffectRackPointer addStandardEffectRack();
    StandardEffectRackPointer getStandardEffectRack(int rack);

    EqualizerRackPointer addEqualizerRack();
    EqualizerRackPointer getEqualizerRack(int rack);

    QuickEffectRackPointer addQuickEffectRack();
    QuickEffectRackPointer getQuickEffectRack(int rack);

    OutputEffectRackPointer addOutputsEffectRack();
    OutputEffectRackPointer getMasterEffectRack();

    EffectRackPointer getEffectRack(const QString& group);

    void addEffectChain(EffectChainPointer pEffectChain);
    void removeEffectChain(EffectChainPointer pEffectChain);

    // To support cycling through effect chains, there is a global ordering of
    // chains. These methods allow you to get the next or previous chain given
    // your current chain.
    // TODO(rryan): Prevent double-loading of a chain into a slot?
    EffectChainPointer getNextEffectChain(EffectChainPointer pEffectChain);
    EffectChainPointer getPrevEffectChain(EffectChainPointer pEffectChain);

    bool saveEffectChains();
    void loadEffectChains();

    // Reloads all effect to the slots to update parameter assignments
    void refeshAllRacks();

    static const int kNumStandardEffectChains = 4;

    bool isAdoptMetaknobValueEnabled() const;

  private:
    QString debugString() const {
        return "EffectChainManager";
    }

    UserSettingsPointer m_pConfig;
    EffectsManager* m_pEffectsManager;
    QList<StandardEffectRackPointer> m_standardEffectRacks;
    QList<EqualizerRackPointer> m_equalizerEffectRacks;
    QList<QuickEffectRackPointer> m_quickEffectRacks;
    OutputEffectRackPointer m_pOutputEffectRack;
    QHash<QString, EffectRackPointer> m_effectRacksByGroup;
    QList<EffectChainPointer> m_effectChains;
    QSet<ChannelHandleAndGroup> m_registeredInputChannels;
    QSet<ChannelHandleAndGroup> m_registeredOutputChannels;
    DISALLOW_COPY_AND_ASSIGN(EffectChainManager);
};
