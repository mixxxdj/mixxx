#pragma once

// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

class rekordbox_anlz_t;

#include "kaitai/kaitaistruct.h"
#include <stdint.h>
#include <memory>
#include <set>
#include <vector>

#if KAITAI_STRUCT_VERSION < 11000L
#error "Incompatible Kaitai Struct C++/STL API: version 0.11 or later is required"
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
 * \sa https://reverseengineering.stackexchange.com/questions/4311/help-reversing-a-edb-database-file-for-pioneers-rekordbox-software Source
 */

class rekordbox_anlz_t : public kaitai::kstruct {

public:
    class beat_grid_beat_t;
    class beat_grid_tag_t;
    class cue_entry_t;
    class cue_extended_entry_t;
    class cue_extended_tag_t;
    class cue_tag_t;
    class path_tag_t;
    class phrase_high_t;
    class phrase_low_t;
    class phrase_mid_t;
    class song_structure_body_t;
    class song_structure_entry_t;
    class song_structure_tag_t;
    class tagged_section_t;
    class unknown_tag_t;
    class vbr_tag_t;
    class wave_3band_preview_tag_t;
    class wave_3band_scroll_tag_t;
    class wave_color_preview_tag_t;
    class wave_color_scroll_tag_t;
    class wave_preview_tag_t;
    class wave_scroll_tag_t;

    enum cue_entry_status_t {
        CUE_ENTRY_STATUS_DISABLED = 0,
        CUE_ENTRY_STATUS_ENABLED = 1,
        CUE_ENTRY_STATUS_ACTIVE_LOOP = 4
    };
    static bool _is_defined_cue_entry_status_t(cue_entry_status_t v);

private:
    static const std::set<cue_entry_status_t> _values_cue_entry_status_t;

public:

    enum cue_entry_type_t {
        CUE_ENTRY_TYPE_MEMORY_CUE = 1,
        CUE_ENTRY_TYPE_LOOP = 2
    };
    static bool _is_defined_cue_entry_type_t(cue_entry_type_t v);

private:
    static const std::set<cue_entry_type_t> _values_cue_entry_type_t;

public:

    enum cue_list_type_t {
        CUE_LIST_TYPE_MEMORY_CUES = 0,
        CUE_LIST_TYPE_HOT_CUES = 1
    };
    static bool _is_defined_cue_list_type_t(cue_list_type_t v);

private:
    static const std::set<cue_list_type_t> _values_cue_list_type_t;

public:

    enum mood_high_phrase_t {
        MOOD_HIGH_PHRASE_INTRO = 1,
        MOOD_HIGH_PHRASE_UP = 2,
        MOOD_HIGH_PHRASE_DOWN = 3,
        MOOD_HIGH_PHRASE_CHORUS = 5,
        MOOD_HIGH_PHRASE_OUTRO = 6
    };
    static bool _is_defined_mood_high_phrase_t(mood_high_phrase_t v);

private:
    static const std::set<mood_high_phrase_t> _values_mood_high_phrase_t;

public:

    enum mood_low_phrase_t {
        MOOD_LOW_PHRASE_INTRO = 1,
        MOOD_LOW_PHRASE_VERSE_1 = 2,
        MOOD_LOW_PHRASE_VERSE_1B = 3,
        MOOD_LOW_PHRASE_VERSE_1C = 4,
        MOOD_LOW_PHRASE_VERSE_2 = 5,
        MOOD_LOW_PHRASE_VERSE_2B = 6,
        MOOD_LOW_PHRASE_VERSE_2C = 7,
        MOOD_LOW_PHRASE_BRIDGE = 8,
        MOOD_LOW_PHRASE_CHORUS = 9,
        MOOD_LOW_PHRASE_OUTRO = 10
    };
    static bool _is_defined_mood_low_phrase_t(mood_low_phrase_t v);

private:
    static const std::set<mood_low_phrase_t> _values_mood_low_phrase_t;

public:

    enum mood_mid_phrase_t {
        MOOD_MID_PHRASE_INTRO = 1,
        MOOD_MID_PHRASE_VERSE_1 = 2,
        MOOD_MID_PHRASE_VERSE_2 = 3,
        MOOD_MID_PHRASE_VERSE_3 = 4,
        MOOD_MID_PHRASE_VERSE_4 = 5,
        MOOD_MID_PHRASE_VERSE_5 = 6,
        MOOD_MID_PHRASE_VERSE_6 = 7,
        MOOD_MID_PHRASE_BRIDGE = 8,
        MOOD_MID_PHRASE_CHORUS = 9,
        MOOD_MID_PHRASE_OUTRO = 10
    };
    static bool _is_defined_mood_mid_phrase_t(mood_mid_phrase_t v);

private:
    static const std::set<mood_mid_phrase_t> _values_mood_mid_phrase_t;

public:

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
        SECTION_TAGS_WAVE_COLOR_SCROLL = 1347900981,
        SECTION_TAGS_WAVE_3BAND_PREVIEW = 1347900982,
        SECTION_TAGS_WAVE_3BAND_SCROLL = 1347900983
    };
    static bool _is_defined_section_tags_t(section_tags_t v);

private:
    static const std::set<section_tags_t> _values_section_tags_t;

public:

    enum track_bank_t {
        TRACK_BANK_DEFAULT = 0,
        TRACK_BANK_COOL = 1,
        TRACK_BANK_NATURAL = 2,
        TRACK_BANK_HOT = 3,
        TRACK_BANK_SUBTLE = 4,
        TRACK_BANK_WARM = 5,
        TRACK_BANK_VIVID = 6,
        TRACK_BANK_CLUB_1 = 7,
        TRACK_BANK_CLUB_2 = 8
    };
    static bool _is_defined_track_bank_t(track_bank_t v);

private:
    static const std::set<track_bank_t> _values_track_bank_t;

public:

    enum track_mood_t {
        TRACK_MOOD_HIGH = 1,
        TRACK_MOOD_MID = 2,
        TRACK_MOOD_LOW = 3
    };
    static bool _is_defined_track_mood_t(track_mood_t v);

private:
    static const std::set<track_mood_t> _values_track_mood_t;

public:

    rekordbox_anlz_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = nullptr, rekordbox_anlz_t* p__root = nullptr);

private:
    void _read();
    void _clean_up();

public:
    ~rekordbox_anlz_t();

    /**
     * Describes an individual beat in a beat grid.
     */

    class beat_grid_beat_t : public kaitai::kstruct {

    public:

        beat_grid_beat_t(kaitai::kstream* p__io, rekordbox_anlz_t::beat_grid_tag_t* p__parent = nullptr, rekordbox_anlz_t* p__root = nullptr);

    private:
        void _read();
        void _clean_up();

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
     * Holds a list of all the beats found within the track, recording
     * their bar position, the time at which they occur, and the tempo
     * at that point.
     */

    class beat_grid_tag_t : public kaitai::kstruct {

    public:

        beat_grid_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent = nullptr, rekordbox_anlz_t* p__root = nullptr);

    private:
        void _read();
        void _clean_up();

    public:
        ~beat_grid_tag_t();

    private:
        uint32_t m__unnamed0;
        uint32_t m__unnamed1;
        uint32_t m_num_beats;
        std::unique_ptr<std::vector<std::unique_ptr<beat_grid_beat_t>>> m_beats;
        rekordbox_anlz_t* m__root;
        rekordbox_anlz_t::tagged_section_t* m__parent;

    public:
        uint32_t _unnamed0() const { return m__unnamed0; }
        uint32_t _unnamed1() const { return m__unnamed1; }

        /**
         * The number of beat entries which follow.
         */
        uint32_t num_beats() const { return m_num_beats; }

        /**
         * The entries of the beat grid.
         */
        std::vector<std::unique_ptr<beat_grid_beat_t>>* beats() const { return m_beats.get(); }
        rekordbox_anlz_t* _root() const { return m__root; }
        rekordbox_anlz_t::tagged_section_t* _parent() const { return m__parent; }
    };

    /**
     * A cue list entry. Can either represent a memory cue or a loop.
     */

    class cue_entry_t : public kaitai::kstruct {

    public:

        cue_entry_t(kaitai::kstream* p__io, rekordbox_anlz_t::cue_tag_t* p__parent = nullptr, rekordbox_anlz_t* p__root = nullptr);

    private:
        void _read();
        void _clean_up();

    public:
        ~cue_entry_t();

    private:
        std::string m_magic;
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

        /**
         * Identifies this as a cue list entry (cue point).
         */
        std::string magic() const { return m_magic; }
        uint32_t len_header() const { return m_len_header; }
        uint32_t len_entry() const { return m_len_entry; }

        /**
         * If zero, this is an ordinary memory cue, otherwise this a
         * hot cue with the specified number.
         */
        uint32_t hot_cue() const { return m_hot_cue; }

        /**
         * Indicates if this is an active loop.
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
     * A cue extended list entry. Can either describe a memory cue or a
     * loop.
     */

    class cue_extended_entry_t : public kaitai::kstruct {

    public:

        cue_extended_entry_t(kaitai::kstream* p__io, rekordbox_anlz_t::cue_extended_tag_t* p__parent = nullptr, rekordbox_anlz_t* p__root = nullptr);

    private:
        void _read();
        void _clean_up();

    public:
        ~cue_extended_entry_t();

    private:
        std::string m_magic;
        uint32_t m_len_header;
        uint32_t m_len_entry;
        uint32_t m_hot_cue;
        cue_entry_type_t m_type;
        std::string m__unnamed5;
        uint32_t m_time;
        uint32_t m_loop_time;
        uint8_t m_color_id;
        std::string m__unnamed9;
        uint16_t m_loop_numerator;
        uint16_t m_loop_denominator;
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
        std::string m__unnamed18;
        bool n__unnamed18;

    public:
        bool _is_null__unnamed18() { _unnamed18(); return n__unnamed18; };

    private:
        rekordbox_anlz_t* m__root;
        rekordbox_anlz_t::cue_extended_tag_t* m__parent;

    public:

        /**
         * Identifies this as an extended cue list entry (cue point).
         */
        std::string magic() const { return m_magic; }
        uint32_t len_header() const { return m_len_header; }
        uint32_t len_entry() const { return m_len_entry; }

        /**
         * If zero, this is an ordinary memory cue, otherwise this a
         * hot cue with the specified number.
         */
        uint32_t hot_cue() const { return m_hot_cue; }

        /**
         * Indicates whether this is a regular cue point or a loop.
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
         * References a row in the colors table if this is a memory cue or loop
         * and has been assigned a color.
         */
        uint8_t color_id() const { return m_color_id; }
        std::string _unnamed9() const { return m__unnamed9; }

        /**
         * The numerator of the loop length in beats. 
         * Zero if the loop is not quantized.
         */
        uint16_t loop_numerator() const { return m_loop_numerator; }

        /**
         * The denominator of the loop length in beats. 
         * Zero if the loop is not quantized.
         */
        uint16_t loop_denominator() const { return m_loop_denominator; }
        uint32_t len_comment() const { return m_len_comment; }

        /**
         * The comment assigned to this cue by the DJ, if any, with a trailing NUL.
         */
        std::string comment() const { return m_comment; }

        /**
         * A lookup value for a color table? We use this to index to the hot cue colors shown in rekordbox.
         */
        uint8_t color_code() const { return m_color_code; }

        /**
         * The red component of the hot cue color to be displayed.
         */
        uint8_t color_red() const { return m_color_red; }

        /**
         * The green component of the hot cue color to be displayed.
         */
        uint8_t color_green() const { return m_color_green; }

        /**
         * The blue component of the hot cue color to be displayed.
         */
        uint8_t color_blue() const { return m_color_blue; }
        std::string _unnamed18() const { return m__unnamed18; }
        rekordbox_anlz_t* _root() const { return m__root; }
        rekordbox_anlz_t::cue_extended_tag_t* _parent() const { return m__parent; }
    };

    /**
     * A variation of cue_tag which was introduced with the nxs2 line,
     * and adds descriptive names. (Still comes in two forms, either
     * holding memory cues and loop points, or holding hot cues and
     * loop points.) Also includes hot cues D through H and color assignment.
     */

    class cue_extended_tag_t : public kaitai::kstruct {

    public:

        cue_extended_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent = nullptr, rekordbox_anlz_t* p__root = nullptr);

    private:
        void _read();
        void _clean_up();

    public:
        ~cue_extended_tag_t();

    private:
        cue_list_type_t m_type;
        uint16_t m_num_cues;
        std::string m__unnamed2;
        std::unique_ptr<std::vector<std::unique_ptr<cue_extended_entry_t>>> m_cues;
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
        uint16_t num_cues() const { return m_num_cues; }
        std::string _unnamed2() const { return m__unnamed2; }
        std::vector<std::unique_ptr<cue_extended_entry_t>>* cues() const { return m_cues.get(); }
        rekordbox_anlz_t* _root() const { return m__root; }
        rekordbox_anlz_t::tagged_section_t* _parent() const { return m__parent; }
    };

    /**
     * Stores either a list of ordinary memory cues and loop points, or
     * a list of hot cues and loop points.
     */

    class cue_tag_t : public kaitai::kstruct {

    public:

        cue_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent = nullptr, rekordbox_anlz_t* p__root = nullptr);

    private:
        void _read();
        void _clean_up();

    public:
        ~cue_tag_t();

    private:
        cue_list_type_t m_type;
        std::string m__unnamed1;
        uint16_t m_num_cues;
        uint32_t m_memory_count;
        std::unique_ptr<std::vector<std::unique_ptr<cue_entry_t>>> m_cues;
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
        uint16_t num_cues() const { return m_num_cues; }

        /**
         * Unsure what this means.
         */
        uint32_t memory_count() const { return m_memory_count; }
        std::vector<std::unique_ptr<cue_entry_t>>* cues() const { return m_cues.get(); }
        rekordbox_anlz_t* _root() const { return m__root; }
        rekordbox_anlz_t::tagged_section_t* _parent() const { return m__parent; }
    };

    /**
     * Stores the file path of the audio file to which this analysis
     * applies.
     */

    class path_tag_t : public kaitai::kstruct {

    public:

        path_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent = nullptr, rekordbox_anlz_t* p__root = nullptr);

    private:
        void _read();
        void _clean_up();

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

    class phrase_high_t : public kaitai::kstruct {

    public:

        phrase_high_t(kaitai::kstream* p__io, rekordbox_anlz_t::song_structure_entry_t* p__parent = nullptr, rekordbox_anlz_t* p__root = nullptr);

    private:
        void _read();
        void _clean_up();

    public:
        ~phrase_high_t();

    private:
        mood_high_phrase_t m_id;
        rekordbox_anlz_t* m__root;
        rekordbox_anlz_t::song_structure_entry_t* m__parent;

    public:
        mood_high_phrase_t id() const { return m_id; }
        rekordbox_anlz_t* _root() const { return m__root; }
        rekordbox_anlz_t::song_structure_entry_t* _parent() const { return m__parent; }
    };

    class phrase_low_t : public kaitai::kstruct {

    public:

        phrase_low_t(kaitai::kstream* p__io, rekordbox_anlz_t::song_structure_entry_t* p__parent = nullptr, rekordbox_anlz_t* p__root = nullptr);

    private:
        void _read();
        void _clean_up();

    public:
        ~phrase_low_t();

    private:
        mood_low_phrase_t m_id;
        rekordbox_anlz_t* m__root;
        rekordbox_anlz_t::song_structure_entry_t* m__parent;

    public:
        mood_low_phrase_t id() const { return m_id; }
        rekordbox_anlz_t* _root() const { return m__root; }
        rekordbox_anlz_t::song_structure_entry_t* _parent() const { return m__parent; }
    };

    class phrase_mid_t : public kaitai::kstruct {

    public:

        phrase_mid_t(kaitai::kstream* p__io, rekordbox_anlz_t::song_structure_entry_t* p__parent = nullptr, rekordbox_anlz_t* p__root = nullptr);

    private:
        void _read();
        void _clean_up();

    public:
        ~phrase_mid_t();

    private:
        mood_mid_phrase_t m_id;
        rekordbox_anlz_t* m__root;
        rekordbox_anlz_t::song_structure_entry_t* m__parent;

    public:
        mood_mid_phrase_t id() const { return m_id; }
        rekordbox_anlz_t* _root() const { return m__root; }
        rekordbox_anlz_t::song_structure_entry_t* _parent() const { return m__parent; }
    };

    /**
     * Stores the rest of the song structure tag, which can only be
     * parsed after unmasking.
     */

    class song_structure_body_t : public kaitai::kstruct {

    public:

        song_structure_body_t(kaitai::kstream* p__io, rekordbox_anlz_t::song_structure_tag_t* p__parent = nullptr, rekordbox_anlz_t* p__root = nullptr);

    private:
        void _read();
        void _clean_up();

    public:
        ~song_structure_body_t();

    private:
        bool f_bank;
        track_bank_t m_bank;
        bool n_bank;

    public:
        bool _is_null_bank() { bank(); return n_bank; };

    private:

    public:

        /**
         * The stylistic bank which can be assigned to the track in rekordbox Lighting mode, if raw_bank has a legal value.
         */
        track_bank_t bank();

    private:
        track_mood_t m_mood;
        std::string m__unnamed1;
        uint16_t m_end_beat;
        std::string m__unnamed3;
        uint8_t m_raw_bank;
        std::string m__unnamed5;
        std::unique_ptr<std::vector<std::unique_ptr<song_structure_entry_t>>> m_entries;
        rekordbox_anlz_t* m__root;
        rekordbox_anlz_t::song_structure_tag_t* m__parent;

    public:

        /**
         * The mood which rekordbox assigns the track as a whole during phrase analysis.
         */
        track_mood_t mood() const { return m_mood; }
        std::string _unnamed1() const { return m__unnamed1; }

        /**
         * The beat number at which the last phrase ends. The track may
         * continue after the last phrase ends. If this is the case, it will
         * mostly be silence.
         */
        uint16_t end_beat() const { return m_end_beat; }
        std::string _unnamed3() const { return m__unnamed3; }

        /**
         * Number identifying a stylistic bank which can be assigned to the track in rekordbox Lighting mode.
         */
        uint8_t raw_bank() const { return m_raw_bank; }
        std::string _unnamed5() const { return m__unnamed5; }
        std::vector<std::unique_ptr<song_structure_entry_t>>* entries() const { return m_entries.get(); }
        rekordbox_anlz_t* _root() const { return m__root; }
        rekordbox_anlz_t::song_structure_tag_t* _parent() const { return m__parent; }
    };

    /**
     * A song structure entry, represents a single phrase.
     */

    class song_structure_entry_t : public kaitai::kstruct {

    public:

        song_structure_entry_t(kaitai::kstream* p__io, rekordbox_anlz_t::song_structure_body_t* p__parent = nullptr, rekordbox_anlz_t* p__root = nullptr);

    private:
        void _read();
        void _clean_up();

    public:
        ~song_structure_entry_t();

    private:
        uint16_t m_index;
        uint16_t m_beat;
        std::unique_ptr<kaitai::kstruct> m_kind;
        std::string m__unnamed3;
        uint8_t m_k1;
        std::string m__unnamed5;
        uint8_t m_k2;
        std::string m__unnamed7;
        uint8_t m_b;
        uint16_t m_beat2;
        uint16_t m_beat3;
        uint16_t m_beat4;
        std::string m__unnamed12;
        uint8_t m_k3;
        std::string m__unnamed14;
        uint8_t m_fill;
        uint16_t m_beat_fill;
        rekordbox_anlz_t* m__root;
        rekordbox_anlz_t::song_structure_body_t* m__parent;

    public:

        /**
         * The absolute number of the phrase, starting at one.
         */
        uint16_t index() const { return m_index; }

        /**
         * The beat number at which the phrase starts.
         */
        uint16_t beat() const { return m_beat; }

        /**
         * The kind of phrase as displayed in rekordbox.
         */
        kaitai::kstruct* kind() const { return m_kind.get(); }
        std::string _unnamed3() const { return m__unnamed3; }

        /**
         * One of three flags that identify phrase kind variants in high-mood tracks.
         */
        uint8_t k1() const { return m_k1; }
        std::string _unnamed5() const { return m__unnamed5; }

        /**
         * One of three flags that identify phrase kind variants in high-mood tracks.
         */
        uint8_t k2() const { return m_k2; }
        std::string _unnamed7() const { return m__unnamed7; }

        /**
         * Flags how many more beat numbers are in a high-mood "Up 3" phrase.
         */
        uint8_t b() const { return m_b; }

        /**
         * Extra beat number (falling within phrase) always present in high-mood "Up 3" phrases.
         */
        uint16_t beat2() const { return m_beat2; }

        /**
         * Extra beat number (falling within phrase, larger than beat2)
         * present in high-mood "Up 3" phrases when b has value 1.
         */
        uint16_t beat3() const { return m_beat3; }

        /**
         * Extra beat number (falling within phrase, larger than beat3)
         * present in high-mood "Up 3" phrases when b has value 1.
         */
        uint16_t beat4() const { return m_beat4; }
        std::string _unnamed12() const { return m__unnamed12; }

        /**
         * One of three flags that identify phrase kind variants in high-mood tracks.
         */
        uint8_t k3() const { return m_k3; }
        std::string _unnamed14() const { return m__unnamed14; }

        /**
         * If nonzero, fill-in is present at end of phrase.
         */
        uint8_t fill() const { return m_fill; }

        /**
         * The beat number at which fill-in starts.
         */
        uint16_t beat_fill() const { return m_beat_fill; }
        rekordbox_anlz_t* _root() const { return m__root; }
        rekordbox_anlz_t::song_structure_body_t* _parent() const { return m__parent; }
    };

    /**
     * Stores the song structure, also known as phrases (intro, verse,
     * bridge, chorus, up, down, outro).
     */

    class song_structure_tag_t : public kaitai::kstruct {

    public:

        song_structure_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent = nullptr, rekordbox_anlz_t* p__root = nullptr);

    private:
        void _read();
        void _clean_up();

    public:
        ~song_structure_tag_t();

    private:
        bool f_c;
        uint16_t m_c;

    public:
        uint16_t c();

    private:
        bool f_is_masked;
        bool m_is_masked;

    public:
        bool is_masked();

    private:
        bool f_mask;
        std::string m_mask;

    public:
        std::string mask();

    private:
        bool f_raw_mood;
        uint16_t m_raw_mood;

    public:

        /**
         * This is a way to tell whether the rest of the tag has been masked. The value is supposed
         * to range from 1 to 3, but in masked files it will be much larger.
         */
        uint16_t raw_mood();

    private:
        uint32_t m_len_entry_bytes;
        uint16_t m_len_entries;
        std::unique_ptr<song_structure_body_t> m_body;
        rekordbox_anlz_t* m__root;
        rekordbox_anlz_t::tagged_section_t* m__parent;
        std::string m__raw_body;
        std::unique_ptr<kaitai::kstream> m__io__raw_body;
        std::string m__raw__raw_body;

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
         * The rest of the tag, which needs to be unmasked before it
         * can be parsed.
         */
        song_structure_body_t* body() const { return m_body.get(); }
        rekordbox_anlz_t* _root() const { return m__root; }
        rekordbox_anlz_t::tagged_section_t* _parent() const { return m__parent; }
        std::string _raw_body() const { return m__raw_body; }
        kaitai::kstream* _io__raw_body() const { return m__io__raw_body.get(); }
        std::string _raw__raw_body() const { return m__raw__raw_body; }
    };

    /**
     * A type-tagged file section, identified by a four-byte magic
     * sequence, with a header specifying its length, and whose payload
     * is determined by the type tag.
     */

    class tagged_section_t : public kaitai::kstruct {

    public:

        tagged_section_t(kaitai::kstream* p__io, rekordbox_anlz_t* p__parent = nullptr, rekordbox_anlz_t* p__root = nullptr);

    private:
        void _read();
        void _clean_up();

    public:
        ~tagged_section_t();

    private:
        section_tags_t m_fourcc;
        uint32_t m_len_header;
        uint32_t m_len_tag;
        std::unique_ptr<kaitai::kstruct> m_body;
        rekordbox_anlz_t* m__root;
        rekordbox_anlz_t* m__parent;
        std::string m__raw_body;
        std::unique_ptr<kaitai::kstream> m__io__raw_body;

    public:

        /**
         * A tag value indicating what kind of section this is.
         */
        section_tags_t fourcc() const { return m_fourcc; }

        /**
         * The size, in bytes, of the header portion of the tag.
         */
        uint32_t len_header() const { return m_len_header; }

        /**
         * The size, in bytes, of this entire tag, counting the header.
         */
        uint32_t len_tag() const { return m_len_tag; }
        kaitai::kstruct* body() const { return m_body.get(); }
        rekordbox_anlz_t* _root() const { return m__root; }
        rekordbox_anlz_t* _parent() const { return m__parent; }
        std::string _raw_body() const { return m__raw_body; }
        kaitai::kstream* _io__raw_body() const { return m__io__raw_body.get(); }
    };

    class unknown_tag_t : public kaitai::kstruct {

    public:

        unknown_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent = nullptr, rekordbox_anlz_t* p__root = nullptr);

    private:
        void _read();
        void _clean_up();

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
     * Stores an index allowing rapid seeking to particular times
     * within a variable-bitrate audio file.
     */

    class vbr_tag_t : public kaitai::kstruct {

    public:

        vbr_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent = nullptr, rekordbox_anlz_t* p__root = nullptr);

    private:
        void _read();
        void _clean_up();

    public:
        ~vbr_tag_t();

    private:
        uint32_t m__unnamed0;
        std::unique_ptr<std::vector<uint32_t>> m_index;
        rekordbox_anlz_t* m__root;
        rekordbox_anlz_t::tagged_section_t* m__parent;

    public:
        uint32_t _unnamed0() const { return m__unnamed0; }
        std::vector<uint32_t>* index() const { return m_index.get(); }
        rekordbox_anlz_t* _root() const { return m__root; }
        rekordbox_anlz_t::tagged_section_t* _parent() const { return m__parent; }
    };

    /**
     * The minimalist CDJ-3000 waveform preview image suitable for display
     * above the touch strip for jumping to a track position.
     */

    class wave_3band_preview_tag_t : public kaitai::kstruct {

    public:

        wave_3band_preview_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent = nullptr, rekordbox_anlz_t* p__root = nullptr);

    private:
        void _read();
        void _clean_up();

    public:
        ~wave_3band_preview_tag_t();

    private:
        uint32_t m_len_entry_bytes;
        uint32_t m_len_entries;
        std::string m_entries;
        rekordbox_anlz_t* m__root;
        rekordbox_anlz_t::tagged_section_t* m__parent;

    public:

        /**
         * The size of each entry, in bytes. Seems to always be 3.
         */
        uint32_t len_entry_bytes() const { return m_len_entry_bytes; }

        /**
         * The number of waveform data points, each of which takes one
         * byte for each of six channels of information.
         */
        uint32_t len_entries() const { return m_len_entries; }
        std::string entries() const { return m_entries; }
        rekordbox_anlz_t* _root() const { return m__root; }
        rekordbox_anlz_t::tagged_section_t* _parent() const { return m__parent; }
    };

    /**
     * The minimalist CDJ-3000 waveform image suitable for scrolling along
     * as a track plays on newer high-resolution hardware.
     */

    class wave_3band_scroll_tag_t : public kaitai::kstruct {

    public:

        wave_3band_scroll_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent = nullptr, rekordbox_anlz_t* p__root = nullptr);

    private:
        void _read();
        void _clean_up();

    public:
        ~wave_3band_scroll_tag_t();

    private:
        uint32_t m_len_entry_bytes;
        uint32_t m_len_entries;
        uint32_t m__unnamed2;
        std::string m_entries;
        rekordbox_anlz_t* m__root;
        rekordbox_anlz_t::tagged_section_t* m__parent;

    public:

        /**
         * The size of each entry, in bytes. Seems to always be 3.
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
     * A larger, colorful waveform preview image suitable for display
     * above the touch strip for jumping to a track position on newer
     * high-resolution players.
     */

    class wave_color_preview_tag_t : public kaitai::kstruct {

    public:

        wave_color_preview_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent = nullptr, rekordbox_anlz_t* p__root = nullptr);

    private:
        void _read();
        void _clean_up();

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
     * A larger, colorful waveform image suitable for scrolling along
     * as a track plays on newer high-resolution hardware. Also
     * contains a higher-resolution blue/white waveform.
     */

    class wave_color_scroll_tag_t : public kaitai::kstruct {

    public:

        wave_color_scroll_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent = nullptr, rekordbox_anlz_t* p__root = nullptr);

    private:
        void _read();
        void _clean_up();

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
     * Stores a waveform preview image suitable for display above
     * the touch strip for jumping to a track position.
     */

    class wave_preview_tag_t : public kaitai::kstruct {

    public:

        wave_preview_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent = nullptr, rekordbox_anlz_t* p__root = nullptr);

    private:
        void _read();
        void _clean_up();

    public:
        ~wave_preview_tag_t();

    private:
        uint32_t m_len_data;
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
        uint32_t len_data() const { return m_len_data; }
        uint32_t _unnamed1() const { return m__unnamed1; }

        /**
         * The actual bytes of the waveform preview.
         */
        std::string data() const { return m_data; }
        rekordbox_anlz_t* _root() const { return m__root; }
        rekordbox_anlz_t::tagged_section_t* _parent() const { return m__parent; }
    };

    /**
     * A larger waveform image suitable for scrolling along as a track
     * plays.
     */

    class wave_scroll_tag_t : public kaitai::kstruct {

    public:

        wave_scroll_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent = nullptr, rekordbox_anlz_t* p__root = nullptr);

    private:
        void _read();
        void _clean_up();

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

private:
    std::string m_magic;
    uint32_t m_len_header;
    uint32_t m_len_file;
    std::string m__unnamed3;
    std::unique_ptr<std::vector<std::unique_ptr<tagged_section_t>>> m_sections;
    rekordbox_anlz_t* m__root;
    kaitai::kstruct* m__parent;

public:

    /**
     * Identifies this as an analysis file.
     */
    std::string magic() const { return m_magic; }

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
    std::vector<std::unique_ptr<tagged_section_t>>* sections() const { return m_sections.get(); }
    rekordbox_anlz_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
};
