#ifndef RLIMIT_H
#define RLIMIT_H

#ifdef __LINUX__

class RLimit {
  public:
    static unsigned int getCurRtPrio();
    static unsigned int getMaxRtPrio();
    static bool isRtPrioAllowed();
};

#endif // __LINUX__
#endif // RLIMIT_H_
