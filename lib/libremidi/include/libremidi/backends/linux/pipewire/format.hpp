#pragma once
// SPDX-License-Identifier: BSL-1.0
//
// DMA-BUF / SHM video format negotiation for pw_stream consumers.
// See pipewire/doc/dox/internals/dma-buf.dox for the two-pass
// modifier protocol.

#include <libremidi/backends/linux/pipewire/drm_modifiers.hpp>
#include <libremidi/backends/linux/pipewire/loader.hpp>
#include <libremidi/config.hpp>

#include <spa/param/video/raw.h>
#include <spa/pod/builder.h>
#include <spa/pod/iter.h>
#include <spa/pod/parser.h>
#include <spa/pod/pod.h>

#include <cstdint>
#include <cstring>
#include <span>
#include <vector>

#include <spa/param/video/format-utils.h>

namespace libremidi::pipewire
{

// DRM_FORMAT_MOD_* sentinels from <drm_fourcc.h>, redeclared here
// so consumers don't need to pull in the full DRM headers.
constexpr std::uint64_t mod_linear = 0ULL;
constexpr std::uint64_t mod_invalid = 0x00ffffffffffffffULL;

class format_negotiation
{
public:
  enum class result : std::uint8_t
  {
    // Param wasn't a SPA_PARAM_Format.
    unrelated,
    // Caller must test-allocate then re-announce.
    needs_fixation,
    // Single modifier chosen; use SPA_DATA_DmaBuf.
    dmabuf_fixated,
    // Use SPA_DATA_MemFd | SPA_DATA_MemPtr.
    shm_fallback,
  };

  struct param_changed
  {
    result kind{result::unrelated};
    int width{};
    int height{};
    spa_video_format format{SPA_VIDEO_FORMAT_UNKNOWN};
    std::vector<std::uint64_t> candidate_modifiers;
    std::uint64_t chosen_modifier{};
    int n_planes{1};
  };

  void set_size(int width, int height) noexcept
  {
    m_width = width;
    m_height = height;
  }
  void set_framerate(int num, int den, int max_num = 0) noexcept
  {
    m_fps_num = num;
    m_fps_den = den;
    m_max_fps_num = max_num;
  }
  void set_video_format(spa_video_format f) noexcept { m_format = f; }
  void set_dmabuf_modifiers(std::span<const std::uint64_t> mods)
  {
    m_consumer_modifiers.assign(mods.begin(), mods.end());
  }
  void set_shm_fallback(bool enabled = true) noexcept { m_shm_fallback = enabled; }

  void set_producer_modifiers(std::span<const std::uint64_t> mods)
  {
    m_producer_modifiers.assign(mods.begin(), mods.end());
  }

  // `builder`'s underlying byte buffer must outlive pw_stream_connect.
  std::vector<const spa_pod*>
  build_connect_params(spa_pod_builder& builder, pw_direction direction);

  param_changed on_param_changed(std::uint32_t id, const spa_pod* param);

  std::vector<const spa_pod*>
  build_fixation_params(spa_pod_builder& builder, std::uint64_t chosen);

  const spa_pod*
  build_buffers_param(spa_pod_builder& builder, result kind, int blocks, int size, int stride);

  spa_video_format format() const noexcept { return m_format; }
  int width() const noexcept { return m_width; }
  int height() const noexcept { return m_height; }

private:
  int m_width{0};
  int m_height{0};
  int m_fps_num{30};
  int m_fps_den{1};
  int m_max_fps_num{60};
  spa_video_format m_format{SPA_VIDEO_FORMAT_RGBA};
  std::vector<std::uint64_t> m_consumer_modifiers;
  std::vector<std::uint64_t> m_producer_modifiers;
  bool m_shm_fallback{true};
};

namespace detail
{

inline const spa_pod* build_video_enum_format(
    spa_pod_builder& b, spa_video_format fmt, int width, int height, int fps_num, int fps_den,
    int max_fps_num, std::span<const std::uint64_t> modifiers, bool dont_fixate)
{
  spa_pod_frame frame{};
  spa_pod_builder_push_object(&b, &frame, SPA_TYPE_OBJECT_Format, SPA_PARAM_EnumFormat);

  const spa_rectangle size_rect{
      static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height)};
  const spa_fraction fr{static_cast<std::uint32_t>(fps_num), static_cast<std::uint32_t>(fps_den)};
  const spa_fraction max_fr{
      static_cast<std::uint32_t>(max_fps_num > 0 ? max_fps_num : fps_num),
      static_cast<std::uint32_t>(fps_den)};

  spa_pod_builder_add(
      &b, SPA_FORMAT_mediaType, SPA_POD_Id(SPA_MEDIA_TYPE_video), SPA_FORMAT_mediaSubtype,
      SPA_POD_Id(SPA_MEDIA_SUBTYPE_raw), SPA_FORMAT_VIDEO_format, SPA_POD_Id(fmt),
      SPA_FORMAT_VIDEO_size, SPA_POD_Rectangle(&size_rect), SPA_FORMAT_VIDEO_framerate,
      SPA_POD_Fraction(&fr), SPA_FORMAT_VIDEO_maxFramerate, SPA_POD_Fraction(&max_fr), 0);

  if (!modifiers.empty())
  {
    // First pass uses MANDATORY | DONT_FIXATE ("pick one"); the
    // fixation pass drops DONT_FIXATE and emits a single value.
    std::uint32_t prop_flags = SPA_POD_PROP_FLAG_MANDATORY;
    if (dont_fixate)
      prop_flags |= SPA_POD_PROP_FLAG_DONT_FIXATE;
    spa_pod_builder_prop(&b, SPA_FORMAT_VIDEO_modifier, prop_flags);

    if (modifiers.size() > 1 && dont_fixate)
    {
      spa_pod_frame choice_frame{};
      spa_pod_builder_push_choice(&b, &choice_frame, SPA_CHOICE_Enum, 0);
      // SPA_CHOICE_Enum: first value is the default, then alternatives.
      spa_pod_builder_long(&b, static_cast<std::int64_t>(modifiers[0]));
      for (auto m : modifiers)
        spa_pod_builder_long(&b, static_cast<std::int64_t>(m));
      spa_pod_builder_pop(&b, &choice_frame);
    }
    else
    {
      spa_pod_builder_long(&b, static_cast<std::int64_t>(modifiers[0]));
    }
  }

  return static_cast<const spa_pod*>(spa_pod_builder_pop(&b, &frame));
}

// Returns false when no modifier prop is present (SHM fallback).
inline bool extract_modifier_choice(
    const spa_pod* fmt_obj, std::uint64_t& out_single, std::vector<std::uint64_t>& out_choices,
    bool& out_dont_fixate)
{
  out_single = 0;
  out_choices.clear();
  out_dont_fixate = false;
  if (!fmt_obj || !spa_pod_is_object(fmt_obj))
    return false;

  const auto* obj = reinterpret_cast<const spa_pod_object*>(fmt_obj);
  const spa_pod_prop* prop = spa_pod_object_find_prop(obj, nullptr, SPA_FORMAT_VIDEO_modifier);
  if (!prop)
    return false;

  out_dont_fixate = (prop->flags & SPA_POD_PROP_FLAG_DONT_FIXATE) != 0;
  const spa_pod* val = &prop->value;

  if (spa_pod_is_choice(val))
  {
    const auto* choice = reinterpret_cast<const spa_pod_choice*>(val);
    const spa_pod* child = SPA_POD_CHOICE_CHILD(reinterpret_cast<const spa_pod_choice*>(val));
    const std::uint32_t child_size = SPA_POD_BODY_SIZE(child);
    const std::uint32_t n_values
        = (SPA_POD_BODY_SIZE(val) - sizeof(spa_pod_choice_body)) / child_size;
    const std::uint8_t* p = static_cast<const std::uint8_t*>(SPA_POD_CONTENTS(spa_pod_choice, val))
                            + sizeof(spa_pod_choice_body);
    for (std::uint32_t i = 0; i < n_values; ++i)
    {
      std::int64_t v;
      std::memcpy(&v, p, sizeof(v));
      out_choices.push_back(static_cast<std::uint64_t>(v));
      p += child_size;
    }
    (void)choice;
    return true;
  }

  if (spa_pod_is_long(val))
  {
    std::int64_t v = 0;
    spa_pod_get_long(val, &v);
    out_single = static_cast<std::uint64_t>(v);
    return true;
  }

  return false;
}

} // namespace detail

inline std::vector<const spa_pod*>
format_negotiation::build_connect_params(spa_pod_builder& builder, pw_direction /*direction*/)
{
  std::vector<const spa_pod*> out;
  out.reserve(2);

  if (!m_consumer_modifiers.empty())
  {
    if (auto* p = detail::build_video_enum_format(
            builder, m_format, m_width, m_height, m_fps_num, m_fps_den, m_max_fps_num,
            std::span<const std::uint64_t>(m_consumer_modifiers),
            /*dont_fixate=*/true))
      out.push_back(p);
  }

  if (m_shm_fallback)
  {
    if (auto* p = detail::build_video_enum_format(
            builder, m_format, m_width, m_height, m_fps_num, m_fps_den, m_max_fps_num,
            std::span<const std::uint64_t>{}, // no modifiers → SHM
            /*dont_fixate=*/false))
      out.push_back(p);
  }

  // Unconfigured consumer: fall back to a LINEAR-only EnumFormat.
  if (out.empty())
  {
    static constexpr std::uint64_t linear_only[]{0ULL};
    if (auto* p = detail::build_video_enum_format(
            builder, m_format, m_width, m_height, m_fps_num, m_fps_den, m_max_fps_num,
            std::span<const std::uint64_t>(linear_only, 1),
            /*dont_fixate=*/false))
      out.push_back(p);
  }

  return out;
}

inline format_negotiation::param_changed
format_negotiation::on_param_changed(std::uint32_t id, const spa_pod* param)
{
  param_changed pc;
  if (id != SPA_PARAM_Format || !param)
    return pc;

  spa_video_info_raw info{};
  if (spa_format_video_raw_parse(param, &info) < 0)
    return pc;
  pc.width = static_cast<int>(info.size.width);
  pc.height = static_cast<int>(info.size.height);
  pc.format = info.format;

  std::uint64_t single{};
  bool dont_fixate = false;
  const bool has_modifier_prop
      = detail::extract_modifier_choice(param, single, pc.candidate_modifiers, dont_fixate);

  if (!has_modifier_prop)
  {
    // No modifier prop -> producer wants SHM.
    pc.kind = result::shm_fallback;
    pc.chosen_modifier = mod_invalid;
    pc.n_planes = 1;
    return pc;
  }

  if (dont_fixate && !pc.candidate_modifiers.empty())
  {
    pc.kind = result::needs_fixation;
    return pc;
  }

  if (!pc.candidate_modifiers.empty())
    single = pc.candidate_modifiers.front();
  pc.chosen_modifier = single;
  pc.kind = result::dmabuf_fixated;
  pc.n_planes = static_cast<int>(drm_modifier_plane_count(0, single));
  pc.candidate_modifiers.clear();
  return pc;
}

inline std::vector<const spa_pod*>
format_negotiation::build_fixation_params(spa_pod_builder& builder, std::uint64_t chosen)
{
  std::vector<const spa_pod*> out;
  const std::uint64_t mods[]{chosen};
  if (auto* p = detail::build_video_enum_format(
          builder, m_format, m_width, m_height, m_fps_num, m_fps_den, m_max_fps_num,
          std::span<const std::uint64_t>(mods, 1),
          /*dont_fixate=*/false))
    out.push_back(p);
  return out;
}

inline const spa_pod* format_negotiation::build_buffers_param(
    spa_pod_builder& builder, result kind, int blocks, int size, int stride)
{
  std::uint32_t data_type{};
  switch (kind)
  {
    case result::dmabuf_fixated:
      data_type = (1u << SPA_DATA_DmaBuf);
      break;
    case result::shm_fallback:
      data_type = (1u << SPA_DATA_MemFd) | (1u << SPA_DATA_MemPtr);
      break;
    default:
      data_type = (1u << SPA_DATA_MemPtr);
      break;
  }
  // size/stride are RANGES with our tight computation as the minimum:
  // producers commonly deliver row-padded buffers (GPU-allocated strides),
  // and a fixed Int here fails the param intersection against any producer
  // whose size differs — killing the link instead of accepting padding.
  return reinterpret_cast<const spa_pod*>(spa_pod_builder_add_object(
      &builder, SPA_TYPE_OBJECT_ParamBuffers, SPA_PARAM_Buffers, SPA_PARAM_BUFFERS_buffers,
      SPA_POD_CHOICE_RANGE_Int(8, 2, 16), SPA_PARAM_BUFFERS_blocks, SPA_POD_Int(blocks),
      SPA_PARAM_BUFFERS_size, SPA_POD_CHOICE_RANGE_Int(size, size, INT32_MAX),
      SPA_PARAM_BUFFERS_stride, SPA_POD_CHOICE_RANGE_Int(stride, stride, INT32_MAX),
      SPA_PARAM_BUFFERS_dataType, SPA_POD_CHOICE_FLAGS_Int(data_type)));
}

} // namespace libremidi::pipewire
