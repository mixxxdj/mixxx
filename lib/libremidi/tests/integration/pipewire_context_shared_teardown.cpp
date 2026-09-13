// SPDX-License-Identifier: BSL-1.0
//
// Regression test: shared_context() hands the same context to independent
// subsystems (MIDI backends, an audio engine, video devices). reconnect()
// destroys the pw_core, pw_context and pw_thread_loop that connection is built
// on and builds new ones, so any pw_stream another holder created keeps
// pointing at the freed loop — pw_stream_destroy then faults on it.
//
// A context with more than one shared holder must therefore refuse to be torn
// down, and must keep serving the handles its holders already took. A sole
// holder may still reconnect, and gets on_invalidated() first so it can drop
// what it built while the loop is still alive.
//
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

static int fail(const char* msg)
{
  std::fprintf(stderr, "FAIL: %s\n", msg);
  std::fflush(stderr);
  return 1;
}

int main()
{
  auto& pw = lpw::load();
  if (!pw.thread_available || !pw.stream_available)
  {
    std::printf("libpipewire thread-loop/stream not available; skipping\n");
    return 0;
  }

  // Holder A stands in for one subsystem (say the audio engine), holder B for
  // another (a video device) — each acquires the process-wide connection
  // without knowing the other exists.
  auto a = lpw::shared_context();
  if (!a || !a->ok())
  {
    std::printf("cannot connect to pipewire daemon; skipping\n");
    return 0;
  }
  auto b = lpw::shared_context();
  if (!b)
    return fail("shared_context() returned nothing on the second call");
  if (a.get() != b.get())
    return fail("shared_context() did not return the same context twice");

  arm_watchdog(60);

  int failures = 0;

  if (a->shared_holders() != 2)
  {
    std::fprintf(
        stderr, "FAIL: expected 2 shared holders, got %zu\n", a->shared_holders());
    std::fflush(stderr);
    ++failures;
  }

  // B builds a pw_stream on the shared core, exactly as the video devices do.
  pw_stream* stream = nullptr;
  auto* const loop_before = b->thread_loop_handle();
  auto* const core_before = b->pw_core_ptr();

  b->with_lock(
      [&]
      {
        auto* props = pw.properties_new(
            PW_KEY_MEDIA_TYPE, "Video", PW_KEY_MEDIA_CATEGORY, "Source", nullptr);
        stream = pw.stream_new(b->pw_core_ptr(), "libremidi-shared-holder", props);
      });
  if (!stream)
    return fail("could not create a pw_stream on the shared core");

  // A now decides the connection needs rebuilding. B's stream lives on the
  // loop and core tear_down() would free, and A has no way to know that.
  const bool reconnected = a->reconnect();
  if (reconnected)
    failures += fail(
        "reconnect() tore down a context still held by another client;\n"
        "      every pw_stream that holder built now points at a freed loop");

  if (b->thread_loop_handle() != loop_before)
    failures += fail("the thread loop another holder is using was replaced");
  if (b->pw_core_ptr() != core_before)
    failures += fail("the core another holder is using was replaced");
  if (!b->ok())
    failures += fail("the shared context stopped being usable");

  // Must not fault: this is the pw_stream_destroy that crashed in the field.
  b->with_lock([&] { pw.stream_destroy(stream); });
  stream = nullptr;
  std::printf("ok: pw_stream_destroy() survived a concurrent reconnect() attempt\n");

  if (!b->synchronize())
    failures += fail("the shared context stopped synchronizing");

  // Sole holder: reconnect is allowed again, and must announce itself first.
  b.reset();
  if (a->shared_holders() != 1)
  {
    std::fprintf(
        stderr, "FAIL: expected 1 shared holder after release, got %zu\n",
        a->shared_holders());
    std::fflush(stderr);
    ++failures;
  }

  bool invalidated = false;
  bool handles_live_in_callback = false;
  auto sub = a->on_invalidated(
      [&]
      {
        invalidated = true;
        // The loop and core must still be alive here — that is the whole
        // point of the callback: it is when a holder can still drop what it
        // built on them.
        handles_live_in_callback
            = a->thread_loop_handle() != nullptr && a->pw_core_ptr() != nullptr;
      });

  if (!a->reconnect())
    failures += fail("sole-holder reconnect() was refused");
  if (!invalidated)
    failures += fail("on_invalidated() did not fire before teardown");
  if (!handles_live_in_callback)
    failures += fail("handles were already dead when on_invalidated() fired");
  if (!a->ok() || !a->synchronize())
    failures += fail("context is not usable after a sole-holder reconnect");

  if (failures != 0)
    return EXIT_FAILURE;

  std::printf("PASS: pipewire_context_shared_teardown\n");
  return 0;
}
