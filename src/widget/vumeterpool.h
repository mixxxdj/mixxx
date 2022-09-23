#pragma once

#include <QList>
#include <QObject>

#include "util/singleton.h"

class WVuMeter;

class VuMeterPool : public Singleton<VuMeterPool> {
    QList<WVuMeter*> m_pVuMeters;

  public:
    VuMeterPool();
    ~VuMeterPool();

    void add(WVuMeter* pVuMeter);
    void remove(WVuMeter* pVuMeter);
    void adopt();
    void destroyAdopted();
};
