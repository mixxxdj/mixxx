#pragma once
#include <libremidi/config.hpp>

#include <string>

NAMESPACE_LIBREMIDI
{
namespace webmidi_helpers
{
struct device_information
{
  std::string id;
  std::string name;
  bool connected{};
};
}
}
