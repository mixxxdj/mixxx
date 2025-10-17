// Mixxx control types
// Generated file, don't change anything by hand

declare namespace MixxxControls {
                
   type AppControls = 
      | "indicator_250ms"
      | "indicator_500ms"
      | "gui_tick_50ms_period_s"
      | "gui_tick_full_period_s"
      | "samplerate"
      | "output_latency_ms"
      | "audio_latency_overload_count"
      | "audio_latency_usage"
      | "keylock_engine"
      | "num_decks"
      | "num_samplers"
      | "num_preview_decks"
      | "num_microphones"
      | "num_auxiliaries"
   ;

   type groupControls = 
      | "scratch_position_enable"
      | "scratch_position"
      | "rate_ratio"
      | "rate_dir"
      | "rateRange"
      | "rate"
      | "rateSearch"
      | "rate_temp_down"
      | "rate_temp_down_small"
      | "rate_temp_up"
      | "rate_temp_up_small"
      | "rate_perm_down"
      | "rate_perm_down_small"
      | "rate_perm_up"
      | "rate_perm_up_small"
      | "reverse"
      | "reverseroll"
      | "fwd"
      | "back"
      | "wheel"
      | "scratch2"
      | "scratch2_enable"
      | "jog"
      | "play"
      | "input_configured"
      | "vinylcontrol_status"
      | "vinylcontrol_speed_type"
      | "vinylcontrol_seek"
      | "vinylcontrol_rate"
      | "vinylcontrol_scratching"
      | "vinylcontrol_enabled"
      | "vinylcontrol_wantenabled"
      | "vinylcontrol_mode"
      | "vinylcontrol_cueing"
      | "vinylcontrol_signal_enabled"
   ;

   type stemGroupControls = 
      | "color"
   ;

   type kWaveformGroupControls = 
      | "draw_overview_minute_markers"
      | "beatGridAlpha"
   ;

   namespace ReadOnly {

   }

   namespace Deprecated {

      type MasterControls = 
         | "indicator_250millis"
         | "indicator_500millis"
         | "latency"
         | "audio_latency_usage"
         | "audio_latency_overload"
         | "num_decks"
         | "num_samplers"
         | "num_preview_decks"
         | "num_microphones"
         | "num_auxiliaries"
         | "guiTickTime"
         | "guiTick50ms"
      ;

      type m_groupControls = 
         | "next_chain"
         | "prev_chain"
         | "chain_selector"
      ;

      type groupControls = 
         | "samplerate"
         | "volume"
         | "headVolume"
         | "reloop_exit"
      ;

   }

}