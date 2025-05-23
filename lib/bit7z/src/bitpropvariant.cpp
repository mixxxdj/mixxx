// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "bitexception.hpp"
#include "biterror.hpp"
#include "bitpropvariant.hpp"
#include "internal/dateutil.hpp"
#include "internal/windows.hpp"

#if defined( BIT7Z_USE_NATIVE_STRING ) && defined( _WIN32 ) // Windows
#define BSTR_TO_TSTRING( bstr ) std::wstring( bstr, ::SysStringLen( bstr ) )
#else
#include "internal/stringutil.hpp"
#define BSTR_TO_TSTRING( bstr ) bit7z::narrow( bstr, SysStringLen( bstr ) )
#endif

constexpr auto kCannotAllocateString = "Could not allocate memory for BitPropVariant string";

using namespace bit7z;

auto lookup_type( VARTYPE type ) -> BitPropVariantType {
    switch ( type ) {
        case VT_EMPTY:
            return BitPropVariantType::Empty;
        case VT_BOOL:
            return BitPropVariantType::Bool;
        case VT_BSTR:
            return BitPropVariantType::String;
        case VT_UI1:
            return BitPropVariantType::UInt8;
        case VT_UI2:
            return BitPropVariantType::UInt16;
        case VT_UI4:
        case VT_UINT:
            return BitPropVariantType::UInt32;
        case VT_UI8:
            return BitPropVariantType::UInt64;
        case VT_I1:
            return BitPropVariantType::Int8;
        case VT_I2:
            return BitPropVariantType::Int16;
        case VT_I4:
        case VT_INT:
            return BitPropVariantType::Int32;
        case VT_I8:
            return BitPropVariantType::Int64;
        case VT_FILETIME:
            return BitPropVariantType::FileTime;
        default:
            /* This is unlikely to happen since the property types used in archives
             * are the ones supported by PropertyType enum class.*/
            throw BitException( "Property type is not supported", std::make_error_code( std::errc::invalid_argument ) );
    }
}

namespace bit7z { // Note: Clang doesn't find the operator if it is not inside the namespace.

/* Needed for comparing FILETIME objects in BitPropVariant */
inline auto operator==( FILETIME ft1, FILETIME ft2 ) noexcept -> bool {
#ifdef _WIN32
    return CompareFileTime( &ft1, &ft2 ) == 0;
#else
    return ft1.dwHighDateTime == ft2.dwHighDateTime && ft1.dwLowDateTime == ft2.dwLowDateTime;
#endif
}

} // namespace bit7z

BitPropVariant::BitPropVariant() : PROPVARIANT() {
    /* As in CPropVariant default constructor (Note: it seems that the default vt value is VT_NULL)*/
    vt = VT_EMPTY;
    wReserved1 = 0;
    bstrVal = nullptr;
}

BitPropVariant::BitPropVariant( const BitPropVariant& other ) : PROPVARIANT( other ) {
    if ( vt == VT_BSTR ) { //until now, we've copied only the pointer to the string, hence we need a deep copy!
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        bstrVal = SysAllocStringByteLen( reinterpret_cast< LPCSTR >( other.bstrVal ),
                                         SysStringByteLen( other.bstrVal ) );
        if ( bstrVal == nullptr ) {
            throw BitException( kCannotAllocateString, std::make_error_code( std::errc::not_enough_memory ) );
        }
    }
}

BitPropVariant::BitPropVariant( BitPropVariant&& other ) noexcept: PROPVARIANT( other ) {
    if ( vt == VT_BSTR ) {
        /* this object and "other" share the pointer to the same string, but now the string belongs to this!
         * Hence, if we set the other.bstrVal to nullptr, we prevent the bstrVal from being destroyed when
         * the other object is deleted! */
        other.bstrVal = nullptr;
    }
}

BitPropVariant::BitPropVariant( bool value ) noexcept: PROPVARIANT() {
    vt = VT_BOOL;
    wReserved1 = 0;
    boolVal = ( value ? VARIANT_TRUE : VARIANT_FALSE );
}

BitPropVariant::BitPropVariant( const wchar_t* value ) : PROPVARIANT() {
    vt = VT_BSTR;
    wReserved1 = 0;
    if ( value != nullptr ) {
        bstrVal = ::SysAllocString( value );
        if ( bstrVal == nullptr ) {
            throw BitException( kCannotAllocateString, std::make_error_code( std::errc::not_enough_memory ) );
        }
    } else {
        bstrVal = nullptr;
    }
}

BitPropVariant::BitPropVariant( const std::wstring& value ) : PROPVARIANT() {
    vt = VT_BSTR;
    wReserved1 = 0;
    bstrVal = ::SysAllocStringLen( value.c_str(), static_cast< unsigned int >( value.size() ) );
    if ( bstrVal == nullptr ) {
        throw BitException( kCannotAllocateString, std::make_error_code( std::errc::not_enough_memory ) );
    }
}

BitPropVariant::BitPropVariant( uint8_t value ) noexcept: PROPVARIANT() {
    vt = VT_UI1;
    wReserved1 = 0;
    bVal = value;
}

BitPropVariant::BitPropVariant( uint16_t value ) noexcept: PROPVARIANT() {
    vt = VT_UI2;
    wReserved1 = 0;
    uiVal = value;
}

BitPropVariant::BitPropVariant( uint32_t value ) noexcept: PROPVARIANT() {
    vt = VT_UI4;
    wReserved1 = 0;
    ulVal = value;
}

BitPropVariant::BitPropVariant( uint64_t value ) noexcept: PROPVARIANT() {
    vt = VT_UI8;
    wReserved1 = 0;
    uhVal.QuadPart = value;
}

BitPropVariant::BitPropVariant( int8_t value ) noexcept: PROPVARIANT() {
    vt = VT_I1;
    wReserved1 = 0;
    cVal = static_cast< decltype(cVal) >( value );
}

BitPropVariant::BitPropVariant( int16_t value ) noexcept: PROPVARIANT() {
    vt = VT_I2;
    wReserved1 = 0;
    iVal = value;
}

BitPropVariant::BitPropVariant( int32_t value ) noexcept: PROPVARIANT() {
    vt = VT_I4;
    wReserved1 = 0;
    lVal = value;
}

BitPropVariant::BitPropVariant( int64_t value ) noexcept: PROPVARIANT() {
    vt = VT_I8;
    wReserved1 = 0;
    hVal.QuadPart = value;
}

BitPropVariant::BitPropVariant( FILETIME value ) noexcept: PROPVARIANT() {
    vt = VT_FILETIME;
    wReserved1 = 0;
    filetime = value;
}

BitPropVariant::~BitPropVariant() {
    internalClear();
}

auto BitPropVariant::operator=( const BitPropVariant& other ) -> BitPropVariant& {
    BitPropVariant tmp( other ); //copy construct a tmp variable
    *this = std::move( tmp ); //move assign to this
    return *this;
}

auto BitPropVariant::operator=( BitPropVariant&& other ) noexcept -> BitPropVariant& {
    if ( this != &other ) {
        internalClear();
        vt = other.vt;
        switch ( vt ) {
            case VT_BOOL:
                boolVal = other.boolVal;
                break;
            case VT_BSTR:
                bstrVal = other.bstrVal;
                other.bstrVal = nullptr;
                break;
            case VT_UI1:
                bVal = other.bVal;
                break;
            case VT_UI2:
                uiVal = other.uiVal;
                break;
            case VT_UINT:
                uintVal = other.uintVal;
                break;
            case VT_UI4:
                ulVal = other.ulVal;
                break;
            case VT_UI8:
                uhVal = other.uhVal;
                break;
            case VT_I1:
                cVal = other.cVal;
                break;
            case VT_I2:
                iVal = other.iVal;
                break;
            case VT_INT:
                intVal = other.intVal;
                break;
            case VT_I4:
                lVal = other.lVal;
                break;
            case VT_I8:
                hVal = other.hVal;
                break;
            case VT_FILETIME:
                filetime = other.filetime;
                break;
            default: // The type is not supported
                break;
        }
    }
    return *this;
}

auto BitPropVariant::getBool() const -> bool {
    if ( vt != VT_BOOL ) {
        throw BitException( "BitPropVariant is not a bool", make_error_code( BitError::RequestedWrongVariantType ) );
    }
    return boolVal != VARIANT_FALSE; //simply returning boolVal should work, but this prevents some compiler warnings.
}

auto BitPropVariant::getString() const -> tstring {
    if ( vt != VT_BSTR ) {
        throw BitException( "BitPropVariant is not a string", make_error_code( BitError::RequestedWrongVariantType ) );
    }
    //Note: a nullptr BSTR is semantically equivalent to an empty string!
    return bstrVal == nullptr ? tstring{} : BSTR_TO_TSTRING( bstrVal );
}

auto BitPropVariant::getNativeString() const -> native_string {
#ifdef _WIN32
    if ( vt != VT_BSTR ) {
        throw BitException( "BitPropVariant is not a string", make_error_code( BitError::RequestedWrongVariantType ) );
    }
    //Note: a nullptr BSTR is semantically equivalent to an empty string!
    return bstrVal == nullptr ? native_string{} : native_string{ bstrVal, ::SysStringLen( bstrVal ) };
#else
    return getString();
#endif
}

auto BitPropVariant::getUInt8() const -> uint8_t {
    switch ( vt ) {
        case VT_UI1:
            return bVal;
        default: // not an 8-bits unsigned integer.
            throw BitException( "BitPropVariant is not an 8-bits unsigned integer",
                                make_error_code( BitError::RequestedWrongVariantType ) );
    }
}

auto BitPropVariant::getUInt16() const -> uint16_t {
    switch ( vt ) {
        case VT_UI1:
            return bVal;
        case VT_UI2:
            return uiVal;
        default: // not a 16-bits unsigned integer.
            throw BitException( "BitPropVariant is not a 16-bits unsigned integer",
                                make_error_code( BitError::RequestedWrongVariantType ) );
    }
}

auto BitPropVariant::getUInt32() const -> uint32_t {
    switch ( vt ) {
        case VT_UI1:
            return bVal;
        case VT_UI2:
            return uiVal;
        case VT_UINT:
            return uintVal;
        case VT_UI4:
            return ulVal;
        default: // not a 32-bits unsigned integer.
            throw BitException( "BitPropVariant is not a 32-bits unsigned integer",
                                make_error_code( BitError::RequestedWrongVariantType ) );
    }
}

auto BitPropVariant::getUInt64() const -> uint64_t {
    switch ( vt ) {
        case VT_UI1:
            return bVal;
        case VT_UI2:
            return uiVal;
        case VT_UINT:
            return uintVal;
        case VT_UI4:
            return ulVal;
        case VT_UI8:
            return uhVal.QuadPart;
        default: // not a 64-bits unsigned integer.
            throw BitException( "BitPropVariant is not a 64-bits unsigned integer",
                                make_error_code( BitError::RequestedWrongVariantType ) );
    }
}

auto BitPropVariant::getInt8() const -> int8_t {
    switch ( vt ) {
        case VT_I1:
            return static_cast< int8_t >( cVal );
        default: // not an 8-bits integer.
            throw BitException( "BitPropVariant is not an 8-bits integer",
                                make_error_code( BitError::RequestedWrongVariantType ) );
    }
}

auto BitPropVariant::getInt16() const -> int16_t {
    switch ( vt ) {
        case VT_I1:
            return cVal;
        case VT_I2:
            return iVal;
        default: // not a 16-bits integer.
            throw BitException( "BitPropVariant is not a 16-bits integer",
                                make_error_code( BitError::RequestedWrongVariantType ) );
    }
}

auto BitPropVariant::getInt32() const -> int32_t {
    switch ( vt ) {
        case VT_I1:
            return cVal;
        case VT_I2:
            return iVal;
        case VT_INT:
            return intVal;
        case VT_I4:
            return lVal;
        default: // not a 32-bits integer.
            throw BitException( "BitPropVariant is not a 32-bits integer",
                                make_error_code( BitError::RequestedWrongVariantType ) );
    }
}

auto BitPropVariant::getInt64() const -> int64_t {
    switch ( vt ) {
        case VT_I1:
            return cVal;
        case VT_I2:
            return iVal;
        case VT_INT:
            return intVal;
        case VT_I4:
            return lVal;
        case VT_I8:
            return hVal.QuadPart;
        default: // not a 64-bits integer.
            throw BitException( "BitPropVariant is not a 64-bits integer",
                                make_error_code( BitError::RequestedWrongVariantType ) );
    }
}

auto BitPropVariant::getFileTime() const -> FILETIME {
    if ( vt != VT_FILETIME ) {
        throw BitException( "BitPropVariant is not a FILETIME",
                            make_error_code( BitError::RequestedWrongVariantType ) );
    }
    return filetime;
}

auto BitPropVariant::getTimePoint() const -> bit7z::time_type {
    const FILETIME fileTime = getFileTime();
    return FILETIME_to_time_type( fileTime );
}

auto BitPropVariant::toString() const -> tstring {
    switch ( vt ) {
        case VT_BOOL:
            return boolVal == VARIANT_TRUE ? BIT7Z_STRING( "true" ) : BIT7Z_STRING( "false" );
        case VT_BSTR:
            return BSTR_TO_TSTRING( bstrVal );
        case VT_UI1:
            return to_tstring( bVal );
        case VT_UI2:
            return to_tstring( uiVal );
        case VT_UINT:
            return to_tstring( uintVal );
        case VT_UI4:
            return to_tstring( ulVal );
        case VT_UI8:
            return to_tstring( uhVal.QuadPart );
        case VT_I1:
            return to_tstring( cVal );
        case VT_I2:
            return to_tstring( iVal );
        case VT_INT:
            return to_tstring( intVal );
        case VT_I4:
            return to_tstring( lVal );
        case VT_I8:
            return to_tstring( hVal.QuadPart );
        case VT_FILETIME:
            return to_tstring( filetime.dwHighDateTime ) + BIT7Z_STRING( ", " ) + to_tstring( filetime.dwLowDateTime );
        case VT_EMPTY:
            return tstring{};
        default: // The type is not supported
            throw BitException( "BitPropVariant type code " + std::to_string( vt ) + " not supported.",
                                make_error_code( BitError::UnsupportedVariantType ) );
    }
}

auto BitPropVariant::isEmpty() const noexcept -> bool {
    return vt == VT_EMPTY;
}

auto BitPropVariant::isBool() const noexcept -> bool {
    return vt == VT_BOOL;
}

auto BitPropVariant::isString() const noexcept -> bool {
    return vt == VT_BSTR;
}

auto BitPropVariant::isUInt8() const noexcept -> bool {
    return vt == VT_UI1;
}

auto BitPropVariant::isUInt16() const noexcept -> bool {
    return vt == VT_UI2 || vt == VT_UI1;
}

auto BitPropVariant::isUInt32() const noexcept -> bool {
    return vt == VT_UI4 || vt == VT_UINT || vt == VT_UI2 || vt == VT_UI1;
}

auto BitPropVariant::isUInt64() const noexcept -> bool {
    return vt == VT_UI8 || vt == VT_UI4 || vt == VT_UINT || vt == VT_UI2 || vt == VT_UI1;
}

auto BitPropVariant::isInt8() const noexcept -> bool {
    return vt == VT_I1;
}

auto BitPropVariant::isInt16() const noexcept -> bool {
    return vt == VT_I2 || vt == VT_I1;
}

auto BitPropVariant::isInt32() const noexcept -> bool {
    return vt == VT_I4 || vt == VT_INT || vt == VT_I2 || vt == VT_I1;
}

auto BitPropVariant::isInt64() const noexcept -> bool {
    return vt == VT_I8 || vt == VT_I4 || vt == VT_INT || vt == VT_I2 || vt == VT_I1;
}

auto BitPropVariant::isFileTime() const noexcept -> bool {
    return vt == VT_FILETIME;
}

auto BitPropVariant::type() const -> BitPropVariantType {
    return lookup_type( vt );
}

void BitPropVariant::clear() noexcept {
    if ( vt == VT_EMPTY ) {
        return;
    }
    internalClear();
    vt = VT_EMPTY;
}

void BitPropVariant::internalClear() noexcept {
    if ( vt == VT_BSTR && bstrVal != nullptr ) {
        ::SysFreeString( bstrVal ); //this was a string: since it is not needed anymore, we must free it!
        bstrVal = nullptr;
    }
    wReserved1 = 0;
    wReserved2 = 0;
    wReserved3 = 0;
    uhVal.QuadPart = 0;
}

auto bit7z::operator!=( const BitPropVariant& lhs, const BitPropVariant& rhs ) noexcept -> bool {
    return !( lhs == rhs );
}

auto bit7z::operator==( const BitPropVariant& lhs, const BitPropVariant& rhs ) noexcept -> bool {
    if ( lhs.vt != rhs.vt ) {
        return false;
    }
    switch ( lhs.vt ) { //lhs.vt == rhs.vt
        case VT_EMPTY:
            return true;
        case VT_BOOL:
            return lhs.boolVal == rhs.boolVal;
        case VT_BSTR:
            return wcscmp( lhs.bstrVal, rhs.bstrVal ) == 0;
        case VT_UI1:
            return lhs.bVal == rhs.bVal;
        case VT_UI2:
            return lhs.uiVal == rhs.uiVal;
        case VT_UINT:
            return lhs.uintVal == rhs.uintVal;
        case VT_UI4:
            return lhs.ulVal == rhs.ulVal;
        case VT_UI8:
            return lhs.uhVal.QuadPart == rhs.uhVal.QuadPart;
        case VT_I1:
            return lhs.cVal == rhs.cVal;
        case VT_I2:
            return lhs.iVal == rhs.iVal;
        case VT_INT:
            return lhs.intVal == rhs.intVal;
        case VT_I4:
            return lhs.lVal == rhs.lVal;
        case VT_I8:
            return lhs.hVal.QuadPart == rhs.hVal.QuadPart;
        case VT_FILETIME:
            return lhs.filetime == rhs.filetime;
        default:
            return false;
    }
}
