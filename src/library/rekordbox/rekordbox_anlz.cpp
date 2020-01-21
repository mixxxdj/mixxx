// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "rekordbox_anlz.h"



rekordbox_anlz_t::rekordbox_anlz_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent, rekordbox_anlz_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = this;
    _read();
}

void rekordbox_anlz_t::_read() {
    m__unnamed0 = m__io->ensure_fixed_contents(std::string("\x50\x4D\x41\x49", 4));
    m_len_header = m__io->read_u4be();
    m_len_file = m__io->read_u4be();
    m__unnamed3 = m__io->read_bytes((len_header() - _io()->pos()));
    m_sections = new std::vector<tagged_section_t*>();
    {
        int i = 0;
        while (!m__io->is_eof()) {
            m_sections->push_back(new tagged_section_t(m__io, this, m__root));
            i++;
        }
    }
}

rekordbox_anlz_t::~rekordbox_anlz_t() {
    for (std::vector<tagged_section_t*>::iterator it = m_sections->begin(); it != m_sections->end(); ++it) {
        delete *it;
    }
    delete m_sections;
}

rekordbox_anlz_t::phrase_up_down_t::phrase_up_down_t(kaitai::kstream* p__io, rekordbox_anlz_t::song_structure_entry_t* p__parent, rekordbox_anlz_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void rekordbox_anlz_t::phrase_up_down_t::_read() {
    m_id = static_cast<rekordbox_anlz_t::phrase_up_down_id_t>(m__io->read_u2be());
}

rekordbox_anlz_t::phrase_up_down_t::~phrase_up_down_t() {
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
    if (!n_path) {
    }
}

rekordbox_anlz_t::wave_preview_tag_t::wave_preview_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent, rekordbox_anlz_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void rekordbox_anlz_t::wave_preview_tag_t::_read() {
    m_len_preview = m__io->read_u4be();
    m__unnamed1 = m__io->read_u4be();
    n_data = true;
    if (_parent()->len_tag() > _parent()->len_header()) {
        n_data = false;
        m_data = m__io->read_bytes(len_preview());
    }
}

rekordbox_anlz_t::wave_preview_tag_t::~wave_preview_tag_t() {
    if (!n_data) {
    }
}

rekordbox_anlz_t::beat_grid_tag_t::beat_grid_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent, rekordbox_anlz_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void rekordbox_anlz_t::beat_grid_tag_t::_read() {
    m__unnamed0 = m__io->read_u4be();
    m__unnamed1 = m__io->read_u4be();
    m_len_beats = m__io->read_u4be();
    int l_beats = len_beats();
    m_beats = new std::vector<beat_grid_beat_t*>();
    m_beats->reserve(l_beats);
    for (int i = 0; i < l_beats; i++) {
        m_beats->push_back(new beat_grid_beat_t(m__io, this, m__root));
    }
}

rekordbox_anlz_t::beat_grid_tag_t::~beat_grid_tag_t() {
    for (std::vector<beat_grid_beat_t*>::iterator it = m_beats->begin(); it != m_beats->end(); ++it) {
        delete *it;
    }
    delete m_beats;
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
}

rekordbox_anlz_t::wave_scroll_tag_t::wave_scroll_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent, rekordbox_anlz_t* p__root) : kaitai::kstruct(p__io) {
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
}

rekordbox_anlz_t::phrase_verse_bridge_t::phrase_verse_bridge_t(kaitai::kstream* p__io, rekordbox_anlz_t::song_structure_entry_t* p__parent, rekordbox_anlz_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void rekordbox_anlz_t::phrase_verse_bridge_t::_read() {
    m_id = static_cast<rekordbox_anlz_t::phrase_verse_bridge_id_t>(m__io->read_u2be());
}

rekordbox_anlz_t::phrase_verse_bridge_t::~phrase_verse_bridge_t() {
}

rekordbox_anlz_t::song_structure_tag_t::song_structure_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent, rekordbox_anlz_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void rekordbox_anlz_t::song_structure_tag_t::_read() {
    m_len_entry_bytes = m__io->read_u4be();
    m_len_entries = m__io->read_u2be();
    m_style = m__io->read_u2be();
    m__unnamed3 = m__io->read_bytes(6);
    m_end_beat = m__io->read_u2be();
    m__unnamed5 = m__io->read_bytes(4);
    int l_entries = len_entries();
    m_entries = new std::vector<song_structure_entry_t*>();
    m_entries->reserve(l_entries);
    for (int i = 0; i < l_entries; i++) {
        m_entries->push_back(new song_structure_entry_t(m__io, this, m__root));
    }
}

rekordbox_anlz_t::song_structure_tag_t::~song_structure_tag_t() {
    for (std::vector<song_structure_entry_t*>::iterator it = m_entries->begin(); it != m_entries->end(); ++it) {
        delete *it;
    }
    delete m_entries;
}

rekordbox_anlz_t::cue_extended_entry_t::cue_extended_entry_t(kaitai::kstream* p__io, rekordbox_anlz_t::cue_extended_tag_t* p__parent, rekordbox_anlz_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void rekordbox_anlz_t::cue_extended_entry_t::_read() {
    m__unnamed0 = m__io->ensure_fixed_contents(std::string("\x50\x43\x50\x32", 4));
    m_len_header = m__io->read_u4be();
    m_len_entry = m__io->read_u4be();
    m_hot_cue = m__io->read_u4be();
    m_type = static_cast<rekordbox_anlz_t::cue_entry_type_t>(m__io->read_u1());
    m__unnamed5 = m__io->read_bytes(3);
    m_time = m__io->read_u4be();
    m_loop_time = m__io->read_u4be();
    m_color_id = m__io->read_u1();
    m__unnamed9 = m__io->read_bytes(11);
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
    n__unnamed16 = true;
    if ((len_entry() - len_comment()) > 48) {
        n__unnamed16 = false;
        m__unnamed16 = m__io->read_bytes(((len_entry() - 48) - len_comment()));
    }
}

rekordbox_anlz_t::cue_extended_entry_t::~cue_extended_entry_t() {
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
    if (!n__unnamed16) {
    }
}

rekordbox_anlz_t::vbr_tag_t::vbr_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent, rekordbox_anlz_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void rekordbox_anlz_t::vbr_tag_t::_read() {
    m__unnamed0 = m__io->read_u4be();
    int l_index = 400;
    m_index = new std::vector<uint32_t>();
    m_index->reserve(l_index);
    for (int i = 0; i < l_index; i++) {
        m_index->push_back(m__io->read_u4be());
    }
}

rekordbox_anlz_t::vbr_tag_t::~vbr_tag_t() {
    delete m_index;
}

rekordbox_anlz_t::song_structure_entry_t::song_structure_entry_t(kaitai::kstream* p__io, rekordbox_anlz_t::song_structure_tag_t* p__parent, rekordbox_anlz_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void rekordbox_anlz_t::song_structure_entry_t::_read() {
    m_phrase_number = m__io->read_u2be();
    m_beat_number = m__io->read_u2be();
    switch (_parent()->style()) {
    case 1: {
        m_phrase_id = new phrase_up_down_t(m__io, this, m__root);
        break;
    }
    case 2: {
        m_phrase_id = new phrase_verse_bridge_t(m__io, this, m__root);
        break;
    }
    default: {
        m_phrase_id = new phrase_verse_bridge_t(m__io, this, m__root);
        break;
    }
    }
    m__unnamed3 = m__io->read_bytes((_parent()->len_entry_bytes() - 9));
    m_fill_in = m__io->read_u1();
    m_fill_in_beat_number = m__io->read_u2be();
}

rekordbox_anlz_t::song_structure_entry_t::~song_structure_entry_t() {
    delete m_phrase_id;
}

rekordbox_anlz_t::cue_entry_t::cue_entry_t(kaitai::kstream* p__io, rekordbox_anlz_t::cue_tag_t* p__parent, rekordbox_anlz_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void rekordbox_anlz_t::cue_entry_t::_read() {
    m__unnamed0 = m__io->ensure_fixed_contents(std::string("\x50\x43\x50\x54", 4));
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
}

rekordbox_anlz_t::cue_extended_tag_t::cue_extended_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent, rekordbox_anlz_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void rekordbox_anlz_t::cue_extended_tag_t::_read() {
    m_type = static_cast<rekordbox_anlz_t::cue_list_type_t>(m__io->read_u4be());
    m_len_cues = m__io->read_u2be();
    m__unnamed2 = m__io->read_bytes(2);
    int l_cues = len_cues();
    m_cues = new std::vector<cue_extended_entry_t*>();
    m_cues->reserve(l_cues);
    for (int i = 0; i < l_cues; i++) {
        m_cues->push_back(new cue_extended_entry_t(m__io, this, m__root));
    }
}

rekordbox_anlz_t::cue_extended_tag_t::~cue_extended_tag_t() {
    for (std::vector<cue_extended_entry_t*>::iterator it = m_cues->begin(); it != m_cues->end(); ++it) {
        delete *it;
    }
    delete m_cues;
}

rekordbox_anlz_t::unknown_tag_t::unknown_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent, rekordbox_anlz_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void rekordbox_anlz_t::unknown_tag_t::_read() {
}

rekordbox_anlz_t::unknown_tag_t::~unknown_tag_t() {
}

rekordbox_anlz_t::tagged_section_t::tagged_section_t(kaitai::kstream* p__io, rekordbox_anlz_t* p__parent, rekordbox_anlz_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void rekordbox_anlz_t::tagged_section_t::_read() {
    m_fourcc = m__io->read_s4be();
    m_len_header = m__io->read_u4be();
    m_len_tag = m__io->read_u4be();
    switch (fourcc()) {
    case 1346588482: {
        m__raw_body = m__io->read_bytes((len_tag() - 12));
        m__io__raw_body = new kaitai::kstream(m__raw_body);
        m_body = new cue_tag_t(m__io__raw_body, this, m__root);
        break;
    }
    case 1347900978: {
        m__raw_body = m__io->read_bytes((len_tag() - 12));
        m__io__raw_body = new kaitai::kstream(m__raw_body);
        m_body = new wave_preview_tag_t(m__io__raw_body, this, m__root);
        break;
    }
    case 1347900980: {
        m__raw_body = m__io->read_bytes((len_tag() - 12));
        m__io__raw_body = new kaitai::kstream(m__raw_body);
        m_body = new wave_color_preview_tag_t(m__io__raw_body, this, m__root);
        break;
    }
    case 1347895638: {
        m__raw_body = m__io->read_bytes((len_tag() - 12));
        m__io__raw_body = new kaitai::kstream(m__raw_body);
        m_body = new wave_preview_tag_t(m__io__raw_body, this, m__root);
        break;
    }
    case 1347900979: {
        m__raw_body = m__io->read_bytes((len_tag() - 12));
        m__io__raw_body = new kaitai::kstream(m__raw_body);
        m_body = new wave_scroll_tag_t(m__io__raw_body, this, m__root);
        break;
    }
    case 1347638089: {
        m__raw_body = m__io->read_bytes((len_tag() - 12));
        m__io__raw_body = new kaitai::kstream(m__raw_body);
        m_body = new song_structure_tag_t(m__io__raw_body, this, m__root);
        break;
    }
    case 1347507290: {
        m__raw_body = m__io->read_bytes((len_tag() - 12));
        m__io__raw_body = new kaitai::kstream(m__raw_body);
        m_body = new beat_grid_tag_t(m__io__raw_body, this, m__root);
        break;
    }
    case 1347830354: {
        m__raw_body = m__io->read_bytes((len_tag() - 12));
        m__io__raw_body = new kaitai::kstream(m__raw_body);
        m_body = new vbr_tag_t(m__io__raw_body, this, m__root);
        break;
    }
    case 1347900981: {
        m__raw_body = m__io->read_bytes((len_tag() - 12));
        m__io__raw_body = new kaitai::kstream(m__raw_body);
        m_body = new wave_color_scroll_tag_t(m__io__raw_body, this, m__root);
        break;
    }
    case 1346588466: {
        m__raw_body = m__io->read_bytes((len_tag() - 12));
        m__io__raw_body = new kaitai::kstream(m__raw_body);
        m_body = new cue_extended_tag_t(m__io__raw_body, this, m__root);
        break;
    }
    case 1347441736: {
        m__raw_body = m__io->read_bytes((len_tag() - 12));
        m__io__raw_body = new kaitai::kstream(m__raw_body);
        m_body = new path_tag_t(m__io__raw_body, this, m__root);
        break;
    }
    default: {
        m__raw_body = m__io->read_bytes((len_tag() - 12));
        m__io__raw_body = new kaitai::kstream(m__raw_body);
        m_body = new unknown_tag_t(m__io__raw_body, this, m__root);
        break;
    }
    }
}

rekordbox_anlz_t::tagged_section_t::~tagged_section_t() {
    delete m__io__raw_body;
    delete m_body;
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
}

rekordbox_anlz_t::cue_tag_t::cue_tag_t(kaitai::kstream* p__io, rekordbox_anlz_t::tagged_section_t* p__parent, rekordbox_anlz_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = p__root;
    _read();
}

void rekordbox_anlz_t::cue_tag_t::_read() {
    m_type = static_cast<rekordbox_anlz_t::cue_list_type_t>(m__io->read_u4be());
    m__unnamed1 = m__io->read_bytes(2);
    m_len_cues = m__io->read_u2be();
    m_memory_count = m__io->read_u4be();
    int l_cues = len_cues();
    m_cues = new std::vector<cue_entry_t*>();
    m_cues->reserve(l_cues);
    for (int i = 0; i < l_cues; i++) {
        m_cues->push_back(new cue_entry_t(m__io, this, m__root));
    }
}

rekordbox_anlz_t::cue_tag_t::~cue_tag_t() {
    for (std::vector<cue_entry_t*>::iterator it = m_cues->begin(); it != m_cues->end(); ++it) {
        delete *it;
    }
    delete m_cues;
}
