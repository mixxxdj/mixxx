#pragma once
#include <libremidi/backends/keyboard/config.hpp>
#include <libremidi/detail/midi_in.hpp>

#include <chrono>
#include <unordered_map>

NAMESPACE_LIBREMIDI
{
class midi_in_kbd final
    : public midi1::in_api
    , public error_handler
{
public:
  using midi_api::client_open_;
  struct
      : input_configuration
      , kbd_input_configuration
  {
  } configuration;

  explicit midi_in_kbd(input_configuration&& conf, kbd_input_configuration&& apiconf)
      : configuration{std::move(conf), std::move(apiconf)}
  {
  }

  ~midi_in_kbd() override { }

  libremidi::API get_current_api() const noexcept override { return libremidi::API::KEYBOARD; }

  stdx::error open_port(const input_port&, std::string_view) override
  {
    configuration.set_input_scancode_callbacks(
        [this](int v) { on_keypress(v); }, [this](int v) { on_keyrelease(v); });

    return stdx::error{};
  }

  stdx::error close_port() override { return stdx::error{}; }

  timestamp absolute_timestamp() const noexcept override
  {
    return std::chrono::steady_clock::now().time_since_epoch().count();
  }

  void on_keypress(int scancode)
  {
    using kevent = libremidi::kbd_event;

    auto it = configuration.scancode_map.find(scancode);
    if (it == configuration.scancode_map.end())
      return;

    if (it->second >= kevent::NOTE_0 && it->second < (kevent::NOTE_0 + 128))
    {
      int note = it->second - kevent::NOTE_0 + 12 * m_current_octave;
      this->configuration.on_message(
          libremidi::channel_events::note_on(0, note, m_current_velocity));
      m_current_notes_scancodes[scancode] = note;
    }
    else if (it->second >= kevent::VEL_0 && it->second < (kevent::VEL_0 + 128))
    {
      m_current_velocity = it->second - kevent::VEL_0;
    }
    else if (it->second >= kevent::OCT_0 && it->second < (kevent::OCT_0 + 128))
    {
      m_current_octave = it->second - kevent::OCT_0;
    }
    else
    {
      switch (it->second)
      {
        case kevent::VELOCITY_MINUS:
          m_current_velocity = std::clamp(m_current_velocity - 10, 0, 127);
          break;
        case kevent::VELOCITY_PLUS:
          m_current_velocity = std::clamp(m_current_velocity + 10, 0, 127);
          break;
        case kevent::OCTAVE_MINUS:
          m_current_octave = std::clamp(m_current_octave - 1, 0, 127);
          break;
        case kevent::OCTAVE_PLUS:
          m_current_octave = std::clamp(m_current_octave + 1, 0, 127);
          break;
      }
    }
  }

  void on_keyrelease(int scancode)
  {
    using kevent = libremidi::kbd_event;

    auto it = configuration.scancode_map.find(scancode);
    if (it == configuration.scancode_map.end())
      return;

    if (it->second >= kevent::NOTE_0 && it->second < (kevent::NOTE_0 + 128))
    {
      if (auto note_it = m_current_notes_scancodes.find(scancode);
          note_it != m_current_notes_scancodes.end())
      {
        this->configuration.on_message(libremidi::channel_events::note_off(0, note_it->second, 0));
        m_current_notes_scancodes.erase(note_it);
      }
    }
  }

  int m_current_octave{3};
  int m_current_velocity{80};
  std::unordered_map<int, int> m_current_notes_scancodes;
};
}
