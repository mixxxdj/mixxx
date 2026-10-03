#pragma once
#include <libremidi/backends/net/config.hpp>
#include <libremidi/backends/net/helpers.hpp>
#include <libremidi/detail/midi_in.hpp>
#include <libremidi/detail/midi_stream_decoder.hpp>

#include <thread>
NAMESPACE_LIBREMIDI
{
template <typename Impl, typename F>
struct osc_parser
{
  [[no_unique_address]]
  Impl impl;
  F on_message;
  std::string_view port_name;

  stdx::error parse_bundle(const char* data, std::size_t sz)
  {
    std::string_view txt(data, sz);
    if (!txt.starts_with("#bundle"))
      return std::errc::bad_message;
    return std::errc::protocol_not_supported;
  }

  stdx::error parse_message(const char* data, std::size_t sz)
  {
    const auto begin = data;
    const auto end = data + sz;
    std::size_t pattern_len = strnlen(data, sz);
    if (pattern_len == sz)
      return std::errc::bad_message;

    if (auto pat = std::string_view(data, pattern_len); pat != port_name)
      return std::errc::bad_address;

    data += pattern_len;
    for (; data < end; ++data)
      if (*data == 0)
        continue;
      else
        break;

    // Now we shold reach the typetag beginning, ","
    if (data == end)
      return std::errc::bad_message;
    if ((data - begin) % 4 != 0)
      return std::errc::bad_message;
    if (*data++ != ',')
      return std::errc::bad_message;

    // Count the ,mmmmm arguments... yummy
    int num_msgs = 0;
    for (; data < end; ++data)
      if (*data == Impl::typetag)
        num_msgs++;
      else if (*data == 0)
        break;
      else
        return std::errc::bad_message;

    if (num_msgs == 0)
      return std::errc::no_message;

    // By now there's at least 5 bytes left as the ,mmm has to be round-up to 4 and padded with zeros:
    if (end - data < 4)
      return std::errc::bad_message;

    switch ((1 + num_msgs) % 4)
    {
      case 0:
        data += 4;
        break;
      case 1:
        data += 3;
        break;
      case 2:
        data += 2;
        break;
      case 3:
        data += 1;
        break;
    }

    // Data starts
    return impl.process_midi_bytes(on_message, num_msgs, data, end - data);
  }

  stdx::error parse_int_message(const char* /* data */, std::size_t /* sz */)
  {
    return std::errc::protocol_not_supported;
  }

  stdx::error parse_packet(const char* data, std::size_t sz)
  {
    if (sz == 0)
      return std::errc::no_message;

    switch (data[0])
    {
      case '#':
        return parse_bundle(data, sz);
        break;
      case '/':
        return parse_message(data, sz);
        break;
      case '\0':
        return parse_int_message(data, sz);
        break;
      default:
        return std::errc::bad_message;
    }
  }
};

}

NAMESPACE_LIBREMIDI::net
{

struct osc_parser_midi1
{
  static constexpr char typetag = 'm';

  stdx::error
  process_midi_bytes(auto& on_message, std::size_t num_msgs, const char* data, std::size_t sz)
  {
    if (sz != num_msgs * 4)
      return std::errc::bad_message;

    auto end = data + sz;
    for (; data < end; data += 4)
      on_message(data + 1);

    return {};
  }
};

class midi_in final
    : public midi1::in_api
    , public error_handler
{
public:
  using midi_api::client_open_;
  struct
      : input_configuration
      , dgram_input_configuration
  {
  } configuration;

  explicit midi_in(input_configuration&& conf, dgram_input_configuration&& apiconf)
      : configuration{std::move(conf), std::move(apiconf)}
      , m_ctx{configuration.io_context}
      , m_socket{m_ctx.get()}
  {
    m_socket.open(boost::asio::ip::udp::v4());
    m_socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));

    m_endpoint
        = {boost::asio::ip::make_address(this->configuration.accept),
           static_cast<unsigned short>(this->configuration.port)};

    boost::system::error_code ec;
    m_socket.bind(m_endpoint, ec);
    if (ec != boost::system::error_code{})
    {
      client_open_ = std::errc::address_in_use;
      return;
    }

    client_open_ = stdx::error{};
  }

  ~midi_in() override { close_port(); }

  libremidi::API get_current_api() const noexcept override { return libremidi::API::NETWORK; }

  stdx::error open_port(const input_port&, std::string_view port) override
  {
    return open_virtual_port(port);
  }

  stdx::error open_virtual_port(std::string_view port) override
  {
    m_portname = std::string(port);

    receive();

    if (m_ctx.is_owned())
    {
      m_thread = std::jthread{[this, &ctx = m_ctx.get()] {
        auto wg = boost::asio::make_work_guard(m_ctx);
        ctx.run();
      }};
    }

    return stdx::error{};
  }

  void receive()
  {
    m_socket.async_receive_from(
        boost::asio::mutable_buffer(&m_data[0], std::size(m_data)), m_endpoint,
        [this](auto ec, std::size_t sz) {
      if (ec == boost::asio::error::operation_aborted)
        return;

      if (!ec && sz > 0)
        this->on_bytes(reinterpret_cast<const char*>(m_data), sz);

      this->receive();
    });
  }

  stdx::error close_port() override
  {
    // FIXME async close
    if (m_socket.is_open())
      m_socket.close();
    return {};
  }

  timestamp absolute_timestamp() const noexcept override
  {
    return std::chrono::steady_clock::now().time_since_epoch().count();
  }

  void on_bytes(const char* data, std::size_t size)
  {
    const auto on_msg = [this](const char* bytes) { this->on_message(bytes); };
    osc_parser<osc_parser_midi1, decltype(on_msg)> parser{{}, on_msg, m_portname};
    parser.parse_packet(data, size);
  }

  void on_message(const char* data)
  {
    // OSC enforces always 3 bytes
    static constexpr timestamp_backend_info timestamp_info{
        .has_absolute_timestamps = false,
        .absolute_is_monotonic = false,
        .has_samples = false,
    };

    static constexpr auto to_ns = []() {
      return std::chrono::duration_cast<std::chrono::nanoseconds>(
                 std::chrono::steady_clock::now().time_since_epoch())
          .count();
    };

    auto ptr = reinterpret_cast<const uint8_t*>(data);
    m_processing.on_bytes({ptr, ptr + 3}, m_processing.timestamp<timestamp_info>(to_ns, 0));
  }

  midi1::input_state_machine m_processing{this->configuration};

  libremidi::optionally_owned<boost::asio::io_context> m_ctx;
  boost::asio::ip::udp::endpoint m_endpoint;
  boost::asio::ip::udp::socket m_socket;

  std::jthread m_thread;

  std::string m_portname;
  alignas(16) unsigned char m_data[65535];
};
}

NAMESPACE_LIBREMIDI::net_ump
{
struct osc_parser_midi2
{
  static constexpr char typetag = 'M';

  stdx::error
  process_midi_bytes(auto& on_message, std::size_t num_msgs, const char* data, std::size_t byte_sz)
  {
    if (byte_sz % 4 != 0)
      return std::errc::bad_message;

    auto sz = byte_sz / 4;

    auto begin = reinterpret_cast<const uint32_t*>(data);
    auto end = begin + sz;
    std::size_t accounted = 0;
    for (auto it = begin; it < end && accounted < num_msgs;)
    {
      const auto N = cmidi2_ump_get_message_size_bytes(it) / 4;
      switch (N)
      {
        case 1:
        case 2:
        case 4:
          if (it + N <= end)
          {
            on_message(it, N);
            accounted++;
          }
          else
            return std::errc::bad_message;
          break;
        default:
          return std::errc::bad_message;
      }
      it += N;
    }

    return {};
  }
};

class midi_in final
    : public midi2::in_api
    , public error_handler
{
public:
  using midi_api::client_open_;
  struct
      : ump_input_configuration
      , dgram_input_configuration
  {
  } configuration;

  explicit midi_in(ump_input_configuration&& conf, dgram_input_configuration&& apiconf)
      : configuration{std::move(conf), std::move(apiconf)}
      , m_ctx{configuration.io_context}
      , m_socket{m_ctx.get()}
  {
    m_socket.open(boost::asio::ip::udp::v4());
    m_socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));

    m_endpoint
        = {boost::asio::ip::make_address(this->configuration.accept),
           static_cast<unsigned short>(this->configuration.port)};

    boost::system::error_code ec;
    m_socket.bind(m_endpoint, ec);
    if (ec != boost::system::error_code{})
    {
      client_open_ = std::errc::address_in_use;
      return;
    }

    client_open_ = stdx::error{};
  }

  ~midi_in() override { close_port(); }

  libremidi::API get_current_api() const noexcept override { return libremidi::API::NETWORK_UMP; }

  stdx::error open_port(const input_port&, std::string_view port) override
  {
    return open_virtual_port(port);
  }

  stdx::error open_virtual_port(std::string_view port) override
  {
    m_portname = std::string(port);

    receive();

    if (m_ctx.is_owned())
    {
      m_thread = std::jthread{[this, &ctx = m_ctx.get()] {
        auto wg = boost::asio::make_work_guard(m_ctx);
        ctx.run();
      }};
    }
    return stdx::error{};
  }

  void receive()
  {
    m_socket.async_receive_from(
        boost::asio::mutable_buffer(&m_data[0], std::size(m_data)), m_endpoint,
        [this](auto ec, std::size_t sz) {
      if (ec == boost::asio::error::operation_aborted)
        return;

      if (!ec && sz > 0)
        this->on_bytes(reinterpret_cast<const char*>(m_data), sz);

      this->receive();
    });
  }

  stdx::error close_port() override
  {
    // FIXME async close
    if (m_ctx.is_owned())
      m_ctx.get().stop();

    if (m_socket.is_open())
      m_socket.close();
    return {};
  }

  timestamp absolute_timestamp() const noexcept override
  {
    return std::chrono::steady_clock::now().time_since_epoch().count();
  }

  void on_bytes(const char* data, std::size_t size)
  {
    const auto on_msg
        = [this](const uint32_t* bytes, std::size_t N) { this->on_message(bytes, N); };
    osc_parser<osc_parser_midi2, decltype(on_msg)> parser{{}, on_msg, m_portname};
    parser.parse_packet(data, size);
  }

  void on_message(const uint32_t* ump, std::size_t sz)
  {
    static constexpr timestamp_backend_info timestamp_info{
        .has_absolute_timestamps = false,
        .absolute_is_monotonic = false,
        .has_samples = false,
    };

    static constexpr auto to_ns = []() {
      return std::chrono::duration_cast<std::chrono::nanoseconds>(
                 std::chrono::steady_clock::now().time_since_epoch())
          .count();
    };

    m_processing.on_bytes({ump, sz}, m_processing.timestamp<timestamp_info>(to_ns, 0));
  }

  midi2::input_state_machine m_processing{this->configuration};

  libremidi::optionally_owned<boost::asio::io_context> m_ctx;
  boost::asio::ip::udp::endpoint m_endpoint;
  boost::asio::ip::udp::socket m_socket;

  std::jthread m_thread;

  std::string m_portname;
  alignas(16) unsigned char m_data[65535];
};
}
