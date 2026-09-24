#pragma once
#include <libremidi/config.hpp>

#include <string>

#if !defined(BOOST_ASIO_IO_CONTEXT_HPP)
#if __has_include(<boost/asio/detail/config.hpp>)
  #include <boost/asio/detail/config.hpp>
#endif

namespace boost::asio
{
#if defined(BOOST_ASIO_INLINE_NAMESPACE_BEGIN)
BOOST_ASIO_INLINE_NAMESPACE_BEGIN
class io_context;
BOOST_ASIO_INLINE_NAMESPACE_END
#else
class io_context;
#endif
}
#endif

NAMESPACE_LIBREMIDI::net
{
enum class protocol
{
  OSC_MIDI,
};

struct dgram_input_configuration
{
  std::string client_name = "libremidi client";

  enum protocol protocol = protocol::OSC_MIDI;
  std::string accept = "0.0.0.0";
  int port{};

  boost::asio::io_context* io_context{};
};

struct dgram_output_configuration
{
  std::string client_name = "libremidi client";

  enum protocol protocol = protocol::OSC_MIDI;
  std::string host = "127.0.0.1";
  int port{};
  bool broadcast{};

  boost::asio::io_context* io_context{};
};

struct dgram_observer_configuration
{
  std::string client_name = "libremidi client";

  boost::asio::io_context* io_context{};
};

}

NAMESPACE_LIBREMIDI::net_ump
{
enum class protocol
{
  OSC_MIDI2,
};

struct dgram_input_configuration
{
  std::string client_name = "libremidi client";

  enum protocol protocol = protocol::OSC_MIDI2;
  std::string accept = "0.0.0.0";
  int port{};

  boost::asio::io_context* io_context{};
};

struct dgram_output_configuration
{
  std::string client_name = "libremidi client";

  enum protocol protocol = protocol::OSC_MIDI2;
  std::string host = "127.0.0.1";
  int port{};
  bool broadcast{};

  boost::asio::io_context* io_context{};
};

struct dgram_observer_configuration
{
  std::string client_name = "libremidi client";

  boost::asio::io_context* io_context{};
};

}
