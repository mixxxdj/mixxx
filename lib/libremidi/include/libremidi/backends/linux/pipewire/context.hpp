#pragma once

#if defined(__clang__)
  #pragma clang diagnostic push
  // pw_loop_invoke macro expands to spa_callbacks_call_res which
  // compares uint32_t version against int literal 0.
  #pragma clang diagnostic ignored "-Wsign-compare"
#endif

#include <libremidi/backends/linux/pipewire/instance.hpp>
#include <libremidi/backends/linux/pipewire/loader.hpp>
#include <libremidi/backends/linux/pipewire/subscription.hpp>
#include <libremidi/backends/linux/pipewire/types.hpp>
#include <libremidi/config.hpp>

#include <pipewire/core.h>
#include <pipewire/keys.h>
#include <pipewire/loop.h>
#include <pipewire/node.h>
#include <pipewire/port.h>
#include <pipewire/proxy.h>
#include <spa/utils/dict.h>
#include <spa/utils/hook.h>
#include <spa/utils/result.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include <pipewire/extensions/metadata.h>
#include <pipewire/main-loop.h>
#include <pipewire/thread-loop.h>

namespace libremidi::pipewire
{

enum class loop_kind : std::uint8_t
{
  thread,
  main,
};

struct context_config
{
  loop_kind kind{loop_kind::thread};
  const char* loop_name{"libremidi-pw"};

  std::chrono::milliseconds sync_deadline{std::chrono::seconds{2}};

  // Reconnect path stops the thread loop from inside that thread
  // — UB per pipewire docs. Off until reworked.
  bool auto_reconnect{false};

  std::chrono::milliseconds reconnect_backoff{std::chrono::seconds{1}};
  std::chrono::milliseconds reconnect_backoff_max{std::chrono::seconds{30}};

  int fd{-1};

  // Non-null pointers here are adopted without ownership; prefer the
  // `borrow()` factories rather than setting these directly.
  pw_thread_loop* borrow_thread_loop{};
  pw_main_loop* borrow_main_loop{};
  pw_core* borrow_core{};
};

class context : public std::enable_shared_from_this<context>
{
public:
  using config = context_config;

  context(std::shared_ptr<instance> inst, config cfg = {})
      : m_instance{std::move(inst)}
      , m_cfg{cfg}
  {
    if (m_cfg.borrow_thread_loop)
    {
      m_cfg.kind = loop_kind::thread;
      m_owns_loop = false;
    }
    else if (m_cfg.borrow_main_loop)
    {
      m_cfg.kind = loop_kind::main;
      m_owns_loop = false;
    }
    if (m_cfg.borrow_core)
      m_owns_core = false;

    if (!m_instance || !m_instance->initialized())
    {
      m_state.store(connection_state::broken, std::memory_order_release);
      return;
    }

    if (!build_connection())
      m_state.store(connection_state::broken, std::memory_order_release);

    if (m_cfg.auto_reconnect)
      start_supervisor();
  }

  ~context()
  {
    stop_supervisor();
    tear_down(true);
  }

  context(const context&) = delete;
  context& operator=(const context&) = delete;
  context(context&&) = delete;
  context& operator=(context&&) = delete;

  static std::shared_ptr<context> make(std::shared_ptr<instance> inst, config cfg = {})
  {
    auto p = std::make_shared<context>(std::move(inst), cfg);
    if (p->state() != connection_state::broken)
    {
      // Pass 1 drives the daemon's initial registry dump (binds the
      // proxies + installs listeners); pass 2 drains the resulting
      // info events so port.physical etc. is populated before return.
      (void)p->synchronize();
      (void)p->synchronize();
    }
    return p;
  }

  static std::shared_ptr<context>
  borrow(pw_thread_loop* tl, pw_core* core, config cfg = {})
  {
    if (!tl || !core)
      return {};
    cfg.borrow_thread_loop = tl;
    cfg.borrow_core = core;
    auto inst = shared_instance();
    if (!inst)
      return {};
    auto p = std::make_shared<context>(std::move(inst), cfg);
    if (p->state() != connection_state::broken)
    {
      (void)p->synchronize();
      (void)p->synchronize();
    }
    return p;
  }

  static std::shared_ptr<context>
  borrow(pw_main_loop* ml, pw_core* core, config cfg = {})
  {
    if (!ml || !core)
      return {};
    cfg.borrow_main_loop = ml;
    cfg.borrow_core = core;
    auto inst = shared_instance();
    if (!inst)
      return {};
    auto p = std::make_shared<context>(std::move(inst), cfg);
    if (p->state() != connection_state::broken)
    {
      (void)p->synchronize();
      (void)p->synchronize();
    }
    return p;
  }

  bool ok() const noexcept { return state() == connection_state::connected; }

  connection_state state() const noexcept { return m_state.load(std::memory_order_acquire); }

  std::uint32_t generation() const noexcept
  {
    return m_generation.load(std::memory_order_acquire);
  }

  bool synchronize() { return synchronize(m_cfg.sync_deadline); }

  bool synchronize(std::chrono::milliseconds deadline)
  {
    if (state() == connection_state::broken)
      return false;

    auto& pw = load();
    if (!pw.core_available || !m_core || !m_loop)
      return false;

    using clk = std::chrono::steady_clock;
    const auto t_end = clk::now() + deadline;

    if (m_cfg.kind == loop_kind::thread && !is_in_loop_thread())
    {
      return do_sync_thread(t_end);
    }
    else
    {
      // Main-loop mode, or already on the thread-loop worker: we own the loop
      // here, so drive iteration directly. Blocking from the worker would
      // starve the loop, and re-locking pipewire's recursive mutex across a
      // condition wait corrupts its accounting ('recurse > 0' in do_unlock).
      return do_sync_iterate(t_end);
    }
  }

  // Number of independent shared_context() holders currently alive. Copies of
  // one holder's pointer count once; contexts built directly with make() or
  // borrow() report 0.
  std::size_t shared_holders() const noexcept
  {
    std::lock_guard lk{m_holders_mtx};
    return m_shared_holders;
  }

  // Wraps `self` in a pointer that also counts as one shared holder. Used by
  // shared_context(); copies of the result share the same holder slot.
  static std::shared_ptr<context> make_shared_holder(std::shared_ptr<context> self)
  {
    if (!self)
      return {};
    auto* raw = self.get();
    auto* tok = new holder_token{std::move(self)};
    std::shared_ptr<holder_token> owner{tok, [](holder_token* t) { delete t; }};
    return std::shared_ptr<context>{std::move(owner), raw};
  }

  bool reconnect()
  {
    // tear_down() stops and destroys the thread loop, so reconnect MUST run
    // on a thread other than the worker — never via invoke_sync. Doing it on
    // the loop thread self-destructs the loop: pthread_join(self) is a silent
    // no-op and pw_thread_loop_destroy then corrupts the lock it runs under
    // ('recurse > 0' failure) and the invoke never completes (hang).
    if (is_in_loop_thread())
      return false;

    // Held across the rebuild so a holder arriving mid-reconnect waits and
    // then observes the new handles rather than the ones being freed.
    std::lock_guard holders_lk{m_holders_mtx};

    // tear_down() frees the loop, core and context this connection is built
    // on. Other shared_context() holders may own pw_streams and proxies bound
    // to them, and the rebuild cannot migrate those, so a context that is
    // shared is never torn down on one holder's behalf.
    if (m_shared_holders > 1)
      return false;

    dispatch_invalidated();

    tear_down(/*final=*/false);
    m_state.store(connection_state::connecting, std::memory_order_release);
    if (!build_connection())
    {
      m_state.store(connection_state::broken, std::memory_order_release);
      return false;
    }
    if (!synchronize())
      return false;
    m_state.store(connection_state::connected, std::memory_order_release);
    m_generation.fetch_add(1, std::memory_order_release);
    dispatch_state(connection_state::connected);
    return true;
  }
  bool is_in_loop_thread() const noexcept
  {
    if (m_cfg.kind != loop_kind::thread || !m_thread_loop)
      return false;
    auto& pw = load();
    return pw.thread_loop_in_thread && pw.thread_loop_in_thread(m_thread_loop);
  }

  template <typename F>
  void with_lock(F&& fn)
  {
    if (m_cfg.kind != loop_kind::thread || !m_thread_loop || is_in_loop_thread())
    {
      std::forward<F>(fn)();
      return;
    }
    auto& pw = load();
    pw.thread_loop_lock(m_thread_loop);
    struct unlock_guard
    {
      const api& pw;
      pw_thread_loop* loop;
      ~unlock_guard() { pw.thread_loop_unlock(loop); }
    } guard{pw, m_thread_loop};
    std::forward<F>(fn)();
  }

  template <typename F>
  void invoke_async(F&& fn)
  {
    if (!m_loop)
      return;
    if (is_in_loop_thread())
    {
      std::forward<F>(fn)();
      return;
    }
    using FR = std::remove_cvref_t<F>;
    auto* held = new FR{std::forward<F>(fn)};
    constexpr auto trampoline
        = +[](spa_loop* /*l*/, bool /*async*/, std::uint32_t /*seq*/, const void* /*data*/,
              size_t /*size*/, void* user_data) noexcept -> int {
      auto* p = static_cast<FR*>(user_data);
      try
      {
        (*p)();
      }
      catch (...)
      {
      }
      delete p;
      return 0;
    };
    pw_loop_invoke(m_loop, trampoline, 0, nullptr, 0, /*block=*/false, held);
  }

  // Runs fn on the loop thread and waits for the result. We queue a
  // NON-blocking pw_loop_invoke rather than block=true because the blocking
  // path's locking contract is not stable across PipeWire versions: some
  // releases fire the thread-loop control hooks around the wait (caller must
  // hold the lock exactly once), the 1.3.0-1.3.80 and 1.5.0-1.5.80 dev series
  // don't (holding the lock deadlocks). A non-blocking invoke fires no hooks
  // and needs no lock on any version; we wait on our own condition variable,
  // bounded by sync_deadline so a teardown race returns instead of hanging.
  //
  // The payload is shared_ptr-owned by both sides. On timeout the caller sets
  // `abandoned` under the payload mutex; the trampoline checks it under the
  // same mutex, so fn (which captures the caller's frame by reference) either
  // completes before the timeout or is skipped — never run against a dead frame.
  template <typename F>
  auto invoke_sync(F&& fn) -> std::invoke_result_t<std::remove_cvref_t<F>&>
  {
    using FR = std::remove_cvref_t<F>;
    using R = std::invoke_result_t<FR&>;
    if (!m_loop || !m_thread_loop || is_in_loop_thread()
        || m_cfg.kind != loop_kind::thread)
    {
      return std::forward<F>(fn)();
    }

    struct payload
    {
      FR fn;
      std::mutex m;
      std::condition_variable cv;
      bool done{false};
      bool abandoned{false};
      std::conditional_t<std::is_void_v<R>, char, std::optional<R>> result{};

      explicit payload(FR&& f)
          : fn{std::move(f)}
      {
      }
    };

    auto sp = std::make_shared<payload>(FR{std::forward<F>(fn)});
    constexpr auto trampoline = +[](spa_loop*, bool, std::uint32_t, const void*,
                                    size_t, void* user_data) noexcept -> int {
      auto* spp = static_cast<std::shared_ptr<payload>*>(user_data);
      auto& pl = **spp;
      {
        std::lock_guard lk{pl.m};
        if (!pl.abandoned)
        {
          try
          {
            if constexpr (std::is_void_v<R>)
              pl.fn();
            else
              pl.result.emplace(pl.fn());
          }
          catch (...)
          {
          }
          pl.done = true;
        }
      }
      pl.cv.notify_all();
      delete spp;
      return 0;
    };

    auto* held = new std::shared_ptr<payload>(sp);
    if (pw_loop_invoke(m_loop, trampoline, 0, nullptr, 0, /*block=*/false, held) < 0)
    {
      // Loop rejected the item (teardown/OOM) and will never run it: safe inline.
      delete held;
      return sp->fn();
    }

    std::unique_lock lk{sp->m};
    if (!sp->cv.wait_for(lk, m_cfg.sync_deadline, [&] { return sp->done; }))
    {
      // Timed out: abandon the item; the trampoline skips fn if it ever runs.
      sp->abandoned = true;
      if constexpr (std::is_void_v<R>)
        return;
      else if constexpr (std::is_default_constructible_v<R>)
        return R{};
      else
        throw std::runtime_error("libremidi: pipewire invoke_sync timed out");
    }
    if constexpr (!std::is_void_v<R>)
      return std::move(*sp->result);
  }

  graph_snapshot snapshot() const
  {
    graph_snapshot out;
    std::lock_guard lock{m_graph_mtx};
    out.nodes.reserve(m_nodes.size());
    for (const auto& [id, n] : m_nodes)
      out.nodes.push_back(n);
    return out;
  }

  [[nodiscard]] subscription on_state_changed(std::function<void(connection_state)> fn)
  {
    return add_subscriber(m_subs_state, std::move(fn));
  }

  // Fires from reconnect(), on the calling thread, immediately before the
  // loop, context and core are destroyed — they are still valid inside the
  // callback, which is where a holder must destroy the pw_streams and proxies
  // it built on them. The loop lock is not held; take it as usual.
  [[nodiscard]] subscription on_invalidated(std::function<void()> fn)
  {
    return add_subscriber(m_subs_invalidated, std::move(fn));
  }

  // Live events only; observer's notify_in_constructor handles the
  // initial walk (matches alsa_seq / jack convention).
  [[nodiscard]] subscription on_node_added(std::function<void(const node_info&)> fn)
  {
    return add_subscriber(m_subs_node_added, std::move(fn));
  }

  [[nodiscard]] subscription on_node_removed(std::function<void(std::uint32_t)> fn)
  {
    return add_subscriber(m_subs_node_removed, std::move(fn));
  }

  [[nodiscard]] subscription on_port_added(std::function<void(const port_info&)> fn)
  {
    return add_subscriber(m_subs_port_added, std::move(fn));
  }

  [[nodiscard]] subscription on_port_removed(std::function<void(std::uint32_t)> fn)
  {
    return add_subscriber(m_subs_port_removed, std::move(fn));
  }

  void unsubscribe(std::uint64_t id) noexcept
  {
    // Synchronous so a caller dropping its subscription can safely
    // destroy state captured by the subscriber lambda.
    invoke_sync([this, id] {
      remove_subscriber(m_subs_state, id);
      remove_subscriber(m_subs_invalidated, id);
      remove_subscriber(m_subs_node_added, id);
      remove_subscriber(m_subs_node_removed, id);
      remove_subscriber(m_subs_port_added, id);
      remove_subscriber(m_subs_port_removed, id);
    });
  }

  pw_thread_loop* thread_loop_handle() noexcept { return m_thread_loop; }
  pw_main_loop* main_loop_handle() noexcept { return m_main_loop; }
  pw_loop* bare_loop() noexcept { return m_loop; }
  pw_context* pw_context_ptr() noexcept { return m_context; }
  pw_core* pw_core_ptr() noexcept { return m_core; }
  pw_registry* registry() noexcept { return m_registry; }

  int get_fd() const noexcept
  {
    if (m_cfg.kind != loop_kind::main || !m_loop)
      return -1;
    return pw_loop_get_fd(m_loop);
  }

  int iterate(int timeout_ms) noexcept
  {
    if (m_cfg.kind != loop_kind::main || !m_loop)
      return -EINVAL;
    return pw_loop_iterate(m_loop, timeout_ms);
  }

private:
  std::shared_ptr<instance> m_instance;
  config m_cfg;

  // Exactly one of m_main_loop / m_thread_loop is non-null.
  pw_main_loop* m_main_loop{};
  pw_thread_loop* m_thread_loop{};
  pw_loop* m_loop{};

  pw_context* m_context{};
  pw_core* m_core{};
  pw_registry* m_registry{};
  spa_hook m_registry_listener{};
  spa_hook m_core_listener{};

  // WirePlumber default-target metadata: JSON props
  // `default.audio.sink` / `default.audio.source`. Strings guarded by m_graph_mtx.
  pw_proxy* m_default_metadata_proxy{};
  spa_hook m_default_metadata_listener{};
  std::string m_default_sink_name;
  std::string m_default_source_name;

  std::atomic<int> m_sync_seq{0};
  std::atomic<int> m_sync_done{0};
  std::atomic<int> m_sync_error{0};

  // do_sync_thread() waits here; wake_sync_waiter() (worker thread) notifies.
  // A private CV keeps application waiting off pipewire's recursive loop lock.
  std::mutex m_sync_mtx;
  std::condition_variable m_sync_cv;

  // FIXME lock-free?
  mutable std::mutex m_graph_mtx;

  struct bound_node
  {
    pw_proxy* proxy{};
    std::unique_ptr<spa_hook> hook;
    bool info_seen{};
    bool emitted_added{};
    node_info info;
  };
  struct bound_port
  {
    pw_proxy* proxy{};
    std::unique_ptr<spa_hook> hook;
    bool info_seen{};
    bool emitted_added{};
    port_info info;
  };

  std::vector<std::pair<std::uint32_t, bound_node>> m_bound_nodes;
  std::vector<std::pair<std::uint32_t, bound_port>> m_bound_ports;

  std::unordered_map<std::uint32_t, node_info> m_nodes;

  // False when the corresponding pointer is adopted via cfg.borrow_*.
  bool m_owns_loop{true};
  bool m_owns_core{true};

  // Recursive: an on_invalidated subscriber may release or acquire a holder
  // from inside reconnect(), which runs under this lock.
  mutable std::recursive_mutex m_holders_mtx;
  std::size_t m_shared_holders{0};

  struct holder_token
  {
    std::shared_ptr<context> ctx;
    explicit holder_token(std::shared_ptr<context> c)
        : ctx{std::move(c)}
    {
      std::lock_guard lk{ctx->m_holders_mtx};
      ++ctx->m_shared_holders;
    }
    ~holder_token()
    {
      std::lock_guard lk{ctx->m_holders_mtx};
      --ctx->m_shared_holders;
    }
    holder_token(const holder_token&) = delete;
    holder_token& operator=(const holder_token&) = delete;
  };

  std::atomic<connection_state> m_state{connection_state::connecting};
  std::atomic<std::uint32_t> m_generation{0};

  std::thread m_supervisor;
  std::mutex m_supervisor_mtx;
  std::condition_variable m_supervisor_cv;
  std::atomic_bool m_supervisor_stop{false};

  template <typename Fn>
  struct subscriber_slot
  {
    std::uint64_t id{};
    Fn fn;
  };
  template <typename Fn>
  struct subscriber_list
  {
    std::vector<subscriber_slot<Fn>> list;
  };

  subscriber_list<std::function<void(connection_state)>> m_subs_state;
  subscriber_list<std::function<void()>> m_subs_invalidated;
  subscriber_list<std::function<void(const node_info&)>> m_subs_node_added;
  subscriber_list<std::function<void(std::uint32_t)>> m_subs_node_removed;
  subscriber_list<std::function<void(const port_info&)>> m_subs_port_added;
  subscriber_list<std::function<void(std::uint32_t)>> m_subs_port_removed;

  std::atomic<std::uint64_t> m_next_sub_id{1};

  template <typename Fn>
  subscription add_subscriber(subscriber_list<Fn>& list, Fn fn)
  {
    std::uint64_t id = m_next_sub_id.fetch_add(1, std::memory_order_relaxed);
    // Under loop lock to serialize with dispatch.
    with_lock([&] { list.list.push_back({id, std::move(fn)}); });
    return subscription{weak_from_this(), id};
  }

  template <typename Fn>
  void remove_subscriber(subscriber_list<Fn>& list, std::uint64_t id) noexcept
  {
    auto it = std::find_if(
        list.list.begin(), list.list.end(), [id](const auto& s) { return s.id == id; });
    if (it != list.list.end())
      list.list.erase(it);
  }

  void dispatch_invalidated()
  {
    for (const auto& sub : m_subs_invalidated.list)
      if (sub.fn)
      {
        try
        {
          sub.fn();
        }
        catch (...)
        {
        }
      }
  }

  void dispatch_state(connection_state s)
  {
    for (const auto& sub : m_subs_state.list)
      if (sub.fn)
        sub.fn(s);

    notify_supervisor();
  }
  void dispatch_node_added(const node_info& n)
  {
    for (const auto& sub : m_subs_node_added.list)
      if (sub.fn)
        sub.fn(n);
  }
  void dispatch_node_removed(std::uint32_t id)
  {
    for (const auto& sub : m_subs_node_removed.list)
      if (sub.fn)
        sub.fn(id);
  }
  void dispatch_port_added(const port_info& p)
  {
    for (const auto& sub : m_subs_port_added.list)
      if (sub.fn)
        sub.fn(p);
  }
  void dispatch_port_removed(std::uint32_t id)
  {
    for (const auto& sub : m_subs_port_removed.list)
      if (sub.fn)
        sub.fn(id);
  }

  bool build_connection()
  {
    auto& pw = load();
    if (!pw.core_available)
      return false;
    if (m_cfg.kind == loop_kind::thread && !pw.thread_available)
      return false;

    if (m_cfg.kind == loop_kind::thread)
    {
      if (m_cfg.borrow_thread_loop)
      {
        m_thread_loop = m_cfg.borrow_thread_loop;
      }
      else
      {
        m_thread_loop = pw.thread_loop_new(m_cfg.loop_name, nullptr);
        if (!m_thread_loop)
          return false;
      }
      m_loop = pw.thread_loop_get_loop(m_thread_loop);
    }
    else
    {
      if (m_cfg.borrow_main_loop)
      {
        m_main_loop = m_cfg.borrow_main_loop;
      }
      else
      {
        m_main_loop = pw.main_loop_new(nullptr);
        if (!m_main_loop)
          return false;
      }
      m_loop = pw.main_loop_get_loop(m_main_loop);
    }
    if (!m_loop)
      return false;

    if (m_cfg.borrow_core)
    {
      m_core = m_cfg.borrow_core;
    }
    else
    {
      m_context = pw.context_new(m_loop, nullptr, 0);
      if (!m_context)
        return false;

      if (m_cfg.fd != -1)
      {
        // fd ownership transfers to pipewire (closed in pw_core_disconnect).
        m_core = pw.context_connect_fd(m_context, m_cfg.fd, nullptr, 0);
        m_cfg.fd = -1;
      }
      else
      {
        m_core = pw.context_connect(m_context, nullptr, 0);
      }
      if (!m_core)
        return false;
    }

    // Borrowed thread loop is already running — the worker iterates
    // listener lists we are about to mutate, so take the lock.
    const bool need_lock
        = (m_cfg.kind == loop_kind::thread) && m_thread_loop && !m_owns_loop;
    if (need_lock)
      pw.thread_loop_lock(m_thread_loop);
    struct unlock_guard
    {
      const api& pw;
      pw_thread_loop* tl;
      bool active;
      ~unlock_guard()
      {
        if (active)
          pw.thread_loop_unlock(tl);
      }
    } guard{pw, m_thread_loop, need_lock};

    install_core_listener();

    m_registry = pw_core_get_registry(m_core, PW_VERSION_REGISTRY, 0);
    if (!m_registry)
      return false;

    install_registry_listener();

    // Start *after* listeners are installed or we race the event burst.
    if (m_cfg.kind == loop_kind::thread && m_owns_loop)
    {
      if (pw.thread_loop_start(m_thread_loop) < 0)
        return false;
    }

    m_state.store(connection_state::connecting, std::memory_order_release);
    return true;
  }

  void start_supervisor()
  {
    m_supervisor = std::thread{[this] { supervisor_main(); }};
  }

  void stop_supervisor() noexcept
  {
    {
      std::lock_guard lk{m_supervisor_mtx};
      m_supervisor_stop.store(true, std::memory_order_release);
    }
    m_supervisor_cv.notify_all();
    if (m_supervisor.joinable())
      m_supervisor.join();
  }

  void supervisor_main()
  {
    auto backoff = m_cfg.reconnect_backoff;
    std::unique_lock lk{m_supervisor_mtx};
    while (!m_supervisor_stop.load(std::memory_order_acquire))
    {
      m_supervisor_cv.wait_for(
          lk, backoff, [this] { return m_supervisor_stop.load(std::memory_order_acquire); });
      if (m_supervisor_stop.load(std::memory_order_acquire))
        return;

      const auto s = m_state.load(std::memory_order_acquire);
      if (s == connection_state::broken)
      {
        lk.unlock();
        bool ok = false;
        try
        {
          ok = reconnect();
        }
        catch (...)
        {
          ok = false;
        }
        lk.lock();
        if (m_supervisor_stop.load(std::memory_order_acquire))
          return;
        if (ok)
        {
          backoff = m_cfg.reconnect_backoff;
        }
        else
        {
          auto next = backoff * 2;
          if (next > m_cfg.reconnect_backoff_max)
            next = m_cfg.reconnect_backoff_max;
          backoff = next;
        }
      }
      else
      {
        backoff = m_cfg.reconnect_backoff;
      }
    }
  }

  void notify_supervisor() noexcept
  {
    if (!m_cfg.auto_reconnect)
      return;
    m_supervisor_cv.notify_all();
  }

  void tear_down(bool final) noexcept
  {
    auto& pw = load();

    if (m_thread_loop && pw.thread_loop_stop && m_owns_loop)
      pw.thread_loop_stop(m_thread_loop);

    {
      // Loop stopped: no synchronisation needed.
      for (auto& [id, b] : m_bound_nodes)
      {
        if (b.hook)
          spa_hook_remove(b.hook.get());
        if (b.proxy && pw.proxy_destroy)
          pw.proxy_destroy(b.proxy);
      }
      m_bound_nodes.clear();
      for (auto& [id, b] : m_bound_ports)
      {
        if (b.hook)
          spa_hook_remove(b.hook.get());
        if (b.proxy && pw.proxy_destroy)
          pw.proxy_destroy(b.proxy);
      }
      m_bound_ports.clear();
      std::lock_guard lock{m_graph_mtx};
      m_nodes.clear();
    }

    spa_hook_remove(&m_core_listener);
    spa_hook_remove(&m_registry_listener);
    if (m_default_metadata_proxy)
    {
      spa_hook_remove(&m_default_metadata_listener);
      if (pw.proxy_destroy)
        pw.proxy_destroy(m_default_metadata_proxy);
      m_default_metadata_proxy = nullptr;
    }
    if (m_registry && pw.proxy_destroy)
      pw.proxy_destroy(reinterpret_cast<pw_proxy*>(m_registry));
    m_registry = nullptr;

    if (m_core && pw.core_disconnect && m_owns_core)
      pw.core_disconnect(m_core);
    m_core = nullptr;

    if (m_context && pw.context_destroy)
      pw.context_destroy(m_context);
    m_context = nullptr;

    if (m_thread_loop && pw.thread_loop_destroy && m_owns_loop)
      pw.thread_loop_destroy(m_thread_loop);
    m_thread_loop = nullptr;

    if (m_main_loop && pw.main_loop_destroy && m_owns_loop)
      pw.main_loop_destroy(m_main_loop);
    m_main_loop = nullptr;

    m_loop = nullptr;

    if (!final)
    {
      // Subscriber lists survive reconnect.
      m_sync_seq.store(0, std::memory_order_relaxed);
      m_sync_done.store(0, std::memory_order_relaxed);
      m_sync_error.store(0, std::memory_order_relaxed);
    }
    else
    {
      m_state.store(connection_state::disconnected, std::memory_order_release);
    }
  }

  static void on_core_info(void* /*data*/, const pw_core_info* /*info*/) noexcept { }

  static void on_core_done(void* data, std::uint32_t id, int seq) noexcept
  {
    auto* self = static_cast<context*>(data);
    if (id == PW_ID_CORE && seq == self->m_sync_seq.load(std::memory_order_acquire))
    {
      self->m_sync_done.store(1, std::memory_order_release);
      self->wake_sync_waiter();
    }
  }

  // pw_core_events::error reports the proxy the error belongs to in `id`;
  // PW_ID_CORE is the connection itself, anything else is a per-object error
  // that says nothing about the socket.
  static void on_core_error(
      void* data, std::uint32_t id, int /*seq*/, int res, const char* /*message*/) noexcept
  {
    auto* self = static_cast<context*>(data);
    if (id != PW_ID_CORE)
      return;

    self->m_sync_error.store(res, std::memory_order_release);
    // -ENOENT is "no such object/factory", which is recoverable; connection
    // loss surfaces as a socket error.
    if (res == -EPIPE || res == -ECONNRESET || res == -ENOTCONN)
    {
      self->m_state.store(connection_state::broken, std::memory_order_release);
    }
    self->wake_sync_waiter();
  }

  // Wakes do_sync_thread(), which waits on m_sync_cv. Runs from the worker
  // thread during dispatch; the brief m_sync_mtx lock orders the flag store
  // with the waiter without nesting an application wait inside pipewire's
  // recursive loop lock.
  void wake_sync_waiter() noexcept
  {
    {
      std::lock_guard<std::mutex> lk{m_sync_mtx};
    }
    m_sync_cv.notify_all();
  }

  void install_core_listener() noexcept
  {
    spa_zero(m_core_listener);
    static constexpr pw_core_events events = {
        .version = PW_VERSION_CORE_EVENTS,
        .info = &on_core_info,
        .done = &on_core_done,
        .ping = nullptr,
        .error = &on_core_error,
        .remove_id = nullptr,
        .bound_id = nullptr,
        .add_mem = nullptr,
        .remove_mem = nullptr,
#if PW_VERSION_CORE_EVENTS >= 1
        .bound_props = nullptr,
#endif
    };
    pw_core_add_listener(m_core, &m_core_listener, &events, this);
  }

  // Thread-loop sync from a foreign thread: the worker owns iteration, so take
  // the loop lock only briefly to issue the sync, then wait on a private
  // condition variable. We must neither iterate the loop ourselves (two
  // threads iterating one pw_loop corrupt its per-source dispatch state) nor
  // hold pipewire's recursive loop lock across the wait (pthread_cond_wait
  // cannot fully release a recursively-held mutex, so it starves the worker
  // and corrupts the lock: 'recurse > 0' in do_unlock).
  bool do_sync_thread(std::chrono::steady_clock::time_point deadline)
  {
    auto& pw = load();
    m_sync_done.store(0, std::memory_order_release);
    m_sync_error.store(0, std::memory_order_release);

    {
      pw.thread_loop_lock(m_thread_loop);
      int seq = pw_core_sync(m_core, PW_ID_CORE, 0);
      m_sync_seq.store(seq, std::memory_order_release);
      pw.thread_loop_unlock(m_thread_loop);
    }

    {
      std::unique_lock<std::mutex> lk{m_sync_mtx};
      const bool ready = m_sync_cv.wait_until(lk, deadline, [this] {
        return m_sync_done.load(std::memory_order_acquire) != 0
               || m_sync_error.load(std::memory_order_acquire) != 0;
      });
      if (!ready)
      {
        m_state.store(connection_state::broken, std::memory_order_release);
        return false;
      }
    }
    return finalize_sync();
  }

  // Main-loop mode: no worker thread exists, so the caller owns iteration.
  bool do_sync_iterate(std::chrono::steady_clock::time_point deadline)
  {
    m_sync_done.store(0, std::memory_order_release);
    m_sync_error.store(0, std::memory_order_release);

    int seq = pw_core_sync(m_core, PW_ID_CORE, 0);
    m_sync_seq.store(seq, std::memory_order_release);

    using clk = std::chrono::steady_clock;
    while (m_sync_done.load(std::memory_order_acquire) == 0
           && m_sync_error.load(std::memory_order_acquire) == 0)
    {
      auto now = clk::now();
      if (now >= deadline)
      {
        m_state.store(connection_state::broken, std::memory_order_release);
        return false;
      }
      auto remaining_ms
          = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now).count();
      const int iter_ms = static_cast<int>(remaining_ms < 8 ? remaining_ms : 8);
      int r = pw_loop_iterate(m_loop, iter_ms);
      if (r < 0)
      {
        m_state.store(connection_state::broken, std::memory_order_release);
        return false;
      }
    }
    return finalize_sync();
  }

  bool finalize_sync() noexcept
  {
    // Only on_core_error() decides whether an error is fatal to the
    // connection; a failed round-trip on its own is not.
    if (m_sync_error.load(std::memory_order_acquire) != 0)
      return false;

    if (m_state.load(std::memory_order_acquire) == connection_state::connecting)
    {
      m_state.store(connection_state::connected, std::memory_order_release);
      dispatch_state(connection_state::connected);
    }
    return true;
  }

  static void on_registry_global(
      void* data, std::uint32_t id, std::uint32_t /*permissions*/, const char* type,
      std::uint32_t /*version*/, const spa_dict* props) noexcept
  {
    auto* self = static_cast<context*>(data);
    if (!type)
      return;
    if (std::strcmp(type, PW_TYPE_INTERFACE_Node) == 0)
      self->register_node(id, props);
    else if (std::strcmp(type, PW_TYPE_INTERFACE_Port) == 0)
      self->register_port(id, props);
    else if (std::strcmp(type, PW_TYPE_INTERFACE_Metadata) == 0)
    {
      // Only the "default" metadata is WirePlumber's default-target
      // marker (publishes default.audio.{sink,source}); ignore others.
      const auto name = dict_get(props, PW_KEY_METADATA_NAME);
      if (name == "default")
        self->register_default_metadata(id);
    }
  }

  static void on_registry_global_remove(void* data, std::uint32_t id) noexcept
  {
    auto* self = static_cast<context*>(data);
    self->unregister_global(id);
  }

  void install_registry_listener() noexcept
  {
    spa_zero(m_registry_listener);
    static constexpr pw_registry_events events = {
        .version = PW_VERSION_REGISTRY_EVENTS,
        .global = &on_registry_global,
        .global_remove = &on_registry_global_remove,
    };
    pw_registry_add_listener(m_registry, &m_registry_listener, &events, this);
  }

  void register_node(std::uint32_t id, const spa_dict* props) noexcept
  {
    auto* proxy = reinterpret_cast<pw_proxy*>(
        pw_registry_bind(m_registry, id, PW_TYPE_INTERFACE_Node, PW_VERSION_NODE, 0));
    if (!proxy)
      return;

    bound_node b;
    b.proxy = proxy;
    b.hook = std::make_unique<spa_hook>();
    spa_zero(*b.hook);
    b.info.id = id;

    b.info.name = std::string{dict_get(props, PW_KEY_NODE_NAME)};
    b.info.description = std::string{dict_get(props, PW_KEY_NODE_DESCRIPTION)};
    b.info.media_class_str = std::string{dict_get(props, PW_KEY_MEDIA_CLASS)};
    b.info.media_role = std::string{dict_get(props, PW_KEY_MEDIA_ROLE)};
    b.info.kind = classify_media_class(b.info.media_class_str);

    m_bound_nodes.emplace_back(id, std::move(b));
    auto& slot = m_bound_nodes.back().second;

    static constexpr pw_node_events events = {
        .version = PW_VERSION_NODE_EVENTS,
        .info = &on_node_info,
        .param = nullptr,
    };
    auto* node_proxy = reinterpret_cast<pw_node*>(slot.proxy);
    pw_node_add_listener(node_proxy, slot.hook.get(), &events, this);

    if (!slot.info.name.empty() || !slot.info.media_class_str.empty())
    {
      slot.info_seen = true;
      mirror_node_to_snapshot(slot.info);
      if (!slot.emitted_added)
      {
        slot.emitted_added = true;
        dispatch_node_added(slot.info);
      }
    }
  }

  static void on_node_info(void* data, const pw_node_info* info) noexcept
  {
    auto* self = static_cast<context*>(data);
    if (!info)
      return;
    // info->props is only valid when PW_NODE_CHANGE_MASK_PROPS is set;
    // otherwise re-reading clobbers cached values with empty placeholders.
    const bool props_valid
        = info->props && (info->change_mask & PW_NODE_CHANGE_MASK_PROPS);
    for (auto& [k, slot] : self->m_bound_nodes)
    {
      if (k != info->id)
        continue;
      if (props_valid)
      {
        slot.info.name = std::string{dict_get(info->props, PW_KEY_NODE_NAME)};
        slot.info.description = std::string{dict_get(info->props, PW_KEY_NODE_DESCRIPTION)};
        slot.info.media_class_str = std::string{dict_get(info->props, PW_KEY_MEDIA_CLASS)};
        slot.info.media_role = std::string{dict_get(info->props, PW_KEY_MEDIA_ROLE)};
        slot.info.kind = classify_media_class(slot.info.media_class_str);
      }
      slot.info_seen = true;
      self->mirror_node_to_snapshot(slot.info);
      if (!slot.emitted_added)
      {
        slot.emitted_added = true;
        self->dispatch_node_added(slot.info);
      }
      break;
    }
  }

  void mirror_node_to_snapshot(const node_info& n)
  {
    std::lock_guard lock{m_graph_mtx};
    auto& entry = m_nodes[n.id];
    // Preserve port lists populated by mirror_port_*.
    auto inputs = std::move(entry.inputs);
    auto outputs = std::move(entry.outputs);
    bool phys = entry.physical;
    entry = n;
    entry.inputs = std::move(inputs);
    entry.outputs = std::move(outputs);
    entry.physical = phys;
  }

  // Hand-parse Spa:String:JSON `{"name":"foo"}` (no JSON dep).
  static std::string parse_default_target_json(const char* value) noexcept
  {
    if (!value)
      return {};
    std::string_view v{value};
    constexpr std::string_view tag = "\"name\"";
    auto kpos = v.find(tag);
    if (kpos == std::string_view::npos)
      return {};
    auto colon = v.find(':', kpos + tag.size());
    if (colon == std::string_view::npos)
      return {};
    auto open = v.find('"', colon + 1);
    if (open == std::string_view::npos)
      return {};
    auto close = v.find('"', open + 1);
    if (close == std::string_view::npos)
      return {};
    return std::string{v.substr(open + 1, close - open - 1)};
  }

  void register_default_metadata(std::uint32_t id) noexcept
  {
    auto* proxy = reinterpret_cast<pw_proxy*>(pw_registry_bind(
        m_registry, id, PW_TYPE_INTERFACE_Metadata, PW_VERSION_METADATA, 0));
    if (!proxy)
      return;
    if (m_default_metadata_proxy)
    {
      auto& pw = load();
      if (pw.proxy_destroy)
        pw.proxy_destroy(proxy);
      return;
    }
    m_default_metadata_proxy = proxy;
    spa_zero(m_default_metadata_listener);

    static constexpr pw_metadata_events events = {
        .version = PW_VERSION_METADATA_EVENTS,
        .property = &on_default_metadata_property,
    };
    pw_metadata_add_listener(
        reinterpret_cast<pw_metadata*>(proxy), &m_default_metadata_listener,
        &events, this);
  }

  static int on_default_metadata_property(
      void* data, std::uint32_t /*subject*/, const char* key,
      const char* /*type*/, const char* value) noexcept
  {
    if (!key)
      return 0;
    auto* self = static_cast<context*>(data);
    const std::string parsed = parse_default_target_json(value);
    std::lock_guard lock{self->m_graph_mtx};
    // `default.audio.*` (currently-routed) is what we want;
    // `default.configured.audio.*` (sticky user choice) is ignored.
    std::string_view k{key};
    if (k == "default.audio.sink")
      self->m_default_sink_name = parsed;
    else if (k == "default.audio.source")
      self->m_default_source_name = parsed;
    return 0;
  }

public:
  std::string default_audio_sink_name() const
  {
    std::lock_guard lock{m_graph_mtx};
    return m_default_sink_name;
  }

  std::string default_audio_source_name() const
  {
    std::lock_guard lock{m_graph_mtx};
    return m_default_source_name;
  }

private:

  void register_port(std::uint32_t id, const spa_dict* props) noexcept
  {
    auto* proxy = reinterpret_cast<pw_proxy*>(
        pw_registry_bind(m_registry, id, PW_TYPE_INTERFACE_Port, PW_VERSION_PORT, 0));
    if (!proxy)
      return;

    bound_port b;
    b.proxy = proxy;
    b.hook = std::make_unique<spa_hook>();
    spa_zero(*b.hook);
    b.info.id = id;
    if (props)
      fill_port_from_props(b.info, props);

    m_bound_ports.emplace_back(id, std::move(b));
    auto& slot = m_bound_ports.back().second;

    static constexpr pw_port_events events = {
        .version = PW_VERSION_PORT_EVENTS,
        .info = &on_port_info,
        .param = nullptr,
    };
    auto* port_proxy = reinterpret_cast<pw_port*>(slot.proxy);
    pw_port_add_listener(port_proxy, slot.hook.get(), &events, this);
  }

  static void on_port_info(void* data, const pw_port_info* info) noexcept
  {
    auto* self = static_cast<context*>(data);
    if (!info)
      return;
    // See on_node_info: props only valid under PW_PORT_CHANGE_MASK_PROPS.
    const bool props_valid
        = info->props && (info->change_mask & PW_PORT_CHANGE_MASK_PROPS);
    for (auto& [k, slot] : self->m_bound_ports)
    {
      if (k != info->id)
        continue;
      slot.info.id = info->id;
      slot.info.direction = static_cast<int>(info->direction);
      if (props_valid)
        self->fill_port_from_props(slot.info, info->props);
      slot.info_seen = true;
      self->mirror_port_to_snapshot(slot.info);
      if (!slot.emitted_added && slot.info.node_id != 0)
      {
        slot.emitted_added = true;
        self->dispatch_port_added(slot.info);
      }
      break;
    }
  }

  void fill_port_from_props(port_info& p, const spa_dict* props) noexcept
  {
    if (auto v = dict_get(props, "format.dsp"); !v.empty())
      p.format = std::string{v};
    if (auto k = classify_port_props(props); k != media_class::other)
      p.kind = k;
    if (auto v = dict_get(props, PW_KEY_PORT_NAME); !v.empty())
      p.port_name = std::string{v};
    if (auto v = dict_get(props, PW_KEY_PORT_ALIAS); !v.empty())
      p.port_alias = std::string{v};
    if (auto v = dict_get(props, PW_KEY_OBJECT_PATH); !v.empty())
      p.object_path = std::string{v};
    if (auto v = dict_get(props, PW_KEY_PORT_ID); !v.empty())
      p.port_id = std::string{v};
    if (auto v = dict_get(props, PW_KEY_NODE_ID); !v.empty())
    {
      // PW_KEY_NODE_ID is stringified.
      try
      {
        p.node_id = static_cast<std::uint32_t>(std::stoul(std::string{v}));
      }
      catch (...)
      {
      }
    }
    p.physical = dict_get(props, PW_KEY_PORT_PHYSICAL) == "true";
    p.terminal = dict_get(props, PW_KEY_PORT_TERMINAL) == "true";
    p.monitor = dict_get(props, PW_KEY_PORT_MONITOR) == "true";
    if (auto v = dict_get(props, PW_KEY_PORT_DIRECTION); v == "out")
      p.direction = 1; // SPA_DIRECTION_OUTPUT
    else if (v == "in")
      p.direction = 0; // SPA_DIRECTION_INPUT
  }

  void mirror_port_to_snapshot(const port_info& p)
  {
    if (p.node_id == 0)
      return;
    std::lock_guard lock{m_graph_mtx};
    auto& node = m_nodes[p.node_id];
    if (node.id == 0)
      node.id = p.node_id;
    auto& bucket = (p.direction == 1) ? node.outputs : node.inputs;
    auto it
        = std::find_if(bucket.begin(), bucket.end(), [&](const auto& q) { return q.id == p.id; });
    if (it == bucket.end())
      bucket.push_back(p);
    else
      *it = p;
    if (p.physical)
      node.physical = true;
  }

  void unregister_global(std::uint32_t id) noexcept
  {
    auto& pw = load();
    {
      auto it = std::find_if(m_bound_nodes.begin(), m_bound_nodes.end(), [id](const auto& e) {
        return e.first == id;
      });
      if (it != m_bound_nodes.end())
      {
        if (it->second.hook)
          spa_hook_remove(it->second.hook.get());
        if (it->second.proxy && pw.proxy_destroy)
          pw.proxy_destroy(it->second.proxy);
        m_bound_nodes.erase(it);
        {
          std::lock_guard lock{m_graph_mtx};
          m_nodes.erase(id);
        }
        dispatch_node_removed(id);
        return;
      }
    }
    {
      auto it = std::find_if(m_bound_ports.begin(), m_bound_ports.end(), [id](const auto& e) {
        return e.first == id;
      });
      if (it != m_bound_ports.end())
      {
        std::uint32_t parent_node = it->second.info.node_id;
        if (it->second.hook)
          spa_hook_remove(it->second.hook.get());
        if (it->second.proxy && pw.proxy_destroy)
          pw.proxy_destroy(it->second.proxy);
        m_bound_ports.erase(it);
        if (parent_node != 0)
        {
          std::lock_guard lock{m_graph_mtx};
          auto nit = m_nodes.find(parent_node);
          if (nit != m_nodes.end())
          {
            auto& n = nit->second;
            n.inputs.erase(
                std::remove_if(
                    n.inputs.begin(), n.inputs.end(), [id](const auto& p) { return p.id == id; }),
                n.inputs.end());
            n.outputs.erase(
                std::remove_if(
                    n.outputs.begin(), n.outputs.end(),
                    [id](const auto& p) { return p.id == id; }),
                n.outputs.end());
          }
        }
        dispatch_port_removed(id);
        return;
      }
    }
  }
};

inline void subscription::reset() noexcept
{
  if (m_id == 0)
    return;
  if (auto ctx = m_ctx.lock())
    ctx->unsubscribe(m_id);
  m_ctx.reset();
  m_id = 0;
}

inline subscription::~subscription()
{
  reset();
}

// Defined in context.cpp — see shared_instance() for the visibility
// rationale.
LIBREMIDI_EXPORT std::shared_ptr<context>
shared_context(context::config cfg = context::config{}) noexcept;

}

#if defined(__clang__)
  #pragma clang diagnostic pop
#endif

#if defined(LIBREMIDI_HEADER_ONLY)
  #include <libremidi/backends/linux/pipewire/context.cpp>
#endif
