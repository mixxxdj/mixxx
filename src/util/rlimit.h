#pragma once

#ifdef __LINUX__

class RLimit {
  public:
    static unsigned int getCurRtPrio();
    static unsigned int getMaxRtPrio();
    static bool isRtPrioAllowed();
};

#endif // __LINUX__
