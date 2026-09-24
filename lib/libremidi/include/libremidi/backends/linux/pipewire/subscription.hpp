#pragma once
#include <libremidi/config.hpp>

#include <cstdint>
#include <memory>
#include <utility>

namespace libremidi::pipewire
{

class context;

class subscription
{
public:
  subscription() noexcept = default;

  subscription(std::weak_ptr<context> ctx, std::uint64_t id) noexcept
      : m_ctx{std::move(ctx)}
      , m_id{id}
  {
  }

  subscription(const subscription&) = delete;
  subscription& operator=(const subscription&) = delete;

  subscription(subscription&& other) noexcept
      : m_ctx{std::move(other.m_ctx)}
      , m_id{std::exchange(other.m_id, 0)}
  {
  }

  subscription& operator=(subscription&& other) noexcept
  {
    if (this != &other)
    {
      reset();
      m_ctx = std::move(other.m_ctx);
      m_id = std::exchange(other.m_id, 0);
    }
    return *this;
  }

  ~subscription();

  void reset() noexcept;
  bool empty() const noexcept { return m_id == 0; }
  explicit operator bool() const noexcept { return !empty(); }
  std::uint64_t id() const noexcept { return m_id; }

private:
  std::weak_ptr<context> m_ctx;
  std::uint64_t m_id{};
};

}
