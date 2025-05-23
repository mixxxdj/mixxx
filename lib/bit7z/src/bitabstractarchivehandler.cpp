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

#include "bitabstractarchivehandler.hpp"

using namespace bit7z;

BitAbstractArchiveHandler::BitAbstractArchiveHandler( const Bit7zLibrary& lib,
                                                      tstring password,
                                                      OverwriteMode overwriteMode )
    : mLibrary{ lib },
      mPassword{ std::move( password ) },
      mRetainDirectories{ true },
      mOverwriteMode{ overwriteMode } {}

auto BitAbstractArchiveHandler::library() const noexcept -> const Bit7zLibrary& {
    return mLibrary;
}

auto BitAbstractArchiveHandler::password() const -> tstring {
    return mPassword;
}

auto BitAbstractArchiveHandler::retainDirectories() const noexcept -> bool {
    return mRetainDirectories;
}

auto BitAbstractArchiveHandler::isPasswordDefined() const noexcept -> bool {
    return !mPassword.empty();
}

auto BitAbstractArchiveHandler::totalCallback() const -> TotalCallback {
    return mTotalCallback;
}

auto BitAbstractArchiveHandler::progressCallback() const -> ProgressCallback {
    return mProgressCallback;
}

auto BitAbstractArchiveHandler::ratioCallback() const -> RatioCallback {
    return mRatioCallback;
}

auto BitAbstractArchiveHandler::fileCallback() const -> FileCallback {
    return mFileCallback;
}

auto BitAbstractArchiveHandler::passwordCallback() const -> PasswordCallback {
    return mPasswordCallback;
}

auto BitAbstractArchiveHandler::overwriteMode() const -> OverwriteMode {
    return mOverwriteMode;
}

void BitAbstractArchiveHandler::setPassword( const tstring& password ) {
    mPassword = password;
}

void BitAbstractArchiveHandler::clearPassword() noexcept {
    mPassword.clear();
}

void BitAbstractArchiveHandler::setRetainDirectories( bool retain ) noexcept {
    mRetainDirectories = retain;
}

void BitAbstractArchiveHandler::setTotalCallback( const TotalCallback& callback ) {
    mTotalCallback = callback;
}

void BitAbstractArchiveHandler::setProgressCallback( const ProgressCallback& callback ) {
    mProgressCallback = callback;
}

void BitAbstractArchiveHandler::setRatioCallback( const RatioCallback& callback ) {
    mRatioCallback = callback;
}

void BitAbstractArchiveHandler::setFileCallback( const FileCallback& callback ) {
    mFileCallback = callback;
}

void BitAbstractArchiveHandler::setPasswordCallback( const PasswordCallback& callback ) {
    mPasswordCallback = callback;
}

void BitAbstractArchiveHandler::setOverwriteMode( OverwriteMode mode ) {
    mOverwriteMode = mode;
}
