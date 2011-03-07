#include <QMutexLocker>

#include "effects/effectchainslot.h"
#include "sampleutil.h"

EffectChainSlot::EffectChainSlot(QObject* pParent, unsigned int iChainNumber)
        : QObject(),
          m_mutex(QMutex::Recursive),
          m_iChainNumber(iChainNumber),
          // The control group names are 1-indexed while internally everything is 0-indexed.
          m_group(formatGroupString(iChainNumber)) {

    m_pControlNumEffects = new ControlObject(ConfigKey(m_group, "num_effects"));
    m_pControlNumEffects->set(0.0f);
    connect(m_pControlNumEffects, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlNumEffects(double)));

    m_pControlChainEnabled = new ControlObject(ConfigKey(m_group, "enabled"));
    connect(m_pControlChainEnabled, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlChainEnabled(double)));
    // Default chain to enabled.
    m_pControlChainEnabled->set(1.0f);

    m_pControlChainMix = new ControlObject(ConfigKey(m_group, "mix"));
    connect(m_pControlChainMix, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlChainMix(double)));

    m_pControlChainParameter = new ControlObject(ConfigKey(m_group, "parameter"));
    connect(m_pControlChainParameter, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlChainParameter(double)));

    m_pControlChainNextPreset = new ControlObject(ConfigKey(m_group, "next_preset"));
    connect(m_pControlChainNextPreset, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlChainNextPreset(double)));

    m_pControlChainPrevPreset = new ControlObject(ConfigKey(m_group, "prev_preset"));
    connect(m_pControlChainPrevPreset, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlChainPrevPreset(double)));
}

EffectChainSlot::~EffectChainSlot() {
    qDebug() << debugString() << "destroyed";
    delete m_pControlNumEffects;
    delete m_pControlChainEnabled;
    delete m_pControlChainMix;
    delete m_pControlChainParameter;
    delete m_pControlChainPrevPreset;
    delete m_pControlChainNextPreset;
    m_slots.clear();
    m_pEffectChain.clear();
}

QString EffectChainSlot::id() const {
    QMutexLocker locker(&m_mutex);
    if (m_pEffectChain)
        return m_pEffectChain->id();
    return "";
}

QString EffectChainSlot::name() const {
    QMutexLocker locker(&m_mutex);
    if (m_pEffectChain)
        return m_pEffectChain->name();
    return tr("None");
}

void EffectChainSlot::slotChainUpdated() {
    qDebug() << debugString() << "slotChainUpdated";
    if (m_pEffectChain) {
        QList<EffectPointer> effects = m_pEffectChain->getEffects();
        for (int i = 0; i < effects.size(); ++i) {
            EffectPointer pEffect = effects[i];
            while (i >= m_slots.size()) {
                addEffectSlot();
            }
            EffectSlotPointer pSlot = m_slots[i];
            if (pSlot && pEffect) {
                pSlot->loadEffect(pEffect);
            }
        }

        m_pControlNumEffects->set(m_pEffectChain->numEffects());
        m_pControlChainParameter->set(m_pEffectChain->parameter());

        // TODO(rryan) is this a reasonable decision? Keep the enabled and mix
        // values the same because a) it keeps the controls from getting out of
        // sync with your MIDI controller, and b) they aren't something you'd
        // care about saving as an attribute of your prototypical EffectChain.

        //m_pControlChainEnabled->set()
        //m_pControlChainMix->set()
    }
}

void EffectChainSlot::loadEffectChain(EffectChainPointer pEffectChain) {
    qDebug() << debugString() << "loadEffectChain" << pEffectChain->id();
    QMutexLocker locker(&m_mutex);

    // Clear any loaded EffectChain
    clear();

    if (pEffectChain) {
        m_pEffectChain = pEffectChain;
        connect(m_pEffectChain.data(), SIGNAL(updated()),
                this, SLOT(slotChainUpdated()));
        slotChainUpdated();
    }

    locker.unlock();
    emit(effectChainLoaded(pEffectChain));
}

EffectChainPointer EffectChainSlot::getEffectChain() const {
    return m_pEffectChain;
}

void EffectChainSlot::clear() {
    // Stop listening to signals from any loaded effect
    if (m_pEffectChain) {
        m_pEffectChain->disconnect(this);
        m_pEffectChain.clear();
    }
    m_pControlNumEffects->set(0.0f);
    m_pControlChainEnabled->set(0.0f);
    m_pControlChainMix->set(0.0f);
    m_pControlChainParameter->set(0.0f);
}

bool EffectChainSlot::isEnabled() const {
    QMutexLocker locker(&m_mutex);
    return privateIsEnabled();
}

bool EffectChainSlot::isEnabledForChannel(QString channelId) const {
    QMutexLocker locker(&m_mutex);
    if (!m_channelEnableControls.contains(channelId)) {
        qDebug() << "WARNING: Checking whether chain is enabled for channel that hasn't been registered.";
        return false;
    }
    return m_channelEnableControls[channelId]->get() > 0.0f && privateIsEnabled();
}

bool EffectChainSlot::privateIsEnabled() const {
    return m_pControlChainEnabled->get() > 0.0f;
}

void EffectChainSlot::process(const QString channelId,
                              const CSAMPLE* pInput,
                              CSAMPLE* pOutput,
                              const unsigned int numSamples) {
    qDebug() << debugString() << "process" << channelId << numSamples;
    QMutexLocker locker(&m_mutex);
    EffectChainPointer pEffectChain = m_pEffectChain;
    bool isEnabled = pEffectChain && isEnabledForChannel(channelId);

    ////////////////////////////////////////////////////////////////////////////
    // AFTER THIS LINE, THE MUTEX IS UNLOCKED. DONT TOUCH ANY MEMBER STATE
    ////////////////////////////////////////////////////////////////////////////
    locker.unlock();

    if (isEnabled) {
        pEffectChain->process(channelId, pInput, pOutput, numSamples);
    } else {
        // SampleUtil handles shortcuts when aliased, and gains of 1.0, etc.
        return SampleUtil::copyWithGain(pOutput, pInput, 1.0f, numSamples);
    }
}

unsigned int EffectChainSlot::numSlots() const {
    qDebug() << debugString() << "numSlots";
    QMutexLocker locker(&m_mutex);
    return m_slots.size();
}

void EffectChainSlot::addEffectSlot() {
    qDebug() << debugString() << "addEffectSlot";
    QMutexLocker locker(&m_mutex);

    EffectSlot* pEffectSlot = new EffectSlot(this, m_iChainNumber, m_slots.size());
    // Rebroadcast effectLoaded signals
    connect(pEffectSlot, SIGNAL(effectLoaded(EffectPointer, unsigned int)),
            this, SLOT(slotEffectLoaded(EffectPointer, unsigned int)));
    m_slots.append(EffectSlotPointer(pEffectSlot));
}

void EffectChainSlot::registerChannel(const QString channelId) {
    if (m_channelEnableControls.contains(channelId)) {
        qDebug() << debugString() << "WARNING: registerChannel already has channel registered:" << channelId;
        return;
    }
    ControlObject* pEnableControl = new ControlObject(
        ConfigKey(m_group, QString("channel_%1").arg(channelId)));
    pEnableControl->set(0.0f);
    m_channelEnableControls[channelId] = pEnableControl;
}

void EffectChainSlot::slotEffectLoaded(EffectPointer pEffect, unsigned int slotNumber) {
    // const int is a safe read... don't bother locking
    emit(effectLoaded(pEffect, m_iChainNumber, slotNumber));
}

EffectSlotPointer EffectChainSlot::getEffectSlot(unsigned int slotNumber) {
    qDebug() << debugString() << "getEffectSlot" << slotNumber;
    QMutexLocker locker(&m_mutex);
    if (slotNumber >= m_slots.size()) {
        qDebug() << "WARNING: slotNumber out of range";
        return EffectSlotPointer();
    }
    return m_slots[slotNumber];
}

void EffectChainSlot::slotControlNumEffects(double v) {
    qDebug() << debugString() << "slotControlNumEffects" << v;
    QMutexLocker locker(&m_mutex);
    qDebug() << "WARNING: Somebody has set a read-only control. Stability may be compromised.";
}

void EffectChainSlot::slotControlChainEnabled(double v) {
    qDebug() << debugString() << "slotControlChainEnabled" << v;
    QMutexLocker locker(&m_mutex);

    bool bEnabled = v > 0.0f;
}

void EffectChainSlot::slotControlChainMix(double v) {
    qDebug() << debugString() << "slotControlChainMix" << v;
    QMutexLocker locker(&m_mutex);

    // Convert from stupid control system
    v = v / 127.0f;

    // Clamp to [0.0, 1.0]
    if (v < 0.0f || v > 1.0f) {
        qDebug() << debugString() << "value out of limits";
        v = math_clamp(v, 0.0f, 1.0f);
        m_pControlChainMix->set(v);
    }
}

void EffectChainSlot::slotControlChainParameter(double v) {
    qDebug() << debugString() << "slotControlChainParameter" << v;
    QMutexLocker locker(&m_mutex);

    // Convert from stupid control system
    v = v / 127.0f;

    // Clamp to [0.0, 1.0]
    if (v < 0.0f || v > 1.0f) {
        qDebug() << debugString() << "value out of limits";
        v = math_clamp(v, 0.0f, 1.0f);
        m_pControlChainParameter->set(v);
    }
    m_pEffectChain->setParameter(v);
}

void EffectChainSlot::slotControlChainNextPreset(double v) {
    qDebug() << debugString() << "slotControlChainNextPreset" << v;
    //QMutexLocker locker(&m_mutex);
    // const int read is not worth locking for
    if (v > 0)
        emit(nextPreset(m_iChainNumber));
}

void EffectChainSlot::slotControlChainPrevPreset(double v) {
    qDebug() << debugString() << "slotControlChainPrevPreset" << v;
    //QMutexLocker locker(&m_mutex);
    // const int read is not worth locking for
    if (v > 0)
        emit(prevPreset(m_iChainNumber));
}
