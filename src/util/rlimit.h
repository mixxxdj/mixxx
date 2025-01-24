#pragma once

#ifdef __LINUX__

namespace RLimit {

unsigned int getCurRtPrio();
unsigned int getMaxRtPrio();
bool isRtPrioAllowed();

} // namespace RLimit

#endif // __LINUX__
