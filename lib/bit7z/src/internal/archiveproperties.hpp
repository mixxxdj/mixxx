/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef ARCHIVEPROPERTIES_HPP
#define ARCHIVEPROPERTIES_HPP

#include <map>
#include <string>
#include <vector>

#include "bitpropvariant.hpp"

namespace bit7z {

class ArchiveProperties final {
        std::vector< const wchar_t* > mNames{};
        std::vector< BitPropVariant > mValues{};

        template< typename T, typename = typename std::enable_if< std::is_integral< T >::value >::type >
        inline void setProperty( const wchar_t* name, T value ) {
            mNames.emplace_back( name );
            mValues.emplace_back( value );
        }

        template< typename T, typename = typename std::enable_if< !std::is_integral< T >::value >::type >
        inline void setProperty( const wchar_t* name, const T& value ) {
            mNames.emplace_back( name );
            mValues.emplace_back( value );
        }

        void addProperties( const std::map< std::wstring, BitPropVariant >& otherProperties ) {
            for ( const auto& entry : otherProperties ) {
                mNames.emplace_back( entry.first.c_str() );
                mValues.emplace_back( entry.second );
            }
        }

        friend class BitAbstractArchiveCreator;

    public:
        BIT7Z_NODISCARD
        inline auto empty() const -> bool {
            return mNames.empty();
        }

        BIT7Z_NODISCARD
        inline auto names() const -> const wchar_t* const* {
            return mNames.data();
        }

        BIT7Z_NODISCARD
        inline auto values() const -> const PROPVARIANT* {
            return mValues.data();
        }

        BIT7Z_NODISCARD
        inline auto size() const -> size_t {
            return mNames.size();
        }
};

} // namespace bit7z

#endif //ARCHIVEPROPERTIES_HPP
