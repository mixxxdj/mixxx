#pragma once

// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "kaitaistruct.h"

#include <stdint.h>
#include <vector>

#if KAITAI_STRUCT_VERSION < 7000L
#error "Incompatible Kaitai Struct C++/STL API: version 0.7 or later is required"
#endif

/**
 * This is a relational database format designed to be efficiently used
 * by very low power devices (there were deployments on 16 bit devices
 * with 32K of RAM). Today you are most likely to encounter it within
 * the Pioneer Professional DJ ecosystem, because it is the format that
 * their rekordbox software uses to write USB and SD media which can be
 * mounted in DJ controllers and used to play and mix music.
 *
 * It has been reverse-engineered to facilitate sophisticated
 * integrations with light and laser shows, videos, and other musical
 * instruments, by supporting deep knowledge of what is playing and
 * what is coming next through monitoring the network communications of
 * the players.
 *
 * The file is divided into fixed-size blocks. The first block has a
 * header that establishes the block size, and lists the tables
 * available in the database, identifying their types and the index of
 * the first of the series of linked pages that make up that table.
 *
 * Each table is made up of a series of rows which may be spread across
 * any number of pages. The pages start with a header describing the
 * page and linking to the next page. The rest of the page is used as a
 * heap: rows are scattered around it, and located using an index
 * structure that builds backwards from the end of the page. Each row
 * of a given type has a fixed size structure which links to any
 * variable-sized strings by their offsets within the page.
 *
 * As changes are made to the table, some records may become unused,
 * and there may be gaps within the heap that are too small to be used
 * by other data. There is a bit map in the row index that identifies
 * which rows are actually present. Rows that are not present must be
 * ignored: they do not contain valid (or even necessarily well-formed)
 * data.
 *
 * The majority of the work in reverse-engineering this format was
 * performed by @henrybetts and @flesniak, for which I am hugely
 * grateful. @GreyCat helped me learn the intricacies (and best
 * practices) of Kaitai far faster than I would have managed on my own.
 * \sa Source
 */

class rekordbox_pdb_t : public kaitai::kstruct {

public:
    class device_sql_string_t;
    class playlist_tree_row_t;
    class color_row_t;
    class device_sql_short_ascii_t;
    class album_row_t;
    class page_t;
    class row_group_t;
    class genre_row_t;
    class artwork_row_t;
    class device_sql_long_ascii_t;
    class artist_row_t;
    class page_ref_t;
    class device_sql_long_utf16be_t;
    class track_row_t;
    class key_row_t;
    class playlist_entry_row_t;
    class label_row_t;
    class table_t;
    class row_ref_t;

    enum page_type_t {
        PAGE_TYPE_TRACKS = 0,
        PAGE_TYPE_GENRES = 1,
        PAGE_TYPE_ARTISTS = 2,
        PAGE_TYPE_ALBUMS = 3,
        PAGE_TYPE_LABELS = 4,
        PAGE_TYPE_KEYS = 5,
        PAGE_TYPE_COLORS = 6,
        PAGE_TYPE_PLAYLIST_TREE = 7,
        PAGE_TYPE_PLAYLIST_ENTRIES = 8,
        PAGE_TYPE_UNKNOWN_9 = 9,
        PAGE_TYPE_UNKNOWN_10 = 10,
        PAGE_TYPE_UNKNOWN_11 = 11,
        PAGE_TYPE_UNKNOWN_12 = 12,
        PAGE_TYPE_ARTWORK = 13,
        PAGE_TYPE_UNKNOWN_14 = 14,
        PAGE_TYPE_UNKNOWN_15 = 15,
        PAGE_TYPE_COLUMNS = 16,
        PAGE_TYPE_UNKNOWN_17 = 17,
        PAGE_TYPE_UNKNOWN_18 = 18,
        PAGE_TYPE_HISTORY = 19
    };

    rekordbox_pdb_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, rekordbox_pdb_t* p__root = 0);

private:
    void _read();

public:
    ~rekordbox_pdb_t();

    /**
     * A variable length string which can be stored in a variety of
     * different encodings.
     */

    class device_sql_string_t : public kaitai::kstruct {

    public:

        device_sql_string_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, rekordbox_pdb_t* p__root = 0);

    private:
        void _read();

    public:
        ~device_sql_string_t();

    private:
        uint8_t m_length_and_kind;
        kaitai::kstruct* m_body;
        rekordbox_pdb_t* m__root;
        kaitai::kstruct* m__parent;

    public:

        /**
         * Mangled length of an ordinary ASCII string if odd, or a flag
         * indicating another encoding with a longer length value to
         * follow.
         */
        uint8_t length_and_kind() const { return m_length_and_kind; }
        kaitai::kstruct* body() const { return m_body; }
        rekordbox_pdb_t* _root() const { return m__root; }
        kaitai::kstruct* _parent() const { return m__parent; }
    };

    /**
     * A row that holds a playlist name, ID, indication of whether it
     * is an ordinary playlist or a folder of other playlists, a link
     * to its parent folder, and its sort order.
     */

    class playlist_tree_row_t : public kaitai::kstruct {

    public:

        playlist_tree_row_t(kaitai::kstream* p__io, rekordbox_pdb_t::row_ref_t* p__parent = 0, rekordbox_pdb_t* p__root = 0);

    private:
        void _read();

    public:
        ~playlist_tree_row_t();

    private:
        bool f_is_folder;
        bool m_is_folder;

    public:
        bool is_folder();

    private:
        uint32_t m_parent_id;
        std::string m__unnamed1;
        uint32_t m_sort_order;
        uint32_t m_id;
        uint32_t m_raw_is_folder;
        device_sql_string_t* m_name;
        rekordbox_pdb_t* m__root;
        rekordbox_pdb_t::row_ref_t* m__parent;

    public:

        /**
         * The ID of the `playlist_tree_row` in which this one can be
         * found, or `0` if this playlist exists at the root level.
         */
        uint32_t parent_id() const { return m_parent_id; }
        std::string _unnamed1() const { return m__unnamed1; }

        /**
         * The order in which the entries of this playlist are sorted.
         */
        uint32_t sort_order() const { return m_sort_order; }

        /**
         * The unique identifier by which this playlist or folder can
         * be requested and linked from other rows.
         */
        uint32_t id() const { return m_id; }

        /**
         * Has a non-zero value if this is actually a folder rather
         * than a playlist.
         */
        uint32_t raw_is_folder() const { return m_raw_is_folder; }

        /**
         * The variable-length string naming the playlist.
         */
        device_sql_string_t* name() const { return m_name; }
        rekordbox_pdb_t* _root() const { return m__root; }
        rekordbox_pdb_t::row_ref_t* _parent() const { return m__parent; }
    };

    /**
     * A row that holds a color name and the associated ID.
     */

    class color_row_t : public kaitai::kstruct {

    public:

        color_row_t(kaitai::kstream* p__io, rekordbox_pdb_t::row_ref_t* p__parent = 0, rekordbox_pdb_t* p__root = 0);

    private:
        void _read();

    public:
        ~color_row_t();

    private:
        std::string m__unnamed0;
        uint16_t m_id;
        uint8_t m__unnamed2;
        device_sql_string_t* m_name;
        rekordbox_pdb_t* m__root;
        rekordbox_pdb_t::row_ref_t* m__parent;

    public:
        std::string _unnamed0() const { return m__unnamed0; }

        /**
         * The unique identifier by which this color can be requested
         * and linked from other rows (such as tracks).
         */
        uint16_t id() const { return m_id; }
        uint8_t _unnamed2() const { return m__unnamed2; }

        /**
         * The variable-length string naming the color.
         */
        device_sql_string_t* name() const { return m_name; }
        rekordbox_pdb_t* _root() const { return m__root; }
        rekordbox_pdb_t::row_ref_t* _parent() const { return m__parent; }
    };

    /**
     * An ASCII-encoded string up to 127 bytes long.
     */

    class device_sql_short_ascii_t : public kaitai::kstruct {

    public:

        device_sql_short_ascii_t(uint8_t p_mangled_length, kaitai::kstream* p__io, rekordbox_pdb_t::device_sql_string_t* p__parent = 0, rekordbox_pdb_t* p__root = 0);

    private:
        void _read();

    public:
        ~device_sql_short_ascii_t();

    private:
        bool f_length;
        int32_t m_length;

    public:

        /**
         * The un-mangled length of the string, in bytes.
         */
        int32_t length();

    private:
        std::string m_text;
        bool n_text;

    public:
        bool _is_null_text() { text(); return n_text; };

    private:
        uint8_t m_mangled_length;
        rekordbox_pdb_t* m__root;
        rekordbox_pdb_t::device_sql_string_t* m__parent;

    public:

        /**
         * The content of the string.
         */
        std::string text() const { return m_text; }

        /**
         * Contains the actual length, incremented, doubled, and
         * incremented again. Go figure.
         */
        uint8_t mangled_length() const { return m_mangled_length; }
        rekordbox_pdb_t* _root() const { return m__root; }
        rekordbox_pdb_t::device_sql_string_t* _parent() const { return m__parent; }
    };

    /**
     * A row that holds an album name and ID.
     */

    class album_row_t : public kaitai::kstruct {

    public:

        album_row_t(kaitai::kstream* p__io, rekordbox_pdb_t::row_ref_t* p__parent = 0, rekordbox_pdb_t* p__root = 0);

    private:
        void _read();

    public:
        ~album_row_t();

    private:
        bool f_name;
        device_sql_string_t* m_name;

    public:

        /**
         * The name of this album.
         */
        device_sql_string_t* name();

    private:
        uint16_t m__unnamed0;
        uint16_t m_index_shift;
        uint32_t m__unnamed2;
        uint32_t m_artist_id;
        uint32_t m_id;
        uint32_t m__unnamed5;
        uint8_t m__unnamed6;
        uint8_t m_ofs_name;
        rekordbox_pdb_t* m__root;
        rekordbox_pdb_t::row_ref_t* m__parent;

    public:

        /**
         * Some kind of magic word? Usually 0x80, 0x00.
         */
        uint16_t _unnamed0() const { return m__unnamed0; }

        /**
         * TODO name from @flesniak, but what does it mean?
         */
        uint16_t index_shift() const { return m_index_shift; }
        uint32_t _unnamed2() const { return m__unnamed2; }

        /**
         * Identifies the artist associated with the album.
         */
        uint32_t artist_id() const { return m_artist_id; }

        /**
         * The unique identifier by which this album can be requested
         * and linked from other rows (such as tracks).
         */
        uint32_t id() const { return m_id; }
        uint32_t _unnamed5() const { return m__unnamed5; }

        /**
         * @flesniak says: "alwayx 0x03, maybe an unindexed empty string"
         */
        uint8_t _unnamed6() const { return m__unnamed6; }

        /**
         * The location of the variable-length name string, relative to
         * the start of this row.
         */
        uint8_t ofs_name() const { return m_ofs_name; }
        rekordbox_pdb_t* _root() const { return m__root; }
        rekordbox_pdb_t::row_ref_t* _parent() const { return m__parent; }
    };

    /**
     * A table page, consisting of a short header describing the
     * content of the page and linking to the next page, followed by a
     * heap in which row data is found. At the end of the page there is
     * an index which locates all rows present in the heap via their
     * offsets past the end of the page header.
     */

    class page_t : public kaitai::kstruct {

    public:

        page_t(kaitai::kstream* p__io, rekordbox_pdb_t::page_ref_t* p__parent = 0, rekordbox_pdb_t* p__root = 0);

    private:
        void _read();

    public:
        ~page_t();

    private:
        bool f_num_rows;
        uint16_t m_num_rows;

    public:

        /**
         * The number of rows on this page (controls the number of row
         * index entries there are, but some of those may not be marked
         * as present in the table due to deletion).
         */
        uint16_t num_rows();

    private:
        bool f_num_groups;
        int32_t m_num_groups;

    public:

        /**
         * The number of row groups that are present in the index. Each
         * group can hold up to sixteen rows. All but the final one
         * will hold sixteen rows.
         */
        int32_t num_groups();

    private:
        bool f_row_groups;
        std::vector<row_group_t*>* m_row_groups;
        bool n_row_groups;

    public:
        bool _is_null_row_groups() { row_groups(); return n_row_groups; };

    private:

    public:

        /**
         * The actual row groups making up the row index. Each group
         * can hold up to sixteen rows. Non-data pages do not have
         * actual rows, and attempting to parse them can crash.
         */
        std::vector<row_group_t*>* row_groups();

    private:
        bool f_heap_pos;
        int32_t m_heap_pos;

    public:
        int32_t heap_pos();

    private:
        bool f_is_data_page;
        bool m_is_data_page;

    public:
        bool is_data_page();

    private:
        std::string m__unnamed0;
        uint32_t m_page_index;
        page_type_t m_type;
        page_ref_t* m_next_page;
        uint32_t m__unnamed4;
        std::string m__unnamed5;
        uint8_t m_num_rows_small;
        uint8_t m__unnamed7;
        uint8_t m__unnamed8;
        uint8_t m_page_flags;
        uint16_t m_free_size;
        uint16_t m_used_size;
        uint16_t m__unnamed12;
        uint16_t m_num_rows_large;
        uint16_t m__unnamed14;
        uint16_t m__unnamed15;
        std::string m_heap;
        bool n_heap;

    public:
        bool _is_null_heap() { heap(); return n_heap; };

    private:
        rekordbox_pdb_t* m__root;
        rekordbox_pdb_t::page_ref_t* m__parent;

    public:
        std::string _unnamed0() const { return m__unnamed0; }

        /**
         * Matches the index we used to look up the page, sanity check?
         */
        uint32_t page_index() const { return m_page_index; }

        /**
         * Identifies the type of information stored in the rows of this page.
         */
        page_type_t type() const { return m_type; }

        /**
         * Index of the next page containing this type of rows. Points past
         * the end of the file if there are no more.
         */
        page_ref_t* next_page() const { return m_next_page; }

        /**
         * @flesniak said: "sequence number (0->1: 8->13, 1->2: 22, 2->3: 27)"
         */
        uint32_t _unnamed4() const { return m__unnamed4; }
        std::string _unnamed5() const { return m__unnamed5; }

        /**
         * Holds the value used for `num_rows` (see below) unless
         * `num_rows_large` is larger (but not equal to `0x1fff`). This
         * seems like some strange mechanism to deal with the fact that
         * lots of tiny entries, such as are found in the
         * `playlist_entries` table, are too big to count with a single
         * byte. But why not just always use `num_rows_large`, then?
         */
        uint8_t num_rows_small() const { return m_num_rows_small; }

        /**
         * @flesniak said: "a bitmask (1st track: 32)"
         */
        uint8_t _unnamed7() const { return m__unnamed7; }

        /**
         * @flesniak said: "often 0, sometimes larger, esp. for pages
         * with high real_entry_count (e.g. 12 for 101 entries)"
         */
        uint8_t _unnamed8() const { return m__unnamed8; }

        /**
         * @flesniak said: "strange pages: 0x44, 0x64; otherwise seen: 0x24, 0x34"
         */
        uint8_t page_flags() const { return m_page_flags; }

        /**
         * Unused space (in bytes) in the page heap, excluding the row
         * index at end of page.
         */
        uint16_t free_size() const { return m_free_size; }

        /**
         * The number of bytes that are in use in the page heap.
         */
        uint16_t used_size() const { return m_used_size; }

        /**
         * @flesniak said: "(0->1: 2)"
         */
        uint16_t _unnamed12() const { return m__unnamed12; }

        /**
         * Holds the value used for `num_rows` (as described above)
         * when that is too large to fit into `num_rows_small`, and
         * that situation seems to be indicated when this value is
         * larger than `num_rows_small`, but not equal to `0x1fff`.
         * This seems like some strange mechanism to deal with the fact
         * that lots of tiny entries, such as are found in the
         * `playlist_entries` table, are too big to count with a single
         * byte. But why not just always use this value, then?
         */
        uint16_t num_rows_large() const { return m_num_rows_large; }

        /**
         * @flesniak said: "1004 for strange blocks, 0 otherwise"
         */
        uint16_t _unnamed14() const { return m__unnamed14; }

        /**
         * @flesniak said: "always 0 except 1 for history pages, num
         * entries for strange pages?"
         */
        uint16_t _unnamed15() const { return m__unnamed15; }
        std::string heap() const { return m_heap; }
        rekordbox_pdb_t* _root() const { return m__root; }
        rekordbox_pdb_t::page_ref_t* _parent() const { return m__parent; }
    };

    /**
     * A group of row indices, which are built backwards from the end
     * of the page. Holds up to sixteen row offsets, along with a bit
     * mask that indicates whether each row is actually present in the
     * table.
     */

    class row_group_t : public kaitai::kstruct {

    public:

        row_group_t(uint16_t p_group_index, kaitai::kstream* p__io, rekordbox_pdb_t::page_t* p__parent = 0, rekordbox_pdb_t* p__root = 0);

    private:
        void _read();

    public:
        ~row_group_t();

    private:
        bool f_base;
        int32_t m_base;

    public:

        /**
         * The starting point of this group of row indices.
         */
        int32_t base();

    private:
        bool f_row_present_flags;
        uint16_t m_row_present_flags;

    public:

        /**
         * Each bit specifies whether a particular row is present. The
         * low order bit corresponds to the first row in this index,
         * whose offset immediately precedes these flag bits. The
         * second bit corresponds to the row whose offset precedes
         * that, and so on.
         */
        uint16_t row_present_flags();

    private:
        bool f_rows;
        std::vector<row_ref_t*>* m_rows;

    public:

        /**
         * The row offsets in this group.
         */
        std::vector<row_ref_t*>* rows();

    private:
        uint16_t m_group_index;
        rekordbox_pdb_t* m__root;
        rekordbox_pdb_t::page_t* m__parent;

    public:

        /**
         * Identifies which group is being generated. They build backwards
         * from the end of the page.
         */
        uint16_t group_index() const { return m_group_index; }
        rekordbox_pdb_t* _root() const { return m__root; }
        rekordbox_pdb_t::page_t* _parent() const { return m__parent; }
    };

    /**
     * A row that holds a genre name and the associated ID.
     */

    class genre_row_t : public kaitai::kstruct {

    public:

        genre_row_t(kaitai::kstream* p__io, rekordbox_pdb_t::row_ref_t* p__parent = 0, rekordbox_pdb_t* p__root = 0);

    private:
        void _read();

    public:
        ~genre_row_t();

    private:
        uint32_t m_id;
        device_sql_string_t* m_name;
        rekordbox_pdb_t* m__root;
        rekordbox_pdb_t::row_ref_t* m__parent;

    public:

        /**
         * The unique identifier by which this genre can be requested
         * and linked from other rows (such as tracks).
         */
        uint32_t id() const { return m_id; }

        /**
         * The variable-length string naming the genre.
         */
        device_sql_string_t* name() const { return m_name; }
        rekordbox_pdb_t* _root() const { return m__root; }
        rekordbox_pdb_t::row_ref_t* _parent() const { return m__parent; }
    };

    /**
     * A row that holds the path to an album art image file and the
     * associated artwork ID.
     */

    class artwork_row_t : public kaitai::kstruct {

    public:

        artwork_row_t(kaitai::kstream* p__io, rekordbox_pdb_t::row_ref_t* p__parent = 0, rekordbox_pdb_t* p__root = 0);

    private:
        void _read();

    public:
        ~artwork_row_t();

    private:
        uint32_t m_id;
        device_sql_string_t* m_path;
        rekordbox_pdb_t* m__root;
        rekordbox_pdb_t::row_ref_t* m__parent;

    public:

        /**
         * The unique identifier by which this art can be requested
         * and linked from other rows (such as tracks).
         */
        uint32_t id() const { return m_id; }

        /**
         * The variable-length file path string at which the art file
         * can be found.
         */
        device_sql_string_t* path() const { return m_path; }
        rekordbox_pdb_t* _root() const { return m__root; }
        rekordbox_pdb_t::row_ref_t* _parent() const { return m__parent; }
    };

    /**
     * An ASCII-encoded string preceded by a two-byte length field.
     */

    class device_sql_long_ascii_t : public kaitai::kstruct {

    public:

        device_sql_long_ascii_t(kaitai::kstream* p__io, rekordbox_pdb_t::device_sql_string_t* p__parent = 0, rekordbox_pdb_t* p__root = 0);

    private:
        void _read();

    public:
        ~device_sql_long_ascii_t();

    private:
        uint16_t m_length;
        std::string m_text;
        rekordbox_pdb_t* m__root;
        rekordbox_pdb_t::device_sql_string_t* m__parent;

    public:

        /**
         * Contains the length of the string in bytes.
         */
        uint16_t length() const { return m_length; }

        /**
         * The content of the string.
         */
        std::string text() const { return m_text; }
        rekordbox_pdb_t* _root() const { return m__root; }
        rekordbox_pdb_t::device_sql_string_t* _parent() const { return m__parent; }
    };

    /**
     * A row that holds an artist name and ID.
     */

    class artist_row_t : public kaitai::kstruct {

    public:

        artist_row_t(kaitai::kstream* p__io, rekordbox_pdb_t::row_ref_t* p__parent = 0, rekordbox_pdb_t* p__root = 0);

    private:
        void _read();

    public:
        ~artist_row_t();

    private:
        bool f_ofs_name_far;
        uint16_t m_ofs_name_far;
        bool n_ofs_name_far;

    public:
        bool _is_null_ofs_name_far() { ofs_name_far(); return n_ofs_name_far; };

    private:

    public:

        /**
         * For names that might be further than 0xff bytes from the
         * start of this row, this holds a two-byte offset, and is
         * signalled by the subtype value.
         */
        uint16_t ofs_name_far();

    private:
        bool f_name;
        device_sql_string_t* m_name;

    public:

        /**
         * The name of this artist.
         */
        device_sql_string_t* name();

    private:
        uint16_t m_subtype;
        uint16_t m_index_shift;
        uint32_t m_id;
        uint8_t m__unnamed3;
        uint8_t m_ofs_name_near;
        rekordbox_pdb_t* m__root;
        rekordbox_pdb_t::row_ref_t* m__parent;

    public:

        /**
         * Usually 0x60, but 0x64 means we have a long name offset
         * embedded in the row.
         */
        uint16_t subtype() const { return m_subtype; }

        /**
         * TODO name from @flesniak, but what does it mean?
         */
        uint16_t index_shift() const { return m_index_shift; }

        /**
         * The unique identifier by which this artist can be requested
         * and linked from other rows (such as tracks).
         */
        uint32_t id() const { return m_id; }

        /**
         * @flesniak says: "always 0x03, maybe an unindexed empty string"
         */
        uint8_t _unnamed3() const { return m__unnamed3; }

        /**
         * The location of the variable-length name string, relative to
         * the start of this row, unless subtype is 0x64.
         */
        uint8_t ofs_name_near() const { return m_ofs_name_near; }
        rekordbox_pdb_t* _root() const { return m__root; }
        rekordbox_pdb_t::row_ref_t* _parent() const { return m__parent; }
    };

    /**
     * An index which points to a table page (its offset can be found
     * by multiplying the index by the `page_len` value in the file
     * header). This type allows the linked page to be lazy loaded.
     */

    class page_ref_t : public kaitai::kstruct {

    public:

        page_ref_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, rekordbox_pdb_t* p__root = 0);

    private:
        void _read();

    public:
        ~page_ref_t();

    private:
        bool f_body;
        page_t* m_body;

    public:

        /**
         * When referenced, loads the specified page and parses its
         * contents appropriately for the type of data it contains.
         */
        page_t* body();

    private:
        uint32_t m_index;
        rekordbox_pdb_t* m__root;
        kaitai::kstruct* m__parent;
        std::string m__raw_body;
        kaitai::kstream* m__io__raw_body;

    public:

        /**
         * Identifies the desired page number.
         */
        uint32_t index() const { return m_index; }
        rekordbox_pdb_t* _root() const { return m__root; }
        kaitai::kstruct* _parent() const { return m__parent; }
        std::string _raw_body() const { return m__raw_body; }
        kaitai::kstream* _io__raw_body() const { return m__io__raw_body; }
    };

    /**
     * A UTF-16BE-encoded string preceded by a two-byte length field.
     */

    class device_sql_long_utf16be_t : public kaitai::kstruct {

    public:

        device_sql_long_utf16be_t(kaitai::kstream* p__io, rekordbox_pdb_t::device_sql_string_t* p__parent = 0, rekordbox_pdb_t* p__root = 0);

    private:
        void _read();

    public:
        ~device_sql_long_utf16be_t();

    private:
        uint16_t m_length;
        std::string m_text;
        rekordbox_pdb_t* m__root;
        rekordbox_pdb_t::device_sql_string_t* m__parent;

    public:

        /**
         * Contains the length of the string in bytes, including two trailing nulls.
         */
        uint16_t length() const { return m_length; }

        /**
         * The content of the string.
         */
        std::string text() const { return m_text; }
        rekordbox_pdb_t* _root() const { return m__root; }
        rekordbox_pdb_t::device_sql_string_t* _parent() const { return m__parent; }
    };

    /**
     * A row that describes a track that can be played, with many
     * details about the music, and links to other tables like artists,
     * albums, keys, etc.
     */

    class track_row_t : public kaitai::kstruct {

    public:

        track_row_t(kaitai::kstream* p__io, rekordbox_pdb_t::row_ref_t* p__parent = 0, rekordbox_pdb_t* p__root = 0);

    private:
        void _read();

    public:
        ~track_row_t();

    private:
        bool f_unknown_string_8;
        device_sql_string_t* m_unknown_string_8;

    public:

        /**
         * A string of unknown purpose, usually empty.
         */
        device_sql_string_t* unknown_string_8();

    private:
        bool f_unknown_string_6;
        device_sql_string_t* m_unknown_string_6;

    public:

        /**
         * A string of unknown purpose, usually empty.
         */
        device_sql_string_t* unknown_string_6();

    private:
        bool f_analyze_date;
        device_sql_string_t* m_analyze_date;

    public:

        /**
         * A string containing the date this track was analyzed by rekordbox.
         */
        device_sql_string_t* analyze_date();

    private:
        bool f_file_path;
        device_sql_string_t* m_file_path;

    public:

        /**
         * The file path of the track audio file.
         */
        device_sql_string_t* file_path();

    private:
        bool f_autoload_hotcues;
        device_sql_string_t* m_autoload_hotcues;

    public:

        /**
         * A string whose value is always either empty or "ON", and
         * which apparently for some insane reason is used, rather than
         * a single bit somewhere, to control whether hot-cues are
         * auto-loaded for the track.
         */
        device_sql_string_t* autoload_hotcues();

    private:
        bool f_date_added;
        device_sql_string_t* m_date_added;

    public:

        /**
         * A string containing the date this track was added to the collection.
         */
        device_sql_string_t* date_added();

    private:
        bool f_unknown_string_3;
        device_sql_string_t* m_unknown_string_3;

    public:

        /**
         * A string of unknown purpose; @flesniak said "strange
         * strings, often zero length, sometimes low binary values
         * 0x01/0x02 as content"
         */
        device_sql_string_t* unknown_string_3();

    private:
        bool f_texter;
        device_sql_string_t* m_texter;

    public:

        /**
         * A string of unknown purpose, which @flesnik named.
         */
        device_sql_string_t* texter();

    private:
        bool f_kuvo_public;
        device_sql_string_t* m_kuvo_public;

    public:

        /**
         * A string whose value is always either empty or "ON", and
         * which apparently for some insane reason is used, rather than
         * a single bit somewhere, to control whether the track
         * information is visible on Kuvo.
         */
        device_sql_string_t* kuvo_public();

    private:
        bool f_mix_name;
        device_sql_string_t* m_mix_name;

    public:

        /**
         * A string naming the remix of the track, if known.
         */
        device_sql_string_t* mix_name();

    private:
        bool f_unknown_string_5;
        device_sql_string_t* m_unknown_string_5;

    public:

        /**
         * A string of unknown purpose.
         */
        device_sql_string_t* unknown_string_5();

    private:
        bool f_unknown_string_4;
        device_sql_string_t* m_unknown_string_4;

    public:

        /**
         * A string of unknown purpose; @flesniak said "strange
         * strings, often zero length, sometimes low binary values
         * 0x01/0x02 as content"
         */
        device_sql_string_t* unknown_string_4();

    private:
        bool f_message;
        device_sql_string_t* m_message;

    public:

        /**
         * A string of unknown purpose, which @flesnik named.
         */
        device_sql_string_t* message();

    private:
        bool f_unknown_string_2;
        device_sql_string_t* m_unknown_string_2;

    public:

        /**
         * A string of unknown purpose; @flesniak said "thought
         * tracknumber -> wrong!"
         */
        device_sql_string_t* unknown_string_2();

    private:
        bool f_unknown_string_1;
        device_sql_string_t* m_unknown_string_1;

    public:

        /**
         * A string of unknown purpose, which has so far only been
         * empty.
         */
        device_sql_string_t* unknown_string_1();

    private:
        bool f_unknown_string_7;
        device_sql_string_t* m_unknown_string_7;

    public:

        /**
         * A string of unknown purpose, usually empty.
         */
        device_sql_string_t* unknown_string_7();

    private:
        bool f_filename;
        device_sql_string_t* m_filename;

    public:

        /**
         * The file name of the track audio file.
         */
        device_sql_string_t* filename();

    private:
        bool f_analyze_path;
        device_sql_string_t* m_analyze_path;

    public:

        /**
         * The file path of the track analysis, which allows rapid
         * seeking to particular times in variable bit-rate files,
         * jumping to particular beats, visual waveform previews, and
         * stores cue points and loops.
         */
        device_sql_string_t* analyze_path();

    private:
        bool f_comment;
        device_sql_string_t* m_comment;

    public:

        /**
         * The comment assigned to the track by the DJ, if any.
         */
        device_sql_string_t* comment();

    private:
        bool f_release_date;
        device_sql_string_t* m_release_date;

    public:

        /**
         * A string containing the date this track was released, if known.
         */
        device_sql_string_t* release_date();

    private:
        bool f_title;
        device_sql_string_t* m_title;

    public:

        /**
         * The title of the track.
         */
        device_sql_string_t* title();

    private:
        uint16_t m__unnamed0;
        uint16_t m_index_shift;
        uint32_t m_bitmask;
        uint32_t m_sample_rate;
        uint32_t m_composer_id;
        uint32_t m_file_size;
        uint32_t m__unnamed6;
        uint16_t m__unnamed7;
        uint16_t m__unnamed8;
        uint32_t m_artwork_id;
        uint32_t m_key_id;
        uint32_t m_original_artist_id;
        uint32_t m_label_id;
        uint32_t m_remixer_id;
        uint32_t m_bitrate;
        uint32_t m_track_number;
        uint32_t m_tempo;
        uint32_t m_genre_id;
        uint32_t m_album_id;
        uint32_t m_artist_id;
        uint32_t m_id;
        uint16_t m_disc_number;
        uint16_t m_play_count;
        uint16_t m_year;
        uint16_t m_sample_depth;
        uint16_t m_duration;
        uint16_t m__unnamed26;
        uint8_t m_color_id;
        uint8_t m_rating;
        uint16_t m__unnamed29;
        uint16_t m__unnamed30;
        std::vector<uint16_t>* m_ofs_strings;
        rekordbox_pdb_t* m__root;
        rekordbox_pdb_t::row_ref_t* m__parent;

    public:

        /**
         * Some kind of magic word? Usually 0x24, 0x00.
         */
        uint16_t _unnamed0() const { return m__unnamed0; }

        /**
         * TODO name from @flesniak, but what does it mean?
         */
        uint16_t index_shift() const { return m_index_shift; }

        /**
         * TODO what do the bits mean?
         */
        uint32_t bitmask() const { return m_bitmask; }

        /**
         * Playback sample rate of the audio file.
         */
        uint32_t sample_rate() const { return m_sample_rate; }

        /**
         * References a row in the artist table if the composer is
         * known.
         */
        uint32_t composer_id() const { return m_composer_id; }

        /**
         * The length of the audio file, in bytes.
         */
        uint32_t file_size() const { return m_file_size; }

        /**
         * Some ID? Purpose as yet unknown.
         */
        uint32_t _unnamed6() const { return m__unnamed6; }

        /**
         * From @flesniak: "always 19048?"
         */
        uint16_t _unnamed7() const { return m__unnamed7; }

        /**
         * From @flesniak: "always 30967?"
         */
        uint16_t _unnamed8() const { return m__unnamed8; }

        /**
         * References a row in the artwork table if there is album art.
         */
        uint32_t artwork_id() const { return m_artwork_id; }

        /**
         * References a row in the keys table if the track has a known
         * main musical key.
         */
        uint32_t key_id() const { return m_key_id; }

        /**
         * References a row in the artwork table if this is a cover
         * performance and the original artist is known.
         */
        uint32_t original_artist_id() const { return m_original_artist_id; }

        /**
         * References a row in the labels table if the track has a
         * known record label.
         */
        uint32_t label_id() const { return m_label_id; }

        /**
         * References a row in the artists table if the track has a
         * known remixer.
         */
        uint32_t remixer_id() const { return m_remixer_id; }

        /**
         * Playback bit rate of the audio file.
         */
        uint32_t bitrate() const { return m_bitrate; }

        /**
         * The position of the track within an album.
         */
        uint32_t track_number() const { return m_track_number; }

        /**
         * The tempo at the start of the track in beats per minute,
         * multiplied by 100.
         */
        uint32_t tempo() const { return m_tempo; }

        /**
         * References a row in the genres table if the track has a
         * known musical genre.
         */
        uint32_t genre_id() const { return m_genre_id; }

        /**
         * References a row in the albums table if the track has a
         * known album.
         */
        uint32_t album_id() const { return m_album_id; }

        /**
         * References a row in the artists table if the track has a
         * known performer.
         */
        uint32_t artist_id() const { return m_artist_id; }

        /**
         * The id by which this track can be looked up; players will
         * report this value in their status packets when they are
         * playing the track.
         */
        uint32_t id() const { return m_id; }

        /**
         * The number of the disc on which this track is found, if it
         * is known to be part of a multi-disc album.
         */
        uint16_t disc_number() const { return m_disc_number; }

        /**
         * The number of times this track has been played.
         */
        uint16_t play_count() const { return m_play_count; }

        /**
         * The year in which this track was released.
         */
        uint16_t year() const { return m_year; }

        /**
         * The number of bits per sample of the audio file.
         */
        uint16_t sample_depth() const { return m_sample_depth; }

        /**
         * The length, in seconds, of the track when played at normal
         * speed.
         */
        uint16_t duration() const { return m_duration; }

        /**
         * From @flesniak: "always 41?"
         */
        uint16_t _unnamed26() const { return m__unnamed26; }

        /**
         * References a row in the colors table if the track has been
         * assigned a color.
         */
        uint8_t color_id() const { return m_color_id; }

        /**
         * The number of stars to display for the track, 0 to 5.
         */
        uint8_t rating() const { return m_rating; }

        /**
         * From @flesniak: "always 1?"
         */
        uint16_t _unnamed29() const { return m__unnamed29; }

        /**
         * From @flesniak: "alternating 2 or 3"
         */
        uint16_t _unnamed30() const { return m__unnamed30; }

        /**
         * The location, relative to the start of this row, of a
         * variety of variable-length strings.
         */
        std::vector<uint16_t>* ofs_strings() const { return m_ofs_strings; }
        rekordbox_pdb_t* _root() const { return m__root; }
        rekordbox_pdb_t::row_ref_t* _parent() const { return m__parent; }
    };

    /**
     * A row that holds a musical key and the associated ID.
     */

    class key_row_t : public kaitai::kstruct {

    public:

        key_row_t(kaitai::kstream* p__io, rekordbox_pdb_t::row_ref_t* p__parent = 0, rekordbox_pdb_t* p__root = 0);

    private:
        void _read();

    public:
        ~key_row_t();

    private:
        uint32_t m_id;
        uint32_t m_id2;
        device_sql_string_t* m_name;
        rekordbox_pdb_t* m__root;
        rekordbox_pdb_t::row_ref_t* m__parent;

    public:

        /**
         * The unique identifier by which this key can be requested
         * and linked from other rows (such as tracks).
         */
        uint32_t id() const { return m_id; }

        /**
         * Seems to be a second copy of the ID?
         */
        uint32_t id2() const { return m_id2; }

        /**
         * The variable-length string naming the key.
         */
        device_sql_string_t* name() const { return m_name; }
        rekordbox_pdb_t* _root() const { return m__root; }
        rekordbox_pdb_t::row_ref_t* _parent() const { return m__parent; }
    };

    /**
     * A row that associates a track with a position in a playlist.
     */

    class playlist_entry_row_t : public kaitai::kstruct {

    public:

        playlist_entry_row_t(kaitai::kstream* p__io, rekordbox_pdb_t::row_ref_t* p__parent = 0, rekordbox_pdb_t* p__root = 0);

    private:
        void _read();

    public:
        ~playlist_entry_row_t();

    private:
        uint32_t m_entry_index;
        uint32_t m_track_id;
        uint32_t m_playlist_id;
        rekordbox_pdb_t* m__root;
        rekordbox_pdb_t::row_ref_t* m__parent;

    public:

        /**
         * The position within the playlist represented by this entry.
         */
        uint32_t entry_index() const { return m_entry_index; }

        /**
         * The track found at this position in the playlist.
         */
        uint32_t track_id() const { return m_track_id; }

        /**
         * The playlist to which this entry belongs.
         */
        uint32_t playlist_id() const { return m_playlist_id; }
        rekordbox_pdb_t* _root() const { return m__root; }
        rekordbox_pdb_t::row_ref_t* _parent() const { return m__parent; }
    };

    /**
     * A row that holds a label name and the associated ID.
     */

    class label_row_t : public kaitai::kstruct {

    public:

        label_row_t(kaitai::kstream* p__io, rekordbox_pdb_t::row_ref_t* p__parent = 0, rekordbox_pdb_t* p__root = 0);

    private:
        void _read();

    public:
        ~label_row_t();

    private:
        uint32_t m_id;
        device_sql_string_t* m_name;
        rekordbox_pdb_t* m__root;
        rekordbox_pdb_t::row_ref_t* m__parent;

    public:

        /**
         * The unique identifier by which this label can be requested
         * and linked from other rows (such as tracks).
         */
        uint32_t id() const { return m_id; }

        /**
         * The variable-length string naming the label.
         */
        device_sql_string_t* name() const { return m_name; }
        rekordbox_pdb_t* _root() const { return m__root; }
        rekordbox_pdb_t::row_ref_t* _parent() const { return m__parent; }
    };

    /**
     * Each table is a linked list of pages containing rows of a single
     * type. This header describes the nature of the table and links to
     * its pages by index.
     */

    class table_t : public kaitai::kstruct {

    public:

        table_t(kaitai::kstream* p__io, rekordbox_pdb_t* p__parent = 0, rekordbox_pdb_t* p__root = 0);

    private:
        void _read();

    public:
        ~table_t();

    private:
        page_type_t m_type;
        uint32_t m_empty_candidate;
        page_ref_t* m_first_page;
        page_ref_t* m_last_page;
        rekordbox_pdb_t* m__root;
        rekordbox_pdb_t* m__parent;

    public:

        /**
         * Identifies the kind of rows that are found in this table.
         */
        page_type_t type() const { return m_type; }
        uint32_t empty_candidate() const { return m_empty_candidate; }

        /**
         * Links to the chain of pages making up that table. The first
         * page seems to always contain similar garbage patterns and
         * zero rows, but the next page it links to contains the start
         * of the meaningful data rows.
         */
        page_ref_t* first_page() const { return m_first_page; }

        /**
         * Holds the index of the last page that makes up this table.
         * When following the linked list of pages of the table, you
         * either need to stop when you reach this page, or when you
         * notice that the `next_page` link you followed took you to a
         * page of a different `type`.
         */
        page_ref_t* last_page() const { return m_last_page; }
        rekordbox_pdb_t* _root() const { return m__root; }
        rekordbox_pdb_t* _parent() const { return m__parent; }
    };

    /**
     * An offset which points to a row in the table, whose actual
     * presence is controlled by one of the bits in
     * `row_present_flags`. This instance allows the row itself to be
     * lazily loaded, unless it is not present, in which case there is
     * no content to be loaded.
     */

    class row_ref_t : public kaitai::kstruct {

    public:

        row_ref_t(uint16_t p_row_index, kaitai::kstream* p__io, rekordbox_pdb_t::row_group_t* p__parent = 0, rekordbox_pdb_t* p__root = 0);

    private:
        void _read();

    public:
        ~row_ref_t();

    private:
        bool f_ofs_row;
        uint16_t m_ofs_row;

    public:

        /**
         * The offset of the start of the row (in bytes past the end of
         * the page header).
         */
        uint16_t ofs_row();

    private:
        bool f_row_base;
        int32_t m_row_base;

    public:

        /**
         * The location of this row relative to the start of the page.
         * A variety of pointers (such as all device_sql_string values)
         * are calculated with respect to this position.
         */
        int32_t row_base();

    private:
        bool f_present;
        bool m_present;

    public:

        /**
         * Indicates whether the row index considers this row to be
         * present in the table. Will be `false` if the row has been
         * deleted.
         */
        bool present();

    private:
        bool f_body;
        kaitai::kstruct* m_body;
        bool n_body;

    public:
        bool _is_null_body() { body(); return n_body; };

    private:

    public:

        /**
         * The actual content of the row, as long as it is present.
         */
        kaitai::kstruct* body();

    private:
        uint16_t m_row_index;
        rekordbox_pdb_t* m__root;
        rekordbox_pdb_t::row_group_t* m__parent;

    public:

        /**
         * Identifies which row within the row index this reference
         * came from, so the correct flag can be checked for the row
         * presence and the correct row offset can be found.
         */
        uint16_t row_index() const { return m_row_index; }
        rekordbox_pdb_t* _root() const { return m__root; }
        rekordbox_pdb_t::row_group_t* _parent() const { return m__parent; }
    };

private:
    uint32_t m__unnamed0;
    uint32_t m_len_page;
    uint32_t m_num_tables;
    uint32_t m_next_unused_page;
    uint32_t m__unnamed4;
    uint32_t m_sequence;
    std::string m__unnamed6;
    std::vector<table_t*>* m_tables;
    rekordbox_pdb_t* m__root;
    kaitai::kstruct* m__parent;

public:

    /**
     * Unknown purpose, perhaps an unoriginal signature, seems to
     * always have the value 0.
     */
    uint32_t _unnamed0() const { return m__unnamed0; }

    /**
     * The database page size, in bytes. Pages are referred to by
     * index, so this size is needed to calculate their offset, and
     * table pages have a row index structure which is built from the
     * end of the page backwards, so finding that also requires this
     * value.
     */
    uint32_t len_page() const { return m_len_page; }

    /**
     * Determines the number of table entries that are present. Each
     * table is a linked list of pages containing rows of a particular
     * type.
     */
    uint32_t num_tables() const { return m_num_tables; }

    /**
     * @flesinak said: "Not used as any `empty_candidate`, points
     * past the end of the file."
     */
    uint32_t next_unused_page() const { return m_next_unused_page; }
    uint32_t _unnamed4() const { return m__unnamed4; }

    /**
     * @flesniak said: "Always incremented by at least one,
     * sometimes by two or three."
     */
    uint32_t sequence() const { return m_sequence; }
    std::string _unnamed6() const { return m__unnamed6; }

    /**
     * Describes and links to the tables present in the database.
     */
    std::vector<table_t*>* tables() const { return m_tables; }
    rekordbox_pdb_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
};
