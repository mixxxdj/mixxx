import libremidi;

#include <cstdlib>
#include <iostream>

inline std::ostream& operator<<(std::ostream& s, const libremidi::message& message)
{
  auto nBytes = message.size();
  s << "[ ";
  for (auto i = 0U; i < nBytes; i++)
    s << std::hex << (int)message[i] << std::dec << " ";
  s << "]";
  if (nBytes > 0)
    s << " ; stamp = " << message.timestamp;
  return s;
}

int main()
{
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
