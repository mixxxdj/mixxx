#include "widget/vumeterpool.h"

#include <cassert>

#include "widget/wvumeter.h"

VuMeterPool::VuMeterPool() {
}

VuMeterPool::~VuMeterPool() {
    destroyAdopted();
    assert(m_pVuMeters.isEmpty());
}

void VuMeterPool::destroyAdopted() {
    auto it = m_pVuMeters.begin();
    while (it != m_pVuMeters.end()) {
        auto pVuMeter = *it;
        if (pVuMeter->parent()) {
            it++;
        } else {
            it = m_pVuMeters.erase(it);
            delete pVuMeter;
        }
    }
}

void VuMeterPool::add(WVuMeter* pVuMeter) {
    m_pVuMeters.append(pVuMeter);
}

void VuMeterPool::remove(WVuMeter* pVuMeter) {
    m_pVuMeters.removeOne(pVuMeter);
}

void VuMeterPool::adopt() {
    // Remove the parent of the vumeters,, but we keep their pointers,
    // effectively "adopting" them.
    for (auto pVuMeter : m_pVuMeters) {
        pVuMeter->disconnect();
        pVuMeter->setParent(Q_NULLPTR);
    }
}
