/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef UPDATECALLBACK_HPP
#define UPDATECALLBACK_HPP

#include "bitoutputarchive.hpp"
#include "internal/callback.hpp"
#include "internal/macros.hpp"

#include <7zip/Archive/IArchive.h>
#include <7zip/ICoder.h>
#include <7zip/IPassword.h>

namespace bit7z {

class UpdateCallback final : public Callback,
                             public IArchiveUpdateCallback2,
                             public ICompressProgressInfo,
                             protected ICryptoGetTextPassword2 {
    public:
        explicit UpdateCallback( const BitOutputArchive& output );

        UpdateCallback( const UpdateCallback& ) = delete;

        UpdateCallback( UpdateCallback&& ) = delete;

        auto operator=( const UpdateCallback& ) -> UpdateCallback& = delete;

        auto operator=( UpdateCallback&& ) -> UpdateCallback& = delete;

        ~UpdateCallback() override;

        auto finalize() noexcept -> HRESULT;

        // IProgress from IArchiveUpdateCallback2
        BIT7Z_STDMETHOD( SetTotal, UInt64 size );

        BIT7Z_STDMETHOD( SetCompleted, const UInt64* completeValue );

        // ICompressProgressInfo
        BIT7Z_STDMETHOD( SetRatioInfo, const UInt64* inSize, const UInt64* outSize );

        // IArchiveUpdateCallback2
        BIT7Z_STDMETHOD( GetProperty, UInt32 index, PROPID propID, PROPVARIANT* value );

        BIT7Z_STDMETHOD( GetStream, UInt32 index, ISequentialInStream** inStream );

        BIT7Z_STDMETHOD( GetVolumeSize, UInt32 index, UInt64 * size );

        BIT7Z_STDMETHOD( GetVolumeStream, UInt32 index, ISequentialOutStream** volumeStream );

        BIT7Z_STDMETHOD( GetUpdateItemInfo, UInt32 index,
                         Int32* newData,
                         Int32* newProperties,
                         UInt32* indexInArchive );

        BIT7Z_STDMETHOD( SetOperationResult, Int32 operationResult );

        // ICryptoGetTextPassword2
        BIT7Z_STDMETHOD( CryptoGetTextPassword2, Int32* passwordIsDefined, BSTR* password );

        // NOLINTNEXTLINE(modernize-use-noexcept, modernize-use-trailing-return-type, readability-identifier-length)
        MY_UNKNOWN_IMP3( IArchiveUpdateCallback2, ICompressProgressInfo, ICryptoGetTextPassword2 ) //-V2507 //-V2511 //-V835

    private:
        const BitOutputArchive& mOutputArchive;
        bool mNeedBeClosed;
};

}  // namespace bit7z

#endif // UPDATECALLBACK_HPP
