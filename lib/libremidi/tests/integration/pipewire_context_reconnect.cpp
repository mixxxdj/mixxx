// SPDX-License-Identifier: BSL-1.0
//
// Regression test: context::reconnect() must run on the calling thread, not the
// worker. tear_down() stops and destroys the thread loop, so reconnecting via
// invoke_sync() self-destructs the loop from within — a hang and lock corruption.
//
// Calls reconnect() repeatedly; each must return promptly without hanging.
// Requires a running PipeWire daemon; skips (exit 0) if none is reachable.

#include <libremidi/backends/linux/pipewire/context.hpp>
#include <libremidi/backends/linux/pipewire/instance.hpp>
#include <libremidi/backends/linux/pipewire/loader.hpp>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <thread>

namespace lpw = libremidi::pipewire;

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

  auto ctx = lpw::context::make(inst);
  if (!ctx || !ctx->ok())
  {
    std::printf("cannot connect to pipewire daemon; skipping\n");
    return 0;
  }

  arm_watchdog(60);

  // Subscriptions must survive a reconnect, which tears the loop down and
  // rebuilds it from the calling thread.
  auto sub_state = ctx->on_state_changed([](lpw::connection_state) {});

  const std::uint32_t gen0 = ctx->generation();
  for (int i = 0; i < 8; ++i)
  {
    if (!ctx->reconnect())
    {
      std::fprintf(stderr, "FAIL: reconnect() returned false while daemon is up\n");
      return EXIT_FAILURE;
    }
    if (!ctx->ok())
    {
      std::fprintf(stderr, "FAIL: context not connected after reconnect()\n");
      return EXIT_FAILURE;
    }
    // Prove the loop lock still works after the rebuild.
    if (!ctx->synchronize())
    {
      std::fprintf(stderr, "FAIL: synchronize() failed after reconnect()\n");
      return EXIT_FAILURE;
    }
  }

  if (ctx->generation() == gen0)
  {
    std::fprintf(stderr, "FAIL: generation did not advance across reconnects\n");
    return EXIT_FAILURE;
  }

  std::printf("PASS: pipewire_context_reconnect (generation %u -> %u)\n", gen0, ctx->generation());
  return 0;
}
