#include "pch.h"

namespace FallbackLayer
{
#define DXBC_BLOB_NOT_FOUND ((UINT)-1)

    class CDXBCParser
    {
    public:
#define DXBC_HASH_SIZE 16
        typedef struct DXBCHash
        {
            unsigned char Digest[DXBC_HASH_SIZE];
        } DXBCHash;

        typedef struct DXBCVersion
        {
            UINT16 Major;
            UINT16 Minor;
        } DXBCVersion;

        typedef struct DXBCHeader
        {
            UINT        DXBCHeaderFourCC;
            DXBCHash    Hash;
            DXBCVersion Version;
            UINT32      ContainerSizeInBytes; // Count from start of this header, including all blobs
            UINT32      BlobCount;
            // Structure is followed by UINT32[BlobCount] (the blob index, storing offsets from start of container in bytes 
            //                                             to the start of each blob's header)
        } DXBCHeader;

        typedef struct DXBCBlobHeader
        {
            UINT32      BlobFourCC; // originally of type enum DXBCFourCC
            UINT32      BlobSize;    // Byte count for BlobData
                                     // Structure is followed by BYTE[BlobSize] (the blob's data)
        } DXBCBlobHeader;

        static const UINT32 DXBCSizeOffset = offsetof(struct DXBCHeader, ContainerSizeInBytes);

        // DXBCGetSizeAssumingValidPointer()
        static UINT DXBCGetSizeAssumingValidPointer(const void* pDXBC)
        {
            if (!pDXBC) return 0;

            return *(UINT*)((const BYTE*)pDXBC + DXBCSizeOffset);
        }

#define DXBC_FOURCC(ch0, ch1, ch2, ch3)                              \
            ((UINT)(BYTE)(ch0) | ((UINT)(BYTE)(ch1) << 8) |   \
            ((UINT)(BYTE)(ch2) << 16) | ((UINT)(BYTE)(ch3) << 24 ))

        typedef enum DXBCFourCC
        {
            DXBC_RuntimeData = DXBC_FOURCC('R', 'D', 'A', 'T'),
        } DXBCFourCC;

        UINT FindNextMatchingBlob(DXBCFourCC SearchFourCC, UINT SearchStartBlobIndex)
        {
            if (!m_pHeader || !m_pIndex)
            {
                return DXBC_BLOB_NOT_FOUND;
            }
            for (UINT b = SearchStartBlobIndex; b < m_pHeader->BlobCount; b++)
            {
                DXBCBlobHeader* pBlob = (DXBCBlobHeader*)((BYTE*)m_pHeader + m_pIndex[b]);
                if (pBlob->BlobFourCC == (UINT32)SearchFourCC)
                {
                    return b;
                }
            }
            return DXBC_BLOB_NOT_FOUND;
        }

        HRESULT ReadDXBC(const void* pContainer, UINT ContainerSizeInBytes)
        {
            if (!pContainer)
            {
                return E_FAIL;
            }
            if (ContainerSizeInBytes < sizeof(DXBCHeader))
            {
                return E_FAIL;
            }
            DXBCHeader* pHeader = (DXBCHeader*)pContainer;
            if (pHeader->ContainerSizeInBytes != ContainerSizeInBytes)
            {
                return E_FAIL;
            }

            const void *pContainerEnd = ((const BYTE*)pHeader + ContainerSizeInBytes);
            if (pContainerEnd < pContainer)
            {
                return E_FAIL;
            }
            UINT* pIndex = (UINT*)((BYTE*)pHeader + sizeof(DXBCHeader));
            if ((const BYTE*)pContainer + sizeof(UINT)*pHeader->BlobCount < (const BYTE*)pContainer)
            {
                return E_FAIL; // overflow would break the calculation of OffsetOfCurrentSegmentEnd below
            }
            UINT OffsetOfCurrentSegmentEnd = (UINT)((BYTE*)pIndex - (const BYTE*)pContainer + sizeof(UINT)*pHeader->BlobCount - 1);
            // Is the entire index within the container?
            if (OffsetOfCurrentSegmentEnd > ContainerSizeInBytes)
            {
                return E_FAIL;
            }
            // Is each blob in the index directly after the previous entry and not past the end of the container?
            UINT OffsetOfLastSegmentEnd = OffsetOfCurrentSegmentEnd;
            for (UINT b = 0; b < pHeader->BlobCount; b++)
            {
                DXBCBlobHeader* pBlobHeader = (DXBCBlobHeader*)((const BYTE*)pContainer + pIndex[b]);
                DXBCBlobHeader* pAfterBlobHeader = pBlobHeader + 1;

                if (pAfterBlobHeader < pBlobHeader || pAfterBlobHeader > pContainerEnd)
                {
                    return E_FAIL;
                }
                if (((BYTE*)pBlobHeader < (const BYTE*)pContainer) || (pIndex[b] + sizeof(DXBCBlobHeader) < pIndex[b]))
                {
                    return E_FAIL; // overflow because of bad pIndex[b] value
                }
                if (pIndex[b] + sizeof(DXBCBlobHeader) + pBlobHeader->BlobSize < pIndex[b])
                {
                    return E_FAIL; // overflow because of bad pBlobHeader->BlobSize value
                }
                OffsetOfCurrentSegmentEnd = pIndex[b] + sizeof(DXBCBlobHeader) + pBlobHeader->BlobSize - 1;
                if (OffsetOfCurrentSegmentEnd > ContainerSizeInBytes)
                {
                    return E_FAIL;
                }
                if (OffsetOfLastSegmentEnd != pIndex[b] - 1)
                {
                    return E_FAIL;
                }
                OffsetOfLastSegmentEnd = OffsetOfCurrentSegmentEnd;
            }

            // Ok, satisfied with integrity of container, store info.
            m_pHeader = pHeader;
            m_pIndex = pIndex;
            return S_OK;
        }

        //---------------------------------------------------------------------------------------------------------------------------------
        // CDXBCParser::GetBlob()
        const void* CDXBCParser::GetBlob(UINT BlobIndex)
        {
            if (!m_pHeader || !m_pIndex || m_pHeader->BlobCount <= BlobIndex)
            {
                return NULL;
            }
            return (BYTE*)m_pHeader + m_pIndex[BlobIndex] + sizeof(DXBCBlobHeader);
        }

        // CDXBCParser::GetBlobSize()
        UINT CDXBCParser::GetBlobSize(UINT BlobIndex)
        {
            if (!m_pHeader || !m_pIndex || m_pHeader->BlobCount <= BlobIndex)
            {
                return 0;
            }
            return ((DXBCBlobHeader*)((BYTE*)m_pHeader + m_pIndex[BlobIndex]))->BlobSize;
        }

        const DXBCHeader*    m_pHeader;
        const UINT32*        m_pIndex;
    };

    HRESULT GetRuntimeData(const void* pShaderByteCode, const UINT **ppRuntimeData, UINT *pRuntimeDataSizeInBytes)
    {
        HRESULT hr = S_OK;
        CDXBCParser dxbcParser;
        if (FAILED(hr = dxbcParser.ReadDXBC(pShaderByteCode, CDXBCParser::DXBCGetSizeAssumingValidPointer(pShaderByteCode))))
        {
            *ppRuntimeData = NULL;
            return hr;
        }
        UINT BlobIndex = dxbcParser.FindNextMatchingBlob(CDXBCParser::DXBC_RuntimeData, 0);
        if (DXBC_BLOB_NOT_FOUND == BlobIndex)
        {
            *ppRuntimeData = NULL;
            return E_FAIL;
        }
        *ppRuntimeData = (UINT *)dxbcParser.GetBlob(BlobIndex);
        if (!*ppRuntimeData)
        {
            return E_FAIL;
        }
        if (pRuntimeDataSizeInBytes)
        {
            *pRuntimeDataSizeInBytes = dxbcParser.GetBlobSize(BlobIndex);
        }
        return S_OK;
    }
}