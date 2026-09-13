#pragma once
#include <libremidi/config.hpp>

#include <pipewire/keys.h>
#include <pipewire/port.h>
#include <spa/utils/dict.h>

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace libremidi::pipewire
{
enum class connection_state : std::uint8_t
{
  connecting,
  connected,
  broken,
  disconnected,
};
enum class media_class : std::uint8_t
{
  other = 0,
  audio,
  midi,
  ump,
  video,
};

constexpr bool is_audio(media_class c) noexcept
{
  return c == media_class::audio;
}
constexpr bool is_midi_like(media_class c) noexcept
{
  return c == media_class::midi || c == media_class::ump;
}
constexpr bool is_video(media_class c) noexcept
{
  return c == media_class::video;
}

inline media_class classify_format_dsp(std::string_view fmt) noexcept
{
  if (fmt.empty())
    return media_class::other;

  if (fmt.find("UMP") != std::string_view::npos)
    return media_class::ump;
  if (fmt.find("ump") != std::string_view::npos)
    return media_class::ump;
  if (fmt.find("midi") != std::string_view::npos || fmt.find("Midi") != std::string_view::npos
      || fmt.find("MIDI") != std::string_view::npos)
    return media_class::midi;

  if (fmt.find("audio") != std::string_view::npos || fmt.find("Audio") != std::string_view::npos)
    return media_class::audio;

  if (fmt.find("video") != std::string_view::npos || fmt.find("Video") != std::string_view::npos)
    return media_class::video;
  return media_class::other;
}

inline media_class classify_media_class(std::string_view media_class_str) noexcept
{
  if (media_class_str.empty())
    return media_class::other;

  constexpr std::string_view stream_prefix = "Stream/";
  std::string_view rest = media_class_str;
  if (rest.size() > stream_prefix.size() && rest.substr(0, stream_prefix.size()) == stream_prefix)
  {
    // Strip "Stream/<Input|Output>/".
    rest.remove_prefix(stream_prefix.size());
    if (auto slash = rest.find('/'); slash != std::string_view::npos)
      rest = rest.substr(slash + 1);
  }
  if (rest.find("Audio") != std::string_view::npos)
    return media_class::audio;

  if (rest.find("Video") != std::string_view::npos)
    return media_class::video;

  if (rest.find("UMP") != std::string_view::npos || rest.find("ump") != std::string_view::npos)
    return media_class::ump;

  if (rest.find("Midi") != std::string_view::npos || rest.find("midi") != std::string_view::npos
      || rest.find("MIDI") != std::string_view::npos)
    return media_class::midi;
  return media_class::other;
}

inline std::string_view dict_get(const spa_dict* dict, const char* key) noexcept
{
  if (!dict || !key)
    return {};
  const char* v = spa_dict_lookup(dict, key);
  return v ? std::string_view{v} : std::string_view{};
}

inline media_class classify_node_props(const spa_dict* props) noexcept
{
  return classify_media_class(dict_get(props, PW_KEY_MEDIA_CLASS));
}

inline media_class classify_port_props(const spa_dict* props) noexcept
{
  if (dict_get(props, "control.ump") == "true")
    return media_class::ump;
  return classify_format_dsp(dict_get(props, "format.dsp"));
}

struct port_info
{
  std::uint32_t id{};

  std::uint32_t node_id{};

  std::string format;

  media_class kind{media_class::other};

  std::string port_name;   ///< `PW_KEY_PORT_NAME`
  std::string port_alias;  ///< `PW_KEY_PORT_ALIAS`
  std::string object_path; ///< `PW_KEY_OBJECT_PATH`
  std::string port_id;     ///< `PW_KEY_PORT_ID` (stringified)

  bool physical{}; ///< `port.physical = true`
  bool terminal{}; ///< `port.terminal = true`
  bool monitor{};  ///< `port.monitor  = true`

  int direction{}; /// < `SPA_DIRECTION_INPUT=0, _OUTPUT=1`
};

struct node_info
{
  std::uint32_t id{};

  std::string name;            ///< `PW_KEY_NODE_NAME`
  std::string description;     ///< `PW_KEY_NODE_DESCRIPTION`
  std::string media_class_str; ///< `PW_KEY_MEDIA_CLASS` raw string
  std::string media_role;      ///< `PW_KEY_MEDIA_ROLE`

  media_class kind{media_class::other};

  // True if any of this node's ports has `port.physical = true`
  bool physical{};

  std::vector<port_info> inputs;
  std::vector<port_info> outputs;
};

struct graph_snapshot
{
  std::vector<node_info> nodes;

  std::vector<node_info> nodes_of(media_class c) const
  {
    std::vector<node_info> out;
    out.reserve(nodes.size());
    for (const auto& n : nodes)
      if (n.kind == c)
        out.push_back(n);
    return out;
  }

  const node_info* find_by_name(std::string_view name) const noexcept
  {
    for (const auto& n : nodes)
      if (n.name == name)
        return &n;
    return nullptr;
  }

  const node_info* find_by_id(std::uint32_t id) const noexcept
  {
    for (const auto& n : nodes)
      if (n.id == id)
        return &n;
    return nullptr;
  }
};

}
