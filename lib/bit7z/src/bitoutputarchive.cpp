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

#include "biterror.hpp"
#include "bitexception.hpp"
#include "bitoutputarchive.hpp"
#include "internal/archiveproperties.hpp"
#include "internal/cbufferoutstream.hpp"
#include "internal/cmultivolumeoutstream.hpp"
#include "internal/genericinputitem.hpp"
#include "internal/stringutil.hpp"
#include "internal/updatecallback.hpp"
#include "internal/util.hpp"

namespace bit7z {

BitOutputArchive::BitOutputArchive( const BitAbstractArchiveCreator& creator )
    : mArchiveCreator{ creator }, mInputArchiveItemsCount{ 0 } {}

BitOutputArchive::BitOutputArchive( const BitAbstractArchiveCreator& creator,
                                    const tstring& inFile,
                                    ArchiveStartOffset startOffset )
    : BitOutputArchive( creator, tstring_to_path( inFile ), startOffset ) {}

BitOutputArchive::BitOutputArchive( const BitAbstractArchiveCreator& creator,
                                    const fs::path& inArc,
                                    ArchiveStartOffset archiveStart )
    : mArchiveCreator{ creator }, mInputArchiveItemsCount{ 0 } {
    if ( mArchiveCreator.overwriteMode() != OverwriteMode::None ) {
        return;
    }

    if ( inArc.empty() ) { // No input file specified, so we are creating a totally new archive.
        return;
    }

    std::error_code error;
    if ( !fs::exists( inArc, error ) ) { // An input file was specified, but it doesn't exist, so we ignore it.
        return;
    }

    if ( mArchiveCreator.updateMode() == UpdateMode::None ) {
        throw BitException( "Cannot update the existing archive",
                            make_error_code( BitError::WrongUpdateMode ) );
    }

    if ( !mArchiveCreator.compressionFormat().hasFeature( FormatFeatures::MultipleFiles ) ) {
        //Update mode is set, but the format does not support adding more files.
        throw BitException( "Cannot update the existing archive",
                            make_error_code( BitError::FormatFeatureNotSupported ) );
    }

    mInputArchive = std::make_unique< BitInputArchive >( creator, inArc, archiveStart );
    mInputArchiveItemsCount = mInputArchive->itemsCount();
}

BitOutputArchive::BitOutputArchive( const BitAbstractArchiveCreator& creator,
                                    const buffer_t& inBuffer,
                                    ArchiveStartOffset startOffset )
    : mArchiveCreator{ creator }, mInputArchiveItemsCount{ 0 } {
    if ( !inBuffer.empty() ) {
        mInputArchive = std::make_unique< BitInputArchive >( creator, inBuffer, startOffset );
        mInputArchiveItemsCount = mInputArchive->itemsCount();
    }
}

BitOutputArchive::BitOutputArchive( const BitAbstractArchiveCreator& creator,
                                    std::istream& inStream,
                                    ArchiveStartOffset startOffset )
    : mArchiveCreator{ creator }, mInputArchiveItemsCount{ 0 } {
    if ( inStream.good() ) {
        mInputArchive = std::make_unique< BitInputArchive >( creator, inStream, startOffset );
        mInputArchiveItemsCount = mInputArchive->itemsCount();
    }
}

void BitOutputArchive::addItems( const std::vector< tstring >& inPaths ) {
    IndexingOptions options{};
    options.retainFolderStructure = mArchiveCreator.retainDirectories();
    options.followSymlinks = !mArchiveCreator.storeSymbolicLinks();
    mNewItemsVector.indexPaths( inPaths, options );
}

void BitOutputArchive::addItems( const std::map< tstring, tstring >& inPaths ) {
    IndexingOptions options{};
    options.followSymlinks = !mArchiveCreator.storeSymbolicLinks();
    mNewItemsVector.indexPathsMap( inPaths, options );
}

void BitOutputArchive::addFile( const tstring& inFile, const tstring& name ) {
    mNewItemsVector.indexFile( inFile,
                               mArchiveCreator.retainDirectories() ? inFile : name,
                               !mArchiveCreator.storeSymbolicLinks() );
}

void BitOutputArchive::addFile( const std::vector< byte_t >& inBuffer, const tstring& name ) {
    mNewItemsVector.indexBuffer( inBuffer, name );
}

void BitOutputArchive::addFile( std::istream& inStream, const tstring& name ) {
    mNewItemsVector.indexStream( inStream, name );
}

void BitOutputArchive::addFiles( const std::vector< tstring >& inFiles ) {
    IndexingOptions options{};
    options.recursive = false;
    options.retainFolderStructure = mArchiveCreator.retainDirectories();
    options.onlyFiles = true;
    options.followSymlinks = !mArchiveCreator.storeSymbolicLinks();
    mNewItemsVector.indexPaths( inFiles, options );
}

void BitOutputArchive::addFiles( const tstring& inDir, const tstring& filter, bool recursive ) {
    addFiles( inDir, filter, FilterPolicy::Include, recursive );
}

void BitOutputArchive::addFiles( const tstring& inDir, const tstring& filter, FilterPolicy policy, bool recursive ) {
    IndexingOptions options{};
    options.recursive = recursive;
    options.retainFolderStructure = mArchiveCreator.retainDirectories();
    options.onlyFiles = true;
    options.followSymlinks = !mArchiveCreator.storeSymbolicLinks();
    mNewItemsVector.indexDirectory( tstring_to_path( inDir ), filter, policy, options );
}

void BitOutputArchive::addDirectory( const tstring& inDir ) {
    IndexingOptions options{};
    options.retainFolderStructure = mArchiveCreator.retainDirectories();
    options.followSymlinks = !mArchiveCreator.storeSymbolicLinks();
    mNewItemsVector.indexDirectory( tstring_to_path( inDir ), BIT7Z_STRING( "" ), FilterPolicy::Include, options );
}

void BitOutputArchive::addDirectoryContents( const tstring& inDir, const tstring& filter, bool recursive ) {
    addDirectoryContents( inDir, filter, FilterPolicy::Include, recursive );
}

void BitOutputArchive::addDirectoryContents( const tstring& inDir,
                                             const tstring& filter,
                                             FilterPolicy policy,
                                             bool recursive ) {
    IndexingOptions options{};
    options.recursive = recursive;
    options.onlyFiles = !recursive;
    options.retainFolderStructure = mArchiveCreator.retainDirectories();
    options.followSymlinks = !mArchiveCreator.storeSymbolicLinks();
    std::error_code error;
    mNewItemsVector.indexDirectory( fs::absolute( tstring_to_path( inDir ), error ), filter, policy, options );
}

auto BitOutputArchive::initOutArchive() const -> CMyComPtr< IOutArchive > {
    CMyComPtr< IOutArchive > newArc;
    if ( mInputArchive == nullptr ) {
        newArc = mArchiveCreator.library().initOutArchive( mArchiveCreator.compressionFormat() );
    } else {
        (void)mInputArchive->initUpdatableArchive( &newArc ); // TODO: Handle errors
    }
    setArchiveProperties( newArc );
    return newArc;
}

auto BitOutputArchive::initOutFileStream( const fs::path& outArchive,
                                          bool updatingArchive ) const -> CMyComPtr< IOutStream > {
    if ( mArchiveCreator.volumeSize() > 0 ) {
        return bit7z::make_com< CMultiVolumeOutStream, IOutStream >( mArchiveCreator.volumeSize(), outArchive );
    }

    fs::path outPath = outArchive;
    if ( updatingArchive ) {
        outPath += ".tmp";
    }

    return bit7z::make_com< CFileOutStream, IOutStream >( outPath, updatingArchive );
}

void BitOutputArchive::compressOut( IOutArchive* outArc,
                                    IOutStream* outStream,
                                    UpdateCallback* updateCallback ) {
    if ( mInputArchive != nullptr && mArchiveCreator.updateMode() == UpdateMode::Update ) {
        for ( const auto& newItem : mNewItemsVector ) {
            auto newItemPath = path_to_tstring( newItem->inArchivePath() );
            auto updatedItem = mInputArchive->find( newItemPath );
            if ( updatedItem != mInputArchive->cend() ) {
                setDeletedIndex( updatedItem->index() );
            }
        }
    }
    updateInputIndices();

    const HRESULT result = outArc->UpdateItems( outStream, itemsCount(), updateCallback );

    if ( result == E_NOTIMPL ) {
        throw BitException( "Unsupported operation", bit7z::make_hresult_code( result ) );
    }

    if ( result != S_OK ) {
        throw BitException( "Error while compressing files", make_hresult_code( result ), std::move( mFailedFiles ) );
    }
}

void BitOutputArchive::compressToFile( const fs::path& outFile, UpdateCallback* updateCallback ) {
    // Note: if mInputArchive != nullptr, newArc will actually point to the same IInArchive object used by the old_arc
    // (see initUpdatableArchive function of BitInputArchive)!
    const bool updatingArchive = mInputArchive != nullptr && tstring_to_path( mInputArchive->archivePath() ) == outFile;
    const CMyComPtr< IOutArchive > newArc = initOutArchive();
    CMyComPtr< IOutStream > outStream = initOutFileStream( outFile, updatingArchive );
    compressOut( newArc, outStream, updateCallback );

    if ( updatingArchive ) { //we updated the input archive
        auto closeResult = mInputArchive->close();
        if ( closeResult != S_OK ) {
            throw BitException( "Failed to close the archive", make_hresult_code( closeResult ),
                                mInputArchive->archivePath() );
        }
        /* NOTE: In the following instruction, we use the (dot) operator, not the -> (arrow) operator:
         *       in fact, both CMyComPtr and IOutStream have a Release() method, and we need to call only
         *       the one of CMyComPtr (which in turns calls the one of IOutStream)! */
        outStream.Release(); //Releasing the output stream so that we can rename it as the original file.

        std::error_code error;
#if defined( __MINGW32__ ) && defined( BIT7Z_USE_STANDARD_FILESYSTEM )
        /* MinGW seems to not follow the standard since filesystem::rename does not overwrite an already
         * existing destination file (as it should). So we explicitly remove it before! */
        if ( !fs::remove( outFile, error ) ) {
            throw BitException( "Failed to delete the old archive file", error, path_to_tstring( outFile ) );
        }
#endif

        //remove the old file and rename the temporary file (move file with overwriting)
        fs::path tmpFile = outFile;
        tmpFile += ".tmp";
        fs::rename( tmpFile, outFile, error );
        if ( error ) {
            throw BitException( "Failed to overwrite the old archive file", error, path_to_tstring( outFile ) );
        }
    }
}

void BitOutputArchive::compressTo( const tstring& outFile ) {
    using namespace bit7z::filesystem;
    const fs::path outPath = tstring_to_path( outFile );
    std::error_code error;
    if ( fs::exists( outPath, error ) ) {
        const OverwriteMode overwriteMode = mArchiveCreator.overwriteMode();
        if ( overwriteMode == OverwriteMode::Skip ) { // Skipping if the output file already exists
            return;
        }
        if ( overwriteMode == OverwriteMode::Overwrite && !fs::remove( outPath, error ) ) {
            throw BitException( "Failed to delete the old archive file", error, outFile );
        }
        // Note: if overwriteMode is OverwriteMode::None, an exception will be thrown by the CFileOutStream constructor
        // called by the initOutFileStream function.
    }

    auto updateCallback = bit7z::make_com< UpdateCallback >( *this );
    compressToFile( outPath, updateCallback );
}

void BitOutputArchive::compressTo( std::vector< byte_t >& outBuffer ) {
    if ( !outBuffer.empty() ) {
        const OverwriteMode overwriteMode = mArchiveCreator.overwriteMode();
        if ( overwriteMode == OverwriteMode::Skip ) {
            return;
        }
        if ( overwriteMode == OverwriteMode::Overwrite ) {
            outBuffer.clear();
        } else {
            throw BitException( "Cannot compress to buffer", make_error_code( BitError::NonEmptyOutputBuffer ) );
        }
    }

    const CMyComPtr< IOutArchive > newArc = initOutArchive();
    auto outMemStream = bit7z::make_com< CBufferOutStream, IOutStream >( outBuffer );
    auto updateCallback = bit7z::make_com< UpdateCallback >( *this );
    compressOut( newArc, outMemStream, updateCallback );
}

void BitOutputArchive::compressTo( std::ostream& outStream ) {
    const CMyComPtr< IOutArchive > newArc = initOutArchive();
    auto outStdStream = bit7z::make_com< CStdOutStream, IOutStream >( outStream );
    auto updateCallback = bit7z::make_com< UpdateCallback >( *this );
    compressOut( newArc, outStdStream, updateCallback );
}

void BitOutputArchive::setArchiveProperties( IOutArchive* outArchive ) const {
    const ArchiveProperties properties = mArchiveCreator.archiveProperties();
    if ( properties.empty() ) {
        return;
    }

    CMyComPtr< ISetProperties > setProperties;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    HRESULT res = outArchive->QueryInterface( ::IID_ISetProperties, reinterpret_cast< void** >( &setProperties ) );
    if ( res != S_OK ) {
        throw BitException( "ISetProperties unsupported", make_hresult_code( res ) );
    }
    res = setProperties->SetProperties( properties.names(),
                                        properties.values(),
                                        static_cast< uint32_t >( properties.size() ) );
    if ( res != S_OK ) {
        throw BitException( "Cannot set properties of the archive", make_hresult_code( res ) );
    }
}

void BitOutputArchive::updateInputIndices() {
    if ( mDeletedItems.empty() ) {
        return;
    }

    uint32_t offset = 0;
    for ( uint32_t newIndex = 0; newIndex < itemsCount(); ++newIndex ) {
        for ( auto it = mDeletedItems.find( newIndex + offset );
              it != mDeletedItems.end() && *it == newIndex + offset;
              ++it ) {
            ++offset;
        }
        mInputIndices.push_back( static_cast< InputIndex >( newIndex + offset ) );
    }
}

auto BitOutputArchive::itemsCount() const -> uint32_t {
    auto result = static_cast< uint32_t >( mNewItemsVector.size() );
    if ( mInputArchive != nullptr ) {
        result += mInputArchive->itemsCount() - static_cast< uint32_t >( mDeletedItems.size() );
    }
    return result;
}

auto BitOutputArchive::itemProperty( InputIndex index, BitProperty property ) const -> BitPropVariant {
    const auto newItemIndex = static_cast< size_t >( index ) - static_cast< size_t >( mInputArchiveItemsCount );
    const GenericInputItem& newItem = mNewItemsVector[ newItemIndex ];
    return newItem.itemProperty( property );
}

auto BitOutputArchive::itemStream( InputIndex index, ISequentialInStream** inStream ) const -> HRESULT {
    const auto newItemIndex = static_cast< size_t >( index ) - static_cast< size_t >( mInputArchiveItemsCount );
    const GenericInputItem& newItem = mNewItemsVector[ newItemIndex ];

    const HRESULT res = newItem.getStream( inStream );
    if ( FAILED( res ) ) {
        auto path = tstring_to_path( newItem.path() );
        std::error_code error;
        if ( fs::exists( path, error ) ) {
            error = std::make_error_code( std::errc::file_exists );
        }
        mFailedFiles.emplace_back( path_to_tstring( path ), error );
    }
    return res;
}

auto BitOutputArchive::hasNewData( uint32_t index ) const noexcept -> bool {
    const auto originalIndex = static_cast< uint32_t >( itemInputIndex( index ) );
    return originalIndex >= mInputArchiveItemsCount;
}

auto BitOutputArchive::hasNewProperties( uint32_t index ) const noexcept -> bool {
    /* Note: in BitOutputArchive, you can only add new items or overwrite (delete + add) existing ones.
     * So if we have new data, we also have new properties! This is not true for BitArchiveEditor! */
    return hasNewData( index );
}

auto BitOutputArchive::itemInputIndex( uint32_t newIndex ) const noexcept -> InputIndex {
    const auto index = static_cast< decltype( mInputIndices )::size_type >( newIndex );
    if ( index < mInputIndices.size() ) {
        return mInputIndices[ index ];
    }
    // if we are here, the user didn't delete any item, so the InputIndex is essentially equivalent to the newIndex
    return static_cast< InputIndex >( newIndex );
}

auto BitOutputArchive::outputItemProperty( uint32_t index, BitProperty property ) const -> BitPropVariant {
    const auto mappedIndex = itemInputIndex( index );
    return itemProperty( mappedIndex, property );
}

auto BitOutputArchive::outputItemStream( uint32_t index, ISequentialInStream** inStream ) const -> HRESULT {
    const auto mappedIndex = itemInputIndex( index );
    return itemStream( mappedIndex, inStream );
}

auto BitOutputArchive::indexInArchive( uint32_t index ) const noexcept -> uint32_t {
    const auto originalIndex = static_cast< uint32_t >( itemInputIndex( index ) );
    return originalIndex < mInputArchiveItemsCount ? originalIndex : static_cast< uint32_t >( -1 );
}

auto BitOutputArchive::handler() const noexcept -> const BitAbstractArchiveHandler& {
    return mArchiveCreator;
}

auto BitOutputArchive::creator() const noexcept -> const BitAbstractArchiveCreator& {
    return mArchiveCreator;
}

} // namespace bit7z