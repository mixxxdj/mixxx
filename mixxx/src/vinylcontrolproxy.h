#ifndef __VINYLCONTROLPROXY_H__
#define __VINYLCONTROLPROXY_H__

#include "vinylcontrol.h"

//#include "vinylcontrolscratchlib.h"

class VinylControlProxy : public VinylControl
{
    public:
        VinylControlProxy(ConfigObject<ConfigValue> *pConfig, const char *_group);
        ~VinylControlProxy();
        bool isEnabled();
        void AnalyseSamples(short* samples, size_t size);
        void ToggleVinylControl(bool enable);
        void run();
        float getSpeed();
        float getSignalLeft();
        float getSignalRight();
        float getTimecodeQuality();
    protected:
        VinylControl* m_pVinylControl; //Pointer to active VinylControl object
};

#endif
