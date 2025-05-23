/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef OPENCALLBACK_HPP
#define OPENCALLBACK_HPP

#include "bitabstractarchivehandler.hpp"
#include "internal/callback.hpp"
#include "internal/com.hpp"
#include "internal/fsitem.hpp"
#include "internal/macros.hpp"

#include <7zip/Archive/IArchive.h>
#include <7zip/IPassword.h>

namespace bit7z {

using filesystem::FilesystemItem;

class OpenCallback final : public IArchiveOpenCallback,
                           public IArchiveOpenVolumeCallback,
                           public IArchiveOpenSetSubArchiveName,
                           public ICryptoGetTextPassword,
                           public Callback {
    public:
        explicit OpenCallback( const BitAbstractArchiveHandler& handler, fs::path archivePath = fs::path{} );

        OpenCallback( const OpenCallback& ) = delete;

        OpenCallback( OpenCallback&& ) = delete;

        auto operator=( const OpenCallback& ) -> OpenCallback& = delete;

        auto operator=( OpenCallback&& ) -> OpenCallback& = delete;

        ~OpenCallback() override = default;

        BIT7Z_NODISCARD
        auto passwordWasAsked() const -> bool;

        // IArchiveOpenCallback
        BIT7Z_STDMETHOD( SetTotal, const UInt64* files, const UInt64* bytes );

        BIT7Z_STDMETHOD( SetCompleted, const UInt64* files, const UInt64* bytes );

        // IArchiveOpenVolumeCallback
        BIT7Z_STDMETHOD( GetProperty, PROPID propID, PROPVARIANT* value );

        BIT7Z_STDMETHOD( GetStream, const wchar_t* name, IInStream** inStream );

        // IArchiveOpenSetSubArchiveName
        BIT7Z_STDMETHOD( SetSubArchiveName, const wchar_t* name );

        // ICryptoGetTextPassword
        BIT7Z_STDMETHOD( CryptoGetTextPassword, BSTR* password );

        // NOLINTNEXTLINE(modernize-use-noexcept, modernize-use-trailing-return-type, readability-identifier-length)
        MY_UNKNOWN_IMP3( IArchiveOpenVolumeCallback, IArchiveOpenSetSubArchiveName, ICryptoGetTextPassword ) //-V2507 //-V2511 //-V835

    private:
        bool mSubArchiveMode;
        std::wstring mSubArchiveName;
        fs::path mArchivePath;
        bool mPasswordWasAsked;
};

}  // namespace bit7z

#endif // OPENCALLBACK_HPP
