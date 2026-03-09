#include "utils.hpp"

#include <libremidi/libremidi.hpp>

#if defined(_WIN32) && __has_include(<winrt/base.h>)
  #include <winrt/base.h>
#endif

#include <boost/asio/detached.hpp>
#include <boost/cobalt.hpp>

namespace asio = boost::asio;
namespace cobalt = boost::cobalt;

// Could go in a libremidi/cobalt.hpp helper
namespace libremidi
{
struct channel
{
  cobalt::channel<libremidi::message>& impl;
  void operator()(const libremidi::message& message)
  {
    cobalt::spawn(impl.get_executor(), task(message), asio::detached);
  }

  cobalt::task<void> task(libremidi::message message) { co_await impl.write(std::move(message)); }
};
}

cobalt::main co_main(int argc, char** argv)
{
#if defined(_WIN32) && __has_include(<winrt/base.h>)
  // Necessary for using WinUWP and WinMIDI, must be done as early as possible in your main()
  winrt::init_apartment();
#endif

  cobalt::channel<libremidi::message> channel_impl{64};
  libremidi::channel channel{channel_impl};
  libremidi::midi_in midiin{{.on_message = channel}};
  midiin.open_port(*libremidi::midi1::in_default_port());

  for (;;)
  {
    auto msg = co_await channel_impl.read();
    std::cerr << msg << "\n";
  }
  co_return 0;
}
