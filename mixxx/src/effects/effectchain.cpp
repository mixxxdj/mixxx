#include <QMutexLocker>

#include "effects/effectchain.h"
#include "sampleutil.h"

EffectChain::EffectChain(QObject* pParent, unsigned int iChainNumber)
        : QObject(),
          m_mutex(QMutex::Recursive),
          m_iChainNumber(iChainNumber),
          // The control group names are 1-indexed while internally everything is 0-indexed.
          m_group(formatGroupString(iChainNumber)),
          m_id(""),
          m_name("") {

    m_pControlNumEffectSlots = new ControlObject(ConfigKey(m_group, "num_effectslots"));
    m_pControlNumEffectSlots->set(0.0f);
    connect(m_pControlNumEffectSlots, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlNumEffectSlots(double)));

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

EffectChain::~EffectChain() {
    qDebug() << debugString() << "destroyed";
    delete m_pControlNumEffectSlots;
    delete m_pControlChainEnabled;
    delete m_pControlChainMix;
    delete m_pControlChainParameter;
    delete m_pControlChainPrevPreset;
    delete m_pControlChainNextPreset;
    m_slots.clear();
}

QString EffectChain::id() const {
    QMutexLocker locker(&m_mutex);
    return m_id;
}

void EffectChain::setId(const QString id) {
    QMutexLocker locker(&m_mutex);
    m_id = id;
}

QString EffectChain::name() const {
    QMutexLocker locker(&m_mutex);
    return m_name;
}

void EffectChain::setName(const QString name) {
    QMutexLocker locker(&m_mutex);
    m_name = name;
}

bool EffectChain::isEnabled() const {
    QMutexLocker locker(&m_mutex);
    return privateIsEnabled();
}

bool EffectChain::isEnabledForChannel(QString channelId) const {
    QMutexLocker locker(&m_mutex);
    if (!m_channelEnableControls.contains(channelId)) {
        qDebug() << "WARNING: Checking whether chain is enabled for channel that hasn't been registered.";
        return false;
    }
    return m_channelEnableControls[channelId]->get() > 0.0f && privateIsEnabled();
}

bool EffectChain::privateIsEnabled() const {
    return m_pControlChainEnabled->get() > 0.0f;
}

void EffectChain::process(const QString channelId,
                          const CSAMPLE* pInput,
                          CSAMPLE* pOutput,
                          const unsigned int numSamples) {
    qDebug() << debugString() << "process" << channelId << numSamples;
    QMutexLocker locker(&m_mutex);

    if (!privateIsEnabled()) {
        // SampleUtil handles shortcuts when aliased, and gains of 1.0, etc.
        return SampleUtil::copyWithGain(pOutput, pInput, 1.0f, numSamples);
    }

    foreach (EffectSlotPointer effectSlot, m_slots) {
        EffectPointer pEffect = effectSlot->getEffect();

        if (pEffect) {
            pEffect->process(channelId, pInput, pOutput, numSamples);
        }
    }
}

unsigned int EffectChain::numSlots() const {
    qDebug() << debugString() << "numSlots";
    QMutexLocker locker(&m_mutex);
    return m_slots.size();
}

void EffectChain::addEffectSlot() {
    qDebug() << debugString() << "addEffectSlot";
    QMutexLocker locker(&m_mutex);

    EffectSlot* pEffectSlot = new EffectSlot(this, m_iChainNumber, m_slots.size());
    // Rebroadcast effectLoaded signals
    connect(pEffectSlot, SIGNAL(effectLoaded(EffectPointer, unsigned int)),
            this, SLOT(slotEffectLoaded(EffectPointer, unsigned int)));
    m_slots.append(EffectSlotPointer(pEffectSlot));
    m_pControlNumEffectSlots->add(1.0f);
}

void EffectChain::registerChannel(const QString channelId) {
    if (m_channelEnableControls.contains(channelId)) {
        qDebug() << debugString() << "WARNING: registerChannel already has channel registered:" << channelId;
        return;
    }
    ControlObject* pEnableControl = new ControlObject(
        ConfigKey(m_group, QString("channel_%1").arg(channelId)));
    pEnableControl->set(0.0f);
    m_channelEnableControls[channelId] = pEnableControl;
}

void EffectChain::slotEffectLoaded(EffectPointer pEffect, unsigned int slotNumber) {
    // const int is a safe read... don't bother locking
    emit(effectLoaded(pEffect, m_iChainNumber, slotNumber));
}

EffectSlotPointer EffectChain::getEffectSlot(unsigned int slotNumber) {
    qDebug() << debugString() << "getEffectSlot" << slotNumber;
    QMutexLocker locker(&m_mutex);
    if (slotNumber >= m_slots.size()) {
        qDebug() << "WARNING: slotNumber out of range";
        return EffectSlotPointer();
    }
    return m_slots[slotNumber];
}

void EffectChain::slotControlNumEffectSlots(double v) {
    qDebug() << debugString() << "slotControlNumEffectSlots" << v;
    QMutexLocker locker(&m_mutex);
    qDebug() << "WARNING: Somebody has set a read-only control. Stability may be compromised.";
}

void EffectChain::slotControlChainEnabled(double v) {
    qDebug() << debugString() << "slotControlChainEnabled" << v;
    //QMutexLocker locker(&m_mutex);
}

void EffectChain::slotControlChainMix(double v) {
    qDebug() << debugString() << "slotControlChainMix" << v;
    //QMutexLocker locker(&m_mutex);
}

void EffectChain::slotControlChainParameter(double v) {
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
}

void EffectChain::slotControlChainNextPreset(double v) {
    qDebug() << debugString() << "slotControlChainNextPreset" << v;
    //QMutexLocker locker(&m_mutex);
    // const int read is not worth locking for
    if (v > 0)
        emit(nextPreset(m_iChainNumber));
}

void EffectChain::slotControlChainPrevPreset(double v) {
    qDebug() << debugString() << "slotControlChainPrevPreset" << v;
    //QMutexLocker locker(&m_mutex);
    // const int read is not worth locking for
    if (v > 0)
        emit(prevPreset(m_iChainNumber));
}
