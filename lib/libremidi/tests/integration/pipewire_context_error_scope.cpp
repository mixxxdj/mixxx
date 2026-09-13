// SPDX-License-Identifier: BSL-1.0
//
// Regression test: pw_core_events::error carries the id of the proxy the error
// belongs to. Only id == PW_ID_CORE describes the connection itself; every
// other id is a per-object error, which the daemon emits routinely (binding a
// global that just disappeared, a stream whose autoconnect target is not up
// yet, an unknown factory name). Treating those as connection loss marked the
// whole context broken, and callers reacted by reconnecting it — which frees
// the loop other holders still use.
//
// Both cases below are per-object -ENOENT and must leave the context connected.
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

static int check_survived(lpw::context& ctx, const char* what)
{
  // The sync round-trip must still complete: the object error does not cancel
  // the core's `done` reply.
  const bool synced = ctx.synchronize();
  const bool broken = ctx.state() == lpw::connection_state::broken;

  if (broken || !ctx.ok() || !synced)
  {
    std::fprintf(
        stderr,
        "FAIL: %s left the context unusable (ok=%d broken=%d synchronize=%d);\n"
        "      a per-object error must not mark the connection broken\n",
        what, (int)ctx.ok(), (int)broken, (int)synced);
    std::fflush(stderr);
    return 1;
  }
  std::printf("ok: %s left the context connected\n", what);
  return 0;
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

  int failures = 0;

  // 1. Binding a global that does not exist. The daemon answers with -ENOENT
  //    on the registry proxy. This is what a real graph race looks like: a
  //    node vanishes between the registry event and the bind.
  {
    auto ctx = lpw::context::make(inst);
    if (!ctx || !ctx->ok())
    {
      std::printf("cannot connect to pipewire daemon; skipping\n");
      return 0;
    }

    ctx->with_lock(
        [&]
        {
          (void)pw_registry_bind(
              ctx->registry(), 0xFFFFFF00u, PW_TYPE_INTERFACE_Node, PW_VERSION_NODE, 0);
        });

    failures += check_survived(*ctx, "pw_registry_bind() of an unknown global");
  }

  // 2. Creating an object from a factory the daemon does not have, which is
  //    the shape of a pw_stream whose autoconnect target is not up yet.
  {
    auto ctx = lpw::context::make(inst);
    if (!ctx || !ctx->ok())
    {
      std::printf("cannot connect to pipewire daemon; skipping\n");
      return 0;
    }

    ctx->with_lock(
        [&]
        {
          (void)pw_core_create_object(
              ctx->pw_core_ptr(), "libremidi-no-such-factory", PW_TYPE_INTERFACE_Node,
              PW_VERSION_NODE, nullptr, 0);
        });

    failures += check_survived(*ctx, "pw_core_create_object() with an unknown factory");
  }

  // 3. The context must keep working afterwards, not just report connected.
  {
    auto ctx = lpw::context::make(inst);
    if (!ctx || !ctx->ok())
      return failures == 0 ? 0 : EXIT_FAILURE;

    ctx->with_lock(
        [&]
        {
          (void)pw_registry_bind(
              ctx->registry(), 0xFFFFFF01u, PW_TYPE_INTERFACE_Node, PW_VERSION_NODE, 0);
        });

    for (int i = 0; i < 10; ++i)
    {
      if (!ctx->synchronize())
      {
        std::fprintf(stderr, "FAIL: synchronize() failed after a per-object error\n");
        std::fflush(stderr);
        ++failures;
        break;
      }
    }
    if (ctx->snapshot().nodes.empty())
    {
      std::fprintf(stderr, "FAIL: registry walk produced no nodes\n");
      std::fflush(stderr);
      ++failures;
    }
  }

  if (failures != 0)
    return EXIT_FAILURE;

  std::printf("PASS: pipewire_context_error_scope\n");
  return 0;
}
