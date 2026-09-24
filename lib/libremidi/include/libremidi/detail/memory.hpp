#pragma once

#include <libremidi/config.hpp>

#include <memory>
#include <mutex>

NAMESPACE_LIBREMIDI
{
template <auto func>
struct deleter
{
  template <typename U>
  void operator()(U* x) noexcept(noexcept(func(x)))
  {
    func(x);
  }
};

template <typename T, auto func>
using unique_handle = std::unique_ptr<T, deleter<func>>;

template <typename T>
std::shared_ptr<T> instance()
{
  static std::mutex mut;
  static std::weak_ptr<T> cache;

  std::lock_guard _{mut};

  if (auto ptr = cache.lock())
  {
    return ptr;
  }
  else
  {
    auto shared = std::make_shared<T>();
    cache = shared;
    return shared;
  }
}
}
