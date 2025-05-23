/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef CALLBACK_HPP
#define CALLBACK_HPP

#include <string>

#include "bitabstractarchivehandler.hpp"
#include "internal/com.hpp"
#include "internal/guids.hpp"

namespace bit7z {

/* On Windows, 7-zip's CMyUnknownImp class has a virtual destructor if the compiler is MinGW/GCC/Clang.
 * On MSVC or on Unix, the destructor is not virtual. */
#ifdef _WIN32
#   if ( defined(__GNUC__) || defined(__clang__) ) && !defined( SEVENZIP_2301 )
#       define CALLBACK_DESTRUCTOR( x ) x override
#   endif
#endif

#ifndef CALLBACK_DESTRUCTOR // MSVC or Unix compiler
#   define CALLBACK_DESTRUCTOR( x ) virtual x
#endif

class Callback : protected CMyUnknownImp {
    public:
        Callback( const Callback& ) = delete;

        Callback( Callback&& ) = delete;

        auto operator=( const Callback& ) -> Callback& = delete;

        auto operator=( Callback&& ) -> Callback& = delete;

        CALLBACK_DESTRUCTOR( ~Callback() ) = default;

    protected:
        explicit Callback( const BitAbstractArchiveHandler& handler ); // Abstract class

        const BitAbstractArchiveHandler& mHandler;
};

}  // namespace bit7z

#endif // CALLBACK_HPP
