#pragma once
#include <libremidi/backends/winmm/config.hpp>
#include <libremidi/backends/winmm/helpers.hpp>
#include <libremidi/detail/observer.hpp>

#include <thread>

NAMESPACE_LIBREMIDI
{

class observer_winmm : public observer_api
{
public:
  struct
      : observer_configuration
      , winmm_observer_configuration
  {
  } configuration;

  explicit observer_winmm(observer_configuration&& conf, winmm_observer_configuration&& apiconf)
      : configuration{std::move(conf), std::move(apiconf)}
  {
    if (!configuration.has_callbacks())
      return;

    if (configuration.notify_in_constructor)
      check_new_ports<true>();
    else
      check_new_ports<false>();
  }

  ~observer_winmm() { }

  libremidi::API get_current_api() const noexcept override { return libremidi::API::WINDOWS_MM; }

  std::vector<input_port> get_input_ports() const noexcept override
  {
    return get_port_list<true>();
  }

  std::vector<output_port> get_output_ports() const noexcept override
  {
    return get_port_list<false>();
  }

protected:
  template <bool Notify>
  void check_new_ports()
  {
    auto currInputPortList = get_port_list<true>();

    if constexpr (Notify)
    {
      compare_port_lists_and_notify_clients(
          inputPortList, currInputPortList, configuration.input_added,
          configuration.input_removed);
    }
    inputPortList = std::move(currInputPortList);

    auto currOutputPortList = get_port_list<false>();

    if constexpr (Notify)
    {
      compare_port_lists_and_notify_clients(
          outputPortList, currOutputPortList, configuration.output_added,
          configuration.output_removed);
    }
    outputPortList = std::move(currOutputPortList);
  }

  void compare_port_lists_and_notify_clients(
      const auto& prevList, const auto& currList, const auto& portAddedFunc,
      const auto& portRemovedFunc)
  {
    if (portAddedFunc)
    {
      for (const auto& port : currList)
      {
        auto iter
            = std::ranges::find(prevList, port.display_name, &port_information::display_name);
        if (iter == prevList.end())
          portAddedFunc(port);
      }
    }

    if (portRemovedFunc)
    {
      for (const auto& port : prevList)
      {
        auto iter
            = std::ranges::find(currList, port.display_name, &port_information::display_name);
        if (iter == currList.end())
          portRemovedFunc(port);
      }
    }
  }

  input_port to_in_port_info(std::size_t index) const noexcept
  {
    MIDIINCAPS deviceCaps;
    midiInGetDevCaps(index, &deviceCaps, sizeof(MIDIINCAPS));

    auto rawName = ConvertToUTF8(deviceCaps.szPname);
    auto portName = rawName;
    MakeUniqueInPortName(portName, index);

    return {
        {.api = libremidi::API::WINDOWS_MM,
         .client = 0,
         .device
         = usb_device_identifier{.vendor_id = deviceCaps.wMid, .product_id = deviceCaps.wPid},
         .port = index,
         .manufacturer = "",
         .device_name = "",
         .port_name = rawName,
         .display_name = portName}};
  }

  output_port to_out_port_info(std::size_t index) const noexcept
  {
    MIDIOUTCAPS deviceCaps;
    midiOutGetDevCaps(index, &deviceCaps, sizeof(MIDIOUTCAPS));

    auto rawName = ConvertToUTF8(deviceCaps.szPname);
    auto portName = rawName;
    MakeUniqueOutPortName(portName, index);

    return {
        {.api = libremidi::API::WINDOWS_MM,
         .client = 0,
         .device
         = usb_device_identifier{.vendor_id = deviceCaps.wMid, .product_id = deviceCaps.wPid},
         .port = index,
         .manufacturer = "",
         .device_name = "",
         .port_name = rawName,
         .display_name = portName}};
  }

  template <bool Input>
  auto get_port_list() const noexcept
      -> std::vector<std::conditional_t<Input, input_port, output_port>>
  {
    std::vector<std::conditional_t<Input, input_port, output_port>> portList;

    if constexpr (Input)
    {
      std::size_t nDevices = midiInGetNumDevs();
      for (std::size_t i = 0; i < nDevices; ++i)
      {
        portList.push_back(to_in_port_info(i));
      }
    }
    else
    {
      std::size_t nDevices = midiOutGetNumDevs();
      for (std::size_t i = 0; i < nDevices; ++i)
      {
        portList.push_back(to_out_port_info(i));
      }
    }
    return portList;
  }

  static constexpr bool INPUT = true;
  static constexpr bool OUTPUT = false;

  std::vector<input_port> inputPortList;
  std::vector<output_port> outputPortList;
};
}

#if __has_include(<stop_token>) && __cpp_lib_jthread >= 201911L
  #include <stop_token>
NAMESPACE_LIBREMIDI::winmm
{
class observer_threaded final : public observer_winmm
{
public:
  struct
      : observer_configuration
      , winmm_observer_configuration
  {
  } configuration;

  explicit observer_threaded(observer_configuration&& conf, winmm_observer_configuration&& apiconf)
      : observer_winmm{std::move(conf), std::move(apiconf)}
  {
    thread = std::jthread([this](std::stop_token tk) {
      while (!tk.stop_requested())
      {
        check_new_ports<true>();
        std::this_thread::sleep_for(this->configuration.poll_period);
      }
    });
  }

private:
  std::jthread thread;
};
}
#else
  #include <atomic>
  #include <semaphore>
NAMESPACE_LIBREMIDI::winmm
{
class observer_threaded final : public observer_winmm
{
public:
  struct
      : observer_configuration
      , winmm_observer_configuration
  {
  } configuration;

  explicit observer_threaded(observer_configuration&& conf, winmm_observer_configuration&& apiconf)
      : observer_winmm{std::move(conf), std::move(apiconf)}
      , sema{0}
  {
    thread = std::thread([this] {
      while (!stop_flag.test(std::memory_order_acquire))
      {
        check_new_ports<true>();
        std::this_thread::sleep_for(this->configuration.poll_period);
      }
      sema.release();
    });
  }

  ~observer_threaded()
  {
    stop_flag.test_and_set();
    sema.acquire();
    thread.join();
  }

private:
  std::thread thread;
  std::atomic_flag stop_flag = ATOMIC_FLAG_INIT;
  std::binary_semaphore sema;
};
}
#endif

NAMESPACE_LIBREMIDI::winmm
{
class observer_manual final : public observer_winmm
{
public:
  struct
      : observer_configuration
      , winmm_observer_configuration
  {
  } configuration;

  explicit observer_manual(observer_configuration&& conf, winmm_observer_configuration&& apiconf)
      : observer_winmm{std::move(conf), std::move(apiconf)}
  {
    this->configuration.manual_poll({.callback = [this] { this->check_new_ports<true>(); }});
  }

  ~observer_manual() { }
};
}
NAMESPACE_LIBREMIDI
{
template <>
inline std::unique_ptr<observer_api> make<observer_winmm>(
    libremidi::observer_configuration&& conf, libremidi::winmm_observer_configuration&& api)
{
  if (api.manual_poll)
    return std::make_unique<winmm::observer_manual>(std::move(conf), std::move(api));
  else
    return std::make_unique<winmm::observer_threaded>(std::move(conf), std::move(api));
}
}
