// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "rekordbox_anlz.h"

#include "kaitai/exceptions.h"

rekordbox_anlz_t::rekordbox_anlz_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent, rekordbox_anlz_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = this;
    m_sections = nullptr;
    _read();
}

void rekordbox_anlz_t::_read() {
    m_magic = m__io->read_bytes(4);
    if (!(magic() == std::string("\x50\x4D\x41\x49", 4))) {
        throw kaitai::validation_not_equal_error<std::string>(
                std::string("\x50\x4D\x41\x49", 4),
                magic(),
                _io(),
                std::string("/seq/0"));
    }
    m_len_header = m__io->read_u4be();
    m_len_file = m__io->read_u4be();
    m__unnamed3 = m__io->read_bytes((len_header() - _io()->pos()));
    m_sections =
            std::unique_ptr<std::vector<std::unique_ptr<tagged_section_t>>>(
                    new std::vector<std::unique_ptr<tagged_section_t>>());
    {
        int i = 0;
        while (!m__io->is_eof()) {
            m_sections->push_back(std::move(std::unique_ptr<tagged_section_t>(
                    new tagged_section_t(m__io, this, m__root))));
            i++;
        }
    }
}

rekordbox_anlz_t::~rekordbox_anlz_t() {
    _clean_up();
}

void rekordbox_anlz_t::_clean_up() {
}

rekordbox_anlz_t::phrase_mid_t::phrase_mid_t(kaitai::kstream* p__io,
        rekordbox_anlz_t::song_structure_entry_t* p__parent,
        rekordbox_anlz_t* p__root)
        : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void rekordbox_anlz_t::phrase_mid_t::_read() {
    m_id = static_cast<rekordbox_anlz_t::mood_mid_phrase_t>(m__io->read_u2be());
}

rekordbox_anlz_t::phrase_mid_t::~phrase_mid_t() {
    _clean_up();
}

void rekordbox_anlz_t::phrase_mid_t::_clean_up() {
}

rekordbox_anlz_t::path_tag_t::path_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent, rekordbox_anlz_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void rekordbox_anlz_t::path_tag_t::_read() {
    m_len_path = m__io->read_u4be();
    n_path = true;
    if (len_path() > 1) {
        n_path = false;
        m_path = kaitai::kstream::bytes_to_str(m__io->read_bytes((len_path() - 2)), std::string("utf-16be"));
    }
}

rekordbox_anlz_t::path_tag_t::~path_tag_t() {
    _clean_up();
}

void rekordbox_anlz_t::path_tag_t::_clean_up() {
    if (!n_path) {
    }
}

rekordbox_anlz_t::wave_preview_tag_t::wave_preview_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent, rekordbox_anlz_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void rekordbox_anlz_t::wave_preview_tag_t::_read() {
    m_len_data = m__io->read_u4be();
    m__unnamed1 = m__io->read_u4be();
    n_data = true;
    if (_parent()->len_tag() > _parent()->len_header()) {
        n_data = false;
        m_data = m__io->read_bytes(len_data());
    }
}

rekordbox_anlz_t::wave_preview_tag_t::~wave_preview_tag_t() {
    _clean_up();
}

void rekordbox_anlz_t::wave_preview_tag_t::_clean_up() {
    if (!n_data) {
    }
}

rekordbox_anlz_t::beat_grid_tag_t::beat_grid_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent, rekordbox_anlz_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    m_beats = nullptr;
    _read();
}

void rekordbox_anlz_t::beat_grid_tag_t::_read() {
    m__unnamed0 = m__io->read_u4be();
    m__unnamed1 = m__io->read_u4be();
    m_num_beats = m__io->read_u4be();
    m_beats = std::unique_ptr<std::vector<std::unique_ptr<beat_grid_beat_t>>>(
            new std::vector<std::unique_ptr<beat_grid_beat_t>>());
    const int l_beats = num_beats();
    for (int i = 0; i < l_beats; i++) {
        m_beats->push_back(std::move(std::unique_ptr<beat_grid_beat_t>(
                new beat_grid_beat_t(m__io, this, m__root))));
    }
}

rekordbox_anlz_t::beat_grid_tag_t::~beat_grid_tag_t() {
    _clean_up();
}

void rekordbox_anlz_t::beat_grid_tag_t::_clean_up() {
}

rekordbox_anlz_t::song_structure_body_t::song_structure_body_t(
        kaitai::kstream* p__io,
        rekordbox_anlz_t::song_structure_tag_t* p__parent,
        rekordbox_anlz_t* p__root)
        : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    m_entries = nullptr;
    _read();
}

void rekordbox_anlz_t::song_structure_body_t::_read() {
    m_mood = static_cast<rekordbox_anlz_t::track_mood_t>(m__io->read_u2be());
    m__unnamed1 = m__io->read_bytes(6);
    m_end_beat = m__io->read_u2be();
    m__unnamed3 = m__io->read_bytes(2);
    m_bank = static_cast<rekordbox_anlz_t::track_bank_t>(m__io->read_u1());
    m__unnamed5 = m__io->read_bytes(1);
    m_entries = std::unique_ptr<
            std::vector<std::unique_ptr<song_structure_entry_t>>>(
            new std::vector<std::unique_ptr<song_structure_entry_t>>());
    const int l_entries = _parent()->len_entries();
    for (int i = 0; i < l_entries; i++) {
        m_entries->push_back(std::move(std::unique_ptr<song_structure_entry_t>(
                new song_structure_entry_t(m__io, this, m__root))));
    }
}

rekordbox_anlz_t::song_structure_body_t::~song_structure_body_t() {
    _clean_up();
}

void rekordbox_anlz_t::song_structure_body_t::_clean_up() {
}

rekordbox_anlz_t::wave_color_preview_tag_t::wave_color_preview_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent, rekordbox_anlz_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void rekordbox_anlz_t::wave_color_preview_tag_t::_read() {
    m_len_entry_bytes = m__io->read_u4be();
    m_len_entries = m__io->read_u4be();
    m__unnamed2 = m__io->read_u4be();
    m_entries = m__io->read_bytes((len_entries() * len_entry_bytes()));
}

rekordbox_anlz_t::wave_color_preview_tag_t::~wave_color_preview_tag_t() {
    _clean_up();
}

void rekordbox_anlz_t::wave_color_preview_tag_t::_clean_up() {
}

rekordbox_anlz_t::phrase_high_t::phrase_high_t(kaitai::kstream* p__io,
        rekordbox_anlz_t::song_structure_entry_t* p__parent,
        rekordbox_anlz_t* p__root)
        : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void rekordbox_anlz_t::phrase_high_t::_read() {
    m_id = static_cast<rekordbox_anlz_t::mood_high_phrase_t>(m__io->read_u2be());
}

rekordbox_anlz_t::phrase_high_t::~phrase_high_t() {
    _clean_up();
}

void rekordbox_anlz_t::phrase_high_t::_clean_up() {
}

rekordbox_anlz_t::wave_scroll_tag_t::wave_scroll_tag_t(kaitai::kstream* p__io,
        rekordbox_anlz_t::tagged_section_t* p__parent,
        rekordbox_anlz_t* p__root)
        : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void rekordbox_anlz_t::wave_scroll_tag_t::_read() {
    m_len_entry_bytes = m__io->read_u4be();
    m_len_entries = m__io->read_u4be();
    m__unnamed2 = m__io->read_u4be();
    m_entries = m__io->read_bytes((len_entries() * len_entry_bytes()));
}

rekordbox_anlz_t::wave_scroll_tag_t::~wave_scroll_tag_t() {
    _clean_up();
}

void rekordbox_anlz_t::wave_scroll_tag_t::_clean_up() {
}

rekordbox_anlz_t::song_structure_tag_t::song_structure_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent, rekordbox_anlz_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    m_body = nullptr;
    m__io__raw_body = nullptr;
    f_c = false;
    f_raw_mood = false;
    f_is_masked = false;
    _read();
}

void rekordbox_anlz_t::song_structure_tag_t::_read() {
    m_len_entry_bytes = m__io->read_u4be();
    m_len_entries = m__io->read_u2be();
    m__raw_body = m__io->read_bytes_full();
    m__io__raw_body = std::unique_ptr<kaitai::kstream>(new kaitai::kstream(m__raw_body));
    m_body = std::unique_ptr<song_structure_body_t>(
            new song_structure_body_t(m__io__raw_body.get(), this, m__root));
}

rekordbox_anlz_t::song_structure_tag_t::~song_structure_tag_t() {
    _clean_up();
}

void rekordbox_anlz_t::song_structure_tag_t::_clean_up() {
    if (f_raw_mood) {
    }
}

uint16_t rekordbox_anlz_t::song_structure_tag_t::c() {
    if (f_c)
        return m_c;
    m_c = len_entries();
    f_c = true;
    return m_c;
}

uint16_t rekordbox_anlz_t::song_structure_tag_t::raw_mood() {
    if (f_raw_mood)
        return m_raw_mood;
    std::streampos _pos = m__io->pos();
    m__io->seek(6);
    m_raw_mood = m__io->read_u2be();
    m__io->seek(_pos);
    f_raw_mood = true;
    return m_raw_mood;
}

bool rekordbox_anlz_t::song_structure_tag_t::is_masked() {
    if (f_is_masked)
        return m_is_masked;
    m_is_masked = raw_mood() > 20;
    f_is_masked = true;
    return m_is_masked;
}

rekordbox_anlz_t::cue_extended_entry_t::cue_extended_entry_t(kaitai::kstream* p__io, rekordbox_anlz_t::cue_extended_tag_t* p__parent, rekordbox_anlz_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void rekordbox_anlz_t::cue_extended_entry_t::_read() {
    m_magic = m__io->read_bytes(4);
    if (!(magic() == std::string("\x50\x43\x50\x32", 4))) {
        throw kaitai::validation_not_equal_error<std::string>(
                std::string("\x50\x43\x50\x32", 4),
                magic(),
                _io(),
                std::string("/types/cue_extended_entry/seq/0"));
    }
    m_len_header = m__io->read_u4be();
    m_len_entry = m__io->read_u4be();
    m_hot_cue = m__io->read_u4be();
    m_type = static_cast<rekordbox_anlz_t::cue_entry_type_t>(m__io->read_u1());
    m__unnamed5 = m__io->read_bytes(3);
    m_time = m__io->read_u4be();
    m_loop_time = m__io->read_u4be();
    m_color_id = m__io->read_u1();
    m__unnamed9 = m__io->read_bytes(7);
    m_loop_numerator = m__io->read_u2be();
    m_loop_denominator = m__io->read_u2be();
    n_len_comment = true;
    if (len_entry() > 43) {
        n_len_comment = false;
        m_len_comment = m__io->read_u4be();
    }
    n_comment = true;
    if (len_entry() > 43) {
        n_comment = false;
        m_comment = kaitai::kstream::bytes_to_str(m__io->read_bytes(len_comment()), std::string("utf-16be"));
    }
    n_color_code = true;
    if ((len_entry() - len_comment()) > 44) {
        n_color_code = false;
        m_color_code = m__io->read_u1();
    }
    n_color_red = true;
    if ((len_entry() - len_comment()) > 45) {
        n_color_red = false;
        m_color_red = m__io->read_u1();
    }
    n_color_green = true;
    if ((len_entry() - len_comment()) > 46) {
        n_color_green = false;
        m_color_green = m__io->read_u1();
    }
    n_color_blue = true;
    if ((len_entry() - len_comment()) > 47) {
        n_color_blue = false;
        m_color_blue = m__io->read_u1();
    }
    n__unnamed18 = true;
    if ((len_entry() - len_comment()) > 48) {
        n__unnamed18 = false;
        m__unnamed18 = m__io->read_bytes(((len_entry() - 48) - len_comment()));
    }
}

rekordbox_anlz_t::cue_extended_entry_t::~cue_extended_entry_t() {
    _clean_up();
}

void rekordbox_anlz_t::cue_extended_entry_t::_clean_up() {
    if (!n_len_comment) {
    }
    if (!n_comment) {
    }
    if (!n_color_code) {
    }
    if (!n_color_red) {
    }
    if (!n_color_green) {
    }
    if (!n_color_blue) {
    }
    if (!n__unnamed18) {
    }
}

rekordbox_anlz_t::vbr_tag_t::vbr_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent, rekordbox_anlz_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    m_index = nullptr;
    _read();
}

void rekordbox_anlz_t::vbr_tag_t::_read() {
    m__unnamed0 = m__io->read_u4be();
    m_index = std::unique_ptr<std::vector<uint32_t>>(new std::vector<uint32_t>());
    const int l_index = 400;
    for (int i = 0; i < l_index; i++) {
        m_index->push_back(std::move(m__io->read_u4be()));
    }
}

rekordbox_anlz_t::vbr_tag_t::~vbr_tag_t() {
    _clean_up();
}

void rekordbox_anlz_t::vbr_tag_t::_clean_up() {
}

rekordbox_anlz_t::song_structure_entry_t::song_structure_entry_t(
        kaitai::kstream* p__io,
        rekordbox_anlz_t::song_structure_body_t* p__parent,
        rekordbox_anlz_t* p__root)
        : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void rekordbox_anlz_t::song_structure_entry_t::_read() {
    m_index = m__io->read_u2be();
    m_beat = m__io->read_u2be();
    switch (_parent()->mood()) {
    case rekordbox_anlz_t::TRACK_MOOD_HIGH: {
        m_kind = std::unique_ptr<phrase_high_t>(new phrase_high_t(m__io, this, m__root));
        break;
    }
    case rekordbox_anlz_t::TRACK_MOOD_MID: {
        m_kind = std::unique_ptr<phrase_mid_t>(new phrase_mid_t(m__io, this, m__root));
        break;
    }
    case rekordbox_anlz_t::TRACK_MOOD_LOW: {
        m_kind = std::unique_ptr<phrase_low_t>(new phrase_low_t(m__io, this, m__root));
        break;
    }
    default: {
        m_kind = std::unique_ptr<phrase_mid_t>(new phrase_mid_t(m__io, this, m__root));
        break;
    }
    }
    m__unnamed3 = m__io->read_bytes(1);
    m_k1 = m__io->read_u1();
    m__unnamed5 = m__io->read_bytes(1);
    m_k2 = m__io->read_u1();
    m__unnamed7 = m__io->read_bytes(1);
    m_b = m__io->read_u1();
    m_beat2 = m__io->read_u2be();
    m_beat3 = m__io->read_u2be();
    m_beat4 = m__io->read_u2be();
    m__unnamed12 = m__io->read_bytes(1);
    m_k3 = m__io->read_u1();
    m__unnamed14 = m__io->read_bytes(1);
    m_fill = m__io->read_u1();
    m_beat_fill = m__io->read_u2be();
}

rekordbox_anlz_t::song_structure_entry_t::~song_structure_entry_t() {
    _clean_up();
}

void rekordbox_anlz_t::song_structure_entry_t::_clean_up() {
}

rekordbox_anlz_t::cue_entry_t::cue_entry_t(kaitai::kstream* p__io, rekordbox_anlz_t::cue_tag_t* p__parent, rekordbox_anlz_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void rekordbox_anlz_t::cue_entry_t::_read() {
    m_magic = m__io->read_bytes(4);
    if (!(magic() == std::string("\x50\x43\x50\x54", 4))) {
        throw kaitai::validation_not_equal_error<std::string>(
                std::string("\x50\x43\x50\x54", 4),
                magic(),
                _io(),
                std::string("/types/cue_entry/seq/0"));
    }
    m_len_header = m__io->read_u4be();
    m_len_entry = m__io->read_u4be();
    m_hot_cue = m__io->read_u4be();
    m_status = static_cast<rekordbox_anlz_t::cue_entry_status_t>(m__io->read_u4be());
    m__unnamed5 = m__io->read_u4be();
    m_order_first = m__io->read_u2be();
    m_order_last = m__io->read_u2be();
    m_type = static_cast<rekordbox_anlz_t::cue_entry_type_t>(m__io->read_u1());
    m__unnamed9 = m__io->read_bytes(3);
    m_time = m__io->read_u4be();
    m_loop_time = m__io->read_u4be();
    m__unnamed12 = m__io->read_bytes(16);
}

rekordbox_anlz_t::cue_entry_t::~cue_entry_t() {
    _clean_up();
}

void rekordbox_anlz_t::cue_entry_t::_clean_up() {
}

rekordbox_anlz_t::beat_grid_beat_t::beat_grid_beat_t(kaitai::kstream* p__io, rekordbox_anlz_t::beat_grid_tag_t* p__parent, rekordbox_anlz_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void rekordbox_anlz_t::beat_grid_beat_t::_read() {
    m_beat_number = m__io->read_u2be();
    m_tempo = m__io->read_u2be();
    m_time = m__io->read_u4be();
}

rekordbox_anlz_t::beat_grid_beat_t::~beat_grid_beat_t() {
    _clean_up();
}

void rekordbox_anlz_t::beat_grid_beat_t::_clean_up() {
}

rekordbox_anlz_t::cue_extended_tag_t::cue_extended_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent, rekordbox_anlz_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    m_cues = nullptr;
    _read();
}

void rekordbox_anlz_t::cue_extended_tag_t::_read() {
    m_type = static_cast<rekordbox_anlz_t::cue_list_type_t>(m__io->read_u4be());
    m_num_cues = m__io->read_u2be();
    m__unnamed2 = m__io->read_bytes(2);
    m_cues =
            std::unique_ptr<std::vector<std::unique_ptr<cue_extended_entry_t>>>(
                    new std::vector<std::unique_ptr<cue_extended_entry_t>>());
    const int l_cues = num_cues();
    for (int i = 0; i < l_cues; i++) {
        m_cues->push_back(std::move(std::unique_ptr<cue_extended_entry_t>(
                new cue_extended_entry_t(m__io, this, m__root))));
    }
}

rekordbox_anlz_t::cue_extended_tag_t::~cue_extended_tag_t() {
    _clean_up();
}

void rekordbox_anlz_t::cue_extended_tag_t::_clean_up() {
}

rekordbox_anlz_t::phrase_low_t::phrase_low_t(kaitai::kstream* p__io,
        rekordbox_anlz_t::song_structure_entry_t* p__parent,
        rekordbox_anlz_t* p__root)
        : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void rekordbox_anlz_t::phrase_low_t::_read() {
    m_id = static_cast<rekordbox_anlz_t::mood_low_phrase_t>(m__io->read_u2be());
}

rekordbox_anlz_t::phrase_low_t::~phrase_low_t() {
    _clean_up();
}

void rekordbox_anlz_t::phrase_low_t::_clean_up() {
}

rekordbox_anlz_t::unknown_tag_t::unknown_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent, rekordbox_anlz_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void rekordbox_anlz_t::unknown_tag_t::_read() {
}

rekordbox_anlz_t::unknown_tag_t::~unknown_tag_t() {
    _clean_up();
}

void rekordbox_anlz_t::unknown_tag_t::_clean_up() {
}

rekordbox_anlz_t::tagged_section_t::tagged_section_t(kaitai::kstream* p__io, rekordbox_anlz_t* p__parent, rekordbox_anlz_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    m__io__raw_body = nullptr;
    _read();
}

void rekordbox_anlz_t::tagged_section_t::_read() {
    m_fourcc = static_cast<rekordbox_anlz_t::section_tags_t>(m__io->read_s4be());
    m_len_header = m__io->read_u4be();
    m_len_tag = m__io->read_u4be();
    switch (fourcc()) {
    case rekordbox_anlz_t::SECTION_TAGS_WAVE_COLOR_SCROLL: {
        m__raw_body = m__io->read_bytes((len_tag() - 12));
        m__io__raw_body = std::unique_ptr<kaitai::kstream>(new kaitai::kstream(m__raw_body));
        m_body = std::unique_ptr<wave_color_scroll_tag_t>(
                new wave_color_scroll_tag_t(
                        m__io__raw_body.get(), this, m__root));
        break;
    }
    case rekordbox_anlz_t::SECTION_TAGS_WAVE_SCROLL: {
        m__raw_body = m__io->read_bytes((len_tag() - 12));
        m__io__raw_body = std::unique_ptr<kaitai::kstream>(new kaitai::kstream(m__raw_body));
        m_body = std::unique_ptr<wave_scroll_tag_t>(
                new wave_scroll_tag_t(m__io__raw_body.get(), this, m__root));
        break;
    }
    case rekordbox_anlz_t::SECTION_TAGS_VBR: {
        m__raw_body = m__io->read_bytes((len_tag() - 12));
        m__io__raw_body = std::unique_ptr<kaitai::kstream>(new kaitai::kstream(m__raw_body));
        m_body = std::unique_ptr<vbr_tag_t>(new vbr_tag_t(m__io__raw_body.get(), this, m__root));
        break;
    }
    case rekordbox_anlz_t::SECTION_TAGS_CUES_2: {
        m__raw_body = m__io->read_bytes((len_tag() - 12));
        m__io__raw_body = std::unique_ptr<kaitai::kstream>(new kaitai::kstream(m__raw_body));
        m_body = std::unique_ptr<cue_extended_tag_t>(
                new cue_extended_tag_t(m__io__raw_body.get(), this, m__root));
        break;
    }
    case rekordbox_anlz_t::SECTION_TAGS_CUES: {
        m__raw_body = m__io->read_bytes((len_tag() - 12));
        m__io__raw_body = std::unique_ptr<kaitai::kstream>(new kaitai::kstream(m__raw_body));
        m_body = std::unique_ptr<cue_tag_t>(new cue_tag_t(m__io__raw_body.get(), this, m__root));
        break;
    }
    case rekordbox_anlz_t::SECTION_TAGS_SONG_STRUCTURE: {
        m__raw_body = m__io->read_bytes((len_tag() - 12));
        m__io__raw_body = std::unique_ptr<kaitai::kstream>(new kaitai::kstream(m__raw_body));
        m_body = std::unique_ptr<song_structure_tag_t>(
                new song_structure_tag_t(m__io__raw_body.get(), this, m__root));
        break;
    }
    case rekordbox_anlz_t::SECTION_TAGS_BEAT_GRID: {
        m__raw_body = m__io->read_bytes((len_tag() - 12));
        m__io__raw_body = std::unique_ptr<kaitai::kstream>(new kaitai::kstream(m__raw_body));
        m_body = std::unique_ptr<beat_grid_tag_t>(
                new beat_grid_tag_t(m__io__raw_body.get(), this, m__root));
        break;
    }
    case rekordbox_anlz_t::SECTION_TAGS_WAVE_PREVIEW: {
        m__raw_body = m__io->read_bytes((len_tag() - 12));
        m__io__raw_body = std::unique_ptr<kaitai::kstream>(new kaitai::kstream(m__raw_body));
        m_body = std::unique_ptr<wave_preview_tag_t>(
                new wave_preview_tag_t(m__io__raw_body.get(), this, m__root));
        break;
    }
    case rekordbox_anlz_t::SECTION_TAGS_WAVE_COLOR_PREVIEW: {
        m__raw_body = m__io->read_bytes((len_tag() - 12));
        m__io__raw_body = std::unique_ptr<kaitai::kstream>(new kaitai::kstream(m__raw_body));
        m_body = std::unique_ptr<wave_color_preview_tag_t>(
                new wave_color_preview_tag_t(
                        m__io__raw_body.get(), this, m__root));
        break;
    }
    case rekordbox_anlz_t::SECTION_TAGS_PATH: {
        m__raw_body = m__io->read_bytes((len_tag() - 12));
        m__io__raw_body = std::unique_ptr<kaitai::kstream>(new kaitai::kstream(m__raw_body));
        m_body = std::unique_ptr<path_tag_t>(new path_tag_t(m__io__raw_body.get(), this, m__root));
        break;
    }
    case rekordbox_anlz_t::SECTION_TAGS_WAVE_TINY: {
        m__raw_body = m__io->read_bytes((len_tag() - 12));
        m__io__raw_body = std::unique_ptr<kaitai::kstream>(new kaitai::kstream(m__raw_body));
        m_body = std::unique_ptr<wave_preview_tag_t>(
                new wave_preview_tag_t(m__io__raw_body.get(), this, m__root));
        break;
    }
    default: {
        m__raw_body = m__io->read_bytes((len_tag() - 12));
        m__io__raw_body = std::unique_ptr<kaitai::kstream>(new kaitai::kstream(m__raw_body));
        m_body = std::unique_ptr<unknown_tag_t>(
                new unknown_tag_t(m__io__raw_body.get(), this, m__root));
        break;
    }
    }
}

rekordbox_anlz_t::tagged_section_t::~tagged_section_t() {
    _clean_up();
}

void rekordbox_anlz_t::tagged_section_t::_clean_up() {
}

rekordbox_anlz_t::wave_color_scroll_tag_t::wave_color_scroll_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent, rekordbox_anlz_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void rekordbox_anlz_t::wave_color_scroll_tag_t::_read() {
    m_len_entry_bytes = m__io->read_u4be();
    m_len_entries = m__io->read_u4be();
    m__unnamed2 = m__io->read_u4be();
    m_entries = m__io->read_bytes((len_entries() * len_entry_bytes()));
}

rekordbox_anlz_t::wave_color_scroll_tag_t::~wave_color_scroll_tag_t() {
    _clean_up();
}

void rekordbox_anlz_t::wave_color_scroll_tag_t::_clean_up() {
}

rekordbox_anlz_t::cue_tag_t::cue_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent, rekordbox_anlz_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    m_cues = nullptr;
    _read();
}

void rekordbox_anlz_t::cue_tag_t::_read() {
    m_type = static_cast<rekordbox_anlz_t::cue_list_type_t>(m__io->read_u4be());
    m__unnamed1 = m__io->read_bytes(2);
    m_num_cues = m__io->read_u2be();
    m_memory_count = m__io->read_u4be();
    m_cues = std::unique_ptr<std::vector<std::unique_ptr<cue_entry_t>>>(
            new std::vector<std::unique_ptr<cue_entry_t>>());
    const int l_cues = num_cues();
    for (int i = 0; i < l_cues; i++) {
        m_cues->push_back(std::move(std::unique_ptr<cue_entry_t>(
                new cue_entry_t(m__io, this, m__root))));
    }
}

rekordbox_anlz_t::cue_tag_t::~cue_tag_t() {
    _clean_up();
}

void rekordbox_anlz_t::cue_tag_t::_clean_up() {
}
