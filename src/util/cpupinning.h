#include <Qt>

namespace mixxx {

class CpuPinning {
  public:
    /// Pin the current thread to cpuid
    static bool pinThreadToCpu(qint32 cpuid);
#ifdef __LINUX__
    /// Moves the thread to a cpuset cgroup
    static bool moveThreadToCpuset(const QString& cgroup);
#endif
};

} // namespace mixxx
