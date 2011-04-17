#ifndef __VINYLCONTROLPROXY_H__
#define __VINYLCONTROLPROXY_H__

#include "vinylcontrol.h"
#include "soundmanagerutil.h"

class VinylControlProxy : public VinylControl
{
  public:
    VinylControlProxy(ConfigObject<ConfigValue> *pConfig, QString group);
    ~VinylControlProxy();
    bool isEnabled();
    void AnalyseSamples(const short* samples, size_t size);
    void ToggleVinylControl(bool enable);
    void run();
    float getSpeed();
    float getTimecodeQuality();
    unsigned char* getScopeBytemap();
    float getAngle();
  protected:
    VinylControl* m_pVinylControl; //Pointer to active VinylControl object
};

#endif
