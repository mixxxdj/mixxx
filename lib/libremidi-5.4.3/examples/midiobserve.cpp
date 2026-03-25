#include "utils.hpp"

#include <libremidi/libremidi.hpp>

#if defined(_WIN32) && __has_include(<winrt/base.h>)
  #include <winrt/base.h>
#endif

#include <cstdlib>
#include <iostream>
#include <map>

#if defined(__APPLE__)
  #include <CoreFoundation/CoreFoundation.h>
#endif

int main()
{
#if defined(_WIN32) && __has_include(<winrt/base.h>)
  // Necessary for using WinUWP and WinMIDI, must be done as early as possible in your main()
  winrt::init_apartment();
#endif

  std::vector<libremidi::observer> observers;
  for (auto api : libremidi::available_apis())
  {
    std::string_view api_name = libremidi::get_api_display_name(api);

    std::cout << "Displaying ports for: " << api_name << std::endl;
    libremidi::observer_configuration cbs;
    cbs.input_added = [=](const libremidi::input_port& p) {
      std::cout << api_name << " : input added " << p << "\n";
    };
    cbs.input_removed = [=](const libremidi::input_port& p) {
      std::cout << api_name << " : input removed " << p << "\n";
    };
    cbs.output_added = [=](const libremidi::output_port& p) {
      std::cout << api_name << " : output added " << p << "\n";
    };
    cbs.output_removed = [=](const libremidi::output_port& p) {
      std::cout << api_name << " : output removed " << p << "\n";
    };
    observers.emplace_back(cbs, libremidi::observer_configuration_for(api));
    std::cout << "\n" << std::endl;
  }

  std::cout << "... waiting for hotplug events ...\n";

#if defined(__APPLE__)
  // On macOS, observation can *only* be done in the main thread
  // with an active CFRunLoop.
  CFRunLoopRun();
#else
  int c;
  while ((c = getchar()) != '\n' && c != EOF)
    ;
#endif
  return 0;
}
