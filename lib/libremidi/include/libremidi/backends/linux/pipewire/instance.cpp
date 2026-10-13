#if defined(LIBREMIDI_PIPEWIRE)

  #if !defined(LIBREMIDI_HEADER_ONLY)
    #include <libremidi/backends/linux/pipewire/instance.hpp>
  #endif

  #include <mutex>

namespace libremidi::pipewire
{

LIBREMIDI_INLINE
std::shared_ptr<instance> shared_instance() noexcept
{
  static std::mutex mtx;
  static std::weak_ptr<instance> weak;

  std::lock_guard lock{mtx};
  if (auto p = weak.lock())
    return p;

  auto p = std::make_shared<instance>();
  if (!p->initialized())
    return {};
  weak = p;
  return p;
}

}

#endif
