#pragma once
#define NOMINMAX 1
#define WIN32_LEAN_AND_MEAN 1
#include <libremidi/detail/midi_api.hpp>

#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <unknwn.h>

#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Devices.Midi.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.Streams.h>

NAMESPACE_LIBREMIDI
{
inline void winrt_init()
{
  // init_apartment should only be called on the threads we own.
  // Since we're the library we don't own the threads we are called from,
  // so we should not perform this initialization ourselves.
  // winrt::init_apartment();
}

using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Devices::Midi;
using namespace winrt::Windows::Devices::Enumeration;
using namespace winrt::Windows::Storage::Streams;

// Helper function to allow waiting for aynchronous operation completion
// from the thread in STA. The only benefit from it compared to the
// get() function from winrt is that we avoid an assertion if waiting
// from the STA thread.
template <typename T>
LIBREMIDI_STATIC auto get(T const& async)
{
  if (async.Status() != AsyncStatus::Completed)
  {
    slim_mutex m;
    slim_condition_variable cv;
    bool completed = false;

    async.Completed([&](auto&&, auto&&) {
      {
        slim_lock_guard const guard(m);
        completed = true;
      }

      cv.notify_one();
    });

    slim_lock_guard guard(m);
    cv.wait(m, [&] { return completed; });
  }

  return async.GetResults();
}
}
