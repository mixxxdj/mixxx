#pragma once
#include <libremidi/backends/emscripten/midi_in.hpp>
#include <libremidi/backends/emscripten/midi_out.hpp>
#include <libremidi/backends/emscripten/observer.hpp>
#include <libremidi/libremidi.hpp>

#include <emscripten.h>

#include <map>

extern "C" {
EMSCRIPTEN_KEEPALIVE
void libremidi_devices_poll();

EMSCRIPTEN_KEEPALIVE
void libremidi_devices_input(int port, double timestamp, int len, char* bytes);
}

NAMESPACE_LIBREMIDI
{
namespace webmidi_helpers
{

class midi_access_emscripten
{
public:
  static midi_access_emscripten& instance() noexcept
  {
    static midi_access_emscripten inst;
    return inst;
  }

  bool available() const noexcept
  {
    return EM_ASM_INT(return typeof globalThis.__libreMidi_access !== undefined;);
  }

  int input_count() const noexcept
  {
    if (!available())
      return 0;

    return EM_ASM_INT(return globalThis.__libreMidi_currentInputs.length;);
  }

  int output_count() const noexcept
  {
    if (!available())
      return 0;

    return EM_ASM_INT(return globalThis.__libreMidi_currentOutputs.length;);
  }

  void load_current_infos() noexcept
  {
    if (!available())
      return;

#define get_js_string(variable_to_read, ...) \
  (char*)EM_ASM_INT(                            \
        {                                         \
              var jsstr = variable_to_read;           \
              var bytes = lengthBytesUTF8(jsstr) + 1; \
              var str = _malloc(bytes);               \
              stringToUTF8(jsstr, str, bytes);        \
              return str;                             \
        },                                        \
        __VA_ARGS__);

    EM_ASM_INT({
      let inputs = [];
      let outputs = [];
      for (let inpt of globalThis.__libreMidi_access.inputs.values())
      {
        inputs.push(inpt);
      }
      for (let outpt of globalThis.__libreMidi_access.outputs.values())
      {
        outputs.push(outpt);
      }
      globalThis.__libreMidi_currentInputs = inputs;
      globalThis.__libreMidi_currentOutputs = outputs;
    });
    const int inputs = input_count();
    const int outputs = output_count();

    m_current_inputs.resize(inputs);
    m_current_outputs.resize(outputs);

    for (int i = 0; i < inputs; i++)
    {
      int device_index = -1;
      char* midi_id = get_js_string(globalThis.__libreMidi_currentInputs[$0].id, i);
      auto it = m_input_indices.find(midi_id); // TODO transparent comparator, string_view...
      if (it == m_input_indices.end())
      {
        device_index = m_input_indices.size();
        m_input_indices[midi_id] = device_index;
      }
      else
      {
        device_index = it->second;
      }

      char* midi_name = get_js_string(globalThis.__libreMidi_currentInputs[$0].name, i);

      const bool connected
          = EM_ASM_INT(return globalThis.__libreMidi_currentInputs[$0].state === "connected", i);

      m_current_inputs[device_index].id = midi_id;
      m_current_inputs[device_index].name = midi_name;
      m_current_inputs[device_index].connected = connected;

      free(midi_id);
      free(midi_name);
    }

    for (int i = 0; i < outputs; i++)
    {
      int device_index = -1;
      char* midi_id = get_js_string(globalThis.__libreMidi_currentOutputs[$0].id, i);

      auto it = m_output_indices.find(midi_id); // TODO transparent comparator, string_view...
      if (it == m_output_indices.end())
      {
        device_index = m_output_indices.size();
        m_output_indices[midi_id] = device_index;
      }
      else
      {
        device_index = it->second;
      }

      char* midi_name = get_js_string(globalThis.__libreMidi_currentOutputs[$0].name, i);

      const bool connected
          = EM_ASM_INT(return globalThis.__libreMidi_currentOutputs[$0].state === "connected", i);

      m_current_outputs[device_index].id = midi_id;
      m_current_outputs[device_index].name = midi_name;
      m_current_outputs[device_index].connected = connected;

      free(midi_id);
      free(midi_name);
    }

#undef get_js_string
  }

  void register_observer(observer_emscripten& obs)
  {
    m_observers.push_back(&obs);
    if (m_observers.size() == 1)
    {
      start_observing();
    }
  }

  void unregister_observer(observer_emscripten& obs)
  {
    if (m_observers.size() == 1)
    {
      stop_observing();
    }
    auto it = std::find(m_observers.begin(), m_observers.end(), &obs);
    if (it != m_observers.end())
    {
      m_observers.erase(it);
    }
  }

  void devices_poll()
  {
    load_current_infos();

    for (auto& obs : m_observers)
    {
      obs->update(m_current_inputs, m_current_outputs);
    }
  }

  void open_input(int port_index, midi_in_emscripten& input)
  {
    auto& vec = m_opened_inputs[port_index];
    vec.push_back(&input);
    if (vec.size() != 1)
      return;

    start_stream(port_index);
  }

  void close_input(int port_index, midi_in_emscripten& input)
  {
    auto& vec = m_opened_inputs[port_index];
    auto it = std::find(vec.begin(), vec.end(), &input);
    if (it != vec.end())
    {
      vec.erase(it);
    }

    if (vec.empty())
    {
      stop_stream(port_index);
    }
  }

  void devices_input(int port, double timestamp, int len, char* data)
  {
    unsigned char* bytes = reinterpret_cast<unsigned char*>(data);
    for (auto input : m_opened_inputs[port])
    {
      input->on_input(timestamp, bytes, bytes + len);
    }
  }

  stdx::error send_message(int port_index, const char* bytes, int len)
  {
    if (!available())
      return std::errc::operation_not_supported;

    const auto& id = m_current_outputs[port_index].id;
    EM_ASM(
        {
          let data = HEAPU8.subarray($0, $0 + $1);
          const id = UTF8ToString($2);
          let output = globalThis.__libreMidi_access.outputs.get(id);
          let bytes = HEAPU8.subarray($0, $0 + $1);
          output.send(Array.from(bytes));
        },
        bytes, len, id.c_str());

    return stdx::error{};
  }

  const std::vector<device_information>& inputs() const noexcept { return m_current_inputs; }
  const std::vector<device_information>& outputs() const noexcept { return m_current_outputs; }

private:
  midi_access_emscripten() noexcept
  {
    EM_ASM(
        if (navigator.requestMIDIAccess) {
              navigator.requestMIDIAccess().then(
                  (midiAccess) => { globalThis.__libreMidi_access = midiAccess; },
                  () => console.log('MIDI support rejected, MIDI will not be available;'));
        } else { console.log('WebMIDI is not supported in this browser.'); });
  }

  ~midi_access_emscripten() { stop_observing(); }

  void start_observing()
  {
    EM_ASM(
      let id = setInterval(Module._libremidi_devices_poll, 100);
      globalThis.__libreMidi_timer = id;);
  }

  void stop_observing()
  {
    EM_ASM(clearInterval(globalThis.__libreMidi_timer); globalThis.__libreMidi_timer = undefined;);
  }

  void start_stream(int port_index)
  {
    if (!available())
      return;

    // Isn't life great...
    // https://github.com/Planeshifter/emscripten-examples/tree/master/01_PassingArrays
    const auto& id = m_current_inputs[port_index].id;
    EM_ASM(
      const port_index = $0;
      const id = UTF8ToString($1);

      let input = globalThis.__libreMidi_access.inputs.get(id);

      function _arrayToHeap(typedArray){
                      const numBytes = typedArray.length * typedArray.BYTES_PER_ELEMENT;
                      const ptr = Module._malloc(numBytes);
                      const heapBytes = new Uint8Array(Module.HEAPU8.buffer, ptr, numBytes);
                      heapBytes.set(new Uint8Array(typedArray.buffer));
                      return heapBytes;
      }

      function _freeArray(heapBytes){
                      Module._free(heapBytes.byteOffset);
      }

      input.onmidimessage = (message) => {
                          let bytes = message.data;
                          var heapBytes = _arrayToHeap(bytes);
                          Module._libremidi_devices_input(port_index, message.timeStamp, bytes.length, heapBytes.byteOffset);
                          _freeArray(heapBytes);
      };
    , port_index
    , id.c_str()
    );
  }

  void stop_stream(int port_index)
  {
    if (!available())
      return;

    const auto& id = m_current_inputs[port_index].id;
    EM_ASM(const id = UTF8ToString($1);

           let input = globalThis.__libreMidi_access.inputs.get(id);
           input.onmidimessage = undefined;, id.c_str());
  }

  std::vector<observer_emscripten*> m_observers;
  std::vector<device_information> m_current_inputs;
  std::vector<device_information> m_current_outputs;

  std::map<int, std::vector<midi_in_emscripten*>> m_opened_inputs;

  std::map<std::string, int> m_input_indices;
  std::map<std::string, int> m_output_indices;
};

}
}
