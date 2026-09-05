#if defined(LIBREMIDI_PIPEWIRE)

  #if !defined(LIBREMIDI_HEADER_ONLY)
    #include <libremidi/backends/linux/pipewire/context.hpp>
  #endif

  #include <mutex>

namespace libremidi::pipewire
{

LIBREMIDI_INLINE
std::shared_ptr<context> shared_context(context::config cfg) noexcept
{
  static std::mutex mtx;
  static std::weak_ptr<context> weak;

  std::lock_guard lock{mtx};
  if (auto p = weak.lock())
    return context::make_shared_holder(std::move(p));

  auto inst = shared_instance();
  if (!inst)
    return {};

  auto ctx = context::make(std::move(inst), cfg);
  if (!ctx)
    return {};
  weak = ctx;
  return context::make_shared_holder(std::move(ctx));
}

}

#endif
