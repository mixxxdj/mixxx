// SPDX-License-Identifier: BSL-1.0
//
// Regression test for the recursive thread-loop lock. Two paths used to corrupt
// it: unsubscribe() invoked the loop without the proper lock state, and
// synchronize() could re-enter the recursive lock across its wait.
//
// Drops many subscriptions, tears down a MIDI-style filter with live
// subscriptions, and calls synchronize() from a subscriber callback (on the
// loop thread). Must complete without crashing or hanging.
//
// Requires a running PipeWire daemon; skips (exit 0) if none is reachable.

#include <libremidi/backends/linux/pipewire/context.hpp>
#include <libremidi/backends/linux/pipewire/filter.hpp>
#include <libremidi/backends/linux/pipewire/instance.hpp>
#include <libremidi/backends/linux/pipewire/loader.hpp>
#include <libremidi/backends/linux/pipewire/types.hpp>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <memory>
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
  if (!pw.thread_available || !pw.filter_available)
  {
    std::printf("libpipewire thread-loop/filter not available; skipping\n");
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

  // 1. Subscribe/unsubscribe churn: each subscription reset runs unsubscribe()
  //    through invoke_sync() on the loop thread.
  for (int i = 0; i < 50; ++i)
  {
    auto s1 = ctx->on_port_added([](const lpw::port_info&) {});
    auto s2 = ctx->on_port_removed([](std::uint32_t) {});
    auto s3 = ctx->on_node_added([](const lpw::node_info&) {});
    auto s4 = ctx->on_node_removed([](std::uint32_t) {});
  }

  // 2. A MIDI-style filter with a real port, torn down while port/node
  //    subscriptions are live. filter::stop() does disconnect + synchronize +
  //    destroy under the loop lock.
  {
    auto sub_add = ctx->on_port_added([](const lpw::port_info&) {});
    auto sub_rm = ctx->on_port_removed([](std::uint32_t) {});

    static constexpr pw_filter_events events = {
        .version = PW_VERSION_FILTER_EVENTS,
        .destroy = nullptr,
        .state_changed = nullptr,
        .io_changed = nullptr,
        .param_changed = nullptr,
        .add_buffer = nullptr,
        .remove_buffer = nullptr,
        .process = nullptr,
        .drained = nullptr,
#if PW_VERSION_FILTER_EVENTS >= 1
        .command = nullptr,
#endif
    };
    lpw::filter_config cfg;
    cfg.name = "libremidi-regression";
    cfg.media_type = "Midi";
    cfg.media_category = "Filter";
    cfg.media_role = "DSP";
    cfg.format_dsp = "8 bit raw midi";
    cfg.always_process = true;
    cfg.pause_on_idle = false;
    cfg.suspend_on_idle = false;
    cfg.rt_process = true;

    auto flt = std::make_unique<lpw::filter>(ctx, cfg, events, nullptr);
    if (flt->ok())
    {
      flt->start();
      auto port = flt->add_port(SPA_DIRECTION_INPUT, "in", "8 bit raw midi");
      if (!port.valid())
      {
        std::fprintf(stderr, "FAIL: could not add filter port\n");
        return EXIT_FAILURE;
      }
      flt->synchronize_node();
    }
    flt.reset(); // filter::stop()
  }

  // 3. synchronize() from a subscriber callback (on the loop thread): the
  //    is_in_loop_thread() guard must prevent it waiting on itself. Triggered
  //    via reconnect(), whose registry re-walk dispatches node_added on the
  //    worker thread.
  {
    std::atomic<int> loop_thread_syncs{0};
    auto sub = ctx->on_node_added(
        [&](const lpw::node_info&)
        {
          if (loop_thread_syncs.fetch_add(1) == 0)
            (void)ctx->synchronize();
        });

    if (!ctx->reconnect() || !ctx->ok())
    {
      std::fprintf(stderr, "FAIL: reconnect() failed\n");
      return EXIT_FAILURE;
    }
    for (int i = 0; i < 5; ++i)
      (void)ctx->synchronize();
    std::printf("synchronize() from loop thread fired %d time(s)\n", loop_thread_syncs.load());
  }

  std::printf("PASS: pipewire_context_subscriptions\n");
  return 0;
}
