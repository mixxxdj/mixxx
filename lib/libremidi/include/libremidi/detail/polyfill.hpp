#pragma once
#include <libremidi/config.hpp>

NAMESPACE_LIBREMIDI
{
static constexpr auto to_underlying(auto e)
{
  return static_cast<std::underlying_type_t<decltype(e)>>(e);
}
}