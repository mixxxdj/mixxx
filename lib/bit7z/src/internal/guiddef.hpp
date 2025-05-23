/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef GUIDDEF_HPP
#define GUIDDEF_HPP

#include "bitformat.hpp"

#ifdef _WIN32
#include <guiddef.h>
#else

#ifndef GUID_DEFINED
#define GUID_DEFINED
#endif

#include <array>
#include <cstdint>
#include <cstring> //for std::memcmp

#include "internal/windows.hpp"

struct GUID {
    UInt32 Data1;
    UInt16 Data2;
    UInt16 Data3;
    std::array< unsigned char, 8 > Data4;
};

#ifndef DEFINE_GUID
#define DEFINE_GUID( name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8 ) \
    extern "C" const GUID name
#endif

using REFGUID = const GUID&;
using REFIID = REFGUID;

inline auto operator==( REFGUID guid1, REFGUID guid2 ) -> bool {
    return std::memcmp( &guid1, &guid2, sizeof( GUID ) ) == 0;
}

inline auto operator!=( REFGUID guid1, REFGUID guid2 ) -> bool { return !( guid1 == guid2 ); }

#ifndef STDMETHODCALLTYPE
#define STDMETHODCALLTYPE
#endif

#ifndef STDMETHOD_
#define STDMETHOD_( t, f ) virtual t STDMETHODCALLTYPE f
#endif

#ifndef STDMETHOD
#define STDMETHOD( f ) STDMETHOD_(HRESULT, f)
#endif

#ifndef STDMETHODIMP_
#define STDMETHODIMP_( type ) type STDMETHODCALLTYPE
#endif

#ifndef STDMETHODIMP
#define STDMETHODIMP STDMETHODIMP_(HRESULT)
#endif

#ifndef PURE
#define PURE = 0
#endif

struct IUnknown {
    STDMETHOD ( QueryInterface )( REFIID iid, void** outObject ) PURE;

    STDMETHOD_( ULONG, AddRef )() PURE;

    STDMETHOD_( ULONG, Release )() PURE;

#ifdef BIT7Z_USE_LEGACY_IUNKNOWN
    virtual ~IUnknown() = default;
#endif
};

#endif

namespace bit7z {

/**
 * @return the GUID that identifies the file format in the 7z SDK.
 */
inline auto format_guid( const BitInFormat& format ) -> GUID {
    return { 0x23170F69, 0x40C1, 0x278A, { 0x10, 0x00, 0x00, 0x01, 0x10, format.value(), 0x00, 0x00 } }; // NOLINT
}

} // namespace bit7z

#endif //GUIDDEF_HPP