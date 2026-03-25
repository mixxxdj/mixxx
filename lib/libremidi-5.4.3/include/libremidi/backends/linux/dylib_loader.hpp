#pragma once
#include <libremidi/config.hpp>
#if __has_include(<dlfcn.h>)
  #include <dlfcn.h>

  #include <cassert>

NAMESPACE_LIBREMIDI
{
class dylib_loader
{
public:
  explicit dylib_loader(const char* const so)
  {
    impl = dlopen(so, RTLD_LAZY | RTLD_LOCAL | RTLD_NODELETE);
  }

  dylib_loader(const dylib_loader&) noexcept = delete;
  dylib_loader& operator=(const dylib_loader&) noexcept = delete;
  dylib_loader(dylib_loader&& other) noexcept
  {
    impl = other.impl;
    other.impl = nullptr;
  }

  dylib_loader& operator=(dylib_loader&& other) noexcept
  {
    impl = other.impl;
    other.impl = nullptr;
    return *this;
  }

  ~dylib_loader()
  {
    if (impl)
    {
      dlclose(impl);
    }
  }

  template <typename T>
  T symbol(const char* const sym) const noexcept
  {
    assert(impl);
    return reinterpret_cast<T>(dlsym(impl, sym));
  }

  operator bool() const noexcept { return bool(impl); }

private:
  void* impl{};
};

}

  #define LIBREMIDI_SYMBOL_NAME_S(prefix, name) #prefix "_" #name
  #define LIBREMIDI_SYMBOL_NAME(prefix, name) prefix##_##name
  #define LIBREMIDI_SYMBOL_DEF(prefix, name) \
    decltype(&::LIBREMIDI_SYMBOL_NAME(prefix, name)) name{};
  #define LIBREMIDI_SYMBOL_INIT(prefix, name)                                  \
    {                                                                          \
      name = library.symbol<decltype(&::LIBREMIDI_SYMBOL_NAME(prefix, name))>( \
          LIBREMIDI_SYMBOL_NAME_S(prefix, name));                              \
      if (!name)                                                               \
      {                                                                        \
        available = false;                                                     \
        return;                                                                \
      }                                                                        \
    }

// Because some libs have names that are C++ keywords, e.g. udev_new:
  #define LIBREMIDI_SYMBOL_NAME2_S(prefix, name, varname) #prefix "_" #name
  #define LIBREMIDI_SYMBOL_NAME2(prefix, name, varname) prefix##_##name
  #define LIBREMIDI_SYMBOL_DEF2(prefix, name, varname) \
    decltype(&::LIBREMIDI_SYMBOL_NAME2(prefix, name, varname)) varname{};
  #define LIBREMIDI_SYMBOL_INIT2(prefix, name, varname)                                     \
    {                                                                                       \
      varname = library.symbol<decltype(&::LIBREMIDI_SYMBOL_NAME2(prefix, name, varname))>( \
          LIBREMIDI_SYMBOL_NAME2_S(prefix, name, varname));                                 \
      if (!varname)                                                                         \
      {                                                                                     \
        available = false;                                                                  \
        return;                                                                             \
      }                                                                                     \
    }
#endif
