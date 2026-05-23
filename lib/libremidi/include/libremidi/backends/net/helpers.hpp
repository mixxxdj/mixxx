#pragma once
#include <libremidi/config.hpp>

#include <boost/asio/ip/udp.hpp>

#include <memory>
namespace stdx
{ /*

class boost_system_error_domain : public error_domain
{
public:
  constexpr boost_system_error_domain() noexcept
      : error_domain{
            {1 - 0x3c223c0aa3cf45e5ULL, 1 - 0x80dac24345cfb9fcULL},
            default_error_resource_management_t<detail::exception_ptr_wrapper>{}}
  {
  }

  string_ref name() const noexcept override { return "boost::system_error domain"; }

  bool equivalent(const error& lhs, const error& rhs) const noexcept override { return false; }

  string_ref message(const error& e) const noexcept override {
    auto err = e.m_value;

  }

  [[noreturn]] void throw_exception(const error& e) const override
  {
    assert(e.domain() == *this);
    std::rethrow_exception(error_cast<detail::exception_ptr_wrapper>(e).get());
  }
};
*/
}

NAMESPACE_LIBREMIDI
{
template <typename T>
struct optionally_owned
{
public:
  explicit optionally_owned(T* maybe_existing)
      : m_storage{.ref = maybe_existing}
  {
    if (m_storage.ref)
    {
      m_ownership = unowned;
      return;
    }
    else
    {
      std::destroy_at(&m_storage.ref);
      std::construct_at<T>(reinterpret_cast<T*>(m_storage.object));
      m_ownership = owned;
    }
  }

  ~optionally_owned()
  {
    if (is_owned())
      std::destroy_at(reinterpret_cast<T*>(m_storage.object));
  }

  T& get() noexcept
  {
    return *(is_owned() ? reinterpret_cast<T*>(&m_storage.object) : m_storage.ref);
  }
  const T& get() const noexcept
  {
    return *(is_owned() ? reinterpret_cast<T*>(&m_storage.object) : m_storage.ref);
  }

  optionally_owned(const optionally_owned&) = delete;
  optionally_owned(optionally_owned&&) noexcept = delete;
  optionally_owned& operator=(const optionally_owned&) = delete;
  optionally_owned& operator=(optionally_owned&&) noexcept = delete;

  bool is_owned() const noexcept { return m_ownership == owned; }

private:
  union
  {
    alignas(T) unsigned char object[sizeof(T)];
    T* ref;
  } m_storage;
  enum
  {
    owned,
    unowned
  } m_ownership{};
};
}
