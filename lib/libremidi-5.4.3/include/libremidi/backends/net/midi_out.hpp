#pragma once
#include <libremidi/backends/net/config.hpp>
#include <libremidi/backends/net/helpers.hpp>
#include <libremidi/cmidi2.hpp>
#include <libremidi/detail/midi_out.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/endian.hpp>

NAMESPACE_LIBREMIDI::net
{

struct osc_midi1_packet
{
  stdx::error init_packet(std::string_view osc_pattern)
  {
    int i = 0;
    for (; i < std::ssize(osc_pattern); i++)
      if (osc_pattern.data()[i] != 0)
        bytes[i] = osc_pattern.data()[i];
      else
        break;
    bytes[i] = 0;
    while (i % 4 != 3)
    {
      ++i;
      bytes[i] = 0;
    }

    bytes[++i] = ',';
    bytes[++i] = 'm';
    bytes[++i] = 0;
    bytes[++i] = 0;

    // MIDI message (m) spec: port n°, status byte, data 1, data 2
    bytes[++i] = 0;
    message_size = ++i + 3;

    return stdx::error{}; // FIXME
  }

  stdx::error deinit()
  {
    message_size = 0;
    return stdx::error{};
  }

  stdx::error set_packet_content(const unsigned char* message, size_t size)
  {
    if (message_size == 0)
      return std::errc::not_connected;

    if (size != 3)
      return std::errc::message_size;

    std::memcpy(this->bytes + message_size - 3, message, 3);

    return stdx::error{};
  }

  std::span<const char> get_data() { return std::span(this->bytes, this->message_size); }
  char bytes[512 + 8 + 8];
  int message_size{};
};

class midi_out final
    : public midi1::out_api
    , public error_handler
{
public:
  struct
      : output_configuration
      , dgram_output_configuration
  {
  } configuration;

  midi_out(output_configuration&& conf, dgram_output_configuration&& apiconf)
      : configuration{std::move(conf), std::move(apiconf)}
      , ctx{configuration.io_context}
      , m_socket{ctx.get()}
  {
    m_socket.open(boost::asio::ip::udp::v4());
    m_socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
    m_socket.set_option(boost::asio::socket_base::broadcast(true));

    m_endpoint
        = {boost::asio::ip::make_address(this->configuration.host),
           static_cast<unsigned short>(this->configuration.port)};

    this->client_open_ = stdx::error{};
  }

  ~midi_out() override { close_port(); }

  libremidi::API get_current_api() const noexcept override { return libremidi::API::NETWORK; }

  stdx::error open_port(const output_port& /* port */, std::string_view portName) override
  {
    return open_virtual_port(portName);
  }

  stdx::error open_virtual_port(std::string_view portName) override
  {
    // Random arbitrary limit to avoid abuse
    if (portName.size() >= 512)
      return std::errc::invalid_argument;

    pkt.init_packet(portName);

    return stdx::error{};
  }

  stdx::error close_port() override
  {
    pkt.deinit();
    // FIXME async close
    if (m_socket.is_open())
      m_socket.close();
    return {};
  }

  stdx::error send_message(const unsigned char* message, size_t size) override
  {
    if (auto err = pkt.set_packet_content(message, size); err != stdx::error{})
      return err;

    auto dat = pkt.get_data();
    boost::system::error_code ec;
    m_socket.send_to(boost::asio::const_buffer(dat.data(), dat.size()), m_endpoint, 0, ec);
    return stdx::error{}; // FIXME
  }

  stdx::error schedule_message(int64_t, const unsigned char*, size_t) override
  {
    int ret = 0;
    return from_errc(ret);
  }

  libremidi::optionally_owned<boost::asio::io_context> ctx;
  boost::asio::ip::udp::endpoint m_endpoint;
  boost::asio::ip::udp::socket m_socket;

  osc_midi1_packet pkt;
};

}

NAMESPACE_LIBREMIDI::net_ump
{
struct osc_midi2_packet
{
  stdx::error init_packet(std::string_view osc_pattern)
  {
    int i = 0;
    for (; i < std::ssize(osc_pattern); i++)
      if (osc_pattern.data()[i] != 0)
        bytes[i] = osc_pattern.data()[i];
      else
        break;
    bytes[i] = 0;
    while (i % 4 != 3)
    {
      ++i;
      bytes[i] = 0;
    }

    bytes[++i] = ',';
    bytes[++i] = 'M';
    bytes[++i] = 0;
    bytes[++i] = 0;

    // MIDI 2 message (M) spec: an UMP
    header_size = ++i;

    return stdx::error{}; // FIXME
  }

  stdx::error deinit()
  {
    header_size = 0;
    return stdx::error{};
  }

  stdx::error set_packet_content(const uint32_t* message, size_t size)
  {
    if (header_size == 0)
      return std::errc::not_connected;

    if (size > 4)
      return std::errc::message_size;

    message_size = 4 * size;

    std::memcpy(this->bytes + header_size, message, message_size);

    return stdx::error{};
  }

  std::span<const char> get_data()
  {
    return std::span(this->bytes, this->header_size + this->message_size);
  }
  char bytes[512 + 8 + 64];
  int header_size{};
  int message_size{};
};

class midi_out final
    : public midi2::out_api
    , public error_handler
{
public:
  struct
      : output_configuration
      , dgram_output_configuration
  {
  } configuration;

  midi_out(output_configuration&& conf, dgram_output_configuration&& apiconf)
      : configuration{std::move(conf), std::move(apiconf)}
      , ctx{configuration.io_context}
      , m_socket{ctx.get()}
  {
    m_socket.open(boost::asio::ip::udp::v4());
    m_socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
    m_socket.set_option(boost::asio::socket_base::broadcast(true));

    m_endpoint
        = {boost::asio::ip::make_address(this->configuration.host),
           static_cast<unsigned short>(this->configuration.port)};

    this->client_open_ = stdx::error{};
  }

  ~midi_out() override { close_port(); }

  libremidi::API get_current_api() const noexcept override { return libremidi::API::NETWORK_UMP; }

  stdx::error open_port(const output_port& /* port */, std::string_view portName) override
  {
    return open_virtual_port(portName);
  }

  stdx::error open_virtual_port(std::string_view portName) override
  {
    // Random arbitrary limit to avoid abuse
    if (portName.size() >= 512)
      return std::errc::invalid_argument;

    pkt.init_packet(portName);

    return stdx::error{};
  }

  stdx::error close_port() override
  {
    pkt.deinit();
    if (m_socket.is_open())
      m_socket.close();
    return {};
  }

  stdx::error send_ump(const uint32_t* message, size_t size) override
  {
    if (auto err = pkt.set_packet_content(message, size); err != stdx::error{})
      return err;

    auto dat = pkt.get_data();
    boost::system::error_code ec;
    m_socket.send_to(boost::asio::const_buffer(dat.data(), dat.size()), m_endpoint, 0, ec);
    return stdx::error{}; // FIXME
  }

  stdx::error schedule_ump(int64_t, const uint32_t*, size_t) override
  {
    int ret = 0;
    return from_errc(ret);
  }

  libremidi::optionally_owned<boost::asio::io_context> ctx;
  boost::asio::ip::udp::endpoint m_endpoint;
  boost::asio::ip::udp::socket m_socket;

  osc_midi2_packet pkt;
};

}
