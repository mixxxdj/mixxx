//*****************************************//
//  qmidiin.cpp
//  by Gary Scavone, 2003-2004.
//
//  Simple program to test MIDI input and
//  retrieval from the queue.
//
//*****************************************//

#include "utils.hpp"

#include <libremidi/libremidi.hpp>

#if defined(_WIN32) && __has_include(<winrt/base.h>)
  #include <winrt/base.h>
#endif

#include <atomic>
#include <cassert>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <thread>

namespace libremidi
{
// Example implementation of a queue
// NOTE: do not use, instead use a proper lock-free queue!
// e.g. https://github.com/max0x7ba/atomic_queue or
// https://github.com/cameron314/readerwriterqueue
struct basic_queue
{
  unsigned int front{};
  unsigned int back{};
  unsigned int ringSize{};
  std::unique_ptr<message[]> ring{};

  bool push(const message& msg)
  {
    auto [sz, _, b] = get_dimensions();

    if (sz < ringSize - 1)
    {
      ring[b] = msg;
      back = (back + 1) % ringSize;
      return true;
    }

    return false;
  }

  bool pop(message& msg)
  {
    auto [sz, f, _] = get_dimensions();

    if (sz == 0)
    {
      return false;
    }

    // Copy queued message to the vector pointer argument and then "pop" it.
    using namespace std;
    swap(msg, ring[f]);

    // Update front
    front = (front + 1) % ringSize;
    return true;
  }

  struct dimensions
  {
    unsigned int size, front, back;
  };

  dimensions get_dimensions() const
  {
    // Access back/front members exactly once and make stack copies for
    // size calculation ==> completely unneccessary
    // https://godbolt.org/g/HPu9LA

    return {(back >= front) ? back - front : ringSize - front + back, front, back};
  }
};

// Example of how to get back the old queue-based API
struct queued_midi_in
    : basic_queue
    , libremidi::midi_in
{
  explicit queued_midi_in(
      unsigned int queueSizeLimit, libremidi::input_configuration conf, auto&&... args)
      : midi_in{set_queue_callback(queueSizeLimit, conf), args...}
  {
  }

  //! Fill the user-provided vector with the data bytes for the next available
  //! MIDI message in the input queue and return the event delta-time in
  //! seconds.
  /*!
    This function returns immediately whether a new message is
    available or not.  A valid message is indicated by a non-zero
    vector size.  An exception is thrown if an error occurs during
    message retrieval or an input connection was not previously
    established.
  */
  message get_message()
  {
    auto& queue = static_cast<basic_queue&>(*this);

    message m;
    if (queue.pop(m))
    {
      return m;
    }
    return {};
  }

  bool get_message(message& m)
  {
    auto& queue = static_cast<basic_queue&>(*this);
    return queue.pop(m);
  }

private:
  libremidi::input_configuration&
  set_queue_callback(unsigned int queueSizeLimit, libremidi::input_configuration& conf)
  {
    auto& queue = static_cast<basic_queue&>(*this);
    // Allocate the MIDI queue.
    queue.ringSize = queueSizeLimit;
    if (queue.ringSize > 0)
    {
      queue.ring = std::make_unique<libremidi::message[]>(queue.ringSize);
    }

    conf.on_message = [this](libremidi::message m) {
      // As long as we haven't reached our queue size limit, push the
      // message.

      auto& queue = static_cast<basic_queue&>(*this);
      assert(queue.ring);
      if (!queue.push(std::move(m)))
      {
#if defined(__LIBREMIDI_DEBUG__)
        std::cerr << "\nmidi_in: message queue limit reached!!\n\n";
#endif
      }
    };
    return conf;
  }
};
}

int main(int argc, const char** argv)
{
#if defined(_WIN32) && __has_include(<winrt/base.h>)
  // Necessary for using WinUWP and WinMIDI, must be done as early as possible in your main()
  winrt::init_apartment();
#endif

  using namespace std::literals;
  // Read command line arguments
  libremidi::examples::arguments args{argc, argv};

  libremidi::observer obs;
  auto ports = obs.get_input_ports();

  libremidi::queued_midi_in midiin{
      1024,
      libremidi::input_configuration{
          // Don't ignore sysex, timing, or active sensing messages.
          .ignore_sysex = false,
          .ignore_timing = false,
          .ignore_sensing = false,
      },
      libremidi::midi_in_configuration_for(obs)};

  if (!args.open_port(midiin))
    return 1;

  // Install an interrupt handler function.
  static std::atomic_bool done{};
  signal(SIGINT, [](int) { done = true; });

  // Periodically check input queue.
  std::cout << "Reading MIDI from port " << ports[args.input_port].display_name
            << " ... quit with Ctrl-C.\n";
  while (!done)
  {
    for (;;)
    {
      auto msg = midiin.get_message();
      if (msg.empty())
        break;
      std::cout << msg << std::endl;
    }

    std::this_thread::sleep_for(10ms);
  }

  return 0;
}
