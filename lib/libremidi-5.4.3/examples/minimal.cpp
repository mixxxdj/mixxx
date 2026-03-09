
#include "utils.hpp"

#include <libremidi/libremidi.hpp>

#if defined(_WIN32) && __has_include(<winrt/base.h>)
  #include <winrt/base.h>
#endif

#include <cstdlib>
#include <iostream>

int main()
{
#if defined(_WIN32) && __has_include(<winrt/base.h>)
  // Necessary for using WinUWP and WinMIDI, must be done as early as possible in your main()
  winrt::init_apartment();
#endif

  libremidi::observer observer;
  if (observer.get_input_ports().empty())
    return 1;
  if (observer.get_output_ports().empty())
    return 1;

  libremidi::midi_in midi_in{{.on_message = [](const libremidi::message& message) {
    std::cout << message << std::endl;
  }}};

  if (auto err = midi_in.open_port(observer.get_input_ports()[0]); err != stdx::error{})
    err.throw_exception();

  libremidi::midi_out midi_out;
  if (auto err = midi_out.open_port(observer.get_output_ports()[0]); err != stdx::error{})
    err.throw_exception();

  midi_out.send_message(176, 7, 100);

  int c;
  while ((c = getchar()) != '\n' && c != EOF)
    ;
  return 0;
}
