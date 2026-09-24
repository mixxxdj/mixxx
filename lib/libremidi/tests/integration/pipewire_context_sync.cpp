// SPDX-License-Identifier: BSL-1.0
//
// Regression test: synchronize() from a foreign thread must not iterate the
// pw_loop concurrently with the thread-loop worker. Two threads iterating one
// loop corrupt its dispatch bookkeeping and crash in the core-socket callback.
//
// Exercises many create/synchronize/destroy cycles and a tight synchronize()
// loop. Requires a running PipeWire daemon; skips (exit 0) if none is reachable.

#include <libremidi/backends/linux/pipewire/context.hpp>
#include <libremidi/backends/linux/pipewire/instance.hpp>
#include <libremidi/backends/linux/pipewire/loader.hpp>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <thread>

namespace lpw = libremidi::pipewire;

// A deadlock must fail the test rather than hang a CI run.
static void arm_watchdog(int seconds)
{
  std::thread(
      [seconds]
      {
        std::this_thread::sleep_for(std::chrono::seconds(seconds));
        std::fprintf(stderr, "FAIL: watchdog timeout (%ds) - likely deadlock\n", seconds);
        std::fflush(stderr);
        std::_Exit(EXIT_FAILURE);
      })
      .detach();
}

int main()
{
  auto& pw = lpw::load();
  if (!pw.thread_available)
  {
    std::printf("libpipewire thread-loop not available; skipping\n");
    return 0;
  }

  auto inst = lpw::shared_instance();
  if (!inst)
  {
    std::printf("pw_init failed; skipping\n");
    return 0;
  }

  arm_watchdog(60);

  // Pass 1: repeated create/synchronize/destroy. The destructor stops the
  // worker and disconnects the core — the teardown window that used to crash.
  for (int i = 0; i < 20; ++i)
  {
    auto ctx = lpw::context::make(inst);
    if (!ctx || !ctx->ok())
    {
      std::printf("cannot connect to pipewire daemon; skipping\n");
      return 0;
    }

    for (int k = 0; k < 20; ++k)
    {
      if (!ctx->synchronize())
      {
        std::fprintf(stderr, "FAIL: synchronize() failed while connected\n");
        return EXIT_FAILURE;
      }
    }
  }

  // Pass 2: sustained synchronize() round-trips on a single long-lived context.
  {
    auto ctx = lpw::context::make(inst);
    if (!ctx || !ctx->ok())
    {
      std::printf("cannot connect to pipewire daemon; skipping\n");
      return 0;
    }

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
    long count = 0;
    while (std::chrono::steady_clock::now() < deadline)
    {
      if (!ctx->synchronize())
      {
        std::fprintf(stderr, "FAIL: synchronize() failed while connected\n");
        return EXIT_FAILURE;
      }
      ++count;
    }
    std::printf("completed %ld synchronize() round-trips\n", count);
  }

  std::printf("PASS: pipewire_context_sync\n");
  return 0;
}
