//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#include "pch.h"
#include "FileUtility.h"
#include <fstream>
#include <mutex>
#include <zlib.h> // From NuGet package 

using namespace std;
using namespace Utility;

namespace Utility
{
    ByteArray NullFile = make_shared<vector<byte> > (vector<byte>() );
}

ByteArray DecompressZippedFile( wstring& fileName );

ByteArray ReadFileHelper(const wstring& fileName)
{
    struct _stat64 fileStat;
    int fileExists = _wstat64(fileName.c_str(), &fileStat);
    if (fileExists == -1)
        return NullFile;

    ifstream file( fileName, ios::in | ios::binary );
    if (!file)
        return NullFile;

    Utility::ByteArray byteArray = make_shared<vector<byte> >( file.seekg(0, ios::end).tellg() );
    file.seekg(0, ios::beg).read( (char*)byteArray->data(), byteArray->size() );
    file.close();

    ASSERT(byteArray->size() == (size_t)fileStat.st_size);

    return byteArray;
}

ByteArray ReadFileHelperEx( shared_ptr<wstring> fileName)
{
    std::wstring zippedFileName = *fileName + L".gz";
    ByteArray firstTry = DecompressZippedFile(zippedFileName);
    if (firstTry != NullFile)
        return firstTry;

    return ReadFileHelper(*fileName);
}

ByteArray Inflate(ByteArray CompressedSource, int& err, uint32_t ChunkSize = 0x100000 ) 
{
    // Create a dynamic buffer to hold compressed blocks
    vector<unique_ptr<byte> > blocks;

    z_stream strm  = {};
    strm.data_type = Z_BINARY;
    strm.total_in  = strm.avail_in  = (uInt)CompressedSource->size();
    strm.next_in   = CompressedSource->data();

    err = inflateInit2(&strm, (15 + 32)); //15 window bits, and the +32 tells zlib to to detect if using gzip or zlib

    while (err == Z_OK || err == Z_BUF_ERROR)
    {
        strm.avail_out = ChunkSize;
        strm.next_out = (byte*)malloc(ChunkSize);
        blocks.emplace_back(strm.next_out);
        err = inflate(&strm, Z_NO_FLUSH);
    }

    if (err != Z_STREAM_END) 
    {
        inflateEnd(&strm);
        return NullFile;
    }

    ASSERT(strm.total_out > 0, "Nothing to decompress");

    Utility::ByteArray byteArray = make_shared<vector<byte> >( strm.total_out );

    // Allocate actual memory for this.
    // copy the bits into that RAM.
    // Free everything else up!!
    void* curDest = byteArray->data();
    size_t remaining = byteArray->size();

    for (size_t i = 0; i < blocks.size(); ++i)
    {
        ASSERT(remaining > 0);

        size_t CopySize = min(remaining, (size_t)ChunkSize);

        memcpy(curDest, blocks[i].get(), CopySize);
        curDest = (byte*)curDest + CopySize;
        remaining -= CopySize;
    }

    inflateEnd(&strm);

    return byteArray;
}

ByteArray DecompressZippedFile( wstring& fileName )
{
    ByteArray CompressedFile = ReadFileHelper(fileName);
    if (CompressedFile == NullFile)
        return NullFile;

    int error;
    ByteArray DecompressedFile = Inflate(CompressedFile, error);
    if (DecompressedFile->size() == 0)
    {
        Utility::Printf(L"Couldn't unzip file %s:  Error = %d\n", fileName.c_str(), error);
        return NullFile;
    }

    return DecompressedFile;
}

ByteArray Utility::ReadFileSync( const wstring& fileName)
{
    return ReadFileHelperEx(make_shared<wstring>(fileName));
}

task<ByteArray> Utility::ReadFileAsync(const wstring& fileName)
{
    shared_ptr<wstring> SharedPtr = make_shared<wstring>(fileName);
    return create_task( [=] { return ReadFileHelperEx(SharedPtr); } );
}
