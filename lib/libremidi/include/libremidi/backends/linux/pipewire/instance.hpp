#pragma once
#include <libremidi/backends/linux/pipewire/loader.hpp>
#include <libremidi/config.hpp>

#include <memory>

namespace libremidi::pipewire
{
class instance
{
public:
  instance() noexcept
  {
    if (auto& pw = load(); pw.core_available)
    {
      int argc = 0;
      char* argv[] = {nullptr};
      char** aa = argv;
      pw.init(&argc, &aa);
      m_initialized = true;
    }
  }

  ~instance()
  {
    if (m_initialized)
    {
      if (auto& pw = load(); pw.deinit)
        pw.deinit();
    }
  }

  instance(const instance&) = delete;
  instance& operator=(const instance&) = delete;
  instance(instance&&) = delete;
  instance& operator=(instance&&) = delete;

  bool initialized() const noexcept { return m_initialized; }

private:
  bool m_initialized{};
};

// Defined in instance.cpp: LIBREMIDI_EXPORT (default visibility) is
// required to dedup across plugin .so boundaries; inline would init
// pw_init per plugin and race libpipewire's globals.
LIBREMIDI_EXPORT std::shared_ptr<instance> shared_instance() noexcept;

}

#if defined(LIBREMIDI_HEADER_ONLY)
  #include <libremidi/backends/linux/pipewire/instance.cpp>
#endif
