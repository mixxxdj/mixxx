#include <libremidi/client.hpp>

#include <chrono>
#include <iostream>
#include <thread>

int main()
{
  namespace lm = libremidi::midi1;
  int input_k = 1;
  int output_k = 1;

  lm::client client{
      {.api = libremidi::API::ALSA_RAW,
       .client_name = "my client",
       .on_message =
           [&](const libremidi::input_port& port, const libremidi::message& mess) {
    std::cerr << port.display_name << ": " << mess.bytes.size() << std::endl;

    client.send_message(mess.bytes.data(), mess.bytes.size());
  },
       .input_added =
           [&](const libremidi::input_port& port) {
    std::cerr << "Input added: " << port.display_name << " | " << port.port_name << std::endl;
    client.add_input(port, "input_" + std::to_string(input_k++));
  },
       .input_removed =
           [&](const libremidi::input_port& port) {
    std::cerr << "Input removed: " << port.display_name << std::endl;
    client.remove_input(port);
  },
       .output_added =
           [&](const libremidi::output_port& port) {
    std::cerr << "Output added: " << port.display_name << std::endl;
    client.add_output(port, "output_" + std::to_string(output_k++));
  },
       .output_removed = [&](const libremidi::output_port& port) {
    std::cerr << "Output removed: " << port.display_name << std::endl;
    client.remove_output(port);
  }}};

  auto inputs = client.get_input_ports();
  auto outputs = client.get_output_ports();

  for (const auto& port : inputs)
  {
    client.add_input(port, "input_" + std::to_string(input_k++));
  }

  for (const auto& port : outputs)
  {
    client.add_output(port, "output_" + std::to_string(output_k++));
  }

  std::this_thread::sleep_for(std::chrono::seconds(1));
}
