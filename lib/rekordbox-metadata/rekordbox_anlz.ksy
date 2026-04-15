meta:
  id: rekordbox_anlz
  title: rekordbox track analysis file
  application: rekordbox
  file-extension:
    - dat
    - ext
  license: EPL-1.0
  endian: be

doc: |
  These files are created by rekordbox when analyzing audio tracks
  to facilitate DJ performance. They include waveforms, beat grids
  (information about the precise time at which each beat occurs),
  time indices to allow efficient seeking to specific positions
  inside variable bit-rate audio streams, and lists of memory cues
  and loop points. They are used by Pioneer professional DJ
  equipment.

  The format has been reverse-engineered to facilitate sophisticated
  integrations with light and laser shows, videos, and other musical
  instruments, by supporting deep knowledge of what is playing and
  what is coming next through monitoring the network communications
  of the players.

doc-ref: https://reverseengineering.stackexchange.com/questions/4311/help-reversing-a-edb-database-file-for-pioneers-rekordbox-software

seq:
  - id: magic
    contents: "PMAI"
    doc: Identifies this as an analysis file.
  - id: len_header
    type: u4
    doc: |
      The number of bytes of this header section.
  - id: len_file
    type: u4
    doc: |
       The number of bytes in the entire file.
  - size: len_header - _io.pos
  - id: sections
    type: tagged_section
    repeat: eos
    doc: |
      The remainder of the file is a sequence of type-tagged sections,
      identified by a four-byte magic sequence.

types:
  tagged_section:
    doc: |
      A type-tagged file section, identified by a four-byte magic
      sequence, with a header specifying its length, and whose payload
      is determined by the type tag.
    seq:
      - id: fourcc
        type: s4
        enum: section_tags
        doc: |
          A tag value indicating what kind of section this is.
      - id: len_header
        type: u4
        doc: |
          The size, in bytes, of the header portion of the tag.
      - id: len_tag
        type: u4
        doc: |
          The size, in bytes, of this entire tag, counting the header.
      - id: body
        size: len_tag - 12
        type:
          switch-on: fourcc
          cases:
            'section_tags::cues_2': cue_extended_tag                    # PCO2
            'section_tags::cues': cue_tag                               # PCOB
            'section_tags::path': path_tag                              # PPTH
            'section_tags::beat_grid': beat_grid_tag                    # PQTZ
            'section_tags::vbr': vbr_tag                                # PVBR
            'section_tags::wave_preview': wave_preview_tag              # PWAV
            'section_tags::wave_tiny': wave_preview_tag                 # PWV2
            'section_tags::wave_scroll': wave_scroll_tag                # PWV3, seen in .EXT
            'section_tags::wave_color_preview': wave_color_preview_tag  # PWV4, in .EXT
            'section_tags::wave_color_scroll': wave_color_scroll_tag    # PWV5, in .EXT
            'section_tags::wave_3band_preview': wave_3band_preview_tag  # P@V6, in .2EX
            'section_tags::wave_3band_scroll': wave_3band_scroll_tag    # PWV7, in .2EX
            'section_tags::song_structure': song_structure_tag          # PSSI, in .EXT
            _: unknown_tag
    -webide-representation: '{fourcc}'


  beat_grid_tag:
    doc: |
      Holds a list of all the beats found within the track, recording
      their bar position, the time at which they occur, and the tempo
      at that point.
    seq:
      - type: u4
      - type: u4  # @flesniak says this is always 0x80000
      - id: num_beats
        type: u4
        doc: |
          The number of beat entries which follow.
      - id: beats
        type: beat_grid_beat
        repeat: expr
        repeat-expr: num_beats
        doc: The entries of the beat grid.

  beat_grid_beat:
    doc: |
      Describes an individual beat in a beat grid.
    seq:
      - id: beat_number
        type: u2
        doc: |
          The position of the beat within its musical bar, where beat 1
          is the down beat.
      - id: tempo
        type: u2
        doc: |
          The tempo at the time of this beat, in beats per minute,
          multiplied by 100.
      - id: time
        type: u4
        doc: |
          The time, in milliseconds, at which this beat occurs when
          the track is played at normal (100%) pitch.

  cue_tag:
    doc: |
      Stores either a list of ordinary memory cues and loop points, or
      a list of hot cues and loop points.
    seq:
      - id: type
        type: u4
        enum: cue_list_type
        doc: |
          Identifies whether this tag stores ordinary or hot cues.
      - size: 2
      - id: num_cues
        type: u2
        doc: |
          The length of the cue list.
      - id: memory_count
        type: u4
        doc: |
          Unsure what this means.
      - id: cues
        type: cue_entry
        repeat: expr
        repeat-expr: num_cues

  cue_entry:
    doc: |
      A cue list entry. Can either represent a memory cue or a loop.
    seq:
      - id: magic
        contents: "PCPT"
        doc: Identifies this as a cue list entry (cue point).
      - id: len_header
        type: u4
      - id: len_entry
        type: u4
      - id: hot_cue
        type: u4
        doc: |
          If zero, this is an ordinary memory cue, otherwise this a
          hot cue with the specified number.
      - id: status
        type: u4
        enum: cue_entry_status
        doc: |
          Indicates if this is an active loop.
      - type: u4  # Seems to always be 0x10000
      - id: order_first
        type: u2
        doc: |
          @flesniak says: "0xffff for first cue, 0,1,3 for next"
      - id: order_last
        type: u2
        doc: |
          @flesniak says: "1,2,3 for first, second, third cue, 0xffff for last"
      - id: type
        type: u1
        enum: cue_entry_type
        doc: |
          Indicates whether this is a memory cue or a loop.
      - size: 3  # seems to always be 1000
      - id: time
        type: u4
        doc: |
          The position, in milliseconds, at which the cue point lies
          in the track.
      - id: loop_time
        type: u4
        doc: |
          The position, in milliseconds, at which the player loops
          back to the cue time if this is a loop.
      - size: 16

  cue_extended_tag:
    doc: |
      A variation of cue_tag which was introduced with the nxs2 line,
      and adds descriptive names. (Still comes in two forms, either
      holding memory cues and loop points, or holding hot cues and
      loop points.) Also includes hot cues D through H and color assignment.
    seq:
      - id: type
        type: u4
        enum: cue_list_type
        doc: |
          Identifies whether this tag stores ordinary or hot cues.
      - id: num_cues
        type: u2
        doc: |
          The length of the cue comment list.
      - size: 2
      - id: cues
        type: cue_extended_entry
        repeat: expr
        repeat-expr: num_cues

  cue_extended_entry:
    doc: |
      A cue extended list entry. Can either describe a memory cue or a
      loop.
    seq:
      - id: magic
        contents: "PCP2"
        doc: Identifies this as an extended cue list entry (cue point).
      - id: len_header
        type: u4
      - id: len_entry
        type: u4
      - id: hot_cue
        type: u4
        doc: |
          If zero, this is an ordinary memory cue, otherwise this a
          hot cue with the specified number.
      - id: type
        type: u1
        enum: cue_entry_type
        doc: |
          Indicates whether this is a regular cue point or a loop.
      - size: 3  # seems to always be 1000
      - id: time
        type: u4
        doc: |
          The position, in milliseconds, at which the cue point lies
          in the track.
      - id: loop_time
        type: u4
        doc: |
          The position, in milliseconds, at which the player loops
          back to the cue time if this is a loop.
      - id: color_id
        type: u1
        doc: |
          References a row in the colors table if this is a memory cue or loop
          and has been assigned a color.
      - size: 7
      - id: loop_numerator
        type: u2
        doc: |
          The numerator of the loop length in beats.
          Zero if the loop is not quantized.
      - id: loop_denominator
        type: u2
        doc: |
          The denominator of the loop length in beats.
          Zero if the loop is not quantized.
      - id: len_comment
        type: u4
        if: len_entry > 43
      - id: comment
        type: str
        size: len_comment
        encoding: utf-16be
        doc: |
          The comment assigned to this cue by the DJ, if any, with a trailing NUL.
        if: len_entry > 43
      - id: color_code
        type: u1
        doc: |
          A lookup value for a color table? We use this to index to the hot cue colors shown in rekordbox.
        if: (len_entry - len_comment) > 44
      - id: color_red
        type: u1
        doc: |
          The red component of the hot cue color to be displayed.
        if: (len_entry - len_comment) > 45
      - id: color_green
        type: u1
        doc: |
          The green component of the hot cue color to be displayed.
        if: (len_entry - len_comment) > 46
      - id: color_blue
        type: u1
        doc: |
          The blue component of the hot cue color to be displayed.
        if: (len_entry - len_comment) > 47
      - size: len_entry - 48 - len_comment  # The remainder after the color
        if: (len_entry - len_comment) > 48

  path_tag:
    doc: |
      Stores the file path of the audio file to which this analysis
      applies.
    seq:
      - id: len_path
        type: u4
      - id: path
        type: str
        size: len_path - 2
        encoding: utf-16be
        if: len_path > 1
    -webide-representation: '{path}'

  vbr_tag:
    doc: |
      Stores an index allowing rapid seeking to particular times
      within a variable-bitrate audio file.
    seq:
      - type: u4
      - id: index
        type: u4
        repeat: expr
        repeat-expr: 400

  wave_preview_tag:
    doc: |
      Stores a waveform preview image suitable for display above
      the touch strip for jumping to a track position.
    seq:
      - id: len_data
        type: u4
        doc: |
          The length, in bytes, of the preview data itself. This is
          slightly redundant because it can be computed from the
          length of the tag.
      - type: u4  # This seems to always have the value 0x10000
      - id: data
        size: len_data
        doc: |
          The actual bytes of the waveform preview.
        if: _parent.len_tag > _parent.len_header

  wave_scroll_tag:
    doc: |
      A larger waveform image suitable for scrolling along as a track
      plays.
    seq:
      - id: len_entry_bytes
        type: u4
        doc: |
          The size of each entry, in bytes. Seems to always be 1.
      - id: len_entries
        type: u4
        doc: |
          The number of waveform data points, each of which takes one
          byte.
      - type: u4  # Always 0x960000?
      - id: entries
        size: len_entries * len_entry_bytes

  wave_color_preview_tag:
    doc: |
      A larger, colorful waveform preview image suitable for display
      above the touch strip for jumping to a track position on newer
      high-resolution players.
    seq:
      - id: len_entry_bytes
        type: u4
        doc: |
          The size of each entry, in bytes. Seems to always be 6.
      - id: len_entries
        type: u4
        doc: |
          The number of waveform data points, each of which takes one
          byte for each of six channels of information.
      - type: u4
      - id: entries
        size: len_entries * len_entry_bytes

  wave_color_scroll_tag:
    doc: |
      A larger, colorful waveform image suitable for scrolling along
      as a track plays on newer high-resolution hardware. Also
      contains a higher-resolution blue/white waveform.
    seq:
      - id: len_entry_bytes
        type: u4
        doc: |
          The size of each entry, in bytes. Seems to always be 2.
      - id: len_entries
        type: u4
        doc: |
          The number of columns of waveform data (this matches the
          non-color waveform length.
      - type: u4
      - id: entries
        size: len_entries * len_entry_bytes

  wave_3band_preview_tag:
    doc: |
      The minimalist CDJ-3000 waveform preview image suitable for display
      above the touch strip for jumping to a track position.
    seq:
      - id: len_entry_bytes
        type: u4
        doc: |
          The size of each entry, in bytes. Seems to always be 3.
      - id: len_entries
        type: u4
        doc: |
          The number of waveform data points, each of which takes one
          byte for each of six channels of information.
      - id: entries
        size: len_entries * len_entry_bytes

  wave_3band_scroll_tag:
    doc: |
      The minimalist CDJ-3000 waveform image suitable for scrolling along
      as a track plays on newer high-resolution hardware.
    seq:
      - id: len_entry_bytes
        type: u4
        doc: |
          The size of each entry, in bytes. Seems to always be 3.
      - id: len_entries
        type: u4
        doc: |
          The number of columns of waveform data (this matches the
          non-color waveform length.
      - type: u4
      - id: entries
        size: len_entries * len_entry_bytes

  song_structure_tag:
    doc: |
      Stores the song structure, also known as phrases (intro, verse,
      bridge, chorus, up, down, outro).
    seq:
      - id: len_entry_bytes
        type: u4
        doc: |
          The size of each entry, in bytes. Seems to always be 24.
      - id: len_entries
        type: u2
        doc: |
          The number of phrases.
      - id: body
        type: song_structure_body
        doc: |
          The rest of the tag, which needs to be unmasked before it
          can be parsed.
        size-eos: true
        process:  'xor(is_masked ? mask : [0])'
    instances:
      c:
        value: len_entries
      mask:
        value: |
          [
            (0xCB+c).as<s1>, (0xE1+c).as<s1>, (0xEE+c).as<s1>, (0xFA+c).as<s1>, (0xE5+c).as<s1>, (0xEE+c).as<s1>, (0xAD+c).as<s1>, (0xEE+c).as<s1>,
            (0xE9+c).as<s1>, (0xD2+c).as<s1>, (0xE9+c).as<s1>, (0xEB+c).as<s1>, (0xE1+c).as<s1>, (0xE9+c).as<s1>, (0xF3+c).as<s1>, (0xE8+c).as<s1>,
            (0xE9+c).as<s1>, (0xF4+c).as<s1>, (0xE1+c).as<s1>
          ].as<bytes>
      raw_mood:
        type: u2
        pos: 6
        doc: |
          This is a way to tell whether the rest of the tag has been masked. The value is supposed
          to range from 1 to 3, but in masked files it will be much larger.
      is_masked:
        value: 'raw_mood > 20'  # This is almost certainly not true for an unmasked file.
    -webide-representation: '{body.mood}'


  song_structure_body:
    doc: |
      Stores the rest of the song structure tag, which can only be
      parsed after unmasking.
    seq:
      - id: mood
        type: u2
        enum: track_mood
        doc: |
          The mood which rekordbox assigns the track as a whole during phrase analysis.
      - size: 6
      - id: end_beat
        type: u2
        doc: |
          The beat number at which the last phrase ends. The track may
          continue after the last phrase ends. If this is the case, it will
          mostly be silence.
      - size: 2
      - id: raw_bank
        type: u1
        doc: |
          Number identifying a stylistic bank which can be assigned to the track in rekordbox Lighting mode.
      - size: 1
      - id: entries
        type: song_structure_entry
        repeat: expr
        repeat-expr: _parent.len_entries
    instances:
      bank:
        value: raw_bank
        enum: track_bank
        doc: |
          The stylistic bank which can be assigned to the track in rekordbox Lighting mode, if raw_bank has a legal value.
        if: 'raw_bank < 9'


  song_structure_entry:
    doc: |
      A song structure entry, represents a single phrase.
    seq:
      - id: index
        type: u2
        doc: |
          The absolute number of the phrase, starting at one.
      - id: beat
        type: u2
        doc: |
          The beat number at which the phrase starts.
      - id: kind
        type:
          switch-on: _parent.mood
          cases:
            'track_mood::high': phrase_high
            'track_mood::mid': phrase_mid
            'track_mood::low': phrase_low
            _: phrase_mid  # We don't recognize this mood, so pick a generic interpretation.
        doc: |
          The kind of phrase as displayed in rekordbox.
      - size: 1
      - id: k1
        type: u1
        doc: One of three flags that identify phrase kind variants in high-mood tracks.
      - size: 1
      - id: k2
        type: u1
        doc: One of three flags that identify phrase kind variants in high-mood tracks.
      - size: 1
      - id: b
        type: u1
        doc: |
          Flags how many more beat numbers are in a high-mood "Up 3" phrase.
      - id: beat2
        type: u2
        doc: |
          Extra beat number (falling within phrase) always present in high-mood "Up 3" phrases.
      - id: beat3
        type: u2
        doc: |
          Extra beat number (falling within phrase, larger than beat2)
          present in high-mood "Up 3" phrases when b has value 1.
      - id: beat4
        type: u2
        doc: |
          Extra beat number (falling within phrase, larger than beat3)
          present in high-mood "Up 3" phrases when b has value 1.
      - size: 1
      - id: k3
        type: u1
        doc: One of three flags that identify phrase kind variants in high-mood tracks.
      - size: 1
      - id: fill
        type: u1
        doc: |
          If nonzero, fill-in is present at end of phrase.
      - id: beat_fill
        type: u2
        doc: |
          The beat number at which fill-in starts.
    -webide-representation: '{kind.id}'

  phrase_high:
    seq:
      - id: id
        type: u2
        enum: mood_high_phrase

  phrase_mid:
    seq:
      - id: id
        type: u2
        enum: mood_mid_phrase

  phrase_low:
    seq:
      - id: id
        type: u2
        enum: mood_low_phrase

  unknown_tag: {}

enums:
  section_tags:
    0x50434f42: cues                # PCOB
    0x50434f32: cues_2              # PCO2 (seen in .EXT)
    0x50505448: path                # PPTH
    0x50564252: vbr                 # PVBR
    0x5051545a: beat_grid           # PQTZ
    0x50574156: wave_preview        # PWAV
    0x50575632: wave_tiny           # PWV2
    0x50575633: wave_scroll         # PWV3 (seen in .EXT)
    0x50575634: wave_color_preview  # PWV4 (seen in .EXT)
    0x50575635: wave_color_scroll   # PWV5 (seen in .EXT)
    0x50535349: song_structure      # PSSI (seen in .EXT)
    0x50575636: wave_3band_preview  # PWV6 (seen in .2EX)
    0x50575637: wave_3band_scroll   # PWV7 (seen in .2EX)

  cue_list_type:
    0: memory_cues
    1: hot_cues

  cue_entry_type:
    1: memory_cue
    2: loop

  cue_entry_status:
    0: disabled
    1: enabled
    4: active_loop

  track_mood:
    1: high
    2: mid
    3: low

  mood_low_phrase:
    1: intro
    2: verse_1
    3: verse_1b  # Just displayed as "Verse 1" in rekordbox.
    4: verse_1c  # Just displayed as "Verse 1" in rekordbox.
    5: verse_2
    6: verse_2b  # Just displayed as "Verse 2" in rekordbox.
    7: verse_2c  # Just displayed as "Verse 2" in rekordbox.
    8: bridge
    9: chorus
    10: outro

  mood_mid_phrase:
    1: intro
    2: verse_1
    3: verse_2
    4: verse_3
    5: verse_4
    6: verse_5
    7: verse_6
    8: bridge
    9: chorus
    10: outro

  mood_high_phrase:
    1: intro
    2: up
    3: down
    5: chorus
    6: outro

  track_bank:
    0: default
    1: cool
    2: natural
    3: hot
    4: subtle
    5: warm
    6: vivid
    7: club_1
    8: club_2
