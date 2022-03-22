#pragma once

// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "kaitaistruct.h"

#include <stdint.h>
#include <vector>

#if KAITAI_STRUCT_VERSION < 7000L
#error "Incompatible Kaitai Struct C++/STL API: version 0.7 or later is required"
#endif

/**
 * These files are created by rekordbox when analyzing audio tracks
 * to facilitate DJ performance. They include waveforms, beat grids
 * (information about the precise time at which each beat occurs),
 * time indices to allow efficient seeking to specific positions
 * inside variable bit-rate audio streams, and lists of memory cues
 * and loop points. They are used by Pioneer professional DJ
 * equipment.
 *
 * The format has been reverse-engineered to facilitate sophisticated
 * integrations with light and laser shows, videos, and other musical
 * instruments, by supporting deep knowledge of what is playing and
 * what is coming next through monitoring the network communications
 * of the players.
 * \sa Source
 */

class rekordbox_anlz_t : public kaitai::kstruct {

public:
    class phrase_up_down_t;
    class path_tag_t;
    class wave_preview_tag_t;
    class beat_grid_tag_t;
    class wave_color_preview_tag_t;
    class wave_scroll_tag_t;
    class phrase_verse_bridge_t;
    class song_structure_tag_t;
    class cue_extended_entry_t;
    class vbr_tag_t;
    class song_structure_entry_t;
    class cue_entry_t;
    class beat_grid_beat_t;
    class cue_extended_tag_t;
    class unknown_tag_t;
    class tagged_section_t;
    class wave_color_scroll_tag_t;
    class cue_tag_t;

    enum cue_entry_status_t {
        CUE_ENTRY_STATUS_DISABLED = 0,
        CUE_ENTRY_STATUS_ENABLED = 1
    };

    enum cue_list_type_t {
        CUE_LIST_TYPE_MEMORY_CUES = 0,
        CUE_LIST_TYPE_HOT_CUES = 1
    };

    enum phrase_style_t {
        PHRASE_STYLE_UP_DOWN = 1,
        PHRASE_STYLE_VERSE_BRIDGE = 2,
        PHRASE_STYLE_VERSE_BRIDGE_2 = 3
    };

    enum cue_entry_type_t {
        CUE_ENTRY_TYPE_MEMORY_CUE = 1,
        CUE_ENTRY_TYPE_LOOP = 2
    };

    enum section_tags_t {
        SECTION_TAGS_CUES_2 = 1346588466,
        SECTION_TAGS_CUES = 1346588482,
        SECTION_TAGS_PATH = 1347441736,
        SECTION_TAGS_BEAT_GRID = 1347507290,
        SECTION_TAGS_SONG_STRUCTURE = 1347638089,
        SECTION_TAGS_VBR = 1347830354,
        SECTION_TAGS_WAVE_PREVIEW = 1347895638,
        SECTION_TAGS_WAVE_TINY = 1347900978,
        SECTION_TAGS_WAVE_SCROLL = 1347900979,
        SECTION_TAGS_WAVE_COLOR_PREVIEW = 1347900980,
        SECTION_TAGS_WAVE_COLOR_SCROLL = 1347900981
    };

    enum phrase_verse_bridge_id_t {
        PHRASE_VERSE_BRIDGE_ID_INTRO = 1,
        PHRASE_VERSE_BRIDGE_ID_VERSE1 = 2,
        PHRASE_VERSE_BRIDGE_ID_VERSE2 = 3,
        PHRASE_VERSE_BRIDGE_ID_VERSE3 = 4,
        PHRASE_VERSE_BRIDGE_ID_VERSE4 = 5,
        PHRASE_VERSE_BRIDGE_ID_VERSE5 = 6,
        PHRASE_VERSE_BRIDGE_ID_VERSE6 = 7,
        PHRASE_VERSE_BRIDGE_ID_BRIDGE = 8,
        PHRASE_VERSE_BRIDGE_ID_CHORUS = 9,
        PHRASE_VERSE_BRIDGE_ID_OUTRO = 10
    };

    enum phrase_up_down_id_t {
        PHRASE_UP_DOWN_ID_INTRO = 1,
        PHRASE_UP_DOWN_ID_UP = 2,
        PHRASE_UP_DOWN_ID_DOWN = 3,
        PHRASE_UP_DOWN_ID_CHORUS = 5,
        PHRASE_UP_DOWN_ID_OUTRO = 6
    };

    rekordbox_anlz_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, rekordbox_anlz_t* p__root = 0);

private:
    void _read();

public:
    ~rekordbox_anlz_t();

    class phrase_up_down_t : public kaitai::kstruct {

    public:

        phrase_up_down_t(kaitai::kstream* p__io, rekordbox_anlz_t::song_structure_entry_t* p__parent = 0, rekordbox_anlz_t* p__root = 0);

    private:
        void _read();

    public:
        ~phrase_up_down_t();

    private:
        phrase_up_down_id_t m_id;
        rekordbox_anlz_t* m__root;
        rekordbox_anlz_t::song_structure_entry_t* m__parent;

    public:
        phrase_up_down_id_t id() const { return m_id; }
        rekordbox_anlz_t* _root() const { return m__root; }
        rekordbox_anlz_t::song_structure_entry_t* _parent() const { return m__parent; }
    };

    /**
     * Stores the file path of the audio file to which this analysis
     * applies.
     */

    class path_tag_t : public kaitai::kstruct {

    public:

        path_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent = 0, rekordbox_anlz_t* p__root = 0);

    private:
        void _read();

    public:
        ~path_tag_t();

    private:
        uint32_t m_len_path;
        std::string m_path;
        bool n_path;

    public:
        bool _is_null_path() { path(); return n_path; };

    private:
        rekordbox_anlz_t* m__root;
        rekordbox_anlz_t::tagged_section_t* m__parent;

    public:
        uint32_t len_path() const { return m_len_path; }
        std::string path() const { return m_path; }
        rekordbox_anlz_t* _root() const { return m__root; }
        rekordbox_anlz_t::tagged_section_t* _parent() const { return m__parent; }
    };

    /**
     * Stores a waveform preview image suitable for display above
     * the touch strip for jumping to a track position.
     */

    class wave_preview_tag_t : public kaitai::kstruct {

    public:

        wave_preview_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent = 0, rekordbox_anlz_t* p__root = 0);

    private:
        void _read();

    public:
        ~wave_preview_tag_t();

    private:
        uint32_t m_len_preview;
        uint32_t m__unnamed1;
        std::string m_data;
        bool n_data;

    public:
        bool _is_null_data() { data(); return n_data; };

    private:
        rekordbox_anlz_t* m__root;
        rekordbox_anlz_t::tagged_section_t* m__parent;

    public:

        /**
         * The length, in bytes, of the preview data itself. This is
         * slightly redundant because it can be computed from the
         * length of the tag.
         */
        uint32_t len_preview() const { return m_len_preview; }
        uint32_t _unnamed1() const { return m__unnamed1; }

        /**
         * The actual bytes of the waveform preview.
         */
        std::string data() const { return m_data; }
        rekordbox_anlz_t* _root() const { return m__root; }
        rekordbox_anlz_t::tagged_section_t* _parent() const { return m__parent; }
    };

    /**
     * Holds a list of all the beats found within the track, recording
     * their bar position, the time at which they occur, and the tempo
     * at that point.
     */

    class beat_grid_tag_t : public kaitai::kstruct {

    public:

        beat_grid_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent = 0, rekordbox_anlz_t* p__root = 0);

    private:
        void _read();

    public:
        ~beat_grid_tag_t();

    private:
        uint32_t m__unnamed0;
        uint32_t m__unnamed1;
        uint32_t m_len_beats;
        std::vector<beat_grid_beat_t*>* m_beats;
        rekordbox_anlz_t* m__root;
        rekordbox_anlz_t::tagged_section_t* m__parent;

    public:
        uint32_t _unnamed0() const { return m__unnamed0; }
        uint32_t _unnamed1() const { return m__unnamed1; }

        /**
         * The number of beat entries which follow.
         */
        uint32_t len_beats() const { return m_len_beats; }

        /**
         * The entries of the beat grid.
         */
        std::vector<beat_grid_beat_t*>* beats() const { return m_beats; }
        rekordbox_anlz_t* _root() const { return m__root; }
        rekordbox_anlz_t::tagged_section_t* _parent() const { return m__parent; }
    };

    /**
     * A larger, colorful waveform preview image suitable for display
     * above the touch strip for jumping to a track position on newer
     * high-resolution players.
     */

    class wave_color_preview_tag_t : public kaitai::kstruct {

    public:

        wave_color_preview_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent = 0, rekordbox_anlz_t* p__root = 0);

    private:
        void _read();

    public:
        ~wave_color_preview_tag_t();

    private:
        uint32_t m_len_entry_bytes;
        uint32_t m_len_entries;
        uint32_t m__unnamed2;
        std::string m_entries;
        rekordbox_anlz_t* m__root;
        rekordbox_anlz_t::tagged_section_t* m__parent;

    public:

        /**
         * The size of each entry, in bytes. Seems to always be 6.
         */
        uint32_t len_entry_bytes() const { return m_len_entry_bytes; }

        /**
         * The number of waveform data points, each of which takes one
         * byte for each of six channels of information.
         */
        uint32_t len_entries() const { return m_len_entries; }
        uint32_t _unnamed2() const { return m__unnamed2; }
        std::string entries() const { return m_entries; }
        rekordbox_anlz_t* _root() const { return m__root; }
        rekordbox_anlz_t::tagged_section_t* _parent() const { return m__parent; }
    };

    /**
     * A larger waveform image suitable for scrolling along as a track
     * plays.
     */

    class wave_scroll_tag_t : public kaitai::kstruct {

    public:

        wave_scroll_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent = 0, rekordbox_anlz_t* p__root = 0);

    private:
        void _read();

    public:
        ~wave_scroll_tag_t();

    private:
        uint32_t m_len_entry_bytes;
        uint32_t m_len_entries;
        uint32_t m__unnamed2;
        std::string m_entries;
        rekordbox_anlz_t* m__root;
        rekordbox_anlz_t::tagged_section_t* m__parent;

    public:

        /**
         * The size of each entry, in bytes. Seems to always be 1.
         */
        uint32_t len_entry_bytes() const { return m_len_entry_bytes; }

        /**
         * The number of waveform data points, each of which takes one
         * byte.
         */
        uint32_t len_entries() const { return m_len_entries; }
        uint32_t _unnamed2() const { return m__unnamed2; }
        std::string entries() const { return m_entries; }
        rekordbox_anlz_t* _root() const { return m__root; }
        rekordbox_anlz_t::tagged_section_t* _parent() const { return m__parent; }
    };

    class phrase_verse_bridge_t : public kaitai::kstruct {

    public:

        phrase_verse_bridge_t(kaitai::kstream* p__io, rekordbox_anlz_t::song_structure_entry_t* p__parent = 0, rekordbox_anlz_t* p__root = 0);

    private:
        void _read();

    public:
        ~phrase_verse_bridge_t();

    private:
        phrase_verse_bridge_id_t m_id;
        rekordbox_anlz_t* m__root;
        rekordbox_anlz_t::song_structure_entry_t* m__parent;

    public:
        phrase_verse_bridge_id_t id() const { return m_id; }
        rekordbox_anlz_t* _root() const { return m__root; }
        rekordbox_anlz_t::song_structure_entry_t* _parent() const { return m__parent; }
    };

    /**
     * Stores the song structure, also known as phrases (intro, verse,
     * bridge, chorus, up, down, outro).
     */

    class song_structure_tag_t : public kaitai::kstruct {

    public:

        song_structure_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent = 0, rekordbox_anlz_t* p__root = 0);

    private:
        void _read();

    public:
        ~song_structure_tag_t();

    private:
        uint32_t m_len_entry_bytes;
        uint16_t m_len_entries;
        uint16_t m_style;
        std::string m__unnamed3;
        uint16_t m_end_beat;
        std::string m__unnamed5;
        std::vector<song_structure_entry_t*>* m_entries;
        rekordbox_anlz_t* m__root;
        rekordbox_anlz_t::tagged_section_t* m__parent;

    public:

        /**
         * The size of each entry, in bytes. Seems to always be 24.
         */
        uint32_t len_entry_bytes() const { return m_len_entry_bytes; }

        /**
         * The number of phrases.
         */
        uint16_t len_entries() const { return m_len_entries; }

        /**
         * The phrase style. 1 is the up-down style
         * (white label text in rekordbox) where the main phrases consist
         * of up, down, and chorus. 2 is the bridge-verse style
         * (black label text in rekordbox) where the main phrases consist
         * of verse, chorus, and bridge. Style 3 is mostly identical to
         * bridge-verse style except verses 1-3 are labeled VERSE1 and verses
         * 4-6 are labeled VERSE2 in rekordbox.
         */
        uint16_t style() const { return m_style; }
        std::string _unnamed3() const { return m__unnamed3; }

        /**
         * The beat number at which the last phrase ends. The track may
         * continue after the last phrase ends. If this is the case, it will
         * mostly be silence.
         */
        uint16_t end_beat() const { return m_end_beat; }
        std::string _unnamed5() const { return m__unnamed5; }
        std::vector<song_structure_entry_t*>* entries() const { return m_entries; }
        rekordbox_anlz_t* _root() const { return m__root; }
        rekordbox_anlz_t::tagged_section_t* _parent() const { return m__parent; }
    };

    /**
     * A cue extended list entry. Can either describe a memory cue or a
     * loop.
     */

    class cue_extended_entry_t : public kaitai::kstruct {

    public:

        cue_extended_entry_t(kaitai::kstream* p__io, rekordbox_anlz_t::cue_extended_tag_t* p__parent = 0, rekordbox_anlz_t* p__root = 0);

    private:
        void _read();

    public:
        ~cue_extended_entry_t();

    private:
        std::string m__unnamed0;
        uint32_t m_len_header;
        uint32_t m_len_entry;
        uint32_t m_hot_cue;
        cue_entry_type_t m_type;
        std::string m__unnamed5;
        uint32_t m_time;
        uint32_t m_loop_time;
        uint8_t m_color_id;
        std::string m__unnamed9;
        uint32_t m_len_comment;
        bool n_len_comment;

    public:
        bool _is_null_len_comment() { len_comment(); return n_len_comment; };

    private:
        std::string m_comment;
        bool n_comment;

    public:
        bool _is_null_comment() { comment(); return n_comment; };

    private:
        uint8_t m_color_code;
        bool n_color_code;

    public:
        bool _is_null_color_code() { color_code(); return n_color_code; };

    private:
        uint8_t m_color_red;
        bool n_color_red;

    public:
        bool _is_null_color_red() { color_red(); return n_color_red; };

    private:
        uint8_t m_color_green;
        bool n_color_green;

    public:
        bool _is_null_color_green() { color_green(); return n_color_green; };

    private:
        uint8_t m_color_blue;
        bool n_color_blue;

    public:
        bool _is_null_color_blue() { color_blue(); return n_color_blue; };

    private:
        std::string m__unnamed16;
        bool n__unnamed16;

    public:
        bool _is_null__unnamed16() { _unnamed16(); return n__unnamed16; };

    private:
        rekordbox_anlz_t* m__root;
        rekordbox_anlz_t::cue_extended_tag_t* m__parent;

    public:
        std::string _unnamed0() const { return m__unnamed0; }
        uint32_t len_header() const { return m_len_header; }
        uint32_t len_entry() const { return m_len_entry; }

        /**
         * If zero, this is an ordinary memory cue, otherwise this a
         * hot cue with the specified number.
         */
        uint32_t hot_cue() const { return m_hot_cue; }

        /**
         * Indicates whether this is a memory cue or a loop.
         */
        cue_entry_type_t type() const { return m_type; }
        std::string _unnamed5() const { return m__unnamed5; }

        /**
         * The position, in milliseconds, at which the cue point lies
         * in the track.
         */
        uint32_t time() const { return m_time; }

        /**
         * The position, in milliseconds, at which the player loops
         * back to the cue time if this is a loop.
         */
        uint32_t loop_time() const { return m_loop_time; }

        /**
         * References a row in the colors table if the memory cue or loop
         * has been assigned a color
         */
        uint8_t color_id() const { return m_color_id; }
        std::string _unnamed9() const { return m__unnamed9; }
        uint32_t len_comment() const { return m_len_comment; }

        /**
         * The comment assigned to this cue by the DJ, if any, with a trailing NUL.
         */
        std::string comment() const { return m_comment; }

        /**
         * A lookup value for a color table? We use this to index to the colors shown in rekordbox.
         */
        uint8_t color_code() const { return m_color_code; }

        /**
         * The red component of the color to be displayed.
         */
        uint8_t color_red() const { return m_color_red; }

        /**
         * The green component of the color to be displayed.
         */
        uint8_t color_green() const { return m_color_green; }

        /**
         * The blue component of the color to be displayed.
         */
        uint8_t color_blue() const { return m_color_blue; }
        std::string _unnamed16() const { return m__unnamed16; }
        rekordbox_anlz_t* _root() const { return m__root; }
        rekordbox_anlz_t::cue_extended_tag_t* _parent() const { return m__parent; }
    };

    /**
     * Stores an index allowing rapid seeking to particular times
     * within a variable-bitrate audio file.
     */

    class vbr_tag_t : public kaitai::kstruct {

    public:

        vbr_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent = 0, rekordbox_anlz_t* p__root = 0);

    private:
        void _read();

    public:
        ~vbr_tag_t();

    private:
        uint32_t m__unnamed0;
        std::vector<uint32_t>* m_index;
        rekordbox_anlz_t* m__root;
        rekordbox_anlz_t::tagged_section_t* m__parent;

    public:
        uint32_t _unnamed0() const { return m__unnamed0; }
        std::vector<uint32_t>* index() const { return m_index; }
        rekordbox_anlz_t* _root() const { return m__root; }
        rekordbox_anlz_t::tagged_section_t* _parent() const { return m__parent; }
    };

    /**
     * A song structure entry, represents a single phrase.
     */

    class song_structure_entry_t : public kaitai::kstruct {

    public:

        song_structure_entry_t(kaitai::kstream* p__io, rekordbox_anlz_t::song_structure_tag_t* p__parent = 0, rekordbox_anlz_t* p__root = 0);

    private:
        void _read();

    public:
        ~song_structure_entry_t();

    private:
        uint16_t m_phrase_number;
        uint16_t m_beat_number;
        kaitai::kstruct* m_phrase_id;
        std::string m__unnamed3;
        uint8_t m_fill_in;
        uint16_t m_fill_in_beat_number;
        rekordbox_anlz_t* m__root;
        rekordbox_anlz_t::song_structure_tag_t* m__parent;

    public:

        /**
         * The absolute number of the phrase, starting at one.
         */
        uint16_t phrase_number() const { return m_phrase_number; }

        /**
         * The beat number at which the phrase starts.
         */
        uint16_t beat_number() const { return m_beat_number; }

        /**
         * Identifier of the phrase label.
         */
        kaitai::kstruct* phrase_id() const { return m_phrase_id; }
        std::string _unnamed3() const { return m__unnamed3; }

        /**
         * If nonzero, fill-in is present.
         */
        uint8_t fill_in() const { return m_fill_in; }

        /**
         * The beat number at which fill-in starts.
         */
        uint16_t fill_in_beat_number() const { return m_fill_in_beat_number; }
        rekordbox_anlz_t* _root() const { return m__root; }
        rekordbox_anlz_t::song_structure_tag_t* _parent() const { return m__parent; }
    };

    /**
     * A cue list entry. Can either represent a memory cue or a loop.
     */

    class cue_entry_t : public kaitai::kstruct {

    public:

        cue_entry_t(kaitai::kstream* p__io, rekordbox_anlz_t::cue_tag_t* p__parent = 0, rekordbox_anlz_t* p__root = 0);

    private:
        void _read();

    public:
        ~cue_entry_t();

    private:
        std::string m__unnamed0;
        uint32_t m_len_header;
        uint32_t m_len_entry;
        uint32_t m_hot_cue;
        cue_entry_status_t m_status;
        uint32_t m__unnamed5;
        uint16_t m_order_first;
        uint16_t m_order_last;
        cue_entry_type_t m_type;
        std::string m__unnamed9;
        uint32_t m_time;
        uint32_t m_loop_time;
        std::string m__unnamed12;
        rekordbox_anlz_t* m__root;
        rekordbox_anlz_t::cue_tag_t* m__parent;

    public:
        std::string _unnamed0() const { return m__unnamed0; }
        uint32_t len_header() const { return m_len_header; }
        uint32_t len_entry() const { return m_len_entry; }

        /**
         * If zero, this is an ordinary memory cue, otherwise this a
         * hot cue with the specified number.
         */
        uint32_t hot_cue() const { return m_hot_cue; }

        /**
         * If zero, this entry should be ignored.
         */
        cue_entry_status_t status() const { return m_status; }
        uint32_t _unnamed5() const { return m__unnamed5; }

        /**
         * @flesniak says: "0xffff for first cue, 0,1,3 for next"
         */
        uint16_t order_first() const { return m_order_first; }

        /**
         * @flesniak says: "1,2,3 for first, second, third cue, 0xffff for last"
         */
        uint16_t order_last() const { return m_order_last; }

        /**
         * Indicates whether this is a memory cue or a loop.
         */
        cue_entry_type_t type() const { return m_type; }
        std::string _unnamed9() const { return m__unnamed9; }

        /**
         * The position, in milliseconds, at which the cue point lies
         * in the track.
         */
        uint32_t time() const { return m_time; }

        /**
         * The position, in milliseconds, at which the player loops
         * back to the cue time if this is a loop.
         */
        uint32_t loop_time() const { return m_loop_time; }
        std::string _unnamed12() const { return m__unnamed12; }
        rekordbox_anlz_t* _root() const { return m__root; }
        rekordbox_anlz_t::cue_tag_t* _parent() const { return m__parent; }
    };

    /**
     * Describes an individual beat in a beat grid.
     */

    class beat_grid_beat_t : public kaitai::kstruct {

    public:

        beat_grid_beat_t(kaitai::kstream* p__io, rekordbox_anlz_t::beat_grid_tag_t* p__parent = 0, rekordbox_anlz_t* p__root = 0);

    private:
        void _read();

    public:
        ~beat_grid_beat_t();

    private:
        uint16_t m_beat_number;
        uint16_t m_tempo;
        uint32_t m_time;
        rekordbox_anlz_t* m__root;
        rekordbox_anlz_t::beat_grid_tag_t* m__parent;

    public:

        /**
         * The position of the beat within its musical bar, where beat 1
         * is the down beat.
         */
        uint16_t beat_number() const { return m_beat_number; }

        /**
         * The tempo at the time of this beat, in beats per minute,
         * multiplied by 100.
         */
        uint16_t tempo() const { return m_tempo; }

        /**
         * The time, in milliseconds, at which this beat occurs when
         * the track is played at normal (100%) pitch.
         */
        uint32_t time() const { return m_time; }
        rekordbox_anlz_t* _root() const { return m__root; }
        rekordbox_anlz_t::beat_grid_tag_t* _parent() const { return m__parent; }
    };

    /**
     * A variation of cue_tag which was introduced with the nxs2 line,
     * and adds descriptive names. (Still comes in two forms, either
     * holding memory cues and loop points, or holding hot cues and
     * loop points.) Also includes hot cues D through H and color assignment.
     */

    class cue_extended_tag_t : public kaitai::kstruct {

    public:

        cue_extended_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent = 0, rekordbox_anlz_t* p__root = 0);

    private:
        void _read();

    public:
        ~cue_extended_tag_t();

    private:
        cue_list_type_t m_type;
        uint16_t m_len_cues;
        std::string m__unnamed2;
        std::vector<cue_extended_entry_t*>* m_cues;
        rekordbox_anlz_t* m__root;
        rekordbox_anlz_t::tagged_section_t* m__parent;

    public:

        /**
         * Identifies whether this tag stores ordinary or hot cues.
         */
        cue_list_type_t type() const { return m_type; }

        /**
         * The length of the cue comment list.
         */
        uint16_t len_cues() const { return m_len_cues; }
        std::string _unnamed2() const { return m__unnamed2; }
        std::vector<cue_extended_entry_t*>* cues() const { return m_cues; }
        rekordbox_anlz_t* _root() const { return m__root; }
        rekordbox_anlz_t::tagged_section_t* _parent() const { return m__parent; }
    };

    class unknown_tag_t : public kaitai::kstruct {

    public:

        unknown_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent = 0, rekordbox_anlz_t* p__root = 0);

    private:
        void _read();

    public:
        ~unknown_tag_t();

    private:
        rekordbox_anlz_t* m__root;
        rekordbox_anlz_t::tagged_section_t* m__parent;

    public:
        rekordbox_anlz_t* _root() const { return m__root; }
        rekordbox_anlz_t::tagged_section_t* _parent() const { return m__parent; }
    };

    /**
     * A type-tagged file section, identified by a four-byte magic
     * sequence, with a header specifying its length, and whose payload
     * is determined by the type tag.
     */

    class tagged_section_t : public kaitai::kstruct {

    public:

        tagged_section_t(kaitai::kstream* p__io, rekordbox_anlz_t* p__parent = 0, rekordbox_anlz_t* p__root = 0);

    private:
        void _read();

    public:
        ~tagged_section_t();

    private:
        int32_t m_fourcc;
        uint32_t m_len_header;
        uint32_t m_len_tag;
        kaitai::kstruct* m_body;
        rekordbox_anlz_t* m__root;
        rekordbox_anlz_t* m__parent;
        std::string m__raw_body;
        kaitai::kstream* m__io__raw_body;

    public:

        /**
         * A tag value indicating what kind of section this is.
         */
        int32_t fourcc() const { return m_fourcc; }

        /**
         * The size, in bytes, of the header portion of the tag.
         */
        uint32_t len_header() const { return m_len_header; }

        /**
         * The size, in bytes, of this entire tag, counting the header.
         */
        uint32_t len_tag() const { return m_len_tag; }
        kaitai::kstruct* body() const { return m_body; }
        rekordbox_anlz_t* _root() const { return m__root; }
        rekordbox_anlz_t* _parent() const { return m__parent; }
        std::string _raw_body() const { return m__raw_body; }
        kaitai::kstream* _io__raw_body() const { return m__io__raw_body; }
    };

    /**
     * A larger, colorful waveform image suitable for scrolling along
     * as a track plays on newer high-resolution hardware. Also
     * contains a higher-resolution blue/white waveform.
     */

    class wave_color_scroll_tag_t : public kaitai::kstruct {

    public:

        wave_color_scroll_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent = 0, rekordbox_anlz_t* p__root = 0);

    private:
        void _read();

    public:
        ~wave_color_scroll_tag_t();

    private:
        uint32_t m_len_entry_bytes;
        uint32_t m_len_entries;
        uint32_t m__unnamed2;
        std::string m_entries;
        rekordbox_anlz_t* m__root;
        rekordbox_anlz_t::tagged_section_t* m__parent;

    public:

        /**
         * The size of each entry, in bytes. Seems to always be 2.
         */
        uint32_t len_entry_bytes() const { return m_len_entry_bytes; }

        /**
         * The number of columns of waveform data (this matches the
         * non-color waveform length.
         */
        uint32_t len_entries() const { return m_len_entries; }
        uint32_t _unnamed2() const { return m__unnamed2; }
        std::string entries() const { return m_entries; }
        rekordbox_anlz_t* _root() const { return m__root; }
        rekordbox_anlz_t::tagged_section_t* _parent() const { return m__parent; }
    };

    /**
     * Stores either a list of ordinary memory cues and loop points, or
     * a list of hot cues and loop points.
     */

    class cue_tag_t : public kaitai::kstruct {

    public:

        cue_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent = 0, rekordbox_anlz_t* p__root = 0);

    private:
        void _read();

    public:
        ~cue_tag_t();

    private:
        cue_list_type_t m_type;
        std::string m__unnamed1;
        uint16_t m_len_cues;
        uint32_t m_memory_count;
        std::vector<cue_entry_t*>* m_cues;
        rekordbox_anlz_t* m__root;
        rekordbox_anlz_t::tagged_section_t* m__parent;

    public:

        /**
         * Identifies whether this tag stores ordinary or hot cues.
         */
        cue_list_type_t type() const { return m_type; }
        std::string _unnamed1() const { return m__unnamed1; }

        /**
         * The length of the cue list.
         */
        uint16_t len_cues() const { return m_len_cues; }

        /**
         * Unsure what this means.
         */
        uint32_t memory_count() const { return m_memory_count; }
        std::vector<cue_entry_t*>* cues() const { return m_cues; }
        rekordbox_anlz_t* _root() const { return m__root; }
        rekordbox_anlz_t::tagged_section_t* _parent() const { return m__parent; }
    };

private:
    std::string m__unnamed0;
    uint32_t m_len_header;
    uint32_t m_len_file;
    std::string m__unnamed3;
    std::vector<tagged_section_t*>* m_sections;
    rekordbox_anlz_t* m__root;
    kaitai::kstruct* m__parent;

public:
    std::string _unnamed0() const { return m__unnamed0; }

    /**
     * The number of bytes of this header section.
     */
    uint32_t len_header() const { return m_len_header; }

    /**
     * The number of bytes in the entire file.
     */
    uint32_t len_file() const { return m_len_file; }
    std::string _unnamed3() const { return m__unnamed3; }

    /**
     * The remainder of the file is a sequence of type-tagged sections,
     * identified by a four-byte magic sequence.
     */
    std::vector<tagged_section_t*>* sections() const { return m_sections; }
    rekordbox_anlz_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
};
