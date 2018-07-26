/*-------------------------------------------------------------------------------------
 *
 * Copyright (c) Microsoft Corporation
 *
 *-------------------------------------------------------------------------------------*/


/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0622 */



/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

/* verify that the <rpcsal.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCSAL_H_VERSION__
#define __REQUIRED_RPCSAL_H_VERSION__ 100
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __d3d12_1_h__
#define __d3d12_1_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __ID3D12CryptoSession_FWD_DEFINED__
#define __ID3D12CryptoSession_FWD_DEFINED__
typedef interface ID3D12CryptoSession ID3D12CryptoSession;

#endif     /* __ID3D12CryptoSession_FWD_DEFINED__ */


#ifndef __ID3D12CryptoSessionPolicy_FWD_DEFINED__
#define __ID3D12CryptoSessionPolicy_FWD_DEFINED__
typedef interface ID3D12CryptoSessionPolicy ID3D12CryptoSessionPolicy;

#endif     /* __ID3D12CryptoSessionPolicy_FWD_DEFINED__ */


#ifndef __ID3D12ContentProtectionDevice_FWD_DEFINED__
#define __ID3D12ContentProtectionDevice_FWD_DEFINED__
typedef interface ID3D12ContentProtectionDevice ID3D12ContentProtectionDevice;

#endif     /* __ID3D12ContentProtectionDevice_FWD_DEFINED__ */


#ifndef __ID3D12TrackedWorkload_FWD_DEFINED__
#define __ID3D12TrackedWorkload_FWD_DEFINED__
typedef interface ID3D12TrackedWorkload ID3D12TrackedWorkload;

#endif     /* __ID3D12TrackedWorkload_FWD_DEFINED__ */


#ifndef __ID3D12VideoDecodeCommandList2_FWD_DEFINED__
#define __ID3D12VideoDecodeCommandList2_FWD_DEFINED__
typedef interface ID3D12VideoDecodeCommandList2 ID3D12VideoDecodeCommandList2;

#endif     /* __ID3D12VideoDecodeCommandList2_FWD_DEFINED__ */


#ifndef __ID3D12VideoProcessCommandList2_FWD_DEFINED__
#define __ID3D12VideoProcessCommandList2_FWD_DEFINED__
typedef interface ID3D12VideoProcessCommandList2 ID3D12VideoProcessCommandList2;

#endif     /* __ID3D12VideoProcessCommandList2_FWD_DEFINED__ */


#ifndef __ID3D12VideoMotionEstimator_FWD_DEFINED__
#define __ID3D12VideoMotionEstimator_FWD_DEFINED__
typedef interface ID3D12VideoMotionEstimator ID3D12VideoMotionEstimator;

#endif     /* __ID3D12VideoMotionEstimator_FWD_DEFINED__ */


#ifndef __ID3D12VideoMotionVectorHeap_FWD_DEFINED__
#define __ID3D12VideoMotionVectorHeap_FWD_DEFINED__
typedef interface ID3D12VideoMotionVectorHeap ID3D12VideoMotionVectorHeap;

#endif     /* __ID3D12VideoMotionVectorHeap_FWD_DEFINED__ */


#ifndef __ID3D12VideoDevice1_FWD_DEFINED__
#define __ID3D12VideoDevice1_FWD_DEFINED__
typedef interface ID3D12VideoDevice1 ID3D12VideoDevice1;

#endif     /* __ID3D12VideoDevice1_FWD_DEFINED__ */


#ifndef __ID3D12VideoEncodeCommandList_FWD_DEFINED__
#define __ID3D12VideoEncodeCommandList_FWD_DEFINED__
typedef interface ID3D12VideoEncodeCommandList ID3D12VideoEncodeCommandList;

#endif     /* __ID3D12VideoEncodeCommandList_FWD_DEFINED__ */


#ifndef __ID3D12DeviceRaytracingPrototype_FWD_DEFINED__
#define __ID3D12DeviceRaytracingPrototype_FWD_DEFINED__
typedef interface ID3D12DeviceRaytracingPrototype ID3D12DeviceRaytracingPrototype;

#endif     /* __ID3D12DeviceRaytracingPrototype_FWD_DEFINED__ */


#ifndef __ID3D12CommandListRaytracingPrototype_FWD_DEFINED__
#define __ID3D12CommandListRaytracingPrototype_FWD_DEFINED__
typedef interface ID3D12CommandListRaytracingPrototype ID3D12CommandListRaytracingPrototype;

#endif     /* __ID3D12CommandListRaytracingPrototype_FWD_DEFINED__ */


#ifndef __ID3D12GraphicsCommandList5_FWD_DEFINED__
#define __ID3D12GraphicsCommandList5_FWD_DEFINED__
typedef interface ID3D12GraphicsCommandList5 ID3D12GraphicsCommandList5;

#endif     /* __ID3D12GraphicsCommandList5_FWD_DEFINED__ */


#ifndef __ID3D12DeviceTrackedWorkload_FWD_DEFINED__
#define __ID3D12DeviceTrackedWorkload_FWD_DEFINED__
typedef interface ID3D12DeviceTrackedWorkload ID3D12DeviceTrackedWorkload;

#endif     /* __ID3D12DeviceTrackedWorkload_FWD_DEFINED__ */


#ifndef __ID3D12Device_GUIDTextureLayout_FWD_DEFINED__
#define __ID3D12Device_GUIDTextureLayout_FWD_DEFINED__
typedef interface ID3D12Device_GUIDTextureLayout ID3D12Device_GUIDTextureLayout;

#endif     /* __ID3D12Device_GUIDTextureLayout_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "dxgicommon.h"
#include "d3dcommon.h"
#include "d3d12.h"
#include "d3d12video.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_d3d12_1_0000_0000 */
/* [local] */ 

#include <winapifamily.h>
#pragma region App Family
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP | WINAPI_PARTITION_GAMES)
#define D3D12_FEATURE_VIDEO_DECODE_BITSTREAM_ENCRYPTION_SCHEMES       static_cast<D3D12_FEATURE_VIDEO>(4)
#define D3D12_FEATURE_VIDEO_DECODE_BITSTREAM_ENCRYPTION_SCHEME_COUNT  static_cast<D3D12_FEATURE_VIDEO>(12)
#define D3D12_FEATURE_VIDEO_CRYPTO_SESSION_SUPPORT                    static_cast<D3D12_FEATURE_VIDEO>(13)
#define D3D12_FEATURE_VIDEO_CONTENT_PROTECTION_SYSTEM_COUNT           static_cast<D3D12_FEATURE_VIDEO>(14)
#define D3D12_FEATURE_VIDEO_CONTENT_PROTECTION_SYSTEM_SUPPORT         static_cast<D3D12_FEATURE_VIDEO>(15)
#define D3D12_FEATURE_VIDEO_CRYPTO_SESSION_TRANSFORM_SUPPORT          static_cast<D3D12_FEATURE_VIDEO>(16)
#define D3D12_FEATURE_VIDEO_FEATURE_AREA_SUPPORT                      static_cast<D3D12_FEATURE_VIDEO>(19)
#define D3D12_FEATURE_VIDEO_MOTION_ESTIMATOR                          static_cast<D3D12_FEATURE_VIDEO>(20)
#define D3D12_FEATURE_VIDEO_MOTION_ESTIMATOR_SIZE                     static_cast<D3D12_FEATURE_VIDEO>(21)
#define D3D12_BITSTREAM_ENCRYPTION_TYPE_CENC_AES_CTR_128              static_cast<D3D12_BITSTREAM_ENCRYPTION_TYPE>(1)
#define D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE                          static_cast<D3D12_COMMAND_LIST_TYPE>(6)
#define D3D12_COMMAND_LIST_SUPPORT_FLAG_VIDEO_ENCODE                  static_cast<>(1 << D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE)
typedef struct D3D12_FEATURE_DATA_VIDEO_CONTENT_PROTECTION_SYSTEM_COUNT
    {
    UINT NodeIndex;
    UINT ContentProtectionSystemCount;
    }     D3D12_FEATURE_DATA_VIDEO_CONTENT_PROTECTION_SYSTEM_COUNT;

typedef struct D3D12_FEATURE_DATA_VIDEO_CONTENT_PROTECTION_SYSTEM_SUPPORT
    {
    UINT NodeIndex;
    UINT ContentProtectionSystemCount;
    GUID *pContentProtectionSystems;
    }     D3D12_FEATURE_DATA_VIDEO_CONTENT_PROTECTION_SYSTEM_SUPPORT;

typedef 
enum D3D12_CRYPTO_SESSION_SUPPORT_FLAGS
    {
        D3D12_CRYPTO_SESSION_SUPPORT_FLAG_NONE    = 0,
        D3D12_CRYPTO_SESSION_SUPPORT_FLAG_SUPPORTED    = 0x1,
        D3D12_CRYPTO_SESSION_SUPPORT_FLAG_HEADER_DECRYPTION_REQUIRED    = 0x2,
        D3D12_CRYPTO_SESSION_SUPPORT_FLAG_INDEPENDENT_DECRYPTION_REQUIRED    = 0x4,
        D3D12_CRYPTO_SESSION_SUPPORT_FLAG_TRANSCRYPTION_REQUIRED    = 0x8
    }     D3D12_CRYPTO_SESSION_SUPPORT_FLAGS;

DEFINE_ENUM_FLAG_OPERATORS( D3D12_CRYPTO_SESSION_SUPPORT_FLAGS );
typedef 
enum D3D12_CRYPTO_SESSION_FLAGS
    {
        D3D12_CRYPTO_SESSION_FLAG_NONE    = 0,
        D3D12_CRYPTO_SESSION_FLAG_HARDWARE    = 0x1
    }     D3D12_CRYPTO_SESSION_FLAGS;

DEFINE_ENUM_FLAG_OPERATORS( D3D12_CRYPTO_SESSION_FLAGS );
typedef struct D3D12_FEATURE_DATA_VIDEO_DECODE_BITSTREAM_ENCRYPTION_SCHEME_COUNT
    {
    UINT NodeIndex;
    GUID DecodeProfile;
    UINT EncryptionSchemeCount;
    }     D3D12_FEATURE_DATA_VIDEO_DECODE_BITSTREAM_ENCRYPTION_SCHEME_COUNT;

typedef struct D3D12_FEATURE_DATA_VIDEO_DECODE_BITSTREAM_ENCRYPTION_SCHEMES
    {
    UINT NodeIndex;
    GUID DecodeProfile;
    UINT EncryptionSchemeCount;
    _Field_size_full_(EncryptionSchemeCount)  D3D12_BITSTREAM_ENCRYPTION_TYPE *pEncryptionSchemes;
    }     D3D12_FEATURE_DATA_VIDEO_DECODE_BITSTREAM_ENCRYPTION_SCHEMES;

typedef struct D3D12_FEATURE_DATA_VIDEO_CRYPTO_SESSION_SUPPORT
    {
    UINT NodeIndex;
    GUID DecodeProfile;
    GUID ContentProtectionSystem;
    D3D12_CRYPTO_SESSION_FLAGS Flags;
    D3D12_BITSTREAM_ENCRYPTION_TYPE BitstreamEncryption;
    UINT KeyBaseDataSize;
    D3D12_CRYPTO_SESSION_SUPPORT_FLAGS Support;
    }     D3D12_FEATURE_DATA_VIDEO_CRYPTO_SESSION_SUPPORT;

typedef struct D3D12_CRYPTO_SESSION_DESC
    {
    UINT NodeMask;
    GUID DecodeProfile;
    GUID ContentProtectionSystem;
    D3D12_BITSTREAM_ENCRYPTION_TYPE BitstreamEncryption;
    D3D12_CRYPTO_SESSION_FLAGS Flags;
    }     D3D12_CRYPTO_SESSION_DESC;



extern RPC_IF_HANDLE __MIDL_itf_d3d12_1_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_d3d12_1_0000_0000_v0_0_s_ifspec;

#ifndef __ID3D12CryptoSession_INTERFACE_DEFINED__
#define __ID3D12CryptoSession_INTERFACE_DEFINED__

/* interface ID3D12CryptoSession */
/* [unique][local][object][uuid] */ 


EXTERN_C const IID IID_ID3D12CryptoSession;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FC7C6C9D-C27D-4904-835D-A5F2096EC65F")
    ID3D12CryptoSession : public ID3D12ProtectedSession
    {
    public:
        virtual D3D12_CRYPTO_SESSION_DESC STDMETHODCALLTYPE GetDesc( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetKeyBaseData( 
            UINT KeyInputDataSize,
            _In_reads_bytes_( KeyInputDataSize )  const void *pKeyInputData,
            UINT KeyBaseDataSize,
            _Out_writes_bytes_( KeyBaseDataSize )  void *pKeyBaseData) = 0;
        
    };
    
    
#else     /* C style interface */

    typedef struct ID3D12CryptoSessionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ID3D12CryptoSession * This,
            REFIID riid,
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ID3D12CryptoSession * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ID3D12CryptoSession * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPrivateData )( 
            ID3D12CryptoSession * This,
            _In_  REFGUID guid,
            _Inout_  UINT *pDataSize,
            _Out_writes_bytes_opt_( *pDataSize )  void *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetPrivateData )( 
            ID3D12CryptoSession * This,
            _In_  REFGUID guid,
            _In_  UINT DataSize,
            _In_reads_bytes_opt_( DataSize )  const void *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetPrivateDataInterface )( 
            ID3D12CryptoSession * This,
            _In_  REFGUID guid,
            _In_opt_  const IUnknown *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetName )( 
            ID3D12CryptoSession * This,
            _In_z_  LPCWSTR Name);
        
        HRESULT ( STDMETHODCALLTYPE *GetDevice )( 
            ID3D12CryptoSession * This,
            REFIID riid,
            _COM_Outptr_opt_  void **ppvDevice);
        
        HRESULT ( STDMETHODCALLTYPE *GetStatusFence )( 
            ID3D12CryptoSession * This,
            REFIID riid,
            _COM_Outptr_opt_  void **ppFence);
        
        D3D12_PROTECTED_SESSION_STATUS ( STDMETHODCALLTYPE *GetSessionStatus )( 
            ID3D12CryptoSession * This);
        
        D3D12_CRYPTO_SESSION_DESC ( STDMETHODCALLTYPE *GetDesc )( 
            ID3D12CryptoSession * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetKeyBaseData )( 
            ID3D12CryptoSession * This,
            UINT KeyInputDataSize,
            _In_reads_bytes_( KeyInputDataSize )  const void *pKeyInputData,
            UINT KeyBaseDataSize,
            _Out_writes_bytes_( KeyBaseDataSize )  void *pKeyBaseData);
        
        END_INTERFACE
    } ID3D12CryptoSessionVtbl;

    interface ID3D12CryptoSession
    {
        CONST_VTBL struct ID3D12CryptoSessionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ID3D12CryptoSession_QueryInterface(This,riid,ppvObject)    \
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ID3D12CryptoSession_AddRef(This)    \
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ID3D12CryptoSession_Release(This)    \
    ( (This)->lpVtbl -> Release(This) ) 


#define ID3D12CryptoSession_GetPrivateData(This,guid,pDataSize,pData)    \
    ( (This)->lpVtbl -> GetPrivateData(This,guid,pDataSize,pData) ) 

#define ID3D12CryptoSession_SetPrivateData(This,guid,DataSize,pData)    \
    ( (This)->lpVtbl -> SetPrivateData(This,guid,DataSize,pData) ) 

#define ID3D12CryptoSession_SetPrivateDataInterface(This,guid,pData)    \
    ( (This)->lpVtbl -> SetPrivateDataInterface(This,guid,pData) ) 

#define ID3D12CryptoSession_SetName(This,Name)    \
    ( (This)->lpVtbl -> SetName(This,Name) ) 


#define ID3D12CryptoSession_GetDevice(This,riid,ppvDevice)    \
    ( (This)->lpVtbl -> GetDevice(This,riid,ppvDevice) ) 


#define ID3D12CryptoSession_GetStatusFence(This,riid,ppFence)    \
    ( (This)->lpVtbl -> GetStatusFence(This,riid,ppFence) ) 

#define ID3D12CryptoSession_GetSessionStatus(This)    \
    ( (This)->lpVtbl -> GetSessionStatus(This) ) 


#define ID3D12CryptoSession_GetDesc(This)    \
    ( (This)->lpVtbl -> GetDesc(This) ) 

#define ID3D12CryptoSession_GetKeyBaseData(This,KeyInputDataSize,pKeyInputData,KeyBaseDataSize,pKeyBaseData)    \
    ( (This)->lpVtbl -> GetKeyBaseData(This,KeyInputDataSize,pKeyInputData,KeyBaseDataSize,pKeyBaseData) ) 

#endif /* COBJMACROS */


#endif     /* C style interface */




#endif     /* __ID3D12CryptoSession_INTERFACE_DEFINED__ */


#ifndef __ID3D12CryptoSessionPolicy_INTERFACE_DEFINED__
#define __ID3D12CryptoSessionPolicy_INTERFACE_DEFINED__

/* interface ID3D12CryptoSessionPolicy */
/* [unique][local][object][uuid] */ 


EXTERN_C const IID IID_ID3D12CryptoSessionPolicy;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("69FE3108-01A4-4AC3-AB91-F51E377A62AC")
    ID3D12CryptoSessionPolicy : public ID3D12DeviceChild
    {
    public:
        virtual UINT STDMETHODCALLTYPE GetKeyInfoSize( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetKeyInfo( 
            _Out_writes_bytes_( KeyInfoDataSize )  void *pKeyInfo,
            UINT KeyInfoDataSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCryptoSession( 
            REFIID riid,
            _COM_Outptr_opt_  void **ppCryptoSession) = 0;
        
    };
    
    
#else     /* C style interface */

    typedef struct ID3D12CryptoSessionPolicyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ID3D12CryptoSessionPolicy * This,
            REFIID riid,
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ID3D12CryptoSessionPolicy * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ID3D12CryptoSessionPolicy * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPrivateData )( 
            ID3D12CryptoSessionPolicy * This,
            _In_  REFGUID guid,
            _Inout_  UINT *pDataSize,
            _Out_writes_bytes_opt_( *pDataSize )  void *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetPrivateData )( 
            ID3D12CryptoSessionPolicy * This,
            _In_  REFGUID guid,
            _In_  UINT DataSize,
            _In_reads_bytes_opt_( DataSize )  const void *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetPrivateDataInterface )( 
            ID3D12CryptoSessionPolicy * This,
            _In_  REFGUID guid,
            _In_opt_  const IUnknown *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetName )( 
            ID3D12CryptoSessionPolicy * This,
            _In_z_  LPCWSTR Name);
        
        HRESULT ( STDMETHODCALLTYPE *GetDevice )( 
            ID3D12CryptoSessionPolicy * This,
            REFIID riid,
            _COM_Outptr_opt_  void **ppvDevice);
        
        UINT ( STDMETHODCALLTYPE *GetKeyInfoSize )( 
            ID3D12CryptoSessionPolicy * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetKeyInfo )( 
            ID3D12CryptoSessionPolicy * This,
            _Out_writes_bytes_( KeyInfoDataSize )  void *pKeyInfo,
            UINT KeyInfoDataSize);
        
        HRESULT ( STDMETHODCALLTYPE *GetCryptoSession )( 
            ID3D12CryptoSessionPolicy * This,
            REFIID riid,
            _COM_Outptr_opt_  void **ppCryptoSession);
        
        END_INTERFACE
    } ID3D12CryptoSessionPolicyVtbl;

    interface ID3D12CryptoSessionPolicy
    {
        CONST_VTBL struct ID3D12CryptoSessionPolicyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ID3D12CryptoSessionPolicy_QueryInterface(This,riid,ppvObject)    \
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ID3D12CryptoSessionPolicy_AddRef(This)    \
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ID3D12CryptoSessionPolicy_Release(This)    \
    ( (This)->lpVtbl -> Release(This) ) 


#define ID3D12CryptoSessionPolicy_GetPrivateData(This,guid,pDataSize,pData)    \
    ( (This)->lpVtbl -> GetPrivateData(This,guid,pDataSize,pData) ) 

#define ID3D12CryptoSessionPolicy_SetPrivateData(This,guid,DataSize,pData)    \
    ( (This)->lpVtbl -> SetPrivateData(This,guid,DataSize,pData) ) 

#define ID3D12CryptoSessionPolicy_SetPrivateDataInterface(This,guid,pData)    \
    ( (This)->lpVtbl -> SetPrivateDataInterface(This,guid,pData) ) 

#define ID3D12CryptoSessionPolicy_SetName(This,Name)    \
    ( (This)->lpVtbl -> SetName(This,Name) ) 


#define ID3D12CryptoSessionPolicy_GetDevice(This,riid,ppvDevice)    \
    ( (This)->lpVtbl -> GetDevice(This,riid,ppvDevice) ) 


#define ID3D12CryptoSessionPolicy_GetKeyInfoSize(This)    \
    ( (This)->lpVtbl -> GetKeyInfoSize(This) ) 

#define ID3D12CryptoSessionPolicy_GetKeyInfo(This,pKeyInfo,KeyInfoDataSize)    \
    ( (This)->lpVtbl -> GetKeyInfo(This,pKeyInfo,KeyInfoDataSize) ) 

#define ID3D12CryptoSessionPolicy_GetCryptoSession(This,riid,ppCryptoSession)    \
    ( (This)->lpVtbl -> GetCryptoSession(This,riid,ppCryptoSession) ) 

#endif /* COBJMACROS */


#endif     /* C style interface */




#endif     /* __ID3D12CryptoSessionPolicy_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_d3d12_1_0000_0002 */
/* [local] */ 

typedef 
enum D3D12_CRYPTO_SESSION_TRANSFORM_OPERATION
    {
        D3D12_CRYPTO_SESSION_TRANSFORM_OPERATION_NONE    = 0,
        D3D12_CRYPTO_SESSION_TRANSFORM_OPERATION_DECRYPT    = 1,
        D3D12_CRYPTO_SESSION_TRANSFORM_OPERATION_DECRYPT_WITH_HEADER    = 2,
        D3D12_CRYPTO_SESSION_TRANSFORM_OPERATION_TRANSCRYPT    = 3,
        D3D12_CRYPTO_SESSION_TRANSFORM_OPERATION_TRANSCRYPT_WITH_HEADER    = 4,
        D3D12_CRYPTO_SESSION_TRANSFORM_OPERATION_DECRYPT_HEADER    = 5
    }     D3D12_CRYPTO_SESSION_TRANSFORM_OPERATION;

typedef 
enum D3D12_CRYPTO_SESSION_TRANSFORM_SUPPORT_FLAGS
    {
        D3D12_CRYPTO_SESSION_TRANSFORM_SUPPORT_FLAG_NONE    = 0,
        D3D12_CRYPTO_SESSION_TRANSFORM_SUPPORT_FLAG_SUPPORTED    = 0x1
    }     D3D12_CRYPTO_SESSION_TRANSFORM_SUPPORT_FLAGS;

DEFINE_ENUM_FLAG_OPERATORS( D3D12_CRYPTO_SESSION_TRANSFORM_SUPPORT_FLAGS );
typedef struct D3D12_FEATURE_DATA_VIDEO_CRYPTO_SESSION_TRANSFORM_SUPPORT
    {
    UINT NodeIndex;
    GUID DecodeProfile;
    GUID ContentProtectionSystem;
    D3D12_CRYPTO_SESSION_FLAGS Flags;
    D3D12_BITSTREAM_ENCRYPTION_TYPE BitstreamEncryption;
    D3D12_CRYPTO_SESSION_TRANSFORM_OPERATION Operation;
    BOOL ProtectedOutputRequired;
    UINT64 InputAlignment;
    UINT64 InputPreambleSize;
    UINT64 OutputAlignment;
    UINT64 OutputPreambleSize;
    D3D12_CRYPTO_SESSION_TRANSFORM_SUPPORT_FLAGS Support;
    }     D3D12_FEATURE_DATA_VIDEO_CRYPTO_SESSION_TRANSFORM_SUPPORT;

typedef struct D3D12_CRYPTO_SESSION_TRANSFORM_DECRYPT_HEADER_INPUT_ARGUMENTS
    {
    BOOL Enable;
    _Field_size_bytes_full_(SliceHeadersSize)  const void *pSliceHeaders;
    UINT64 SliceHeadersSize;
    _Field_size_full_(SliceHeaderCount)  const DWORD *pSliceHeadersOffsets;
    UINT64 SliceHeaderCount;
    _Field_size_bytes_full_(ContextSize)  const void *pContext;
    UINT64 ContextSize;
    }     D3D12_CRYPTO_SESSION_TRANSFORM_DECRYPT_HEADER_INPUT_ARGUMENTS;

typedef struct D3D12_CRYPTO_SESSION_TRANSFORM_INPUT_ARGUMENTS
    {
    ID3D12CryptoSessionPolicy *pCryptoSessionPolicy;
    ID3D12Resource *pBuffer;
    UINT64 Size;
    UINT64 Offset;
    _Field_size_bytes_full_(IVSize)  const void *pIV;
    UINT IVSize;
    UINT SubSampleMappingCount;
    _Field_size_bytes_full_(SubSampleMappingCount)  const void *pSubSampleMappingBlock;
    UINT64 ContextSize;
    _Field_size_bytes_full_(ContextSize)  const void *pContext;
    D3D12_CRYPTO_SESSION_TRANSFORM_DECRYPT_HEADER_INPUT_ARGUMENTS EncryptedHeader;
    }     D3D12_CRYPTO_SESSION_TRANSFORM_INPUT_ARGUMENTS;

typedef struct D3D12_CRYPTO_SESSION_TRANSFORM_DECRYPT_OUTPUT_ARGUMENTS
    {
    BOOL Enable;
    ID3D12ProtectedSession *pProtectedSession;
    ID3D12Resource *pBuffer;
    UINT64 Size;
    UINT64 Offset;
    }     D3D12_CRYPTO_SESSION_TRANSFORM_DECRYPT_OUTPUT_ARGUMENTS;

typedef struct D3D12_CRYPTO_SESSION_TRANSFORM_TRANSCRYPT_OUTPUT_ARGUMENTS
    {
    BOOL Enable;
    _Field_size_bytes_full_(IVSize)  void *pIV;
    UINT IVSize;
    }     D3D12_CRYPTO_SESSION_TRANSFORM_TRANSCRYPT_OUTPUT_ARGUMENTS;

typedef struct D3D12_CRYPTO_SESSION_TRANSFORM_DECRYPT_HEADER_OUTPUT_ARGUMENTS
    {
    BOOL Enable;
    UINT64 SliceHeadersSize;
    _Field_size_bytes_full_(SliceHeadersSize)  const void *pSliceHeaders;
    UINT64 ContextSize;
    _Field_size_bytes_full_(ContextSize)  const void *pContext;
    }     D3D12_CRYPTO_SESSION_TRANSFORM_DECRYPT_HEADER_OUTPUT_ARGUMENTS;

typedef struct D3D12_CRYPTO_SESSION_TRANSFORM_OUTPUT_ARGUMENTS
    {
    D3D12_CRYPTO_SESSION_TRANSFORM_DECRYPT_OUTPUT_ARGUMENTS Decrypt;
    D3D12_CRYPTO_SESSION_TRANSFORM_TRANSCRYPT_OUTPUT_ARGUMENTS Transcrypt;
    D3D12_CRYPTO_SESSION_TRANSFORM_DECRYPT_HEADER_OUTPUT_ARGUMENTS ClearHeader;
    }     D3D12_CRYPTO_SESSION_TRANSFORM_OUTPUT_ARGUMENTS;



extern RPC_IF_HANDLE __MIDL_itf_d3d12_1_0000_0002_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_d3d12_1_0000_0002_v0_0_s_ifspec;

#ifndef __ID3D12ContentProtectionDevice_INTERFACE_DEFINED__
#define __ID3D12ContentProtectionDevice_INTERFACE_DEFINED__

/* interface ID3D12ContentProtectionDevice */
/* [unique][local][object][uuid] */ 


EXTERN_C const IID IID_ID3D12ContentProtectionDevice;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("59975f53-bf5f-42f2-b84f-5e347c1e3d43")
    ID3D12ContentProtectionDevice : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateCryptoSession( 
            _In_  const D3D12_CRYPTO_SESSION_DESC *pDesc,
            _In_  REFIID riid,
            _COM_Outptr_  void **ppSession) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateCryptoSessionPolicy( 
            _In_reads_bytes_( KeyInfoSize )  const void *pKeyInfo,
            UINT KeyInfoSize,
            _In_  ID3D12CryptoSession *pCryptoSession,
            _In_  REFIID riid,
            _COM_Outptr_  void **ppCryptoSessionPolicy) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TransformEncryptedData( 
            D3D12_CRYPTO_SESSION_TRANSFORM_OPERATION Operation,
            _In_  const D3D12_CRYPTO_SESSION_TRANSFORM_OUTPUT_ARGUMENTS *pOutputArguments,
            _In_  const D3D12_CRYPTO_SESSION_TRANSFORM_INPUT_ARGUMENTS *pInputArguments) = 0;
        
    };
    
    
#else     /* C style interface */

    typedef struct ID3D12ContentProtectionDeviceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ID3D12ContentProtectionDevice * This,
            REFIID riid,
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ID3D12ContentProtectionDevice * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ID3D12ContentProtectionDevice * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateCryptoSession )( 
            ID3D12ContentProtectionDevice * This,
            _In_  const D3D12_CRYPTO_SESSION_DESC *pDesc,
            _In_  REFIID riid,
            _COM_Outptr_  void **ppSession);
        
        HRESULT ( STDMETHODCALLTYPE *CreateCryptoSessionPolicy )( 
            ID3D12ContentProtectionDevice * This,
            _In_reads_bytes_( KeyInfoSize )  const void *pKeyInfo,
            UINT KeyInfoSize,
            _In_  ID3D12CryptoSession *pCryptoSession,
            _In_  REFIID riid,
            _COM_Outptr_  void **ppCryptoSessionPolicy);
        
        HRESULT ( STDMETHODCALLTYPE *TransformEncryptedData )( 
            ID3D12ContentProtectionDevice * This,
            D3D12_CRYPTO_SESSION_TRANSFORM_OPERATION Operation,
            _In_  const D3D12_CRYPTO_SESSION_TRANSFORM_OUTPUT_ARGUMENTS *pOutputArguments,
            _In_  const D3D12_CRYPTO_SESSION_TRANSFORM_INPUT_ARGUMENTS *pInputArguments);
        
        END_INTERFACE
    } ID3D12ContentProtectionDeviceVtbl;

    interface ID3D12ContentProtectionDevice
    {
        CONST_VTBL struct ID3D12ContentProtectionDeviceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ID3D12ContentProtectionDevice_QueryInterface(This,riid,ppvObject)    \
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ID3D12ContentProtectionDevice_AddRef(This)    \
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ID3D12ContentProtectionDevice_Release(This)    \
    ( (This)->lpVtbl -> Release(This) ) 


#define ID3D12ContentProtectionDevice_CreateCryptoSession(This,pDesc,riid,ppSession)    \
    ( (This)->lpVtbl -> CreateCryptoSession(This,pDesc,riid,ppSession) ) 

#define ID3D12ContentProtectionDevice_CreateCryptoSessionPolicy(This,pKeyInfo,KeyInfoSize,pCryptoSession,riid,ppCryptoSessionPolicy)    \
    ( (This)->lpVtbl -> CreateCryptoSessionPolicy(This,pKeyInfo,KeyInfoSize,pCryptoSession,riid,ppCryptoSessionPolicy) ) 

#define ID3D12ContentProtectionDevice_TransformEncryptedData(This,Operation,pOutputArguments,pInputArguments)    \
    ( (This)->lpVtbl -> TransformEncryptedData(This,Operation,pOutputArguments,pInputArguments) ) 

#endif /* COBJMACROS */


#endif     /* C style interface */




#endif     /* __ID3D12ContentProtectionDevice_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_d3d12_1_0000_0003 */
/* [local] */ 

typedef struct D3D12_VIDEO_DECODE_DECRYPTION_ARGUMENTS
    {
    ID3D12CryptoSessionPolicy *pCryptoSessionPolicy;
    _Field_size_bytes_full_(IVSize)  const void *pIV;
    UINT IVSize;
    _Field_size_bytes_full_(SubSampleMappingCount)  const void *pSubSampleMappingBlock;
    UINT SubSampleMappingCount;
    UINT cBlocksStripeEncrypted;
    UINT cBlocksStripeClear;
    }     D3D12_VIDEO_DECODE_DECRYPTION_ARGUMENTS;

typedef struct D3D12_VIDEO_DECODE_INPUT_STREAM_ARGUMENTS1
    {
    UINT NumFrameArguments;
    D3D12_VIDEO_DECODE_FRAME_ARGUMENT FrameArguments[ 10 ];
    D3D12_VIDEO_DECODE_REFERENCE_FRAMES ReferenceFrames;
    D3D12_VIDEO_DECODE_COMPRESSED_BITSTREAM CompressedBitstream;
    D3D12_VIDEO_DECODE_DECRYPTION_ARGUMENTS DecryptionArguments;
    ID3D12VideoDecoderHeap *pHeap;
    }     D3D12_VIDEO_DECODE_INPUT_STREAM_ARGUMENTS1;

typedef 
enum D3D12_TRACKED_WORKLOAD_FLAGS
    {
        D3D12_TRACKED_WORKLOAD_FLAG_NONE    = 0,
        D3D12_TRACKED_WORKLOAD_FLAG_PERIODIC    = 0x1,
        D3D12_TRACKED_WORKLOAD_FLAG_SIMILAR_LOAD    = 0x2
    }     D3D12_TRACKED_WORKLOAD_FLAGS;

DEFINE_ENUM_FLAG_OPERATORS( D3D12_TRACKED_WORKLOAD_FLAGS );
typedef 
enum D3D12_TRACKED_WORKLOAD_EXECUTION_POLICY
    {
        D3D12_TRACKED_WORKLOAD_EXECUTION_POLICY_NORMAL    = 0,
        D3D12_TRACKED_WORKLOAD_EXECUTION_POLICY_ENERGY_EFFICIENT    = 1,
        D3D12_TRACKED_WORKLOAD_EXECUTION_POLICY_HIGH_SPEED    = 2
    }     D3D12_TRACKED_WORKLOAD_EXECUTION_POLICY;

typedef 
enum D3D12_TRACKED_WORKLOAD_STATE_FLAGS
    {
        D3D12_TRACKED_WORKLOAD_STATE_FLAG_NONE    = 0,
        D3D12_TRACKED_WORKLOAD_STATE_FLAG_SATURATED    = 0x1
    }     D3D12_TRACKED_WORKLOAD_STATE_FLAGS;

typedef 
enum D3D12_TRACKED_WORKLOAD_DEADLINE_TYPE
    {
        D3D12_TRACKED_WORKLOAD_DEADLINE_TYPE_ABSOLUTE    = 0,
        D3D12_TRACKED_WORKLOAD_DEADLINE_TYPE_VBLANK    = 1
    }     D3D12_TRACKED_WORKLOAD_DEADLINE_TYPE;

typedef struct D3D12_TRACKED_WORKLOAD_DEADLINE
    {
    D3D12_TRACKED_WORKLOAD_DEADLINE_TYPE Type;
    union 
        {
        UINT64 VBlankOffsetHundredsNS;
        UINT64 AbsoluteQPC;
        }     ;
    }     D3D12_TRACKED_WORKLOAD_DEADLINE;

typedef struct D3D12_TRACKED_WORKLOAD_DESC
    {
    UINT NodeMask;
    D3D12_TRACKED_WORKLOAD_FLAGS Flags;
    D3D12_TRACKED_WORKLOAD_EXECUTION_POLICY Policy;
    UINT MaxInstances;
    }     D3D12_TRACKED_WORKLOAD_DESC;

typedef struct D3D12_BASE_STATISTICS
    {
    INT64 Mean;
    INT64 Minimum;
    INT64 Maximum;
    INT64 Variance;
    }     D3D12_BASE_STATISTICS;

typedef struct D3D12_TRACKED_WORKLOAD_STATISTICS
    {
    D3D12_BASE_STATISTICS DeadlineOffsetHundredsNS;
    UINT64 MissedDeadlines;
    D3D12_TRACKED_WORKLOAD_STATE_FLAGS Flags;
    }     D3D12_TRACKED_WORKLOAD_STATISTICS;



extern RPC_IF_HANDLE __MIDL_itf_d3d12_1_0000_0003_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_d3d12_1_0000_0003_v0_0_s_ifspec;

#ifndef __ID3D12TrackedWorkload_INTERFACE_DEFINED__
#define __ID3D12TrackedWorkload_INTERFACE_DEFINED__

/* interface ID3D12TrackedWorkload */
/* [unique][local][object][uuid] */ 


EXTERN_C const IID IID_ID3D12TrackedWorkload;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2C983A52-E1A0-468E-BBE8-991D80901F57")
    ID3D12TrackedWorkload : public ID3D12DeviceChild
    {
    public:
        virtual D3D12_TRACKED_WORKLOAD_DESC STDMETHODCALLTYPE GetDesc( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStatistics( 
            _Out_  D3D12_TRACKED_WORKLOAD_STATISTICS *pStats) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
    };
    
    
#else     /* C style interface */

    typedef struct ID3D12TrackedWorkloadVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ID3D12TrackedWorkload * This,
            REFIID riid,
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ID3D12TrackedWorkload * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ID3D12TrackedWorkload * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPrivateData )( 
            ID3D12TrackedWorkload * This,
            _In_  REFGUID guid,
            _Inout_  UINT *pDataSize,
            _Out_writes_bytes_opt_( *pDataSize )  void *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetPrivateData )( 
            ID3D12TrackedWorkload * This,
            _In_  REFGUID guid,
            _In_  UINT DataSize,
            _In_reads_bytes_opt_( DataSize )  const void *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetPrivateDataInterface )( 
            ID3D12TrackedWorkload * This,
            _In_  REFGUID guid,
            _In_opt_  const IUnknown *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetName )( 
            ID3D12TrackedWorkload * This,
            _In_z_  LPCWSTR Name);
        
        HRESULT ( STDMETHODCALLTYPE *GetDevice )( 
            ID3D12TrackedWorkload * This,
            REFIID riid,
            _COM_Outptr_opt_  void **ppvDevice);
        
        D3D12_TRACKED_WORKLOAD_DESC ( STDMETHODCALLTYPE *GetDesc )( 
            ID3D12TrackedWorkload * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetStatistics )( 
            ID3D12TrackedWorkload * This,
            _Out_  D3D12_TRACKED_WORKLOAD_STATISTICS *pStats);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            ID3D12TrackedWorkload * This);
        
        END_INTERFACE
    } ID3D12TrackedWorkloadVtbl;

    interface ID3D12TrackedWorkload
    {
        CONST_VTBL struct ID3D12TrackedWorkloadVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ID3D12TrackedWorkload_QueryInterface(This,riid,ppvObject)    \
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ID3D12TrackedWorkload_AddRef(This)    \
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ID3D12TrackedWorkload_Release(This)    \
    ( (This)->lpVtbl -> Release(This) ) 


#define ID3D12TrackedWorkload_GetPrivateData(This,guid,pDataSize,pData)    \
    ( (This)->lpVtbl -> GetPrivateData(This,guid,pDataSize,pData) ) 

#define ID3D12TrackedWorkload_SetPrivateData(This,guid,DataSize,pData)    \
    ( (This)->lpVtbl -> SetPrivateData(This,guid,DataSize,pData) ) 

#define ID3D12TrackedWorkload_SetPrivateDataInterface(This,guid,pData)    \
    ( (This)->lpVtbl -> SetPrivateDataInterface(This,guid,pData) ) 

#define ID3D12TrackedWorkload_SetName(This,Name)    \
    ( (This)->lpVtbl -> SetName(This,Name) ) 


#define ID3D12TrackedWorkload_GetDevice(This,riid,ppvDevice)    \
    ( (This)->lpVtbl -> GetDevice(This,riid,ppvDevice) ) 


#define ID3D12TrackedWorkload_GetDesc(This)    \
    ( (This)->lpVtbl -> GetDesc(This) ) 

#define ID3D12TrackedWorkload_GetStatistics(This,pStats)    \
    ( (This)->lpVtbl -> GetStatistics(This,pStats) ) 

#define ID3D12TrackedWorkload_Reset(This)    \
    ( (This)->lpVtbl -> Reset(This) ) 

#endif /* COBJMACROS */


#endif     /* C style interface */




#endif     /* __ID3D12TrackedWorkload_INTERFACE_DEFINED__ */


#ifndef __ID3D12VideoDecodeCommandList2_INTERFACE_DEFINED__
#define __ID3D12VideoDecodeCommandList2_INTERFACE_DEFINED__

/* interface ID3D12VideoDecodeCommandList2 */
/* [unique][local][object][uuid] */ 


EXTERN_C const IID IID_ID3D12VideoDecodeCommandList2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("589A9607-DB1A-4573-9204-A2D9A25B93BD")
    ID3D12VideoDecodeCommandList2 : public ID3D12VideoDecodeCommandList1
    {
    public:
        virtual void STDMETHODCALLTYPE DecodeFrame2( 
            _In_  ID3D12VideoDecoder *pDecoder,
            _In_  const D3D12_VIDEO_DECODE_OUTPUT_STREAM_ARGUMENTS1 *pOutputArguments,
            _In_  const D3D12_VIDEO_DECODE_INPUT_STREAM_ARGUMENTS1 *pInputArguments) = 0;
        
        virtual void STDMETHODCALLTYPE SetProtectedResourceSession( 
            _In_opt_  ID3D12ProtectedResourceSession *pProtectedResourceSession) = 0;
        
        virtual void STDMETHODCALLTYPE BeginTrackedWorkload( 
            _In_  ID3D12TrackedWorkload *pTrackedWorkload) = 0;
        
        virtual void STDMETHODCALLTYPE EndTrackedWorkload( 
            _In_  ID3D12TrackedWorkload *pTrackedWorkload,
            D3D12_TRACKED_WORKLOAD_DEADLINE *pFinishDeadline) = 0;
        
    };
    
    
#else     /* C style interface */

    typedef struct ID3D12VideoDecodeCommandList2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ID3D12VideoDecodeCommandList2 * This,
            REFIID riid,
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ID3D12VideoDecodeCommandList2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ID3D12VideoDecodeCommandList2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPrivateData )( 
            ID3D12VideoDecodeCommandList2 * This,
            _In_  REFGUID guid,
            _Inout_  UINT *pDataSize,
            _Out_writes_bytes_opt_( *pDataSize )  void *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetPrivateData )( 
            ID3D12VideoDecodeCommandList2 * This,
            _In_  REFGUID guid,
            _In_  UINT DataSize,
            _In_reads_bytes_opt_( DataSize )  const void *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetPrivateDataInterface )( 
            ID3D12VideoDecodeCommandList2 * This,
            _In_  REFGUID guid,
            _In_opt_  const IUnknown *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetName )( 
            ID3D12VideoDecodeCommandList2 * This,
            _In_z_  LPCWSTR Name);
        
        HRESULT ( STDMETHODCALLTYPE *GetDevice )( 
            ID3D12VideoDecodeCommandList2 * This,
            REFIID riid,
            _COM_Outptr_opt_  void **ppvDevice);
        
        D3D12_COMMAND_LIST_TYPE ( STDMETHODCALLTYPE *GetType )( 
            ID3D12VideoDecodeCommandList2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *Close )( 
            ID3D12VideoDecodeCommandList2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            ID3D12VideoDecodeCommandList2 * This,
            _In_  ID3D12CommandAllocator *pAllocator);
        
        void ( STDMETHODCALLTYPE *ClearState )( 
            ID3D12VideoDecodeCommandList2 * This);
        
        void ( STDMETHODCALLTYPE *ResourceBarrier )( 
            ID3D12VideoDecodeCommandList2 * This,
            _In_  UINT NumBarriers,
            _In_reads_(NumBarriers)  const D3D12_RESOURCE_BARRIER *pBarriers);
        
        void ( STDMETHODCALLTYPE *DiscardResource )( 
            ID3D12VideoDecodeCommandList2 * This,
            _In_  ID3D12Resource *pResource,
            _In_opt_  const D3D12_DISCARD_REGION *pRegion);
        
        void ( STDMETHODCALLTYPE *BeginQuery )( 
            ID3D12VideoDecodeCommandList2 * This,
            _In_  ID3D12QueryHeap *pQueryHeap,
            _In_  D3D12_QUERY_TYPE Type,
            _In_  UINT Index);
        
        void ( STDMETHODCALLTYPE *EndQuery )( 
            ID3D12VideoDecodeCommandList2 * This,
            _In_  ID3D12QueryHeap *pQueryHeap,
            _In_  D3D12_QUERY_TYPE Type,
            _In_  UINT Index);
        
        void ( STDMETHODCALLTYPE *ResolveQueryData )( 
            ID3D12VideoDecodeCommandList2 * This,
            _In_  ID3D12QueryHeap *pQueryHeap,
            _In_  D3D12_QUERY_TYPE Type,
            _In_  UINT StartIndex,
            _In_  UINT NumQueries,
            _In_  ID3D12Resource *pDestinationBuffer,
            _In_  UINT64 AlignedDestinationBufferOffset);
        
        void ( STDMETHODCALLTYPE *SetPredication )( 
            ID3D12VideoDecodeCommandList2 * This,
            _In_opt_  ID3D12Resource *pBuffer,
            _In_  UINT64 AlignedBufferOffset,
            _In_  D3D12_PREDICATION_OP Operation);
        
        void ( STDMETHODCALLTYPE *SetMarker )( 
            ID3D12VideoDecodeCommandList2 * This,
            UINT Metadata,
            _In_reads_bytes_opt_(Size)  const void *pData,
            UINT Size);
        
        void ( STDMETHODCALLTYPE *BeginEvent )( 
            ID3D12VideoDecodeCommandList2 * This,
            UINT Metadata,
            _In_reads_bytes_opt_(Size)  const void *pData,
            UINT Size);
        
        void ( STDMETHODCALLTYPE *EndEvent )( 
            ID3D12VideoDecodeCommandList2 * This);
        
        void ( STDMETHODCALLTYPE *DecodeFrame )( 
            ID3D12VideoDecodeCommandList2 * This,
            _In_  ID3D12VideoDecoder *pDecoder,
            _In_  const D3D12_VIDEO_DECODE_OUTPUT_STREAM_ARGUMENTS *pOutputArguments,
            _In_  const D3D12_VIDEO_DECODE_INPUT_STREAM_ARGUMENTS *pInputArguments);
        
        void ( STDMETHODCALLTYPE *WriteBufferImmediate )( 
            ID3D12VideoDecodeCommandList2 * This,
            UINT Count,
            _In_reads_(Count)  const D3D12_WRITEBUFFERIMMEDIATE_PARAMETER *pParams,
            _In_reads_opt_(Count)  const D3D12_WRITEBUFFERIMMEDIATE_MODE *pModes);
        
        void ( STDMETHODCALLTYPE *DecodeFrame1 )( 
            ID3D12VideoDecodeCommandList2 * This,
            _In_  ID3D12VideoDecoder *pDecoder,
            _In_  const D3D12_VIDEO_DECODE_OUTPUT_STREAM_ARGUMENTS1 *pOutputArguments,
            _In_  const D3D12_VIDEO_DECODE_INPUT_STREAM_ARGUMENTS *pInputArguments);
        
        void ( STDMETHODCALLTYPE *DecodeFrame2 )( 
            ID3D12VideoDecodeCommandList2 * This,
            _In_  ID3D12VideoDecoder *pDecoder,
            _In_  const D3D12_VIDEO_DECODE_OUTPUT_STREAM_ARGUMENTS1 *pOutputArguments,
            _In_  const D3D12_VIDEO_DECODE_INPUT_STREAM_ARGUMENTS1 *pInputArguments);
        
        void ( STDMETHODCALLTYPE *SetProtectedResourceSession )( 
            ID3D12VideoDecodeCommandList2 * This,
            _In_opt_  ID3D12ProtectedResourceSession *pProtectedResourceSession);
        
        void ( STDMETHODCALLTYPE *BeginTrackedWorkload )( 
            ID3D12VideoDecodeCommandList2 * This,
            _In_  ID3D12TrackedWorkload *pTrackedWorkload);
        
        void ( STDMETHODCALLTYPE *EndTrackedWorkload )( 
            ID3D12VideoDecodeCommandList2 * This,
            _In_  ID3D12TrackedWorkload *pTrackedWorkload,
            D3D12_TRACKED_WORKLOAD_DEADLINE *pFinishDeadline);
        
        END_INTERFACE
    } ID3D12VideoDecodeCommandList2Vtbl;

    interface ID3D12VideoDecodeCommandList2
    {
        CONST_VTBL struct ID3D12VideoDecodeCommandList2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ID3D12VideoDecodeCommandList2_QueryInterface(This,riid,ppvObject)    \
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ID3D12VideoDecodeCommandList2_AddRef(This)    \
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ID3D12VideoDecodeCommandList2_Release(This)    \
    ( (This)->lpVtbl -> Release(This) ) 


#define ID3D12VideoDecodeCommandList2_GetPrivateData(This,guid,pDataSize,pData)    \
    ( (This)->lpVtbl -> GetPrivateData(This,guid,pDataSize,pData) ) 

#define ID3D12VideoDecodeCommandList2_SetPrivateData(This,guid,DataSize,pData)    \
    ( (This)->lpVtbl -> SetPrivateData(This,guid,DataSize,pData) ) 

#define ID3D12VideoDecodeCommandList2_SetPrivateDataInterface(This,guid,pData)    \
    ( (This)->lpVtbl -> SetPrivateDataInterface(This,guid,pData) ) 

#define ID3D12VideoDecodeCommandList2_SetName(This,Name)    \
    ( (This)->lpVtbl -> SetName(This,Name) ) 


#define ID3D12VideoDecodeCommandList2_GetDevice(This,riid,ppvDevice)    \
    ( (This)->lpVtbl -> GetDevice(This,riid,ppvDevice) ) 


#define ID3D12VideoDecodeCommandList2_GetType(This)    \
    ( (This)->lpVtbl -> GetType(This) ) 


#define ID3D12VideoDecodeCommandList2_Close(This)    \
    ( (This)->lpVtbl -> Close(This) ) 

#define ID3D12VideoDecodeCommandList2_Reset(This,pAllocator)    \
    ( (This)->lpVtbl -> Reset(This,pAllocator) ) 

#define ID3D12VideoDecodeCommandList2_ClearState(This)    \
    ( (This)->lpVtbl -> ClearState(This) ) 

#define ID3D12VideoDecodeCommandList2_ResourceBarrier(This,NumBarriers,pBarriers)    \
    ( (This)->lpVtbl -> ResourceBarrier(This,NumBarriers,pBarriers) ) 

#define ID3D12VideoDecodeCommandList2_DiscardResource(This,pResource,pRegion)    \
    ( (This)->lpVtbl -> DiscardResource(This,pResource,pRegion) ) 

#define ID3D12VideoDecodeCommandList2_BeginQuery(This,pQueryHeap,Type,Index)    \
    ( (This)->lpVtbl -> BeginQuery(This,pQueryHeap,Type,Index) ) 

#define ID3D12VideoDecodeCommandList2_EndQuery(This,pQueryHeap,Type,Index)    \
    ( (This)->lpVtbl -> EndQuery(This,pQueryHeap,Type,Index) ) 

#define ID3D12VideoDecodeCommandList2_ResolveQueryData(This,pQueryHeap,Type,StartIndex,NumQueries,pDestinationBuffer,AlignedDestinationBufferOffset)    \
    ( (This)->lpVtbl -> ResolveQueryData(This,pQueryHeap,Type,StartIndex,NumQueries,pDestinationBuffer,AlignedDestinationBufferOffset) ) 

#define ID3D12VideoDecodeCommandList2_SetPredication(This,pBuffer,AlignedBufferOffset,Operation)    \
    ( (This)->lpVtbl -> SetPredication(This,pBuffer,AlignedBufferOffset,Operation) ) 

#define ID3D12VideoDecodeCommandList2_SetMarker(This,Metadata,pData,Size)    \
    ( (This)->lpVtbl -> SetMarker(This,Metadata,pData,Size) ) 

#define ID3D12VideoDecodeCommandList2_BeginEvent(This,Metadata,pData,Size)    \
    ( (This)->lpVtbl -> BeginEvent(This,Metadata,pData,Size) ) 

#define ID3D12VideoDecodeCommandList2_EndEvent(This)    \
    ( (This)->lpVtbl -> EndEvent(This) ) 

#define ID3D12VideoDecodeCommandList2_DecodeFrame(This,pDecoder,pOutputArguments,pInputArguments)    \
    ( (This)->lpVtbl -> DecodeFrame(This,pDecoder,pOutputArguments,pInputArguments) ) 

#define ID3D12VideoDecodeCommandList2_WriteBufferImmediate(This,Count,pParams,pModes)    \
    ( (This)->lpVtbl -> WriteBufferImmediate(This,Count,pParams,pModes) ) 


#define ID3D12VideoDecodeCommandList2_DecodeFrame1(This,pDecoder,pOutputArguments,pInputArguments)    \
    ( (This)->lpVtbl -> DecodeFrame1(This,pDecoder,pOutputArguments,pInputArguments) ) 


#define ID3D12VideoDecodeCommandList2_DecodeFrame2(This,pDecoder,pOutputArguments,pInputArguments)    \
    ( (This)->lpVtbl -> DecodeFrame2(This,pDecoder,pOutputArguments,pInputArguments) ) 

#define ID3D12VideoDecodeCommandList2_SetProtectedResourceSession(This,pProtectedResourceSession)    \
    ( (This)->lpVtbl -> SetProtectedResourceSession(This,pProtectedResourceSession) ) 

#define ID3D12VideoDecodeCommandList2_BeginTrackedWorkload(This,pTrackedWorkload)    \
    ( (This)->lpVtbl -> BeginTrackedWorkload(This,pTrackedWorkload) ) 

#define ID3D12VideoDecodeCommandList2_EndTrackedWorkload(This,pTrackedWorkload,pFinishDeadline)    \
    ( (This)->lpVtbl -> EndTrackedWorkload(This,pTrackedWorkload,pFinishDeadline) ) 

#endif /* COBJMACROS */


#endif     /* C style interface */




#endif     /* __ID3D12VideoDecodeCommandList2_INTERFACE_DEFINED__ */


#ifndef __ID3D12VideoProcessCommandList2_INTERFACE_DEFINED__
#define __ID3D12VideoProcessCommandList2_INTERFACE_DEFINED__

/* interface ID3D12VideoProcessCommandList2 */
/* [unique][local][object][uuid] */ 


EXTERN_C const IID IID_ID3D12VideoProcessCommandList2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6B69BFF8-E8C8-4F8C-B2B4-6E2FC2701274")
    ID3D12VideoProcessCommandList2 : public ID3D12VideoProcessCommandList1
    {
    public:
        virtual void STDMETHODCALLTYPE SetProtectedResourceSession( 
            _In_opt_  ID3D12ProtectedResourceSession *pProtectedResourceSession) = 0;
        
        virtual void STDMETHODCALLTYPE BeginTrackedWorkload( 
            _In_  ID3D12TrackedWorkload *pTrackedWorkload) = 0;
        
        virtual void STDMETHODCALLTYPE EndTrackedWorkload( 
            _In_  ID3D12TrackedWorkload *pTrackedWorkload,
            D3D12_TRACKED_WORKLOAD_DEADLINE *pFinishDeadline) = 0;
        
    };
    
    
#else     /* C style interface */

    typedef struct ID3D12VideoProcessCommandList2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ID3D12VideoProcessCommandList2 * This,
            REFIID riid,
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ID3D12VideoProcessCommandList2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ID3D12VideoProcessCommandList2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPrivateData )( 
            ID3D12VideoProcessCommandList2 * This,
            _In_  REFGUID guid,
            _Inout_  UINT *pDataSize,
            _Out_writes_bytes_opt_( *pDataSize )  void *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetPrivateData )( 
            ID3D12VideoProcessCommandList2 * This,
            _In_  REFGUID guid,
            _In_  UINT DataSize,
            _In_reads_bytes_opt_( DataSize )  const void *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetPrivateDataInterface )( 
            ID3D12VideoProcessCommandList2 * This,
            _In_  REFGUID guid,
            _In_opt_  const IUnknown *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetName )( 
            ID3D12VideoProcessCommandList2 * This,
            _In_z_  LPCWSTR Name);
        
        HRESULT ( STDMETHODCALLTYPE *GetDevice )( 
            ID3D12VideoProcessCommandList2 * This,
            REFIID riid,
            _COM_Outptr_opt_  void **ppvDevice);
        
        D3D12_COMMAND_LIST_TYPE ( STDMETHODCALLTYPE *GetType )( 
            ID3D12VideoProcessCommandList2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *Close )( 
            ID3D12VideoProcessCommandList2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            ID3D12VideoProcessCommandList2 * This,
            _In_  ID3D12CommandAllocator *pAllocator);
        
        void ( STDMETHODCALLTYPE *ClearState )( 
            ID3D12VideoProcessCommandList2 * This);
        
        void ( STDMETHODCALLTYPE *ResourceBarrier )( 
            ID3D12VideoProcessCommandList2 * This,
            _In_  UINT NumBarriers,
            _In_reads_(NumBarriers)  const D3D12_RESOURCE_BARRIER *pBarriers);
        
        void ( STDMETHODCALLTYPE *DiscardResource )( 
            ID3D12VideoProcessCommandList2 * This,
            _In_  ID3D12Resource *pResource,
            _In_opt_  const D3D12_DISCARD_REGION *pRegion);
        
        void ( STDMETHODCALLTYPE *BeginQuery )( 
            ID3D12VideoProcessCommandList2 * This,
            _In_  ID3D12QueryHeap *pQueryHeap,
            _In_  D3D12_QUERY_TYPE Type,
            _In_  UINT Index);
        
        void ( STDMETHODCALLTYPE *EndQuery )( 
            ID3D12VideoProcessCommandList2 * This,
            _In_  ID3D12QueryHeap *pQueryHeap,
            _In_  D3D12_QUERY_TYPE Type,
            _In_  UINT Index);
        
        void ( STDMETHODCALLTYPE *ResolveQueryData )( 
            ID3D12VideoProcessCommandList2 * This,
            _In_  ID3D12QueryHeap *pQueryHeap,
            _In_  D3D12_QUERY_TYPE Type,
            _In_  UINT StartIndex,
            _In_  UINT NumQueries,
            _In_  ID3D12Resource *pDestinationBuffer,
            _In_  UINT64 AlignedDestinationBufferOffset);
        
        void ( STDMETHODCALLTYPE *SetPredication )( 
            ID3D12VideoProcessCommandList2 * This,
            _In_opt_  ID3D12Resource *pBuffer,
            _In_  UINT64 AlignedBufferOffset,
            _In_  D3D12_PREDICATION_OP Operation);
        
        void ( STDMETHODCALLTYPE *SetMarker )( 
            ID3D12VideoProcessCommandList2 * This,
            UINT Metadata,
            _In_reads_bytes_opt_(Size)  const void *pData,
            UINT Size);
        
        void ( STDMETHODCALLTYPE *BeginEvent )( 
            ID3D12VideoProcessCommandList2 * This,
            UINT Metadata,
            _In_reads_bytes_opt_(Size)  const void *pData,
            UINT Size);
        
        void ( STDMETHODCALLTYPE *EndEvent )( 
            ID3D12VideoProcessCommandList2 * This);
        
        void ( STDMETHODCALLTYPE *ProcessFrames )( 
            ID3D12VideoProcessCommandList2 * This,
            _In_  ID3D12VideoProcessor *pVideoProcessor,
            _In_  const D3D12_VIDEO_PROCESS_OUTPUT_STREAM_ARGUMENTS *pOutputArguments,
            UINT NumInputStreams,
            _In_reads_(NumInputStreams)  const D3D12_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS *pInputArguments);
        
        void ( STDMETHODCALLTYPE *WriteBufferImmediate )( 
            ID3D12VideoProcessCommandList2 * This,
            UINT Count,
            _In_reads_(Count)  const D3D12_WRITEBUFFERIMMEDIATE_PARAMETER *pParams,
            _In_reads_opt_(Count)  const D3D12_WRITEBUFFERIMMEDIATE_MODE *pModes);
        
        void ( STDMETHODCALLTYPE *ProcessFrames1 )( 
            ID3D12VideoProcessCommandList2 * This,
            _In_  ID3D12VideoProcessor *pVideoProcessor,
            _In_  const D3D12_VIDEO_PROCESS_OUTPUT_STREAM_ARGUMENTS *pOutputArguments,
            UINT NumInputStreams,
            _In_reads_(NumInputStreams)  const D3D12_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS1 *pInputArguments);
        
        void ( STDMETHODCALLTYPE *SetProtectedResourceSession )( 
            ID3D12VideoProcessCommandList2 * This,
            _In_opt_  ID3D12ProtectedResourceSession *pProtectedResourceSession);
        
        void ( STDMETHODCALLTYPE *BeginTrackedWorkload )( 
            ID3D12VideoProcessCommandList2 * This,
            _In_  ID3D12TrackedWorkload *pTrackedWorkload);
        
        void ( STDMETHODCALLTYPE *EndTrackedWorkload )( 
            ID3D12VideoProcessCommandList2 * This,
            _In_  ID3D12TrackedWorkload *pTrackedWorkload,
            D3D12_TRACKED_WORKLOAD_DEADLINE *pFinishDeadline);
        
        END_INTERFACE
    } ID3D12VideoProcessCommandList2Vtbl;

    interface ID3D12VideoProcessCommandList2
    {
        CONST_VTBL struct ID3D12VideoProcessCommandList2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ID3D12VideoProcessCommandList2_QueryInterface(This,riid,ppvObject)    \
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ID3D12VideoProcessCommandList2_AddRef(This)    \
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ID3D12VideoProcessCommandList2_Release(This)    \
    ( (This)->lpVtbl -> Release(This) ) 


#define ID3D12VideoProcessCommandList2_GetPrivateData(This,guid,pDataSize,pData)    \
    ( (This)->lpVtbl -> GetPrivateData(This,guid,pDataSize,pData) ) 

#define ID3D12VideoProcessCommandList2_SetPrivateData(This,guid,DataSize,pData)    \
    ( (This)->lpVtbl -> SetPrivateData(This,guid,DataSize,pData) ) 

#define ID3D12VideoProcessCommandList2_SetPrivateDataInterface(This,guid,pData)    \
    ( (This)->lpVtbl -> SetPrivateDataInterface(This,guid,pData) ) 

#define ID3D12VideoProcessCommandList2_SetName(This,Name)    \
    ( (This)->lpVtbl -> SetName(This,Name) ) 


#define ID3D12VideoProcessCommandList2_GetDevice(This,riid,ppvDevice)    \
    ( (This)->lpVtbl -> GetDevice(This,riid,ppvDevice) ) 


#define ID3D12VideoProcessCommandList2_GetType(This)    \
    ( (This)->lpVtbl -> GetType(This) ) 


#define ID3D12VideoProcessCommandList2_Close(This)    \
    ( (This)->lpVtbl -> Close(This) ) 

#define ID3D12VideoProcessCommandList2_Reset(This,pAllocator)    \
    ( (This)->lpVtbl -> Reset(This,pAllocator) ) 

#define ID3D12VideoProcessCommandList2_ClearState(This)    \
    ( (This)->lpVtbl -> ClearState(This) ) 

#define ID3D12VideoProcessCommandList2_ResourceBarrier(This,NumBarriers,pBarriers)    \
    ( (This)->lpVtbl -> ResourceBarrier(This,NumBarriers,pBarriers) ) 

#define ID3D12VideoProcessCommandList2_DiscardResource(This,pResource,pRegion)    \
    ( (This)->lpVtbl -> DiscardResource(This,pResource,pRegion) ) 

#define ID3D12VideoProcessCommandList2_BeginQuery(This,pQueryHeap,Type,Index)    \
    ( (This)->lpVtbl -> BeginQuery(This,pQueryHeap,Type,Index) ) 

#define ID3D12VideoProcessCommandList2_EndQuery(This,pQueryHeap,Type,Index)    \
    ( (This)->lpVtbl -> EndQuery(This,pQueryHeap,Type,Index) ) 

#define ID3D12VideoProcessCommandList2_ResolveQueryData(This,pQueryHeap,Type,StartIndex,NumQueries,pDestinationBuffer,AlignedDestinationBufferOffset)    \
    ( (This)->lpVtbl -> ResolveQueryData(This,pQueryHeap,Type,StartIndex,NumQueries,pDestinationBuffer,AlignedDestinationBufferOffset) ) 

#define ID3D12VideoProcessCommandList2_SetPredication(This,pBuffer,AlignedBufferOffset,Operation)    \
    ( (This)->lpVtbl -> SetPredication(This,pBuffer,AlignedBufferOffset,Operation) ) 

#define ID3D12VideoProcessCommandList2_SetMarker(This,Metadata,pData,Size)    \
    ( (This)->lpVtbl -> SetMarker(This,Metadata,pData,Size) ) 

#define ID3D12VideoProcessCommandList2_BeginEvent(This,Metadata,pData,Size)    \
    ( (This)->lpVtbl -> BeginEvent(This,Metadata,pData,Size) ) 

#define ID3D12VideoProcessCommandList2_EndEvent(This)    \
    ( (This)->lpVtbl -> EndEvent(This) ) 

#define ID3D12VideoProcessCommandList2_ProcessFrames(This,pVideoProcessor,pOutputArguments,NumInputStreams,pInputArguments)    \
    ( (This)->lpVtbl -> ProcessFrames(This,pVideoProcessor,pOutputArguments,NumInputStreams,pInputArguments) ) 

#define ID3D12VideoProcessCommandList2_WriteBufferImmediate(This,Count,pParams,pModes)    \
    ( (This)->lpVtbl -> WriteBufferImmediate(This,Count,pParams,pModes) ) 


#define ID3D12VideoProcessCommandList2_ProcessFrames1(This,pVideoProcessor,pOutputArguments,NumInputStreams,pInputArguments)    \
    ( (This)->lpVtbl -> ProcessFrames1(This,pVideoProcessor,pOutputArguments,NumInputStreams,pInputArguments) ) 


#define ID3D12VideoProcessCommandList2_SetProtectedResourceSession(This,pProtectedResourceSession)    \
    ( (This)->lpVtbl -> SetProtectedResourceSession(This,pProtectedResourceSession) ) 

#define ID3D12VideoProcessCommandList2_BeginTrackedWorkload(This,pTrackedWorkload)    \
    ( (This)->lpVtbl -> BeginTrackedWorkload(This,pTrackedWorkload) ) 

#define ID3D12VideoProcessCommandList2_EndTrackedWorkload(This,pTrackedWorkload,pFinishDeadline)    \
    ( (This)->lpVtbl -> EndTrackedWorkload(This,pTrackedWorkload,pFinishDeadline) ) 

#endif /* COBJMACROS */


#endif     /* C style interface */




#endif     /* __ID3D12VideoProcessCommandList2_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_d3d12_1_0000_0006 */
/* [local] */ 

typedef 
enum D3D12_VIDEO_MOTION_ESTIMATOR_SEARCH_BLOCK_SIZE
    {
        D3D12_VIDEO_MOTION_ESTIMATOR_SEARCH_BLOCK_SIZE_8X8    = 0,
        D3D12_VIDEO_MOTION_ESTIMATOR_SEARCH_BLOCK_SIZE_16X16    = 1
    }     D3D12_VIDEO_MOTION_ESTIMATOR_SEARCH_BLOCK_SIZE;

typedef 
enum D3D12_VIDEO_MOTION_ESTIMATOR_SEARCH_BLOCK_SIZE_FLAGS
    {
        D3D12_VIDEO_MOTION_ESTIMATOR_SEARCH_BLOCK_SIZE_FLAG_NONE    = 0,
        D3D12_VIDEO_MOTION_ESTIMATOR_SEARCH_BLOCK_SIZE_FLAG_8X8    = ( 1 << D3D12_VIDEO_MOTION_ESTIMATOR_SEARCH_BLOCK_SIZE_8X8 ) ,
        D3D12_VIDEO_MOTION_ESTIMATOR_SEARCH_BLOCK_SIZE_FLAG_16X16    = ( 1 << D3D12_VIDEO_MOTION_ESTIMATOR_SEARCH_BLOCK_SIZE_16X16 ) 
    }     D3D12_VIDEO_MOTION_ESTIMATOR_SEARCH_BLOCK_SIZE_FLAGS;

DEFINE_ENUM_FLAG_OPERATORS( D3D12_VIDEO_MOTION_ESTIMATOR_SEARCH_BLOCK_SIZE_FLAGS );
typedef 
enum D3D12_VIDEO_MOTION_ESTIMATOR_VECTOR_PRECISION
    {
        D3D12_VIDEO_MOTION_ESTIMATOR_VECTOR_PRECISION_QUARTER_PEL    = 0
    }     D3D12_VIDEO_MOTION_ESTIMATOR_VECTOR_PRECISION;

typedef 
enum D3D12_VIDEO_MOTION_ESTIMATOR_VECTOR_PRECISION_FLAGS
    {
        D3D12_VIDEO_MOTION_ESTIMATOR_VECTOR_PRECISION_FLAG_NONE    = 0,
        D3D12_VIDEO_MOTION_ESTIMATOR_VECTOR_PRECISION_FLAG_QUARTER_PEL    = ( 1 << D3D12_VIDEO_MOTION_ESTIMATOR_VECTOR_PRECISION_QUARTER_PEL ) 
    }     D3D12_VIDEO_MOTION_ESTIMATOR_VECTOR_PRECISION_FLAGS;

DEFINE_ENUM_FLAG_OPERATORS( D3D12_VIDEO_MOTION_ESTIMATOR_VECTOR_PRECISION_FLAGS );
typedef 
enum D3D12_VIDEO_MOTION_ESTIMATOR_DIFFERENCES_METRIC
    {
        D3D12_VIDEO_MOTION_ESTIMATOR_DIFFERENCES_METRIC_NONE    = 0,
        D3D12_VIDEO_MOTION_ESTIMATOR_DIFFERENCES_METRIC_SUM_OF_ABSOLUTE_TRANSFORMED_DIFFERENCES    = 1
    }     D3D12_VIDEO_MOTION_ESTIMATOR_DIFFERENCES_METRIC;

typedef 
enum D3D12_VIDEO_MOTION_ESTIMATOR_DIFFERENCES_METRIC_FLAGS
    {
        D3D12_VIDEO_MOTION_ESTIMATOR_DIFFERENCES_METRIC_FLAG_NONE    = 0,
        D3D12_VIDEO_MOTION_ESTIMATOR_DIFFERENCES_METRIC_FLAG_SUM_OF_ABSOLUTE_TRANSFORMED_DIFFERENCES    = ( 1 << D3D12_VIDEO_MOTION_ESTIMATOR_DIFFERENCES_METRIC_SUM_OF_ABSOLUTE_TRANSFORMED_DIFFERENCES ) 
    }     D3D12_VIDEO_MOTION_ESTIMATOR_DIFFERENCES_METRIC_FLAGS;

DEFINE_ENUM_FLAG_OPERATORS( D3D12_VIDEO_MOTION_ESTIMATOR_DIFFERENCES_METRIC_FLAGS );
typedef struct D3D12_FEATURE_DATA_VIDEO_FEATURE_AREA_SUPPORT
    {
    UINT NodeIndex;
    BOOL VideoDecodeSupport;
    BOOL VideoProcessSupport;
    BOOL VideoEncodeSupport;
    }     D3D12_FEATURE_DATA_VIDEO_FEATURE_AREA_SUPPORT;

typedef struct D3D12_FEATURE_DATA_VIDEO_MOTION_ESTIMATOR
    {
    UINT NodeIndex;
    DXGI_FORMAT InputFormat;
    D3D12_VIDEO_MOTION_ESTIMATOR_SEARCH_BLOCK_SIZE_FLAGS BlockSizeFlags;
    D3D12_VIDEO_MOTION_ESTIMATOR_VECTOR_PRECISION_FLAGS PrecisionFlags;
    D3D12_VIDEO_MOTION_ESTIMATOR_DIFFERENCES_METRIC_FLAGS DifferencesMetricFlags;
    D3D12_VIDEO_SIZE_RANGE SizeRange;
    }     D3D12_FEATURE_DATA_VIDEO_MOTION_ESTIMATOR;

typedef struct D3D12_FEATURE_DATA_VIDEO_MOTION_ESTIMATOR_SIZE
    {
    UINT NodeIndex;
    DXGI_FORMAT InputFormat;
    D3D12_VIDEO_MOTION_ESTIMATOR_SEARCH_BLOCK_SIZE BlockSize;
    D3D12_VIDEO_MOTION_ESTIMATOR_VECTOR_PRECISION Precision;
    D3D12_VIDEO_MOTION_ESTIMATOR_DIFFERENCES_METRIC DifferencesMetric;
    D3D12_VIDEO_SIZE_RANGE SizeRange;
    BOOL Protected;
    UINT64 MotionVectorHeapMemoryPoolL0Size;
    UINT64 MotionVectorHeapMemoryPoolL1Size;
    UINT64 MotionEstimatorMemoryPoolL0Size;
    UINT64 MotionEstimatorMemoryPoolL1Size;
    }     D3D12_FEATURE_DATA_VIDEO_MOTION_ESTIMATOR_SIZE;

typedef struct D3D12_VIDEO_MOTION_ESTIMATOR_DESC
    {
    UINT NodeMask;
    DXGI_FORMAT InputFormat;
    D3D12_VIDEO_MOTION_ESTIMATOR_SEARCH_BLOCK_SIZE BlockSize;
    D3D12_VIDEO_MOTION_ESTIMATOR_VECTOR_PRECISION Precision;
    D3D12_VIDEO_MOTION_ESTIMATOR_DIFFERENCES_METRIC DifferencesMetric;
    D3D12_VIDEO_SIZE_RANGE SizeRange;
    }     D3D12_VIDEO_MOTION_ESTIMATOR_DESC;



extern RPC_IF_HANDLE __MIDL_itf_d3d12_1_0000_0006_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_d3d12_1_0000_0006_v0_0_s_ifspec;

#ifndef __ID3D12VideoMotionEstimator_INTERFACE_DEFINED__
#define __ID3D12VideoMotionEstimator_INTERFACE_DEFINED__

/* interface ID3D12VideoMotionEstimator */
/* [unique][local][object][uuid] */ 


EXTERN_C const IID IID_ID3D12VideoMotionEstimator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("33FDAE0E-098B-428F-87BB-34B695DE08F8")
    ID3D12VideoMotionEstimator : public ID3D12Pageable
    {
    public:
        virtual D3D12_VIDEO_MOTION_ESTIMATOR_DESC STDMETHODCALLTYPE GetDesc( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProtectedResourceSession( 
            REFIID riid,
            _COM_Outptr_opt_  void **ppProtectedSession) = 0;
        
    };
    
    
#else     /* C style interface */

    typedef struct ID3D12VideoMotionEstimatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ID3D12VideoMotionEstimator * This,
            REFIID riid,
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ID3D12VideoMotionEstimator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ID3D12VideoMotionEstimator * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPrivateData )( 
            ID3D12VideoMotionEstimator * This,
            _In_  REFGUID guid,
            _Inout_  UINT *pDataSize,
            _Out_writes_bytes_opt_( *pDataSize )  void *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetPrivateData )( 
            ID3D12VideoMotionEstimator * This,
            _In_  REFGUID guid,
            _In_  UINT DataSize,
            _In_reads_bytes_opt_( DataSize )  const void *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetPrivateDataInterface )( 
            ID3D12VideoMotionEstimator * This,
            _In_  REFGUID guid,
            _In_opt_  const IUnknown *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetName )( 
            ID3D12VideoMotionEstimator * This,
            _In_z_  LPCWSTR Name);
        
        HRESULT ( STDMETHODCALLTYPE *GetDevice )( 
            ID3D12VideoMotionEstimator * This,
            REFIID riid,
            _COM_Outptr_opt_  void **ppvDevice);
        
        D3D12_VIDEO_MOTION_ESTIMATOR_DESC ( STDMETHODCALLTYPE *GetDesc )( 
            ID3D12VideoMotionEstimator * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetProtectedResourceSession )( 
            ID3D12VideoMotionEstimator * This,
            REFIID riid,
            _COM_Outptr_opt_  void **ppProtectedSession);
        
        END_INTERFACE
    } ID3D12VideoMotionEstimatorVtbl;

    interface ID3D12VideoMotionEstimator
    {
        CONST_VTBL struct ID3D12VideoMotionEstimatorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ID3D12VideoMotionEstimator_QueryInterface(This,riid,ppvObject)    \
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ID3D12VideoMotionEstimator_AddRef(This)    \
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ID3D12VideoMotionEstimator_Release(This)    \
    ( (This)->lpVtbl -> Release(This) ) 


#define ID3D12VideoMotionEstimator_GetPrivateData(This,guid,pDataSize,pData)    \
    ( (This)->lpVtbl -> GetPrivateData(This,guid,pDataSize,pData) ) 

#define ID3D12VideoMotionEstimator_SetPrivateData(This,guid,DataSize,pData)    \
    ( (This)->lpVtbl -> SetPrivateData(This,guid,DataSize,pData) ) 

#define ID3D12VideoMotionEstimator_SetPrivateDataInterface(This,guid,pData)    \
    ( (This)->lpVtbl -> SetPrivateDataInterface(This,guid,pData) ) 

#define ID3D12VideoMotionEstimator_SetName(This,Name)    \
    ( (This)->lpVtbl -> SetName(This,Name) ) 


#define ID3D12VideoMotionEstimator_GetDevice(This,riid,ppvDevice)    \
    ( (This)->lpVtbl -> GetDevice(This,riid,ppvDevice) ) 



#define ID3D12VideoMotionEstimator_GetDesc(This)    \
    ( (This)->lpVtbl -> GetDesc(This) ) 

#define ID3D12VideoMotionEstimator_GetProtectedResourceSession(This,riid,ppProtectedSession)    \
    ( (This)->lpVtbl -> GetProtectedResourceSession(This,riid,ppProtectedSession) ) 

#endif /* COBJMACROS */


#endif     /* C style interface */




#endif     /* __ID3D12VideoMotionEstimator_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_d3d12_1_0000_0007 */
/* [local] */ 

typedef struct D3D12_VIDEO_MOTION_VECTOR_HEAP_DESC
    {
    UINT NodeMask;
    DXGI_FORMAT InputFormat;
    D3D12_VIDEO_MOTION_ESTIMATOR_SEARCH_BLOCK_SIZE BlockSize;
    D3D12_VIDEO_MOTION_ESTIMATOR_VECTOR_PRECISION Precision;
    D3D12_VIDEO_MOTION_ESTIMATOR_DIFFERENCES_METRIC DifferencesMetric;
    D3D12_VIDEO_SIZE_RANGE SizeRange;
    }     D3D12_VIDEO_MOTION_VECTOR_HEAP_DESC;



extern RPC_IF_HANDLE __MIDL_itf_d3d12_1_0000_0007_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_d3d12_1_0000_0007_v0_0_s_ifspec;

#ifndef __ID3D12VideoMotionVectorHeap_INTERFACE_DEFINED__
#define __ID3D12VideoMotionVectorHeap_INTERFACE_DEFINED__

/* interface ID3D12VideoMotionVectorHeap */
/* [unique][local][object][uuid] */ 


EXTERN_C const IID IID_ID3D12VideoMotionVectorHeap;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5BE17987-743A-4061-834B-23D22DAEA505")
    ID3D12VideoMotionVectorHeap : public ID3D12Pageable
    {
    public:
        virtual D3D12_VIDEO_MOTION_VECTOR_HEAP_DESC STDMETHODCALLTYPE GetDesc( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProtectedResourceSession( 
            REFIID riid,
            _COM_Outptr_opt_  void **ppProtectedSession) = 0;
        
    };
    
    
#else     /* C style interface */

    typedef struct ID3D12VideoMotionVectorHeapVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ID3D12VideoMotionVectorHeap * This,
            REFIID riid,
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ID3D12VideoMotionVectorHeap * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ID3D12VideoMotionVectorHeap * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPrivateData )( 
            ID3D12VideoMotionVectorHeap * This,
            _In_  REFGUID guid,
            _Inout_  UINT *pDataSize,
            _Out_writes_bytes_opt_( *pDataSize )  void *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetPrivateData )( 
            ID3D12VideoMotionVectorHeap * This,
            _In_  REFGUID guid,
            _In_  UINT DataSize,
            _In_reads_bytes_opt_( DataSize )  const void *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetPrivateDataInterface )( 
            ID3D12VideoMotionVectorHeap * This,
            _In_  REFGUID guid,
            _In_opt_  const IUnknown *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetName )( 
            ID3D12VideoMotionVectorHeap * This,
            _In_z_  LPCWSTR Name);
        
        HRESULT ( STDMETHODCALLTYPE *GetDevice )( 
            ID3D12VideoMotionVectorHeap * This,
            REFIID riid,
            _COM_Outptr_opt_  void **ppvDevice);
        
        D3D12_VIDEO_MOTION_VECTOR_HEAP_DESC ( STDMETHODCALLTYPE *GetDesc )( 
            ID3D12VideoMotionVectorHeap * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetProtectedResourceSession )( 
            ID3D12VideoMotionVectorHeap * This,
            REFIID riid,
            _COM_Outptr_opt_  void **ppProtectedSession);
        
        END_INTERFACE
    } ID3D12VideoMotionVectorHeapVtbl;

    interface ID3D12VideoMotionVectorHeap
    {
        CONST_VTBL struct ID3D12VideoMotionVectorHeapVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ID3D12VideoMotionVectorHeap_QueryInterface(This,riid,ppvObject)    \
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ID3D12VideoMotionVectorHeap_AddRef(This)    \
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ID3D12VideoMotionVectorHeap_Release(This)    \
    ( (This)->lpVtbl -> Release(This) ) 


#define ID3D12VideoMotionVectorHeap_GetPrivateData(This,guid,pDataSize,pData)    \
    ( (This)->lpVtbl -> GetPrivateData(This,guid,pDataSize,pData) ) 

#define ID3D12VideoMotionVectorHeap_SetPrivateData(This,guid,DataSize,pData)    \
    ( (This)->lpVtbl -> SetPrivateData(This,guid,DataSize,pData) ) 

#define ID3D12VideoMotionVectorHeap_SetPrivateDataInterface(This,guid,pData)    \
    ( (This)->lpVtbl -> SetPrivateDataInterface(This,guid,pData) ) 

#define ID3D12VideoMotionVectorHeap_SetName(This,Name)    \
    ( (This)->lpVtbl -> SetName(This,Name) ) 


#define ID3D12VideoMotionVectorHeap_GetDevice(This,riid,ppvDevice)    \
    ( (This)->lpVtbl -> GetDevice(This,riid,ppvDevice) ) 



#define ID3D12VideoMotionVectorHeap_GetDesc(This)    \
    ( (This)->lpVtbl -> GetDesc(This) ) 

#define ID3D12VideoMotionVectorHeap_GetProtectedResourceSession(This,riid,ppProtectedSession)    \
    ( (This)->lpVtbl -> GetProtectedResourceSession(This,riid,ppProtectedSession) ) 

#endif /* COBJMACROS */


#endif     /* C style interface */




#endif     /* __ID3D12VideoMotionVectorHeap_INTERFACE_DEFINED__ */


#ifndef __ID3D12VideoDevice1_INTERFACE_DEFINED__
#define __ID3D12VideoDevice1_INTERFACE_DEFINED__

/* interface ID3D12VideoDevice1 */
/* [unique][local][object][uuid] */ 


EXTERN_C const IID IID_ID3D12VideoDevice1;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1BE98B1F-28C7-4E3C-974F-51C5FF82397D")
    ID3D12VideoDevice1 : public ID3D12VideoDevice
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateVideoMotionEstimator( 
            _In_  const D3D12_VIDEO_MOTION_ESTIMATOR_DESC *pDesc,
            _In_opt_  ID3D12ProtectedResourceSession *pProtectedResourceSession,
            _In_  REFIID riid,
            _COM_Outptr_  void **ppVideoMotionEstimator) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateVideoMotionVectorHeap( 
            _In_  const D3D12_VIDEO_MOTION_VECTOR_HEAP_DESC *pDesc,
            _In_opt_  ID3D12ProtectedResourceSession *pProtectedResourceSession,
            _In_  REFIID riid,
            _COM_Outptr_  void **ppVideoMotionVectorHeap) = 0;
        
    };
    
    
#else     /* C style interface */

    typedef struct ID3D12VideoDevice1Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ID3D12VideoDevice1 * This,
            REFIID riid,
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ID3D12VideoDevice1 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ID3D12VideoDevice1 * This);
        
        HRESULT ( STDMETHODCALLTYPE *CheckFeatureSupport )( 
            ID3D12VideoDevice1 * This,
            D3D12_FEATURE_VIDEO FeatureVideo,
            _Inout_updates_bytes_(FeatureSupportDataSize)  void *pFeatureSupportData,
            UINT FeatureSupportDataSize);
        
        HRESULT ( STDMETHODCALLTYPE *CreateVideoDecoder )( 
            ID3D12VideoDevice1 * This,
            _In_  const D3D12_VIDEO_DECODER_DESC *pDesc,
            _In_  REFIID riid,
            _COM_Outptr_  void **ppVideoDecoder);
        
        HRESULT ( STDMETHODCALLTYPE *CreateVideoDecoderHeap )( 
            ID3D12VideoDevice1 * This,
            _In_  const D3D12_VIDEO_DECODER_HEAP_DESC *pVideoDecoderHeapDesc,
            _In_  REFIID riid,
            _COM_Outptr_  void **ppVideoDecoderHeap);
        
        HRESULT ( STDMETHODCALLTYPE *CreateVideoProcessor )( 
            ID3D12VideoDevice1 * This,
            UINT NodeMask,
            _In_  const D3D12_VIDEO_PROCESS_OUTPUT_STREAM_DESC *pOutputStreamDesc,
            UINT NumInputStreamDescs,
            _In_reads_(NumInputStreamDescs)  const D3D12_VIDEO_PROCESS_INPUT_STREAM_DESC *pInputStreamDescs,
            _In_  REFIID riid,
            _COM_Outptr_  void **ppVideoProcessor);
        
        HRESULT ( STDMETHODCALLTYPE *CreateVideoMotionEstimator )( 
            ID3D12VideoDevice1 * This,
            _In_  const D3D12_VIDEO_MOTION_ESTIMATOR_DESC *pDesc,
            _In_opt_  ID3D12ProtectedResourceSession *pProtectedResourceSession,
            _In_  REFIID riid,
            _COM_Outptr_  void **ppVideoMotionEstimator);
        
        HRESULT ( STDMETHODCALLTYPE *CreateVideoMotionVectorHeap )( 
            ID3D12VideoDevice1 * This,
            _In_  const D3D12_VIDEO_MOTION_VECTOR_HEAP_DESC *pDesc,
            _In_opt_  ID3D12ProtectedResourceSession *pProtectedResourceSession,
            _In_  REFIID riid,
            _COM_Outptr_  void **ppVideoMotionVectorHeap);
        
        END_INTERFACE
    } ID3D12VideoDevice1Vtbl;

    interface ID3D12VideoDevice1
    {
        CONST_VTBL struct ID3D12VideoDevice1Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ID3D12VideoDevice1_QueryInterface(This,riid,ppvObject)    \
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ID3D12VideoDevice1_AddRef(This)    \
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ID3D12VideoDevice1_Release(This)    \
    ( (This)->lpVtbl -> Release(This) ) 


#define ID3D12VideoDevice1_CheckFeatureSupport(This,FeatureVideo,pFeatureSupportData,FeatureSupportDataSize)    \
    ( (This)->lpVtbl -> CheckFeatureSupport(This,FeatureVideo,pFeatureSupportData,FeatureSupportDataSize) ) 

#define ID3D12VideoDevice1_CreateVideoDecoder(This,pDesc,riid,ppVideoDecoder)    \
    ( (This)->lpVtbl -> CreateVideoDecoder(This,pDesc,riid,ppVideoDecoder) ) 

#define ID3D12VideoDevice1_CreateVideoDecoderHeap(This,pVideoDecoderHeapDesc,riid,ppVideoDecoderHeap)    \
    ( (This)->lpVtbl -> CreateVideoDecoderHeap(This,pVideoDecoderHeapDesc,riid,ppVideoDecoderHeap) ) 

#define ID3D12VideoDevice1_CreateVideoProcessor(This,NodeMask,pOutputStreamDesc,NumInputStreamDescs,pInputStreamDescs,riid,ppVideoProcessor)    \
    ( (This)->lpVtbl -> CreateVideoProcessor(This,NodeMask,pOutputStreamDesc,NumInputStreamDescs,pInputStreamDescs,riid,ppVideoProcessor) ) 


#define ID3D12VideoDevice1_CreateVideoMotionEstimator(This,pDesc,pProtectedResourceSession,riid,ppVideoMotionEstimator)    \
    ( (This)->lpVtbl -> CreateVideoMotionEstimator(This,pDesc,pProtectedResourceSession,riid,ppVideoMotionEstimator) ) 

#define ID3D12VideoDevice1_CreateVideoMotionVectorHeap(This,pDesc,pProtectedResourceSession,riid,ppVideoMotionVectorHeap)    \
    ( (This)->lpVtbl -> CreateVideoMotionVectorHeap(This,pDesc,pProtectedResourceSession,riid,ppVideoMotionVectorHeap) ) 

#endif /* COBJMACROS */


#endif     /* C style interface */




#endif     /* __ID3D12VideoDevice1_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_d3d12_1_0000_0009 */
/* [local] */ 

typedef struct D3D12_RESOURCE_COORDINATE
    {
    UINT64 X;
    UINT Y;
    UINT Z;
    UINT SubresourceIndex;
    }     D3D12_RESOURCE_COORDINATE;

typedef struct D3D12_VIDEO_MOTION_ESTIMATOR_OUTPUT
    {
    ID3D12VideoMotionVectorHeap *pMotionVectorHeap;
    }     D3D12_VIDEO_MOTION_ESTIMATOR_OUTPUT;

typedef struct D3D12_VIDEO_MOTION_ESTIMATOR_INPUT
    {
    ID3D12Resource *pInputTexture2D;
    UINT InputSubresourceIndex;
    ID3D12Resource *pReferenceTexture2D;
    UINT ReferenceSubresourceIndex;
    ID3D12VideoMotionVectorHeap *pHintMotionVectorHeap;
    }     D3D12_VIDEO_MOTION_ESTIMATOR_INPUT;

typedef struct D3D12_RESOLVE_VIDEO_MOTION_VECTOR_HEAP_OUTPUT
    {
    ID3D12Resource *pMotionVectorTexture2D;
    D3D12_RESOURCE_COORDINATE MotionVectorCoordinate;
    ID3D12Resource *pDifferencesMetricTexture2D;
    D3D12_RESOURCE_COORDINATE DifferencesMetricCoordinate;
    }     D3D12_RESOLVE_VIDEO_MOTION_VECTOR_HEAP_OUTPUT;

typedef struct D3D12_RESOLVE_VIDEO_MOTION_VECTOR_HEAP_INPUT
    {
    ID3D12VideoMotionVectorHeap *pMotionVectorHeap;
    UINT PixelWidth;
    UINT PixelHeight;
    }     D3D12_RESOLVE_VIDEO_MOTION_VECTOR_HEAP_INPUT;



extern RPC_IF_HANDLE __MIDL_itf_d3d12_1_0000_0009_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_d3d12_1_0000_0009_v0_0_s_ifspec;

#ifndef __ID3D12VideoEncodeCommandList_INTERFACE_DEFINED__
#define __ID3D12VideoEncodeCommandList_INTERFACE_DEFINED__

/* interface ID3D12VideoEncodeCommandList */
/* [unique][local][object][uuid] */ 


EXTERN_C const IID IID_ID3D12VideoEncodeCommandList;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DF605487-4326-4E41-BF7A-4CEEFAF18618")
    ID3D12VideoEncodeCommandList : public ID3D12CommandList
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( 
            _In_  ID3D12CommandAllocator *pAllocator) = 0;
        
        virtual void STDMETHODCALLTYPE ClearState( void) = 0;
        
        virtual void STDMETHODCALLTYPE ResourceBarrier( 
            _In_  UINT NumBarriers,
            _In_reads_(NumBarriers)  const D3D12_RESOURCE_BARRIER *pBarriers) = 0;
        
        virtual void STDMETHODCALLTYPE DiscardResource( 
            _In_  ID3D12Resource *pResource,
            _In_opt_  const D3D12_DISCARD_REGION *pRegion) = 0;
        
        virtual void STDMETHODCALLTYPE BeginQuery( 
            _In_  ID3D12QueryHeap *pQueryHeap,
            _In_  D3D12_QUERY_TYPE Type,
            _In_  UINT Index) = 0;
        
        virtual void STDMETHODCALLTYPE EndQuery( 
            _In_  ID3D12QueryHeap *pQueryHeap,
            _In_  D3D12_QUERY_TYPE Type,
            _In_  UINT Index) = 0;
        
        virtual void STDMETHODCALLTYPE ResolveQueryData( 
            _In_  ID3D12QueryHeap *pQueryHeap,
            _In_  D3D12_QUERY_TYPE Type,
            _In_  UINT StartIndex,
            _In_  UINT NumQueries,
            _In_  ID3D12Resource *pDestinationBuffer,
            _In_  UINT64 AlignedDestinationBufferOffset) = 0;
        
        virtual void STDMETHODCALLTYPE SetPredication( 
            _In_opt_  ID3D12Resource *pBuffer,
            _In_  UINT64 AlignedBufferOffset,
            _In_  D3D12_PREDICATION_OP Operation) = 0;
        
        virtual void STDMETHODCALLTYPE SetMarker( 
            UINT Metadata,
            _In_reads_bytes_opt_(Size)  const void *pData,
            UINT Size) = 0;
        
        virtual void STDMETHODCALLTYPE BeginEvent( 
            UINT Metadata,
            _In_reads_bytes_opt_(Size)  const void *pData,
            UINT Size) = 0;
        
        virtual void STDMETHODCALLTYPE EndEvent( void) = 0;
        
        virtual void STDMETHODCALLTYPE EstimateMotion( 
            _In_  ID3D12VideoMotionEstimator *pMotionEstimator,
            _In_  const D3D12_VIDEO_MOTION_ESTIMATOR_OUTPUT *pOutputArguments,
            _In_  const D3D12_VIDEO_MOTION_ESTIMATOR_INPUT *pInputArguments) = 0;
        
        virtual void STDMETHODCALLTYPE ResolveMotionVectorHeap( 
            const D3D12_RESOLVE_VIDEO_MOTION_VECTOR_HEAP_OUTPUT *pOutputArguments,
            const D3D12_RESOLVE_VIDEO_MOTION_VECTOR_HEAP_INPUT *pInputArguments) = 0;
        
        virtual void STDMETHODCALLTYPE WriteBufferImmediate( 
            UINT Count,
            _In_reads_(Count)  const D3D12_WRITEBUFFERIMMEDIATE_PARAMETER *pParams,
            _In_reads_opt_(Count)  const D3D12_WRITEBUFFERIMMEDIATE_MODE *pModes) = 0;
        
        virtual void STDMETHODCALLTYPE SetProtectedResourceSession( 
            _In_opt_  ID3D12ProtectedResourceSession *pProtectedResourceSession) = 0;
        
        virtual void STDMETHODCALLTYPE BeginTrackedWorkload( 
            _In_  ID3D12TrackedWorkload *pTrackedWorkload) = 0;
        
        virtual void STDMETHODCALLTYPE EndTrackedWorkload( 
            _In_  ID3D12TrackedWorkload *pTrackedWorkload,
            D3D12_TRACKED_WORKLOAD_DEADLINE *pFinishDeadline) = 0;
        
    };
    
    
#else     /* C style interface */

    typedef struct ID3D12VideoEncodeCommandListVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ID3D12VideoEncodeCommandList * This,
            REFIID riid,
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ID3D12VideoEncodeCommandList * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ID3D12VideoEncodeCommandList * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPrivateData )( 
            ID3D12VideoEncodeCommandList * This,
            _In_  REFGUID guid,
            _Inout_  UINT *pDataSize,
            _Out_writes_bytes_opt_( *pDataSize )  void *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetPrivateData )( 
            ID3D12VideoEncodeCommandList * This,
            _In_  REFGUID guid,
            _In_  UINT DataSize,
            _In_reads_bytes_opt_( DataSize )  const void *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetPrivateDataInterface )( 
            ID3D12VideoEncodeCommandList * This,
            _In_  REFGUID guid,
            _In_opt_  const IUnknown *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetName )( 
            ID3D12VideoEncodeCommandList * This,
            _In_z_  LPCWSTR Name);
        
        HRESULT ( STDMETHODCALLTYPE *GetDevice )( 
            ID3D12VideoEncodeCommandList * This,
            REFIID riid,
            _COM_Outptr_opt_  void **ppvDevice);
        
        D3D12_COMMAND_LIST_TYPE ( STDMETHODCALLTYPE *GetType )( 
            ID3D12VideoEncodeCommandList * This);
        
        HRESULT ( STDMETHODCALLTYPE *Close )( 
            ID3D12VideoEncodeCommandList * This);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            ID3D12VideoEncodeCommandList * This,
            _In_  ID3D12CommandAllocator *pAllocator);
        
        void ( STDMETHODCALLTYPE *ClearState )( 
            ID3D12VideoEncodeCommandList * This);
        
        void ( STDMETHODCALLTYPE *ResourceBarrier )( 
            ID3D12VideoEncodeCommandList * This,
            _In_  UINT NumBarriers,
            _In_reads_(NumBarriers)  const D3D12_RESOURCE_BARRIER *pBarriers);
        
        void ( STDMETHODCALLTYPE *DiscardResource )( 
            ID3D12VideoEncodeCommandList * This,
            _In_  ID3D12Resource *pResource,
            _In_opt_  const D3D12_DISCARD_REGION *pRegion);
        
        void ( STDMETHODCALLTYPE *BeginQuery )( 
            ID3D12VideoEncodeCommandList * This,
            _In_  ID3D12QueryHeap *pQueryHeap,
            _In_  D3D12_QUERY_TYPE Type,
            _In_  UINT Index);
        
        void ( STDMETHODCALLTYPE *EndQuery )( 
            ID3D12VideoEncodeCommandList * This,
            _In_  ID3D12QueryHeap *pQueryHeap,
            _In_  D3D12_QUERY_TYPE Type,
            _In_  UINT Index);
        
        void ( STDMETHODCALLTYPE *ResolveQueryData )( 
            ID3D12VideoEncodeCommandList * This,
            _In_  ID3D12QueryHeap *pQueryHeap,
            _In_  D3D12_QUERY_TYPE Type,
            _In_  UINT StartIndex,
            _In_  UINT NumQueries,
            _In_  ID3D12Resource *pDestinationBuffer,
            _In_  UINT64 AlignedDestinationBufferOffset);
        
        void ( STDMETHODCALLTYPE *SetPredication )( 
            ID3D12VideoEncodeCommandList * This,
            _In_opt_  ID3D12Resource *pBuffer,
            _In_  UINT64 AlignedBufferOffset,
            _In_  D3D12_PREDICATION_OP Operation);
        
        void ( STDMETHODCALLTYPE *SetMarker )( 
            ID3D12VideoEncodeCommandList * This,
            UINT Metadata,
            _In_reads_bytes_opt_(Size)  const void *pData,
            UINT Size);
        
        void ( STDMETHODCALLTYPE *BeginEvent )( 
            ID3D12VideoEncodeCommandList * This,
            UINT Metadata,
            _In_reads_bytes_opt_(Size)  const void *pData,
            UINT Size);
        
        void ( STDMETHODCALLTYPE *EndEvent )( 
            ID3D12VideoEncodeCommandList * This);
        
        void ( STDMETHODCALLTYPE *EstimateMotion )( 
            ID3D12VideoEncodeCommandList * This,
            _In_  ID3D12VideoMotionEstimator *pMotionEstimator,
            _In_  const D3D12_VIDEO_MOTION_ESTIMATOR_OUTPUT *pOutputArguments,
            _In_  const D3D12_VIDEO_MOTION_ESTIMATOR_INPUT *pInputArguments);
        
        void ( STDMETHODCALLTYPE *ResolveMotionVectorHeap )( 
            ID3D12VideoEncodeCommandList * This,
            const D3D12_RESOLVE_VIDEO_MOTION_VECTOR_HEAP_OUTPUT *pOutputArguments,
            const D3D12_RESOLVE_VIDEO_MOTION_VECTOR_HEAP_INPUT *pInputArguments);
        
        void ( STDMETHODCALLTYPE *WriteBufferImmediate )( 
            ID3D12VideoEncodeCommandList * This,
            UINT Count,
            _In_reads_(Count)  const D3D12_WRITEBUFFERIMMEDIATE_PARAMETER *pParams,
            _In_reads_opt_(Count)  const D3D12_WRITEBUFFERIMMEDIATE_MODE *pModes);
        
        void ( STDMETHODCALLTYPE *SetProtectedResourceSession )( 
            ID3D12VideoEncodeCommandList * This,
            _In_opt_  ID3D12ProtectedResourceSession *pProtectedResourceSession);
        
        void ( STDMETHODCALLTYPE *BeginTrackedWorkload )( 
            ID3D12VideoEncodeCommandList * This,
            _In_  ID3D12TrackedWorkload *pTrackedWorkload);
        
        void ( STDMETHODCALLTYPE *EndTrackedWorkload )( 
            ID3D12VideoEncodeCommandList * This,
            _In_  ID3D12TrackedWorkload *pTrackedWorkload,
            D3D12_TRACKED_WORKLOAD_DEADLINE *pFinishDeadline);
        
        END_INTERFACE
    } ID3D12VideoEncodeCommandListVtbl;

    interface ID3D12VideoEncodeCommandList
    {
        CONST_VTBL struct ID3D12VideoEncodeCommandListVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ID3D12VideoEncodeCommandList_QueryInterface(This,riid,ppvObject)    \
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ID3D12VideoEncodeCommandList_AddRef(This)    \
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ID3D12VideoEncodeCommandList_Release(This)    \
    ( (This)->lpVtbl -> Release(This) ) 


#define ID3D12VideoEncodeCommandList_GetPrivateData(This,guid,pDataSize,pData)    \
    ( (This)->lpVtbl -> GetPrivateData(This,guid,pDataSize,pData) ) 

#define ID3D12VideoEncodeCommandList_SetPrivateData(This,guid,DataSize,pData)    \
    ( (This)->lpVtbl -> SetPrivateData(This,guid,DataSize,pData) ) 

#define ID3D12VideoEncodeCommandList_SetPrivateDataInterface(This,guid,pData)    \
    ( (This)->lpVtbl -> SetPrivateDataInterface(This,guid,pData) ) 

#define ID3D12VideoEncodeCommandList_SetName(This,Name)    \
    ( (This)->lpVtbl -> SetName(This,Name) ) 


#define ID3D12VideoEncodeCommandList_GetDevice(This,riid,ppvDevice)    \
    ( (This)->lpVtbl -> GetDevice(This,riid,ppvDevice) ) 


#define ID3D12VideoEncodeCommandList_GetType(This)    \
    ( (This)->lpVtbl -> GetType(This) ) 


#define ID3D12VideoEncodeCommandList_Close(This)    \
    ( (This)->lpVtbl -> Close(This) ) 

#define ID3D12VideoEncodeCommandList_Reset(This,pAllocator)    \
    ( (This)->lpVtbl -> Reset(This,pAllocator) ) 

#define ID3D12VideoEncodeCommandList_ClearState(This)    \
    ( (This)->lpVtbl -> ClearState(This) ) 

#define ID3D12VideoEncodeCommandList_ResourceBarrier(This,NumBarriers,pBarriers)    \
    ( (This)->lpVtbl -> ResourceBarrier(This,NumBarriers,pBarriers) ) 

#define ID3D12VideoEncodeCommandList_DiscardResource(This,pResource,pRegion)    \
    ( (This)->lpVtbl -> DiscardResource(This,pResource,pRegion) ) 

#define ID3D12VideoEncodeCommandList_BeginQuery(This,pQueryHeap,Type,Index)    \
    ( (This)->lpVtbl -> BeginQuery(This,pQueryHeap,Type,Index) ) 

#define ID3D12VideoEncodeCommandList_EndQuery(This,pQueryHeap,Type,Index)    \
    ( (This)->lpVtbl -> EndQuery(This,pQueryHeap,Type,Index) ) 

#define ID3D12VideoEncodeCommandList_ResolveQueryData(This,pQueryHeap,Type,StartIndex,NumQueries,pDestinationBuffer,AlignedDestinationBufferOffset)    \
    ( (This)->lpVtbl -> ResolveQueryData(This,pQueryHeap,Type,StartIndex,NumQueries,pDestinationBuffer,AlignedDestinationBufferOffset) ) 

#define ID3D12VideoEncodeCommandList_SetPredication(This,pBuffer,AlignedBufferOffset,Operation)    \
    ( (This)->lpVtbl -> SetPredication(This,pBuffer,AlignedBufferOffset,Operation) ) 

#define ID3D12VideoEncodeCommandList_SetMarker(This,Metadata,pData,Size)    \
    ( (This)->lpVtbl -> SetMarker(This,Metadata,pData,Size) ) 

#define ID3D12VideoEncodeCommandList_BeginEvent(This,Metadata,pData,Size)    \
    ( (This)->lpVtbl -> BeginEvent(This,Metadata,pData,Size) ) 

#define ID3D12VideoEncodeCommandList_EndEvent(This)    \
    ( (This)->lpVtbl -> EndEvent(This) ) 

#define ID3D12VideoEncodeCommandList_EstimateMotion(This,pMotionEstimator,pOutputArguments,pInputArguments)    \
    ( (This)->lpVtbl -> EstimateMotion(This,pMotionEstimator,pOutputArguments,pInputArguments) ) 

#define ID3D12VideoEncodeCommandList_ResolveMotionVectorHeap(This,pOutputArguments,pInputArguments)    \
    ( (This)->lpVtbl -> ResolveMotionVectorHeap(This,pOutputArguments,pInputArguments) ) 

#define ID3D12VideoEncodeCommandList_WriteBufferImmediate(This,Count,pParams,pModes)    \
    ( (This)->lpVtbl -> WriteBufferImmediate(This,Count,pParams,pModes) ) 

#define ID3D12VideoEncodeCommandList_SetProtectedResourceSession(This,pProtectedResourceSession)    \
    ( (This)->lpVtbl -> SetProtectedResourceSession(This,pProtectedResourceSession) ) 

#define ID3D12VideoEncodeCommandList_BeginTrackedWorkload(This,pTrackedWorkload)    \
    ( (This)->lpVtbl -> BeginTrackedWorkload(This,pTrackedWorkload) ) 

#define ID3D12VideoEncodeCommandList_EndTrackedWorkload(This,pTrackedWorkload,pFinishDeadline)    \
    ( (This)->lpVtbl -> EndTrackedWorkload(This,pTrackedWorkload,pFinishDeadline) ) 

#endif /* COBJMACROS */


#endif     /* C style interface */




#endif     /* __ID3D12VideoEncodeCommandList_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_d3d12_1_0000_0010 */
/* [local] */ 

typedef ID3D12StateObject ID3D12StateObjectPrototype;

typedef ID3D12StateObjectProperties ID3D12StateObjectPropertiesPrototype;

typedef struct D3D12_EXPERIMENTAL_EXISTING_COLLECTION_DESC
    {
    ID3D12StateObjectPrototype *pExistingCollection;
    UINT NumExports;
    _In_reads_(NumExports)  D3D12_EXPORT_DESC *pExports;
    }     D3D12_EXPERIMENTAL_EXISTING_COLLECTION_DESC;

typedef struct D3D12_EXPERIMENTAL_HIT_GROUP_DESC
    {
    LPCWSTR HitGroupExport;
    _In_opt_  LPCWSTR AnyHitShaderImport;
    _In_opt_  LPCWSTR ClosestHitShaderImport;
    _In_opt_  LPCWSTR IntersectionShaderImport;
    }     D3D12_EXPERIMENTAL_HIT_GROUP_DESC;

typedef struct D3D12_EXPERIMENTAL_RAYTRACING_GEOMETRY_TRIANGLES_DESC
    {
    D3D12_GPU_VIRTUAL_ADDRESS Transform;
    DXGI_FORMAT IndexFormat;
    DXGI_FORMAT VertexFormat;
    UINT IndexCount;
    UINT VertexCount;
    D3D12_GPU_VIRTUAL_ADDRESS IndexBuffer;
    D3D12_GPU_VIRTUAL_ADDRESS_AND_STRIDE VertexBuffer;
    }     D3D12_EXPERIMENTAL_RAYTRACING_GEOMETRY_TRIANGLES_DESC;

typedef struct D3D12_EXPERIMENTAL_RAYTRACING_GEOMETRY_DESC
    {
    D3D12_RAYTRACING_GEOMETRY_TYPE Type;
    D3D12_RAYTRACING_GEOMETRY_FLAGS Flags;
    union 
        {
        D3D12_EXPERIMENTAL_RAYTRACING_GEOMETRY_TRIANGLES_DESC Triangles;
        D3D12_RAYTRACING_GEOMETRY_AABBS_DESC AABBs;
        }     ;
    }     D3D12_EXPERIMENTAL_RAYTRACING_GEOMETRY_DESC;

typedef struct D3D12_EXPERIMENTAL_RAYTRACING_INSTANCE_DESC
    {
    FLOAT Transform[ 12 ];
    UINT InstanceID    : 24;
    UINT InstanceMask    : 8;
    UINT InstanceContributionToHitGroupIndex    : 24;
    UINT Flags    : 8;
    D3D12_GPU_VIRTUAL_ADDRESS AccelerationStructure;
    }     D3D12_EXPERIMENTAL_RAYTRACING_INSTANCE_DESC;

typedef struct D3D12_EXPERIMENTAL_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC
    {
    D3D12_GPU_VIRTUAL_ADDRESS_RANGE DestAccelerationStructureData;
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE Type;
    UINT NumDescs;
    D3D12_ELEMENTS_LAYOUT DescsLayout;
    union 
        {
        D3D12_GPU_VIRTUAL_ADDRESS InstanceDescs;
        const D3D12_EXPERIMENTAL_RAYTRACING_GEOMETRY_DESC *pGeometryDescs;
        const D3D12_EXPERIMENTAL_RAYTRACING_GEOMETRY_DESC *const *ppGeometryDescs;
        }     ;
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS Flags;
    _In_opt_  D3D12_GPU_VIRTUAL_ADDRESS SourceAccelerationStructureData;
    D3D12_GPU_VIRTUAL_ADDRESS_RANGE ScratchAccelerationStructureData;
    }     D3D12_EXPERIMENTAL_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC;

typedef struct D3D12_EXPERIMENTAL_DISPATCH_RAYS_DESC
    {
    D3D12_GPU_VIRTUAL_ADDRESS_RANGE RayGenerationShaderRecord;
    D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE MissShaderTable;
    D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE HitGroupTable;
    D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE CallableShaderTable;
    UINT Width;
    UINT Height;
    }     D3D12_EXPERIMENTAL_DISPATCH_RAYS_DESC;

typedef struct D3D12_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_DESC
    {
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE Type;
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS Flags;
    UINT NumDescs;
    D3D12_ELEMENTS_LAYOUT DescsLayout;
    union 
        {
        const D3D12_EXPERIMENTAL_RAYTRACING_GEOMETRY_DESC *pGeometryDescs;
        const D3D12_EXPERIMENTAL_RAYTRACING_GEOMETRY_DESC *const *ppGeometryDescs;
        }     ;
    }     D3D12_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_DESC;



extern RPC_IF_HANDLE __MIDL_itf_d3d12_1_0000_0010_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_d3d12_1_0000_0010_v0_0_s_ifspec;

#ifndef __ID3D12DeviceRaytracingPrototype_INTERFACE_DEFINED__
#define __ID3D12DeviceRaytracingPrototype_INTERFACE_DEFINED__

/* interface ID3D12DeviceRaytracingPrototype */
/* [unique][local][object][uuid] */ 


EXTERN_C const IID IID_ID3D12DeviceRaytracingPrototype;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f52ef3ca-f710-4ee4-b873-a7f504e43995")
    ID3D12DeviceRaytracingPrototype : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateStateObject( 
            const D3D12_STATE_OBJECT_DESC *pDesc,
            REFIID riid,
            _COM_Outptr_  void **ppStateObject) = 0;
        
        virtual UINT STDMETHODCALLTYPE GetShaderIdentifierSize( void) = 0;
        
        virtual void STDMETHODCALLTYPE GetRaytracingAccelerationStructurePrebuildInfo( 
            _In_  D3D12_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_DESC *pDesc,
            _Out_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO *pInfo) = 0;
        
    };
    
    
#else     /* C style interface */

    typedef struct ID3D12DeviceRaytracingPrototypeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ID3D12DeviceRaytracingPrototype * This,
            REFIID riid,
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ID3D12DeviceRaytracingPrototype * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ID3D12DeviceRaytracingPrototype * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateStateObject )( 
            ID3D12DeviceRaytracingPrototype * This,
            const D3D12_STATE_OBJECT_DESC *pDesc,
            REFIID riid,
            _COM_Outptr_  void **ppStateObject);
        
        UINT ( STDMETHODCALLTYPE *GetShaderIdentifierSize )( 
            ID3D12DeviceRaytracingPrototype * This);
        
        void ( STDMETHODCALLTYPE *GetRaytracingAccelerationStructurePrebuildInfo )( 
            ID3D12DeviceRaytracingPrototype * This,
            _In_  D3D12_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_DESC *pDesc,
            _Out_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO *pInfo);
        
        END_INTERFACE
    } ID3D12DeviceRaytracingPrototypeVtbl;

    interface ID3D12DeviceRaytracingPrototype
    {
        CONST_VTBL struct ID3D12DeviceRaytracingPrototypeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ID3D12DeviceRaytracingPrototype_QueryInterface(This,riid,ppvObject)    \
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ID3D12DeviceRaytracingPrototype_AddRef(This)    \
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ID3D12DeviceRaytracingPrototype_Release(This)    \
    ( (This)->lpVtbl -> Release(This) ) 


#define ID3D12DeviceRaytracingPrototype_CreateStateObject(This,pDesc,riid,ppStateObject)    \
    ( (This)->lpVtbl -> CreateStateObject(This,pDesc,riid,ppStateObject) ) 

#define ID3D12DeviceRaytracingPrototype_GetShaderIdentifierSize(This)    \
    ( (This)->lpVtbl -> GetShaderIdentifierSize(This) ) 

#define ID3D12DeviceRaytracingPrototype_GetRaytracingAccelerationStructurePrebuildInfo(This,pDesc,pInfo)    \
    ( (This)->lpVtbl -> GetRaytracingAccelerationStructurePrebuildInfo(This,pDesc,pInfo) ) 

#endif /* COBJMACROS */


#endif     /* C style interface */




#endif     /* __ID3D12DeviceRaytracingPrototype_INTERFACE_DEFINED__ */


#ifndef __ID3D12CommandListRaytracingPrototype_INTERFACE_DEFINED__
#define __ID3D12CommandListRaytracingPrototype_INTERFACE_DEFINED__

/* interface ID3D12CommandListRaytracingPrototype */
/* [unique][local][object][uuid] */ 


EXTERN_C const IID IID_ID3D12CommandListRaytracingPrototype;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3c69787a-28fa-4701-970a-37a1ed1f9cab")
    ID3D12CommandListRaytracingPrototype : public IUnknown
    {
    public:
        virtual void STDMETHODCALLTYPE BuildRaytracingAccelerationStructure( 
            _In_  const D3D12_EXPERIMENTAL_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC *pDesc) = 0;
        
        virtual void STDMETHODCALLTYPE EmitRaytracingAccelerationStructurePostBuildInfo( 
            _In_  D3D12_GPU_VIRTUAL_ADDRESS_RANGE DestBuffer,
            _In_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_TYPE InfoType,
            _In_  UINT NumSourceAccelerationStructures,
            _In_reads_( NumSourceAccelerationStructures )  const D3D12_GPU_VIRTUAL_ADDRESS *pSourceAccelerationStructureData) = 0;
        
        virtual void STDMETHODCALLTYPE CopyRaytracingAccelerationStructure( 
            _In_  D3D12_GPU_VIRTUAL_ADDRESS_RANGE DestAccelerationStructureData,
            _In_  D3D12_GPU_VIRTUAL_ADDRESS SourceAccelerationStructureData,
            _In_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE Mode) = 0;
        
        virtual void STDMETHODCALLTYPE DispatchRays( 
            _In_  ID3D12StateObjectPrototype *pRaytracingPipelineState,
            _In_  const D3D12_EXPERIMENTAL_DISPATCH_RAYS_DESC *pDesc) = 0;
        
    };
    
    
#else     /* C style interface */

    typedef struct ID3D12CommandListRaytracingPrototypeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ID3D12CommandListRaytracingPrototype * This,
            REFIID riid,
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ID3D12CommandListRaytracingPrototype * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ID3D12CommandListRaytracingPrototype * This);
        
        void ( STDMETHODCALLTYPE *BuildRaytracingAccelerationStructure )( 
            ID3D12CommandListRaytracingPrototype * This,
            _In_  const D3D12_EXPERIMENTAL_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC *pDesc);
        
        void ( STDMETHODCALLTYPE *EmitRaytracingAccelerationStructurePostBuildInfo )( 
            ID3D12CommandListRaytracingPrototype * This,
            _In_  D3D12_GPU_VIRTUAL_ADDRESS_RANGE DestBuffer,
            _In_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_TYPE InfoType,
            _In_  UINT NumSourceAccelerationStructures,
            _In_reads_( NumSourceAccelerationStructures )  const D3D12_GPU_VIRTUAL_ADDRESS *pSourceAccelerationStructureData);
        
        void ( STDMETHODCALLTYPE *CopyRaytracingAccelerationStructure )( 
            ID3D12CommandListRaytracingPrototype * This,
            _In_  D3D12_GPU_VIRTUAL_ADDRESS_RANGE DestAccelerationStructureData,
            _In_  D3D12_GPU_VIRTUAL_ADDRESS SourceAccelerationStructureData,
            _In_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE Mode);
        
        void ( STDMETHODCALLTYPE *DispatchRays )( 
            ID3D12CommandListRaytracingPrototype * This,
            _In_  ID3D12StateObjectPrototype *pRaytracingPipelineState,
            _In_  const D3D12_EXPERIMENTAL_DISPATCH_RAYS_DESC *pDesc);
        
        END_INTERFACE
    } ID3D12CommandListRaytracingPrototypeVtbl;

    interface ID3D12CommandListRaytracingPrototype
    {
        CONST_VTBL struct ID3D12CommandListRaytracingPrototypeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ID3D12CommandListRaytracingPrototype_QueryInterface(This,riid,ppvObject)    \
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ID3D12CommandListRaytracingPrototype_AddRef(This)    \
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ID3D12CommandListRaytracingPrototype_Release(This)    \
    ( (This)->lpVtbl -> Release(This) ) 


#define ID3D12CommandListRaytracingPrototype_BuildRaytracingAccelerationStructure(This,pDesc)    \
    ( (This)->lpVtbl -> BuildRaytracingAccelerationStructure(This,pDesc) ) 

#define ID3D12CommandListRaytracingPrototype_EmitRaytracingAccelerationStructurePostBuildInfo(This,DestBuffer,InfoType,NumSourceAccelerationStructures,pSourceAccelerationStructureData)    \
    ( (This)->lpVtbl -> EmitRaytracingAccelerationStructurePostBuildInfo(This,DestBuffer,InfoType,NumSourceAccelerationStructures,pSourceAccelerationStructureData) ) 

#define ID3D12CommandListRaytracingPrototype_CopyRaytracingAccelerationStructure(This,DestAccelerationStructureData,SourceAccelerationStructureData,Mode)    \
    ( (This)->lpVtbl -> CopyRaytracingAccelerationStructure(This,DestAccelerationStructureData,SourceAccelerationStructureData,Mode) ) 

#define ID3D12CommandListRaytracingPrototype_DispatchRays(This,pRaytracingPipelineState,pDesc)    \
    ( (This)->lpVtbl -> DispatchRays(This,pRaytracingPipelineState,pDesc) ) 

#endif /* COBJMACROS */


#endif     /* C style interface */




#endif     /* __ID3D12CommandListRaytracingPrototype_INTERFACE_DEFINED__ */


#ifndef __ID3D12GraphicsCommandList5_INTERFACE_DEFINED__
#define __ID3D12GraphicsCommandList5_INTERFACE_DEFINED__

/* interface ID3D12GraphicsCommandList5 */
/* [unique][local][object][uuid] */ 


EXTERN_C const IID IID_ID3D12GraphicsCommandList5;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F5E76B80-AB90-444B-928E-13F4736EB9C2")
    ID3D12GraphicsCommandList5 : public ID3D12GraphicsCommandList4
    {
    public:
        virtual void STDMETHODCALLTYPE BeginTrackedWorkload( 
            _In_  ID3D12TrackedWorkload *pTrackedWorkload) = 0;
        
        virtual void STDMETHODCALLTYPE EndTrackedWorkload( 
            _In_  ID3D12TrackedWorkload *pTrackedWorkload,
            D3D12_TRACKED_WORKLOAD_DEADLINE *pFinishDeadline) = 0;
        
    };
    
    
#else     /* C style interface */

    typedef struct ID3D12GraphicsCommandList5Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ID3D12GraphicsCommandList5 * This,
            REFIID riid,
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ID3D12GraphicsCommandList5 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ID3D12GraphicsCommandList5 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPrivateData )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  REFGUID guid,
            _Inout_  UINT *pDataSize,
            _Out_writes_bytes_opt_( *pDataSize )  void *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetPrivateData )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  REFGUID guid,
            _In_  UINT DataSize,
            _In_reads_bytes_opt_( DataSize )  const void *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetPrivateDataInterface )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  REFGUID guid,
            _In_opt_  const IUnknown *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetName )( 
            ID3D12GraphicsCommandList5 * This,
            _In_z_  LPCWSTR Name);
        
        HRESULT ( STDMETHODCALLTYPE *GetDevice )( 
            ID3D12GraphicsCommandList5 * This,
            REFIID riid,
            _COM_Outptr_opt_  void **ppvDevice);
        
        D3D12_COMMAND_LIST_TYPE ( STDMETHODCALLTYPE *GetType )( 
            ID3D12GraphicsCommandList5 * This);
        
        HRESULT ( STDMETHODCALLTYPE *Close )( 
            ID3D12GraphicsCommandList5 * This);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  ID3D12CommandAllocator *pAllocator,
            _In_opt_  ID3D12PipelineState *pInitialState);
        
        void ( STDMETHODCALLTYPE *ClearState )( 
            ID3D12GraphicsCommandList5 * This,
            _In_opt_  ID3D12PipelineState *pPipelineState);
        
        void ( STDMETHODCALLTYPE *DrawInstanced )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  UINT VertexCountPerInstance,
            _In_  UINT InstanceCount,
            _In_  UINT StartVertexLocation,
            _In_  UINT StartInstanceLocation);
        
        void ( STDMETHODCALLTYPE *DrawIndexedInstanced )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  UINT IndexCountPerInstance,
            _In_  UINT InstanceCount,
            _In_  UINT StartIndexLocation,
            _In_  INT BaseVertexLocation,
            _In_  UINT StartInstanceLocation);
        
        void ( STDMETHODCALLTYPE *Dispatch )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  UINT ThreadGroupCountX,
            _In_  UINT ThreadGroupCountY,
            _In_  UINT ThreadGroupCountZ);
        
        void ( STDMETHODCALLTYPE *CopyBufferRegion )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  ID3D12Resource *pDstBuffer,
            UINT64 DstOffset,
            _In_  ID3D12Resource *pSrcBuffer,
            UINT64 SrcOffset,
            UINT64 NumBytes);
        
        void ( STDMETHODCALLTYPE *CopyTextureRegion )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  const D3D12_TEXTURE_COPY_LOCATION *pDst,
            UINT DstX,
            UINT DstY,
            UINT DstZ,
            _In_  const D3D12_TEXTURE_COPY_LOCATION *pSrc,
            _In_opt_  const D3D12_BOX *pSrcBox);
        
        void ( STDMETHODCALLTYPE *CopyResource )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  ID3D12Resource *pDstResource,
            _In_  ID3D12Resource *pSrcResource);
        
        void ( STDMETHODCALLTYPE *CopyTiles )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  ID3D12Resource *pTiledResource,
            _In_  const D3D12_TILED_RESOURCE_COORDINATE *pTileRegionStartCoordinate,
            _In_  const D3D12_TILE_REGION_SIZE *pTileRegionSize,
            _In_  ID3D12Resource *pBuffer,
            UINT64 BufferStartOffsetInBytes,
            D3D12_TILE_COPY_FLAGS Flags);
        
        void ( STDMETHODCALLTYPE *ResolveSubresource )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  ID3D12Resource *pDstResource,
            _In_  UINT DstSubresource,
            _In_  ID3D12Resource *pSrcResource,
            _In_  UINT SrcSubresource,
            _In_  DXGI_FORMAT Format);
        
        void ( STDMETHODCALLTYPE *IASetPrimitiveTopology )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology);
        
        void ( STDMETHODCALLTYPE *RSSetViewports )( 
            ID3D12GraphicsCommandList5 * This,
            _In_range_(0, D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE)  UINT NumViewports,
            _In_reads_( NumViewports)  const D3D12_VIEWPORT *pViewports);
        
        void ( STDMETHODCALLTYPE *RSSetScissorRects )( 
            ID3D12GraphicsCommandList5 * This,
            _In_range_(0, D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE)  UINT NumRects,
            _In_reads_( NumRects)  const D3D12_RECT *pRects);
        
        void ( STDMETHODCALLTYPE *OMSetBlendFactor )( 
            ID3D12GraphicsCommandList5 * This,
            _In_reads_opt_(4)  const FLOAT BlendFactor[ 4 ]);
        
        void ( STDMETHODCALLTYPE *OMSetStencilRef )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  UINT StencilRef);
        
        void ( STDMETHODCALLTYPE *SetPipelineState )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  ID3D12PipelineState *pPipelineState);
        
        void ( STDMETHODCALLTYPE *ResourceBarrier )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  UINT NumBarriers,
            _In_reads_(NumBarriers)  const D3D12_RESOURCE_BARRIER *pBarriers);
        
        void ( STDMETHODCALLTYPE *ExecuteBundle )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  ID3D12GraphicsCommandList *pCommandList);
        
        void ( STDMETHODCALLTYPE *SetDescriptorHeaps )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  UINT NumDescriptorHeaps,
            _In_reads_(NumDescriptorHeaps)  ID3D12DescriptorHeap *const *ppDescriptorHeaps);
        
        void ( STDMETHODCALLTYPE *SetComputeRootSignature )( 
            ID3D12GraphicsCommandList5 * This,
            _In_opt_  ID3D12RootSignature *pRootSignature);
        
        void ( STDMETHODCALLTYPE *SetGraphicsRootSignature )( 
            ID3D12GraphicsCommandList5 * This,
            _In_opt_  ID3D12RootSignature *pRootSignature);
        
        void ( STDMETHODCALLTYPE *SetComputeRootDescriptorTable )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  UINT RootParameterIndex,
            _In_  D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor);
        
        void ( STDMETHODCALLTYPE *SetGraphicsRootDescriptorTable )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  UINT RootParameterIndex,
            _In_  D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor);
        
        void ( STDMETHODCALLTYPE *SetComputeRoot32BitConstant )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  UINT RootParameterIndex,
            _In_  UINT SrcData,
            _In_  UINT DestOffsetIn32BitValues);
        
        void ( STDMETHODCALLTYPE *SetGraphicsRoot32BitConstant )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  UINT RootParameterIndex,
            _In_  UINT SrcData,
            _In_  UINT DestOffsetIn32BitValues);
        
        void ( STDMETHODCALLTYPE *SetComputeRoot32BitConstants )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  UINT RootParameterIndex,
            _In_  UINT Num32BitValuesToSet,
            _In_reads_(Num32BitValuesToSet*sizeof(UINT))  const void *pSrcData,
            _In_  UINT DestOffsetIn32BitValues);
        
        void ( STDMETHODCALLTYPE *SetGraphicsRoot32BitConstants )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  UINT RootParameterIndex,
            _In_  UINT Num32BitValuesToSet,
            _In_reads_(Num32BitValuesToSet*sizeof(UINT))  const void *pSrcData,
            _In_  UINT DestOffsetIn32BitValues);
        
        void ( STDMETHODCALLTYPE *SetComputeRootConstantBufferView )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  UINT RootParameterIndex,
            _In_  D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);
        
        void ( STDMETHODCALLTYPE *SetGraphicsRootConstantBufferView )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  UINT RootParameterIndex,
            _In_  D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);
        
        void ( STDMETHODCALLTYPE *SetComputeRootShaderResourceView )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  UINT RootParameterIndex,
            _In_  D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);
        
        void ( STDMETHODCALLTYPE *SetGraphicsRootShaderResourceView )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  UINT RootParameterIndex,
            _In_  D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);
        
        void ( STDMETHODCALLTYPE *SetComputeRootUnorderedAccessView )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  UINT RootParameterIndex,
            _In_  D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);
        
        void ( STDMETHODCALLTYPE *SetGraphicsRootUnorderedAccessView )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  UINT RootParameterIndex,
            _In_  D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);
        
        void ( STDMETHODCALLTYPE *IASetIndexBuffer )( 
            ID3D12GraphicsCommandList5 * This,
            _In_opt_  const D3D12_INDEX_BUFFER_VIEW *pView);
        
        void ( STDMETHODCALLTYPE *IASetVertexBuffers )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  UINT StartSlot,
            _In_  UINT NumViews,
            _In_reads_opt_(NumViews)  const D3D12_VERTEX_BUFFER_VIEW *pViews);
        
        void ( STDMETHODCALLTYPE *SOSetTargets )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  UINT StartSlot,
            _In_  UINT NumViews,
            _In_reads_opt_(NumViews)  const D3D12_STREAM_OUTPUT_BUFFER_VIEW *pViews);
        
        void ( STDMETHODCALLTYPE *OMSetRenderTargets )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  UINT NumRenderTargetDescriptors,
            _In_opt_  const D3D12_CPU_DESCRIPTOR_HANDLE *pRenderTargetDescriptors,
            _In_  BOOL RTsSingleHandleToDescriptorRange,
            _In_opt_  const D3D12_CPU_DESCRIPTOR_HANDLE *pDepthStencilDescriptor);
        
        void ( STDMETHODCALLTYPE *ClearDepthStencilView )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView,
            _In_  D3D12_CLEAR_FLAGS ClearFlags,
            _In_  FLOAT Depth,
            _In_  UINT8 Stencil,
            _In_  UINT NumRects,
            _In_reads_(NumRects)  const D3D12_RECT *pRects);
        
        void ( STDMETHODCALLTYPE *ClearRenderTargetView )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView,
            _In_  const FLOAT ColorRGBA[ 4 ],
            _In_  UINT NumRects,
            _In_reads_(NumRects)  const D3D12_RECT *pRects);
        
        void ( STDMETHODCALLTYPE *ClearUnorderedAccessViewUint )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  D3D12_GPU_DESCRIPTOR_HANDLE ViewGPUHandleInCurrentHeap,
            _In_  D3D12_CPU_DESCRIPTOR_HANDLE ViewCPUHandle,
            _In_  ID3D12Resource *pResource,
            _In_  const UINT Values[ 4 ],
            _In_  UINT NumRects,
            _In_reads_(NumRects)  const D3D12_RECT *pRects);
        
        void ( STDMETHODCALLTYPE *ClearUnorderedAccessViewFloat )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  D3D12_GPU_DESCRIPTOR_HANDLE ViewGPUHandleInCurrentHeap,
            _In_  D3D12_CPU_DESCRIPTOR_HANDLE ViewCPUHandle,
            _In_  ID3D12Resource *pResource,
            _In_  const FLOAT Values[ 4 ],
            _In_  UINT NumRects,
            _In_reads_(NumRects)  const D3D12_RECT *pRects);
        
        void ( STDMETHODCALLTYPE *DiscardResource )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  ID3D12Resource *pResource,
            _In_opt_  const D3D12_DISCARD_REGION *pRegion);
        
        void ( STDMETHODCALLTYPE *BeginQuery )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  ID3D12QueryHeap *pQueryHeap,
            _In_  D3D12_QUERY_TYPE Type,
            _In_  UINT Index);
        
        void ( STDMETHODCALLTYPE *EndQuery )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  ID3D12QueryHeap *pQueryHeap,
            _In_  D3D12_QUERY_TYPE Type,
            _In_  UINT Index);
        
        void ( STDMETHODCALLTYPE *ResolveQueryData )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  ID3D12QueryHeap *pQueryHeap,
            _In_  D3D12_QUERY_TYPE Type,
            _In_  UINT StartIndex,
            _In_  UINT NumQueries,
            _In_  ID3D12Resource *pDestinationBuffer,
            _In_  UINT64 AlignedDestinationBufferOffset);
        
        void ( STDMETHODCALLTYPE *SetPredication )( 
            ID3D12GraphicsCommandList5 * This,
            _In_opt_  ID3D12Resource *pBuffer,
            _In_  UINT64 AlignedBufferOffset,
            _In_  D3D12_PREDICATION_OP Operation);
        
        void ( STDMETHODCALLTYPE *SetMarker )( 
            ID3D12GraphicsCommandList5 * This,
            UINT Metadata,
            _In_reads_bytes_opt_(Size)  const void *pData,
            UINT Size);
        
        void ( STDMETHODCALLTYPE *BeginEvent )( 
            ID3D12GraphicsCommandList5 * This,
            UINT Metadata,
            _In_reads_bytes_opt_(Size)  const void *pData,
            UINT Size);
        
        void ( STDMETHODCALLTYPE *EndEvent )( 
            ID3D12GraphicsCommandList5 * This);
        
        void ( STDMETHODCALLTYPE *ExecuteIndirect )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  ID3D12CommandSignature *pCommandSignature,
            _In_  UINT MaxCommandCount,
            _In_  ID3D12Resource *pArgumentBuffer,
            _In_  UINT64 ArgumentBufferOffset,
            _In_opt_  ID3D12Resource *pCountBuffer,
            _In_  UINT64 CountBufferOffset);
        
        void ( STDMETHODCALLTYPE *AtomicCopyBufferUINT )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  ID3D12Resource *pDstBuffer,
            UINT64 DstOffset,
            _In_  ID3D12Resource *pSrcBuffer,
            UINT64 SrcOffset,
            UINT Dependencies,
            _In_reads_(Dependencies)  ID3D12Resource *const *ppDependentResources,
            _In_reads_(Dependencies)  const D3D12_SUBRESOURCE_RANGE_UINT64 *pDependentSubresourceRanges);
        
        void ( STDMETHODCALLTYPE *AtomicCopyBufferUINT64 )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  ID3D12Resource *pDstBuffer,
            UINT64 DstOffset,
            _In_  ID3D12Resource *pSrcBuffer,
            UINT64 SrcOffset,
            UINT Dependencies,
            _In_reads_(Dependencies)  ID3D12Resource *const *ppDependentResources,
            _In_reads_(Dependencies)  const D3D12_SUBRESOURCE_RANGE_UINT64 *pDependentSubresourceRanges);
        
        void ( STDMETHODCALLTYPE *OMSetDepthBounds )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  FLOAT Min,
            _In_  FLOAT Max);
        
        void ( STDMETHODCALLTYPE *SetSamplePositions )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  UINT NumSamplesPerPixel,
            _In_  UINT NumPixels,
            _In_reads_(NumSamplesPerPixel*NumPixels)  D3D12_SAMPLE_POSITION *pSamplePositions);
        
        void ( STDMETHODCALLTYPE *ResolveSubresourceRegion )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  ID3D12Resource *pDstResource,
            _In_  UINT DstSubresource,
            _In_  UINT DstX,
            _In_  UINT DstY,
            _In_  ID3D12Resource *pSrcResource,
            _In_  UINT SrcSubresource,
            _In_opt_  D3D12_RECT *pSrcRect,
            _In_  DXGI_FORMAT Format,
            _In_  D3D12_RESOLVE_MODE ResolveMode);
        
        void ( STDMETHODCALLTYPE *SetViewInstanceMask )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  UINT Mask);
        
        void ( STDMETHODCALLTYPE *WriteBufferImmediate )( 
            ID3D12GraphicsCommandList5 * This,
            UINT Count,
            _In_reads_(Count)  const D3D12_WRITEBUFFERIMMEDIATE_PARAMETER *pParams,
            _In_reads_opt_(Count)  const D3D12_WRITEBUFFERIMMEDIATE_MODE *pModes);
        
        void ( STDMETHODCALLTYPE *SetProtectedResourceSession )( 
            ID3D12GraphicsCommandList5 * This,
            _In_opt_  ID3D12ProtectedResourceSession *pProtectedResourceSession);
        
        void ( STDMETHODCALLTYPE *BeginRenderPass )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  UINT NumRenderTargets,
            _In_reads_opt_(NumRenderTargets)  const D3D12_RENDER_PASS_RENDER_TARGET_DESC *pRenderTargets,
            _In_opt_  const D3D12_RENDER_PASS_DEPTH_STENCIL_DESC *pDepthStencil,
            D3D12_RENDER_PASS_FLAGS Flags);
        
        void ( STDMETHODCALLTYPE *EndRenderPass )( 
            ID3D12GraphicsCommandList5 * This);
        
        void ( STDMETHODCALLTYPE *InitializeMetaCommand )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  ID3D12MetaCommand *pMetaCommand,
            _In_reads_bytes_opt_(InitializationParametersDataSizeInBytes)  const void *pInitializationParametersData,
            _In_  SIZE_T InitializationParametersDataSizeInBytes);
        
        void ( STDMETHODCALLTYPE *ExecuteMetaCommand )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  ID3D12MetaCommand *pMetaCommand,
            _In_reads_bytes_opt_(ExecutionParametersDataSizeInBytes)  const void *pExecutionParametersData,
            _In_  SIZE_T ExecutionParametersDataSizeInBytes);
        
        void ( STDMETHODCALLTYPE *BuildRaytracingAccelerationStructure )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC *pDesc,
            _In_  UINT NumPostbuildInfoDescs,
            _In_reads_opt_(NumPostbuildInfoDescs)  const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC *pPostbuildInfoDescs);
        
        void ( STDMETHODCALLTYPE *EmitRaytracingAccelerationStructurePostbuildInfo )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC *pDesc,
            _In_  UINT NumSourceAccelerationStructures,
            _In_reads_( NumSourceAccelerationStructures )  const D3D12_GPU_VIRTUAL_ADDRESS *pSourceAccelerationStructureData);
        
        void ( STDMETHODCALLTYPE *CopyRaytracingAccelerationStructure )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  D3D12_GPU_VIRTUAL_ADDRESS DestAccelerationStructureData,
            _In_  D3D12_GPU_VIRTUAL_ADDRESS SourceAccelerationStructureData,
            _In_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE Mode);
        
        void ( STDMETHODCALLTYPE *SetPipelineState1 )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  ID3D12StateObject *pStateObject);
        
        void ( STDMETHODCALLTYPE *DispatchRays )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  const D3D12_DISPATCH_RAYS_DESC *pDesc);
        
        void ( STDMETHODCALLTYPE *BeginTrackedWorkload )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  ID3D12TrackedWorkload *pTrackedWorkload);
        
        void ( STDMETHODCALLTYPE *EndTrackedWorkload )( 
            ID3D12GraphicsCommandList5 * This,
            _In_  ID3D12TrackedWorkload *pTrackedWorkload,
            D3D12_TRACKED_WORKLOAD_DEADLINE *pFinishDeadline);
        
        END_INTERFACE
    } ID3D12GraphicsCommandList5Vtbl;

    interface ID3D12GraphicsCommandList5
    {
        CONST_VTBL struct ID3D12GraphicsCommandList5Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ID3D12GraphicsCommandList5_QueryInterface(This,riid,ppvObject)    \
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ID3D12GraphicsCommandList5_AddRef(This)    \
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ID3D12GraphicsCommandList5_Release(This)    \
    ( (This)->lpVtbl -> Release(This) ) 


#define ID3D12GraphicsCommandList5_GetPrivateData(This,guid,pDataSize,pData)    \
    ( (This)->lpVtbl -> GetPrivateData(This,guid,pDataSize,pData) ) 

#define ID3D12GraphicsCommandList5_SetPrivateData(This,guid,DataSize,pData)    \
    ( (This)->lpVtbl -> SetPrivateData(This,guid,DataSize,pData) ) 

#define ID3D12GraphicsCommandList5_SetPrivateDataInterface(This,guid,pData)    \
    ( (This)->lpVtbl -> SetPrivateDataInterface(This,guid,pData) ) 

#define ID3D12GraphicsCommandList5_SetName(This,Name)    \
    ( (This)->lpVtbl -> SetName(This,Name) ) 


#define ID3D12GraphicsCommandList5_GetDevice(This,riid,ppvDevice)    \
    ( (This)->lpVtbl -> GetDevice(This,riid,ppvDevice) ) 


#define ID3D12GraphicsCommandList5_GetType(This)    \
    ( (This)->lpVtbl -> GetType(This) ) 


#define ID3D12GraphicsCommandList5_Close(This)    \
    ( (This)->lpVtbl -> Close(This) ) 

#define ID3D12GraphicsCommandList5_Reset(This,pAllocator,pInitialState)    \
    ( (This)->lpVtbl -> Reset(This,pAllocator,pInitialState) ) 

#define ID3D12GraphicsCommandList5_ClearState(This,pPipelineState)    \
    ( (This)->lpVtbl -> ClearState(This,pPipelineState) ) 

#define ID3D12GraphicsCommandList5_DrawInstanced(This,VertexCountPerInstance,InstanceCount,StartVertexLocation,StartInstanceLocation)    \
    ( (This)->lpVtbl -> DrawInstanced(This,VertexCountPerInstance,InstanceCount,StartVertexLocation,StartInstanceLocation) ) 

#define ID3D12GraphicsCommandList5_DrawIndexedInstanced(This,IndexCountPerInstance,InstanceCount,StartIndexLocation,BaseVertexLocation,StartInstanceLocation)    \
    ( (This)->lpVtbl -> DrawIndexedInstanced(This,IndexCountPerInstance,InstanceCount,StartIndexLocation,BaseVertexLocation,StartInstanceLocation) ) 

#define ID3D12GraphicsCommandList5_Dispatch(This,ThreadGroupCountX,ThreadGroupCountY,ThreadGroupCountZ)    \
    ( (This)->lpVtbl -> Dispatch(This,ThreadGroupCountX,ThreadGroupCountY,ThreadGroupCountZ) ) 

#define ID3D12GraphicsCommandList5_CopyBufferRegion(This,pDstBuffer,DstOffset,pSrcBuffer,SrcOffset,NumBytes)    \
    ( (This)->lpVtbl -> CopyBufferRegion(This,pDstBuffer,DstOffset,pSrcBuffer,SrcOffset,NumBytes) ) 

#define ID3D12GraphicsCommandList5_CopyTextureRegion(This,pDst,DstX,DstY,DstZ,pSrc,pSrcBox)    \
    ( (This)->lpVtbl -> CopyTextureRegion(This,pDst,DstX,DstY,DstZ,pSrc,pSrcBox) ) 

#define ID3D12GraphicsCommandList5_CopyResource(This,pDstResource,pSrcResource)    \
    ( (This)->lpVtbl -> CopyResource(This,pDstResource,pSrcResource) ) 

#define ID3D12GraphicsCommandList5_CopyTiles(This,pTiledResource,pTileRegionStartCoordinate,pTileRegionSize,pBuffer,BufferStartOffsetInBytes,Flags)    \
    ( (This)->lpVtbl -> CopyTiles(This,pTiledResource,pTileRegionStartCoordinate,pTileRegionSize,pBuffer,BufferStartOffsetInBytes,Flags) ) 

#define ID3D12GraphicsCommandList5_ResolveSubresource(This,pDstResource,DstSubresource,pSrcResource,SrcSubresource,Format)    \
    ( (This)->lpVtbl -> ResolveSubresource(This,pDstResource,DstSubresource,pSrcResource,SrcSubresource,Format) ) 

#define ID3D12GraphicsCommandList5_IASetPrimitiveTopology(This,PrimitiveTopology)    \
    ( (This)->lpVtbl -> IASetPrimitiveTopology(This,PrimitiveTopology) ) 

#define ID3D12GraphicsCommandList5_RSSetViewports(This,NumViewports,pViewports)    \
    ( (This)->lpVtbl -> RSSetViewports(This,NumViewports,pViewports) ) 

#define ID3D12GraphicsCommandList5_RSSetScissorRects(This,NumRects,pRects)    \
    ( (This)->lpVtbl -> RSSetScissorRects(This,NumRects,pRects) ) 

#define ID3D12GraphicsCommandList5_OMSetBlendFactor(This,BlendFactor)    \
    ( (This)->lpVtbl -> OMSetBlendFactor(This,BlendFactor) ) 

#define ID3D12GraphicsCommandList5_OMSetStencilRef(This,StencilRef)    \
    ( (This)->lpVtbl -> OMSetStencilRef(This,StencilRef) ) 

#define ID3D12GraphicsCommandList5_SetPipelineState(This,pPipelineState)    \
    ( (This)->lpVtbl -> SetPipelineState(This,pPipelineState) ) 

#define ID3D12GraphicsCommandList5_ResourceBarrier(This,NumBarriers,pBarriers)    \
    ( (This)->lpVtbl -> ResourceBarrier(This,NumBarriers,pBarriers) ) 

#define ID3D12GraphicsCommandList5_ExecuteBundle(This,pCommandList)    \
    ( (This)->lpVtbl -> ExecuteBundle(This,pCommandList) ) 

#define ID3D12GraphicsCommandList5_SetDescriptorHeaps(This,NumDescriptorHeaps,ppDescriptorHeaps)    \
    ( (This)->lpVtbl -> SetDescriptorHeaps(This,NumDescriptorHeaps,ppDescriptorHeaps) ) 

#define ID3D12GraphicsCommandList5_SetComputeRootSignature(This,pRootSignature)    \
    ( (This)->lpVtbl -> SetComputeRootSignature(This,pRootSignature) ) 

#define ID3D12GraphicsCommandList5_SetGraphicsRootSignature(This,pRootSignature)    \
    ( (This)->lpVtbl -> SetGraphicsRootSignature(This,pRootSignature) ) 

#define ID3D12GraphicsCommandList5_SetComputeRootDescriptorTable(This,RootParameterIndex,BaseDescriptor)    \
    ( (This)->lpVtbl -> SetComputeRootDescriptorTable(This,RootParameterIndex,BaseDescriptor) ) 

#define ID3D12GraphicsCommandList5_SetGraphicsRootDescriptorTable(This,RootParameterIndex,BaseDescriptor)    \
    ( (This)->lpVtbl -> SetGraphicsRootDescriptorTable(This,RootParameterIndex,BaseDescriptor) ) 

#define ID3D12GraphicsCommandList5_SetComputeRoot32BitConstant(This,RootParameterIndex,SrcData,DestOffsetIn32BitValues)    \
    ( (This)->lpVtbl -> SetComputeRoot32BitConstant(This,RootParameterIndex,SrcData,DestOffsetIn32BitValues) ) 

#define ID3D12GraphicsCommandList5_SetGraphicsRoot32BitConstant(This,RootParameterIndex,SrcData,DestOffsetIn32BitValues)    \
    ( (This)->lpVtbl -> SetGraphicsRoot32BitConstant(This,RootParameterIndex,SrcData,DestOffsetIn32BitValues) ) 

#define ID3D12GraphicsCommandList5_SetComputeRoot32BitConstants(This,RootParameterIndex,Num32BitValuesToSet,pSrcData,DestOffsetIn32BitValues)    \
    ( (This)->lpVtbl -> SetComputeRoot32BitConstants(This,RootParameterIndex,Num32BitValuesToSet,pSrcData,DestOffsetIn32BitValues) ) 

#define ID3D12GraphicsCommandList5_SetGraphicsRoot32BitConstants(This,RootParameterIndex,Num32BitValuesToSet,pSrcData,DestOffsetIn32BitValues)    \
    ( (This)->lpVtbl -> SetGraphicsRoot32BitConstants(This,RootParameterIndex,Num32BitValuesToSet,pSrcData,DestOffsetIn32BitValues) ) 

#define ID3D12GraphicsCommandList5_SetComputeRootConstantBufferView(This,RootParameterIndex,BufferLocation)    \
    ( (This)->lpVtbl -> SetComputeRootConstantBufferView(This,RootParameterIndex,BufferLocation) ) 

#define ID3D12GraphicsCommandList5_SetGraphicsRootConstantBufferView(This,RootParameterIndex,BufferLocation)    \
    ( (This)->lpVtbl -> SetGraphicsRootConstantBufferView(This,RootParameterIndex,BufferLocation) ) 

#define ID3D12GraphicsCommandList5_SetComputeRootShaderResourceView(This,RootParameterIndex,BufferLocation)    \
    ( (This)->lpVtbl -> SetComputeRootShaderResourceView(This,RootParameterIndex,BufferLocation) ) 

#define ID3D12GraphicsCommandList5_SetGraphicsRootShaderResourceView(This,RootParameterIndex,BufferLocation)    \
    ( (This)->lpVtbl -> SetGraphicsRootShaderResourceView(This,RootParameterIndex,BufferLocation) ) 

#define ID3D12GraphicsCommandList5_SetComputeRootUnorderedAccessView(This,RootParameterIndex,BufferLocation)    \
    ( (This)->lpVtbl -> SetComputeRootUnorderedAccessView(This,RootParameterIndex,BufferLocation) ) 

#define ID3D12GraphicsCommandList5_SetGraphicsRootUnorderedAccessView(This,RootParameterIndex,BufferLocation)    \
    ( (This)->lpVtbl -> SetGraphicsRootUnorderedAccessView(This,RootParameterIndex,BufferLocation) ) 

#define ID3D12GraphicsCommandList5_IASetIndexBuffer(This,pView)    \
    ( (This)->lpVtbl -> IASetIndexBuffer(This,pView) ) 

#define ID3D12GraphicsCommandList5_IASetVertexBuffers(This,StartSlot,NumViews,pViews)    \
    ( (This)->lpVtbl -> IASetVertexBuffers(This,StartSlot,NumViews,pViews) ) 

#define ID3D12GraphicsCommandList5_SOSetTargets(This,StartSlot,NumViews,pViews)    \
    ( (This)->lpVtbl -> SOSetTargets(This,StartSlot,NumViews,pViews) ) 

#define ID3D12GraphicsCommandList5_OMSetRenderTargets(This,NumRenderTargetDescriptors,pRenderTargetDescriptors,RTsSingleHandleToDescriptorRange,pDepthStencilDescriptor)    \
    ( (This)->lpVtbl -> OMSetRenderTargets(This,NumRenderTargetDescriptors,pRenderTargetDescriptors,RTsSingleHandleToDescriptorRange,pDepthStencilDescriptor) ) 

#define ID3D12GraphicsCommandList5_ClearDepthStencilView(This,DepthStencilView,ClearFlags,Depth,Stencil,NumRects,pRects)    \
    ( (This)->lpVtbl -> ClearDepthStencilView(This,DepthStencilView,ClearFlags,Depth,Stencil,NumRects,pRects) ) 

#define ID3D12GraphicsCommandList5_ClearRenderTargetView(This,RenderTargetView,ColorRGBA,NumRects,pRects)    \
    ( (This)->lpVtbl -> ClearRenderTargetView(This,RenderTargetView,ColorRGBA,NumRects,pRects) ) 

#define ID3D12GraphicsCommandList5_ClearUnorderedAccessViewUint(This,ViewGPUHandleInCurrentHeap,ViewCPUHandle,pResource,Values,NumRects,pRects)    \
    ( (This)->lpVtbl -> ClearUnorderedAccessViewUint(This,ViewGPUHandleInCurrentHeap,ViewCPUHandle,pResource,Values,NumRects,pRects) ) 

#define ID3D12GraphicsCommandList5_ClearUnorderedAccessViewFloat(This,ViewGPUHandleInCurrentHeap,ViewCPUHandle,pResource,Values,NumRects,pRects)    \
    ( (This)->lpVtbl -> ClearUnorderedAccessViewFloat(This,ViewGPUHandleInCurrentHeap,ViewCPUHandle,pResource,Values,NumRects,pRects) ) 

#define ID3D12GraphicsCommandList5_DiscardResource(This,pResource,pRegion)    \
    ( (This)->lpVtbl -> DiscardResource(This,pResource,pRegion) ) 

#define ID3D12GraphicsCommandList5_BeginQuery(This,pQueryHeap,Type,Index)    \
    ( (This)->lpVtbl -> BeginQuery(This,pQueryHeap,Type,Index) ) 

#define ID3D12GraphicsCommandList5_EndQuery(This,pQueryHeap,Type,Index)    \
    ( (This)->lpVtbl -> EndQuery(This,pQueryHeap,Type,Index) ) 

#define ID3D12GraphicsCommandList5_ResolveQueryData(This,pQueryHeap,Type,StartIndex,NumQueries,pDestinationBuffer,AlignedDestinationBufferOffset)    \
    ( (This)->lpVtbl -> ResolveQueryData(This,pQueryHeap,Type,StartIndex,NumQueries,pDestinationBuffer,AlignedDestinationBufferOffset) ) 

#define ID3D12GraphicsCommandList5_SetPredication(This,pBuffer,AlignedBufferOffset,Operation)    \
    ( (This)->lpVtbl -> SetPredication(This,pBuffer,AlignedBufferOffset,Operation) ) 

#define ID3D12GraphicsCommandList5_SetMarker(This,Metadata,pData,Size)    \
    ( (This)->lpVtbl -> SetMarker(This,Metadata,pData,Size) ) 

#define ID3D12GraphicsCommandList5_BeginEvent(This,Metadata,pData,Size)    \
    ( (This)->lpVtbl -> BeginEvent(This,Metadata,pData,Size) ) 

#define ID3D12GraphicsCommandList5_EndEvent(This)    \
    ( (This)->lpVtbl -> EndEvent(This) ) 

#define ID3D12GraphicsCommandList5_ExecuteIndirect(This,pCommandSignature,MaxCommandCount,pArgumentBuffer,ArgumentBufferOffset,pCountBuffer,CountBufferOffset)    \
    ( (This)->lpVtbl -> ExecuteIndirect(This,pCommandSignature,MaxCommandCount,pArgumentBuffer,ArgumentBufferOffset,pCountBuffer,CountBufferOffset) ) 


#define ID3D12GraphicsCommandList5_AtomicCopyBufferUINT(This,pDstBuffer,DstOffset,pSrcBuffer,SrcOffset,Dependencies,ppDependentResources,pDependentSubresourceRanges)    \
    ( (This)->lpVtbl -> AtomicCopyBufferUINT(This,pDstBuffer,DstOffset,pSrcBuffer,SrcOffset,Dependencies,ppDependentResources,pDependentSubresourceRanges) ) 

#define ID3D12GraphicsCommandList5_AtomicCopyBufferUINT64(This,pDstBuffer,DstOffset,pSrcBuffer,SrcOffset,Dependencies,ppDependentResources,pDependentSubresourceRanges)    \
    ( (This)->lpVtbl -> AtomicCopyBufferUINT64(This,pDstBuffer,DstOffset,pSrcBuffer,SrcOffset,Dependencies,ppDependentResources,pDependentSubresourceRanges) ) 

#define ID3D12GraphicsCommandList5_OMSetDepthBounds(This,Min,Max)    \
    ( (This)->lpVtbl -> OMSetDepthBounds(This,Min,Max) ) 

#define ID3D12GraphicsCommandList5_SetSamplePositions(This,NumSamplesPerPixel,NumPixels,pSamplePositions)    \
    ( (This)->lpVtbl -> SetSamplePositions(This,NumSamplesPerPixel,NumPixels,pSamplePositions) ) 

#define ID3D12GraphicsCommandList5_ResolveSubresourceRegion(This,pDstResource,DstSubresource,DstX,DstY,pSrcResource,SrcSubresource,pSrcRect,Format,ResolveMode)    \
    ( (This)->lpVtbl -> ResolveSubresourceRegion(This,pDstResource,DstSubresource,DstX,DstY,pSrcResource,SrcSubresource,pSrcRect,Format,ResolveMode) ) 

#define ID3D12GraphicsCommandList5_SetViewInstanceMask(This,Mask)    \
    ( (This)->lpVtbl -> SetViewInstanceMask(This,Mask) ) 


#define ID3D12GraphicsCommandList5_WriteBufferImmediate(This,Count,pParams,pModes)    \
    ( (This)->lpVtbl -> WriteBufferImmediate(This,Count,pParams,pModes) ) 


#define ID3D12GraphicsCommandList5_SetProtectedResourceSession(This,pProtectedResourceSession)    \
    ( (This)->lpVtbl -> SetProtectedResourceSession(This,pProtectedResourceSession) ) 


#define ID3D12GraphicsCommandList5_BeginRenderPass(This,NumRenderTargets,pRenderTargets,pDepthStencil,Flags)    \
    ( (This)->lpVtbl -> BeginRenderPass(This,NumRenderTargets,pRenderTargets,pDepthStencil,Flags) ) 

#define ID3D12GraphicsCommandList5_EndRenderPass(This)    \
    ( (This)->lpVtbl -> EndRenderPass(This) ) 

#define ID3D12GraphicsCommandList5_InitializeMetaCommand(This,pMetaCommand,pInitializationParametersData,InitializationParametersDataSizeInBytes)    \
    ( (This)->lpVtbl -> InitializeMetaCommand(This,pMetaCommand,pInitializationParametersData,InitializationParametersDataSizeInBytes) ) 

#define ID3D12GraphicsCommandList5_ExecuteMetaCommand(This,pMetaCommand,pExecutionParametersData,ExecutionParametersDataSizeInBytes)    \
    ( (This)->lpVtbl -> ExecuteMetaCommand(This,pMetaCommand,pExecutionParametersData,ExecutionParametersDataSizeInBytes) ) 

#define ID3D12GraphicsCommandList5_BuildRaytracingAccelerationStructure(This,pDesc,NumPostbuildInfoDescs,pPostbuildInfoDescs)    \
    ( (This)->lpVtbl -> BuildRaytracingAccelerationStructure(This,pDesc,NumPostbuildInfoDescs,pPostbuildInfoDescs) ) 

#define ID3D12GraphicsCommandList5_EmitRaytracingAccelerationStructurePostbuildInfo(This,pDesc,NumSourceAccelerationStructures,pSourceAccelerationStructureData)    \
    ( (This)->lpVtbl -> EmitRaytracingAccelerationStructurePostbuildInfo(This,pDesc,NumSourceAccelerationStructures,pSourceAccelerationStructureData) ) 

#define ID3D12GraphicsCommandList5_CopyRaytracingAccelerationStructure(This,DestAccelerationStructureData,SourceAccelerationStructureData,Mode)    \
    ( (This)->lpVtbl -> CopyRaytracingAccelerationStructure(This,DestAccelerationStructureData,SourceAccelerationStructureData,Mode) ) 

#define ID3D12GraphicsCommandList5_SetPipelineState1(This,pStateObject)    \
    ( (This)->lpVtbl -> SetPipelineState1(This,pStateObject) ) 

#define ID3D12GraphicsCommandList5_DispatchRays(This,pDesc)    \
    ( (This)->lpVtbl -> DispatchRays(This,pDesc) ) 


#define ID3D12GraphicsCommandList5_BeginTrackedWorkload(This,pTrackedWorkload)    \
    ( (This)->lpVtbl -> BeginTrackedWorkload(This,pTrackedWorkload) ) 

#define ID3D12GraphicsCommandList5_EndTrackedWorkload(This,pTrackedWorkload,pFinishDeadline)    \
    ( (This)->lpVtbl -> EndTrackedWorkload(This,pTrackedWorkload,pFinishDeadline) ) 

#endif /* COBJMACROS */


#endif     /* C style interface */




#endif     /* __ID3D12GraphicsCommandList5_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_d3d12_1_0000_0013 */
/* [local] */ 

typedef ID3D12CommandQueue ID3D12CommandQueueLatest;



extern RPC_IF_HANDLE __MIDL_itf_d3d12_1_0000_0013_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_d3d12_1_0000_0013_v0_0_s_ifspec;

#ifndef __ID3D12DeviceTrackedWorkload_INTERFACE_DEFINED__
#define __ID3D12DeviceTrackedWorkload_INTERFACE_DEFINED__

/* interface ID3D12DeviceTrackedWorkload */
/* [unique][local][object][uuid] */ 


EXTERN_C const IID IID_ID3D12DeviceTrackedWorkload;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AE666F25-BBA9-4E7C-A472-BD288B154140")
    ID3D12DeviceTrackedWorkload : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateTrackedWorkload( 
            _In_  const D3D12_TRACKED_WORKLOAD_DESC *pDesc,
            REFIID riid,
            _COM_Outptr_  void **ppvTrackedWorkload) = 0;
        
    };
    
    
#else     /* C style interface */

    typedef struct ID3D12DeviceTrackedWorkloadVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ID3D12DeviceTrackedWorkload * This,
            REFIID riid,
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ID3D12DeviceTrackedWorkload * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ID3D12DeviceTrackedWorkload * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateTrackedWorkload )( 
            ID3D12DeviceTrackedWorkload * This,
            _In_  const D3D12_TRACKED_WORKLOAD_DESC *pDesc,
            REFIID riid,
            _COM_Outptr_  void **ppvTrackedWorkload);
        
        END_INTERFACE
    } ID3D12DeviceTrackedWorkloadVtbl;

    interface ID3D12DeviceTrackedWorkload
    {
        CONST_VTBL struct ID3D12DeviceTrackedWorkloadVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ID3D12DeviceTrackedWorkload_QueryInterface(This,riid,ppvObject)    \
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ID3D12DeviceTrackedWorkload_AddRef(This)    \
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ID3D12DeviceTrackedWorkload_Release(This)    \
    ( (This)->lpVtbl -> Release(This) ) 


#define ID3D12DeviceTrackedWorkload_CreateTrackedWorkload(This,pDesc,riid,ppvTrackedWorkload)    \
    ( (This)->lpVtbl -> CreateTrackedWorkload(This,pDesc,riid,ppvTrackedWorkload) ) 

#endif /* COBJMACROS */


#endif     /* C style interface */




#endif     /* __ID3D12DeviceTrackedWorkload_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_d3d12_1_0000_0014 */
/* [local] */ 

#define D3D12_SHARED_RESOURCE_COMPATIBILITY_TIER_2 static_cast<D3D12_SHARED_RESOURCE_COMPATIBILITY_TIER>(D3D12_SHARED_RESOURCE_COMPATIBILITY_TIER_1+1)
#define D3D12_FEATURE_DISPLAYABLE static_cast<D3D12_FEATURE>(28)
typedef struct D3D12_FEATURE_DATA_DISPLAYABLE
    {
    _Out_  BOOL DisplayableTexture;
    _Out_  D3D12_SHARED_RESOURCE_COMPATIBILITY_TIER SharedResourceCompatibilityTier;
    }     D3D12_FEATURE_DATA_DISPLAYABLE;

#define D3D12_FEATURE_FORMAT_AND_GUID_SUPPORT static_cast<D3D12_FEATURE>(29)
typedef struct D3D12_FEATURE_DATA_FORMAT_AND_GUID_SUPPORT
    {
    _In_  DXGI_FORMAT Format;
    _In_  GUID Layout;
    _Out_  D3D12_FORMAT_SUPPORT1 Support1;
    _Out_  D3D12_FORMAT_SUPPORT2 Support2;
    }     D3D12_FEATURE_DATA_FORMAT_AND_GUID_SUPPORT;

typedef struct D3D12_RESOURCE_DESC1
    {
    D3D12_RESOURCE_DIMENSION Dimension;
    UINT64 Alignment;
    UINT64 Width;
    UINT Height;
    UINT16 DepthOrArraySize;
    UINT16 MipLevels;
    DXGI_FORMAT Format;
    DXGI_SAMPLE_DESC SampleDesc;
    GUID Layout;
    D3D12_RESOURCE_FLAGS Flags;
    }     D3D12_RESOURCE_DESC1;



extern RPC_IF_HANDLE __MIDL_itf_d3d12_1_0000_0014_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_d3d12_1_0000_0014_v0_0_s_ifspec;

#ifndef __ID3D12Device_GUIDTextureLayout_INTERFACE_DEFINED__
#define __ID3D12Device_GUIDTextureLayout_INTERFACE_DEFINED__

/* interface ID3D12Device_GUIDTextureLayout */
/* [unique][local][object][uuid] */ 


EXTERN_C const IID IID_ID3D12Device_GUIDTextureLayout;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1066D110-10A2-434F-8DAD-F400201220D1")
    ID3D12Device_GUIDTextureLayout : public ID3D12Device5
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CheckTextureGUIDs( 
            _In_  const D3D12_RESOURCE_DESC1 *pDesc,
            _Inout_  UINT *pNumGUIDs,
            _Out_writes_opt_(*pNumGUIDs)  GUID *pGUIDs) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateCommittedResource2( 
            _In_  const D3D12_HEAP_PROPERTIES *pHeapProperties,
            D3D12_HEAP_FLAGS HeapFlags,
            _In_  const D3D12_RESOURCE_DESC1 *pDesc,
            _In_opt_  const D3D12_CLEAR_VALUE *pOptimizedClearValue,
            _In_opt_  ID3D12ProtectedResourceSession *pProtectedSession,
            REFIID riidResource,
            _COM_Outptr_opt_  void **ppvResource) = 0;
        
        virtual D3D12_RESOURCE_DESC1 STDMETHODCALLTYPE GetResourceDesc1( 
            _In_  ID3D12Resource *pResource) = 0;
        
    };
    
    
#else     /* C style interface */

    typedef struct ID3D12Device_GUIDTextureLayoutVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ID3D12Device_GUIDTextureLayout * This,
            REFIID riid,
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ID3D12Device_GUIDTextureLayout * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ID3D12Device_GUIDTextureLayout * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPrivateData )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  REFGUID guid,
            _Inout_  UINT *pDataSize,
            _Out_writes_bytes_opt_( *pDataSize )  void *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetPrivateData )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  REFGUID guid,
            _In_  UINT DataSize,
            _In_reads_bytes_opt_( DataSize )  const void *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetPrivateDataInterface )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  REFGUID guid,
            _In_opt_  const IUnknown *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetName )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_z_  LPCWSTR Name);
        
        UINT ( STDMETHODCALLTYPE *GetNodeCount )( 
            ID3D12Device_GUIDTextureLayout * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateCommandQueue )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  const D3D12_COMMAND_QUEUE_DESC *pDesc,
            REFIID riid,
            _COM_Outptr_  void **ppCommandQueue);
        
        HRESULT ( STDMETHODCALLTYPE *CreateCommandAllocator )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  D3D12_COMMAND_LIST_TYPE type,
            REFIID riid,
            _COM_Outptr_  void **ppCommandAllocator);
        
        HRESULT ( STDMETHODCALLTYPE *CreateGraphicsPipelineState )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  const D3D12_GRAPHICS_PIPELINE_STATE_DESC *pDesc,
            REFIID riid,
            _COM_Outptr_  void **ppPipelineState);
        
        HRESULT ( STDMETHODCALLTYPE *CreateComputePipelineState )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  const D3D12_COMPUTE_PIPELINE_STATE_DESC *pDesc,
            REFIID riid,
            _COM_Outptr_  void **ppPipelineState);
        
        HRESULT ( STDMETHODCALLTYPE *CreateCommandList )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  UINT nodeMask,
            _In_  D3D12_COMMAND_LIST_TYPE type,
            _In_  ID3D12CommandAllocator *pCommandAllocator,
            _In_opt_  ID3D12PipelineState *pInitialState,
            REFIID riid,
            _COM_Outptr_  void **ppCommandList);
        
        HRESULT ( STDMETHODCALLTYPE *CheckFeatureSupport )( 
            ID3D12Device_GUIDTextureLayout * This,
            D3D12_FEATURE Feature,
            _Inout_updates_bytes_(FeatureSupportDataSize)  void *pFeatureSupportData,
            UINT FeatureSupportDataSize);
        
        HRESULT ( STDMETHODCALLTYPE *CreateDescriptorHeap )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  const D3D12_DESCRIPTOR_HEAP_DESC *pDescriptorHeapDesc,
            REFIID riid,
            _COM_Outptr_  void **ppvHeap);
        
        UINT ( STDMETHODCALLTYPE *GetDescriptorHandleIncrementSize )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapType);
        
        HRESULT ( STDMETHODCALLTYPE *CreateRootSignature )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  UINT nodeMask,
            _In_reads_(blobLengthInBytes)  const void *pBlobWithRootSignature,
            _In_  SIZE_T blobLengthInBytes,
            REFIID riid,
            _COM_Outptr_  void **ppvRootSignature);
        
        void ( STDMETHODCALLTYPE *CreateConstantBufferView )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_opt_  const D3D12_CONSTANT_BUFFER_VIEW_DESC *pDesc,
            _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);
        
        void ( STDMETHODCALLTYPE *CreateShaderResourceView )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_opt_  ID3D12Resource *pResource,
            _In_opt_  const D3D12_SHADER_RESOURCE_VIEW_DESC *pDesc,
            _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);
        
        void ( STDMETHODCALLTYPE *CreateUnorderedAccessView )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_opt_  ID3D12Resource *pResource,
            _In_opt_  ID3D12Resource *pCounterResource,
            _In_opt_  const D3D12_UNORDERED_ACCESS_VIEW_DESC *pDesc,
            _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);
        
        void ( STDMETHODCALLTYPE *CreateRenderTargetView )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_opt_  ID3D12Resource *pResource,
            _In_opt_  const D3D12_RENDER_TARGET_VIEW_DESC *pDesc,
            _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);
        
        void ( STDMETHODCALLTYPE *CreateDepthStencilView )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_opt_  ID3D12Resource *pResource,
            _In_opt_  const D3D12_DEPTH_STENCIL_VIEW_DESC *pDesc,
            _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);
        
        void ( STDMETHODCALLTYPE *CreateSampler )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  const D3D12_SAMPLER_DESC *pDesc,
            _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);
        
        void ( STDMETHODCALLTYPE *CopyDescriptors )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  UINT NumDestDescriptorRanges,
            _In_reads_(NumDestDescriptorRanges)  const D3D12_CPU_DESCRIPTOR_HANDLE *pDestDescriptorRangeStarts,
            _In_reads_opt_(NumDestDescriptorRanges)  const UINT *pDestDescriptorRangeSizes,
            _In_  UINT NumSrcDescriptorRanges,
            _In_reads_(NumSrcDescriptorRanges)  const D3D12_CPU_DESCRIPTOR_HANDLE *pSrcDescriptorRangeStarts,
            _In_reads_opt_(NumSrcDescriptorRanges)  const UINT *pSrcDescriptorRangeSizes,
            _In_  D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType);
        
        void ( STDMETHODCALLTYPE *CopyDescriptorsSimple )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  UINT NumDescriptors,
            _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptorRangeStart,
            _In_  D3D12_CPU_DESCRIPTOR_HANDLE SrcDescriptorRangeStart,
            _In_  D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType);
        
        D3D12_RESOURCE_ALLOCATION_INFO ( STDMETHODCALLTYPE *GetResourceAllocationInfo )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  UINT visibleMask,
            _In_  UINT numResourceDescs,
            _In_reads_(numResourceDescs)  const D3D12_RESOURCE_DESC *pResourceDescs);
        
        D3D12_HEAP_PROPERTIES ( STDMETHODCALLTYPE *GetCustomHeapProperties )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  UINT nodeMask,
            D3D12_HEAP_TYPE heapType);
        
        HRESULT ( STDMETHODCALLTYPE *CreateCommittedResource )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  const D3D12_HEAP_PROPERTIES *pHeapProperties,
            D3D12_HEAP_FLAGS HeapFlags,
            _In_  const D3D12_RESOURCE_DESC *pDesc,
            D3D12_RESOURCE_STATES InitialResourceState,
            _In_opt_  const D3D12_CLEAR_VALUE *pOptimizedClearValue,
            REFIID riidResource,
            _COM_Outptr_opt_  void **ppvResource);
        
        HRESULT ( STDMETHODCALLTYPE *CreateHeap )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  const D3D12_HEAP_DESC *pDesc,
            REFIID riid,
            _COM_Outptr_opt_  void **ppvHeap);
        
        HRESULT ( STDMETHODCALLTYPE *CreatePlacedResource )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  ID3D12Heap *pHeap,
            UINT64 HeapOffset,
            _In_  const D3D12_RESOURCE_DESC *pDesc,
            D3D12_RESOURCE_STATES InitialState,
            _In_opt_  const D3D12_CLEAR_VALUE *pOptimizedClearValue,
            REFIID riid,
            _COM_Outptr_opt_  void **ppvResource);
        
        HRESULT ( STDMETHODCALLTYPE *CreateReservedResource )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  const D3D12_RESOURCE_DESC *pDesc,
            D3D12_RESOURCE_STATES InitialState,
            _In_opt_  const D3D12_CLEAR_VALUE *pOptimizedClearValue,
            REFIID riid,
            _COM_Outptr_opt_  void **ppvResource);
        
        HRESULT ( STDMETHODCALLTYPE *CreateSharedHandle )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  ID3D12DeviceChild *pObject,
            _In_opt_  const SECURITY_ATTRIBUTES *pAttributes,
            DWORD Access,
            _In_opt_  LPCWSTR Name,
            _Out_  HANDLE *pHandle);
        
        HRESULT ( STDMETHODCALLTYPE *OpenSharedHandle )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  HANDLE NTHandle,
            REFIID riid,
            _COM_Outptr_opt_  void **ppvObj);
        
        HRESULT ( STDMETHODCALLTYPE *OpenSharedHandleByName )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  LPCWSTR Name,
            DWORD Access,
            /* [annotation][out] */ 
            _Out_  HANDLE *pNTHandle);
        
        HRESULT ( STDMETHODCALLTYPE *MakeResident )( 
            ID3D12Device_GUIDTextureLayout * This,
            UINT NumObjects,
            _In_reads_(NumObjects)  ID3D12Pageable *const *ppObjects);
        
        HRESULT ( STDMETHODCALLTYPE *Evict )( 
            ID3D12Device_GUIDTextureLayout * This,
            UINT NumObjects,
            _In_reads_(NumObjects)  ID3D12Pageable *const *ppObjects);
        
        HRESULT ( STDMETHODCALLTYPE *CreateFence )( 
            ID3D12Device_GUIDTextureLayout * This,
            UINT64 InitialValue,
            D3D12_FENCE_FLAGS Flags,
            REFIID riid,
            _COM_Outptr_  void **ppFence);
        
        HRESULT ( STDMETHODCALLTYPE *GetDeviceRemovedReason )( 
            ID3D12Device_GUIDTextureLayout * This);
        
        void ( STDMETHODCALLTYPE *GetCopyableFootprints )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  const D3D12_RESOURCE_DESC *pResourceDesc,
            _In_range_(0,D3D12_REQ_SUBRESOURCES)  UINT FirstSubresource,
            _In_range_(0,D3D12_REQ_SUBRESOURCES-FirstSubresource)  UINT NumSubresources,
            UINT64 BaseOffset,
            _Out_writes_opt_(NumSubresources)  D3D12_PLACED_SUBRESOURCE_FOOTPRINT *pLayouts,
            _Out_writes_opt_(NumSubresources)  UINT *pNumRows,
            _Out_writes_opt_(NumSubresources)  UINT64 *pRowSizeInBytes,
            _Out_opt_  UINT64 *pTotalBytes);
        
        HRESULT ( STDMETHODCALLTYPE *CreateQueryHeap )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  const D3D12_QUERY_HEAP_DESC *pDesc,
            REFIID riid,
            _COM_Outptr_opt_  void **ppvHeap);
        
        HRESULT ( STDMETHODCALLTYPE *SetStablePowerState )( 
            ID3D12Device_GUIDTextureLayout * This,
            BOOL Enable);
        
        HRESULT ( STDMETHODCALLTYPE *CreateCommandSignature )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  const D3D12_COMMAND_SIGNATURE_DESC *pDesc,
            _In_opt_  ID3D12RootSignature *pRootSignature,
            REFIID riid,
            _COM_Outptr_opt_  void **ppvCommandSignature);
        
        void ( STDMETHODCALLTYPE *GetResourceTiling )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  ID3D12Resource *pTiledResource,
            _Out_opt_  UINT *pNumTilesForEntireResource,
            _Out_opt_  D3D12_PACKED_MIP_INFO *pPackedMipDesc,
            _Out_opt_  D3D12_TILE_SHAPE *pStandardTileShapeForNonPackedMips,
            _Inout_opt_  UINT *pNumSubresourceTilings,
            _In_  UINT FirstSubresourceTilingToGet,
            _Out_writes_(*pNumSubresourceTilings)  D3D12_SUBRESOURCE_TILING *pSubresourceTilingsForNonPackedMips);
        
        LUID ( STDMETHODCALLTYPE *GetAdapterLuid )( 
            ID3D12Device_GUIDTextureLayout * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreatePipelineLibrary )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_reads_(BlobLength)  const void *pLibraryBlob,
            SIZE_T BlobLength,
            REFIID riid,
            _COM_Outptr_  void **ppPipelineLibrary);
        
        HRESULT ( STDMETHODCALLTYPE *SetEventOnMultipleFenceCompletion )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_reads_(NumFences)  ID3D12Fence *const *ppFences,
            _In_reads_(NumFences)  const UINT64 *pFenceValues,
            UINT NumFences,
            D3D12_MULTIPLE_FENCE_WAIT_FLAGS Flags,
            HANDLE hEvent);
        
        HRESULT ( STDMETHODCALLTYPE *SetResidencyPriority )( 
            ID3D12Device_GUIDTextureLayout * This,
            UINT NumObjects,
            _In_reads_(NumObjects)  ID3D12Pageable *const *ppObjects,
            _In_reads_(NumObjects)  const D3D12_RESIDENCY_PRIORITY *pPriorities);
        
        HRESULT ( STDMETHODCALLTYPE *CreatePipelineState )( 
            ID3D12Device_GUIDTextureLayout * This,
            const D3D12_PIPELINE_STATE_STREAM_DESC *pDesc,
            REFIID riid,
            _COM_Outptr_  void **ppPipelineState);
        
        HRESULT ( STDMETHODCALLTYPE *OpenExistingHeapFromAddress )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  const void *pAddress,
            REFIID riid,
            _COM_Outptr_  void **ppvHeap);
        
        HRESULT ( STDMETHODCALLTYPE *OpenExistingHeapFromFileMapping )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  HANDLE hFileMapping,
            REFIID riid,
            _COM_Outptr_  void **ppvHeap);
        
        HRESULT ( STDMETHODCALLTYPE *EnqueueMakeResident )( 
            ID3D12Device_GUIDTextureLayout * This,
            D3D12_RESIDENCY_FLAGS Flags,
            UINT NumObjects,
            _In_reads_(NumObjects)  ID3D12Pageable *const *ppObjects,
            _In_  ID3D12Fence *pFenceToSignal,
            UINT64 FenceValueToSignal);
        
        HRESULT ( STDMETHODCALLTYPE *CreateCommandList1 )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  UINT nodeMask,
            _In_  D3D12_COMMAND_LIST_TYPE type,
            _In_  D3D12_COMMAND_LIST_FLAGS flags,
            REFIID riid,
            _COM_Outptr_  void **ppCommandList);
        
        HRESULT ( STDMETHODCALLTYPE *CreateProtectedResourceSession )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  const D3D12_PROTECTED_RESOURCE_SESSION_DESC *pDesc,
            _In_  REFIID riid,
            _COM_Outptr_  void **ppSession);
        
        HRESULT ( STDMETHODCALLTYPE *CreateCommittedResource1 )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  const D3D12_HEAP_PROPERTIES *pHeapProperties,
            D3D12_HEAP_FLAGS HeapFlags,
            _In_  const D3D12_RESOURCE_DESC *pDesc,
            D3D12_RESOURCE_STATES InitialResourceState,
            _In_opt_  const D3D12_CLEAR_VALUE *pOptimizedClearValue,
            _In_opt_  ID3D12ProtectedResourceSession *pProtectedSession,
            REFIID riidResource,
            _COM_Outptr_opt_  void **ppvResource);
        
        HRESULT ( STDMETHODCALLTYPE *CreateHeap1 )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  const D3D12_HEAP_DESC *pDesc,
            _In_opt_  ID3D12ProtectedResourceSession *pProtectedSession,
            REFIID riid,
            _COM_Outptr_opt_  void **ppvHeap);
        
        HRESULT ( STDMETHODCALLTYPE *CreateReservedResource1 )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  const D3D12_RESOURCE_DESC *pDesc,
            D3D12_RESOURCE_STATES InitialState,
            _In_opt_  const D3D12_CLEAR_VALUE *pOptimizedClearValue,
            _In_opt_  ID3D12ProtectedResourceSession *pProtectedSession,
            REFIID riid,
            _COM_Outptr_opt_  void **ppvResource);
        
        D3D12_RESOURCE_ALLOCATION_INFO ( STDMETHODCALLTYPE *GetResourceAllocationInfo1 )( 
            ID3D12Device_GUIDTextureLayout * This,
            UINT visibleMask,
            UINT numResourceDescs,
            _In_reads_(numResourceDescs)  const D3D12_RESOURCE_DESC *pResourceDescs,
            _Out_writes_opt_(numResourceDescs)  D3D12_RESOURCE_ALLOCATION_INFO1 *pResourceAllocationInfo1);
        
        HRESULT ( STDMETHODCALLTYPE *CreateLifetimeTracker )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  ID3D12LifetimeOwner *pOwner,
            REFIID riid,
            _COM_Outptr_  void **ppvTracker);
        
        void ( STDMETHODCALLTYPE *RemoveDevice )( 
            ID3D12Device_GUIDTextureLayout * This);
        
        HRESULT ( STDMETHODCALLTYPE *EnumerateMetaCommands )( 
            ID3D12Device_GUIDTextureLayout * This,
            _Inout_  UINT *pNumMetaCommands,
            _Out_writes_opt_(*pNumMetaCommands)  D3D12_META_COMMAND_DESC *pDescs);
        
        HRESULT ( STDMETHODCALLTYPE *EnumerateMetaCommandParameters )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  REFGUID CommandId,
            _In_  D3D12_META_COMMAND_PARAMETER_STAGE Stage,
            _Out_opt_  UINT *pTotalStructureSizeInBytes,
            _Inout_  UINT *pParameterCount,
            _Out_writes_opt_(*pParameterCount)  D3D12_META_COMMAND_PARAMETER_DESC *pParameterDescs);
        
        HRESULT ( STDMETHODCALLTYPE *CreateMetaCommand )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  REFGUID CommandId,
            _In_  UINT NodeMask,
            _In_reads_bytes_opt_(CreationParametersDataSizeInBytes)  const void *pCreationParametersData,
            _In_  SIZE_T CreationParametersDataSizeInBytes,
            REFIID riid,
            _COM_Outptr_  void **ppMetaCommand);
        
        HRESULT ( STDMETHODCALLTYPE *CreateStateObject )( 
            ID3D12Device_GUIDTextureLayout * This,
            const D3D12_STATE_OBJECT_DESC *pDesc,
            REFIID riid,
            _COM_Outptr_  void **ppStateObject);
        
        void ( STDMETHODCALLTYPE *GetRaytracingAccelerationStructurePrebuildInfo )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS *pDesc,
            _Out_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO *pInfo);
        
        D3D12_DRIVER_MATCHING_IDENTIFIER_STATUS ( STDMETHODCALLTYPE *CheckDriverMatchingIdentifier )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  D3D12_SERIALIZED_DATA_TYPE SerializedDataType,
            _In_  const D3D12_SERIALIZED_DATA_DRIVER_MATCHING_IDENTIFIER *pIdentifierToCheck);
        
        HRESULT ( STDMETHODCALLTYPE *CheckTextureGUIDs )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  const D3D12_RESOURCE_DESC1 *pDesc,
            _Inout_  UINT *pNumGUIDs,
            _Out_writes_opt_(*pNumGUIDs)  GUID *pGUIDs);
        
        HRESULT ( STDMETHODCALLTYPE *CreateCommittedResource2 )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  const D3D12_HEAP_PROPERTIES *pHeapProperties,
            D3D12_HEAP_FLAGS HeapFlags,
            _In_  const D3D12_RESOURCE_DESC1 *pDesc,
            _In_opt_  const D3D12_CLEAR_VALUE *pOptimizedClearValue,
            _In_opt_  ID3D12ProtectedResourceSession *pProtectedSession,
            REFIID riidResource,
            _COM_Outptr_opt_  void **ppvResource);
        
        D3D12_RESOURCE_DESC1 ( STDMETHODCALLTYPE *GetResourceDesc1 )( 
            ID3D12Device_GUIDTextureLayout * This,
            _In_  ID3D12Resource *pResource);
        
        END_INTERFACE
    } ID3D12Device_GUIDTextureLayoutVtbl;

    interface ID3D12Device_GUIDTextureLayout
    {
        CONST_VTBL struct ID3D12Device_GUIDTextureLayoutVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ID3D12Device_GUIDTextureLayout_QueryInterface(This,riid,ppvObject)    \
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ID3D12Device_GUIDTextureLayout_AddRef(This)    \
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ID3D12Device_GUIDTextureLayout_Release(This)    \
    ( (This)->lpVtbl -> Release(This) ) 


#define ID3D12Device_GUIDTextureLayout_GetPrivateData(This,guid,pDataSize,pData)    \
    ( (This)->lpVtbl -> GetPrivateData(This,guid,pDataSize,pData) ) 

#define ID3D12Device_GUIDTextureLayout_SetPrivateData(This,guid,DataSize,pData)    \
    ( (This)->lpVtbl -> SetPrivateData(This,guid,DataSize,pData) ) 

#define ID3D12Device_GUIDTextureLayout_SetPrivateDataInterface(This,guid,pData)    \
    ( (This)->lpVtbl -> SetPrivateDataInterface(This,guid,pData) ) 

#define ID3D12Device_GUIDTextureLayout_SetName(This,Name)    \
    ( (This)->lpVtbl -> SetName(This,Name) ) 


#define ID3D12Device_GUIDTextureLayout_GetNodeCount(This)    \
    ( (This)->lpVtbl -> GetNodeCount(This) ) 

#define ID3D12Device_GUIDTextureLayout_CreateCommandQueue(This,pDesc,riid,ppCommandQueue)    \
    ( (This)->lpVtbl -> CreateCommandQueue(This,pDesc,riid,ppCommandQueue) ) 

#define ID3D12Device_GUIDTextureLayout_CreateCommandAllocator(This,type,riid,ppCommandAllocator)    \
    ( (This)->lpVtbl -> CreateCommandAllocator(This,type,riid,ppCommandAllocator) ) 

#define ID3D12Device_GUIDTextureLayout_CreateGraphicsPipelineState(This,pDesc,riid,ppPipelineState)    \
    ( (This)->lpVtbl -> CreateGraphicsPipelineState(This,pDesc,riid,ppPipelineState) ) 

#define ID3D12Device_GUIDTextureLayout_CreateComputePipelineState(This,pDesc,riid,ppPipelineState)    \
    ( (This)->lpVtbl -> CreateComputePipelineState(This,pDesc,riid,ppPipelineState) ) 

#define ID3D12Device_GUIDTextureLayout_CreateCommandList(This,nodeMask,type,pCommandAllocator,pInitialState,riid,ppCommandList)    \
    ( (This)->lpVtbl -> CreateCommandList(This,nodeMask,type,pCommandAllocator,pInitialState,riid,ppCommandList) ) 

#define ID3D12Device_GUIDTextureLayout_CheckFeatureSupport(This,Feature,pFeatureSupportData,FeatureSupportDataSize)    \
    ( (This)->lpVtbl -> CheckFeatureSupport(This,Feature,pFeatureSupportData,FeatureSupportDataSize) ) 

#define ID3D12Device_GUIDTextureLayout_CreateDescriptorHeap(This,pDescriptorHeapDesc,riid,ppvHeap)    \
    ( (This)->lpVtbl -> CreateDescriptorHeap(This,pDescriptorHeapDesc,riid,ppvHeap) ) 

#define ID3D12Device_GUIDTextureLayout_GetDescriptorHandleIncrementSize(This,DescriptorHeapType)    \
    ( (This)->lpVtbl -> GetDescriptorHandleIncrementSize(This,DescriptorHeapType) ) 

#define ID3D12Device_GUIDTextureLayout_CreateRootSignature(This,nodeMask,pBlobWithRootSignature,blobLengthInBytes,riid,ppvRootSignature)    \
    ( (This)->lpVtbl -> CreateRootSignature(This,nodeMask,pBlobWithRootSignature,blobLengthInBytes,riid,ppvRootSignature) ) 

#define ID3D12Device_GUIDTextureLayout_CreateConstantBufferView(This,pDesc,DestDescriptor)    \
    ( (This)->lpVtbl -> CreateConstantBufferView(This,pDesc,DestDescriptor) ) 

#define ID3D12Device_GUIDTextureLayout_CreateShaderResourceView(This,pResource,pDesc,DestDescriptor)    \
    ( (This)->lpVtbl -> CreateShaderResourceView(This,pResource,pDesc,DestDescriptor) ) 

#define ID3D12Device_GUIDTextureLayout_CreateUnorderedAccessView(This,pResource,pCounterResource,pDesc,DestDescriptor)    \
    ( (This)->lpVtbl -> CreateUnorderedAccessView(This,pResource,pCounterResource,pDesc,DestDescriptor) ) 

#define ID3D12Device_GUIDTextureLayout_CreateRenderTargetView(This,pResource,pDesc,DestDescriptor)    \
    ( (This)->lpVtbl -> CreateRenderTargetView(This,pResource,pDesc,DestDescriptor) ) 

#define ID3D12Device_GUIDTextureLayout_CreateDepthStencilView(This,pResource,pDesc,DestDescriptor)    \
    ( (This)->lpVtbl -> CreateDepthStencilView(This,pResource,pDesc,DestDescriptor) ) 

#define ID3D12Device_GUIDTextureLayout_CreateSampler(This,pDesc,DestDescriptor)    \
    ( (This)->lpVtbl -> CreateSampler(This,pDesc,DestDescriptor) ) 

#define ID3D12Device_GUIDTextureLayout_CopyDescriptors(This,NumDestDescriptorRanges,pDestDescriptorRangeStarts,pDestDescriptorRangeSizes,NumSrcDescriptorRanges,pSrcDescriptorRangeStarts,pSrcDescriptorRangeSizes,DescriptorHeapsType)    \
    ( (This)->lpVtbl -> CopyDescriptors(This,NumDestDescriptorRanges,pDestDescriptorRangeStarts,pDestDescriptorRangeSizes,NumSrcDescriptorRanges,pSrcDescriptorRangeStarts,pSrcDescriptorRangeSizes,DescriptorHeapsType) ) 

#define ID3D12Device_GUIDTextureLayout_CopyDescriptorsSimple(This,NumDescriptors,DestDescriptorRangeStart,SrcDescriptorRangeStart,DescriptorHeapsType)    \
    ( (This)->lpVtbl -> CopyDescriptorsSimple(This,NumDescriptors,DestDescriptorRangeStart,SrcDescriptorRangeStart,DescriptorHeapsType) ) 

#define ID3D12Device_GUIDTextureLayout_GetResourceAllocationInfo(This,visibleMask,numResourceDescs,pResourceDescs)    \
    ( (This)->lpVtbl -> GetResourceAllocationInfo(This,visibleMask,numResourceDescs,pResourceDescs) ) 

#define ID3D12Device_GUIDTextureLayout_GetCustomHeapProperties(This,nodeMask,heapType)    \
    ( (This)->lpVtbl -> GetCustomHeapProperties(This,nodeMask,heapType) ) 

#define ID3D12Device_GUIDTextureLayout_CreateCommittedResource(This,pHeapProperties,HeapFlags,pDesc,InitialResourceState,pOptimizedClearValue,riidResource,ppvResource)    \
    ( (This)->lpVtbl -> CreateCommittedResource(This,pHeapProperties,HeapFlags,pDesc,InitialResourceState,pOptimizedClearValue,riidResource,ppvResource) ) 

#define ID3D12Device_GUIDTextureLayout_CreateHeap(This,pDesc,riid,ppvHeap)    \
    ( (This)->lpVtbl -> CreateHeap(This,pDesc,riid,ppvHeap) ) 

#define ID3D12Device_GUIDTextureLayout_CreatePlacedResource(This,pHeap,HeapOffset,pDesc,InitialState,pOptimizedClearValue,riid,ppvResource)    \
    ( (This)->lpVtbl -> CreatePlacedResource(This,pHeap,HeapOffset,pDesc,InitialState,pOptimizedClearValue,riid,ppvResource) ) 

#define ID3D12Device_GUIDTextureLayout_CreateReservedResource(This,pDesc,InitialState,pOptimizedClearValue,riid,ppvResource)    \
    ( (This)->lpVtbl -> CreateReservedResource(This,pDesc,InitialState,pOptimizedClearValue,riid,ppvResource) ) 

#define ID3D12Device_GUIDTextureLayout_CreateSharedHandle(This,pObject,pAttributes,Access,Name,pHandle)    \
    ( (This)->lpVtbl -> CreateSharedHandle(This,pObject,pAttributes,Access,Name,pHandle) ) 

#define ID3D12Device_GUIDTextureLayout_OpenSharedHandle(This,NTHandle,riid,ppvObj)    \
    ( (This)->lpVtbl -> OpenSharedHandle(This,NTHandle,riid,ppvObj) ) 

#define ID3D12Device_GUIDTextureLayout_OpenSharedHandleByName(This,Name,Access,pNTHandle)    \
    ( (This)->lpVtbl -> OpenSharedHandleByName(This,Name,Access,pNTHandle) ) 

#define ID3D12Device_GUIDTextureLayout_MakeResident(This,NumObjects,ppObjects)    \
    ( (This)->lpVtbl -> MakeResident(This,NumObjects,ppObjects) ) 

#define ID3D12Device_GUIDTextureLayout_Evict(This,NumObjects,ppObjects)    \
    ( (This)->lpVtbl -> Evict(This,NumObjects,ppObjects) ) 

#define ID3D12Device_GUIDTextureLayout_CreateFence(This,InitialValue,Flags,riid,ppFence)    \
    ( (This)->lpVtbl -> CreateFence(This,InitialValue,Flags,riid,ppFence) ) 

#define ID3D12Device_GUIDTextureLayout_GetDeviceRemovedReason(This)    \
    ( (This)->lpVtbl -> GetDeviceRemovedReason(This) ) 

#define ID3D12Device_GUIDTextureLayout_GetCopyableFootprints(This,pResourceDesc,FirstSubresource,NumSubresources,BaseOffset,pLayouts,pNumRows,pRowSizeInBytes,pTotalBytes)    \
    ( (This)->lpVtbl -> GetCopyableFootprints(This,pResourceDesc,FirstSubresource,NumSubresources,BaseOffset,pLayouts,pNumRows,pRowSizeInBytes,pTotalBytes) ) 

#define ID3D12Device_GUIDTextureLayout_CreateQueryHeap(This,pDesc,riid,ppvHeap)    \
    ( (This)->lpVtbl -> CreateQueryHeap(This,pDesc,riid,ppvHeap) ) 

#define ID3D12Device_GUIDTextureLayout_SetStablePowerState(This,Enable)    \
    ( (This)->lpVtbl -> SetStablePowerState(This,Enable) ) 

#define ID3D12Device_GUIDTextureLayout_CreateCommandSignature(This,pDesc,pRootSignature,riid,ppvCommandSignature)    \
    ( (This)->lpVtbl -> CreateCommandSignature(This,pDesc,pRootSignature,riid,ppvCommandSignature) ) 

#define ID3D12Device_GUIDTextureLayout_GetResourceTiling(This,pTiledResource,pNumTilesForEntireResource,pPackedMipDesc,pStandardTileShapeForNonPackedMips,pNumSubresourceTilings,FirstSubresourceTilingToGet,pSubresourceTilingsForNonPackedMips)    \
    ( (This)->lpVtbl -> GetResourceTiling(This,pTiledResource,pNumTilesForEntireResource,pPackedMipDesc,pStandardTileShapeForNonPackedMips,pNumSubresourceTilings,FirstSubresourceTilingToGet,pSubresourceTilingsForNonPackedMips) ) 

#define ID3D12Device_GUIDTextureLayout_GetAdapterLuid(This)    \
    ( (This)->lpVtbl -> GetAdapterLuid(This) ) 


#define ID3D12Device_GUIDTextureLayout_CreatePipelineLibrary(This,pLibraryBlob,BlobLength,riid,ppPipelineLibrary)    \
    ( (This)->lpVtbl -> CreatePipelineLibrary(This,pLibraryBlob,BlobLength,riid,ppPipelineLibrary) ) 

#define ID3D12Device_GUIDTextureLayout_SetEventOnMultipleFenceCompletion(This,ppFences,pFenceValues,NumFences,Flags,hEvent)    \
    ( (This)->lpVtbl -> SetEventOnMultipleFenceCompletion(This,ppFences,pFenceValues,NumFences,Flags,hEvent) ) 

#define ID3D12Device_GUIDTextureLayout_SetResidencyPriority(This,NumObjects,ppObjects,pPriorities)    \
    ( (This)->lpVtbl -> SetResidencyPriority(This,NumObjects,ppObjects,pPriorities) ) 


#define ID3D12Device_GUIDTextureLayout_CreatePipelineState(This,pDesc,riid,ppPipelineState)    \
    ( (This)->lpVtbl -> CreatePipelineState(This,pDesc,riid,ppPipelineState) ) 


#define ID3D12Device_GUIDTextureLayout_OpenExistingHeapFromAddress(This,pAddress,riid,ppvHeap)    \
    ( (This)->lpVtbl -> OpenExistingHeapFromAddress(This,pAddress,riid,ppvHeap) ) 

#define ID3D12Device_GUIDTextureLayout_OpenExistingHeapFromFileMapping(This,hFileMapping,riid,ppvHeap)    \
    ( (This)->lpVtbl -> OpenExistingHeapFromFileMapping(This,hFileMapping,riid,ppvHeap) ) 

#define ID3D12Device_GUIDTextureLayout_EnqueueMakeResident(This,Flags,NumObjects,ppObjects,pFenceToSignal,FenceValueToSignal)    \
    ( (This)->lpVtbl -> EnqueueMakeResident(This,Flags,NumObjects,ppObjects,pFenceToSignal,FenceValueToSignal) ) 


#define ID3D12Device_GUIDTextureLayout_CreateCommandList1(This,nodeMask,type,flags,riid,ppCommandList)    \
    ( (This)->lpVtbl -> CreateCommandList1(This,nodeMask,type,flags,riid,ppCommandList) ) 

#define ID3D12Device_GUIDTextureLayout_CreateProtectedResourceSession(This,pDesc,riid,ppSession)    \
    ( (This)->lpVtbl -> CreateProtectedResourceSession(This,pDesc,riid,ppSession) ) 

#define ID3D12Device_GUIDTextureLayout_CreateCommittedResource1(This,pHeapProperties,HeapFlags,pDesc,InitialResourceState,pOptimizedClearValue,pProtectedSession,riidResource,ppvResource)    \
    ( (This)->lpVtbl -> CreateCommittedResource1(This,pHeapProperties,HeapFlags,pDesc,InitialResourceState,pOptimizedClearValue,pProtectedSession,riidResource,ppvResource) ) 

#define ID3D12Device_GUIDTextureLayout_CreateHeap1(This,pDesc,pProtectedSession,riid,ppvHeap)    \
    ( (This)->lpVtbl -> CreateHeap1(This,pDesc,pProtectedSession,riid,ppvHeap) ) 

#define ID3D12Device_GUIDTextureLayout_CreateReservedResource1(This,pDesc,InitialState,pOptimizedClearValue,pProtectedSession,riid,ppvResource)    \
    ( (This)->lpVtbl -> CreateReservedResource1(This,pDesc,InitialState,pOptimizedClearValue,pProtectedSession,riid,ppvResource) ) 

#define ID3D12Device_GUIDTextureLayout_GetResourceAllocationInfo1(This,visibleMask,numResourceDescs,pResourceDescs,pResourceAllocationInfo1)    \
    ( (This)->lpVtbl -> GetResourceAllocationInfo1(This,visibleMask,numResourceDescs,pResourceDescs,pResourceAllocationInfo1) ) 


#define ID3D12Device_GUIDTextureLayout_CreateLifetimeTracker(This,pOwner,riid,ppvTracker)    \
    ( (This)->lpVtbl -> CreateLifetimeTracker(This,pOwner,riid,ppvTracker) ) 

#define ID3D12Device_GUIDTextureLayout_RemoveDevice(This)    \
    ( (This)->lpVtbl -> RemoveDevice(This) ) 

#define ID3D12Device_GUIDTextureLayout_EnumerateMetaCommands(This,pNumMetaCommands,pDescs)    \
    ( (This)->lpVtbl -> EnumerateMetaCommands(This,pNumMetaCommands,pDescs) ) 

#define ID3D12Device_GUIDTextureLayout_EnumerateMetaCommandParameters(This,CommandId,Stage,pTotalStructureSizeInBytes,pParameterCount,pParameterDescs)    \
    ( (This)->lpVtbl -> EnumerateMetaCommandParameters(This,CommandId,Stage,pTotalStructureSizeInBytes,pParameterCount,pParameterDescs) ) 

#define ID3D12Device_GUIDTextureLayout_CreateMetaCommand(This,CommandId,NodeMask,pCreationParametersData,CreationParametersDataSizeInBytes,riid,ppMetaCommand)    \
    ( (This)->lpVtbl -> CreateMetaCommand(This,CommandId,NodeMask,pCreationParametersData,CreationParametersDataSizeInBytes,riid,ppMetaCommand) ) 

#define ID3D12Device_GUIDTextureLayout_CreateStateObject(This,pDesc,riid,ppStateObject)    \
    ( (This)->lpVtbl -> CreateStateObject(This,pDesc,riid,ppStateObject) ) 

#define ID3D12Device_GUIDTextureLayout_GetRaytracingAccelerationStructurePrebuildInfo(This,pDesc,pInfo)    \
    ( (This)->lpVtbl -> GetRaytracingAccelerationStructurePrebuildInfo(This,pDesc,pInfo) ) 

#define ID3D12Device_GUIDTextureLayout_CheckDriverMatchingIdentifier(This,SerializedDataType,pIdentifierToCheck)    \
    ( (This)->lpVtbl -> CheckDriverMatchingIdentifier(This,SerializedDataType,pIdentifierToCheck) ) 


#define ID3D12Device_GUIDTextureLayout_CheckTextureGUIDs(This,pDesc,pNumGUIDs,pGUIDs)    \
    ( (This)->lpVtbl -> CheckTextureGUIDs(This,pDesc,pNumGUIDs,pGUIDs) ) 

#define ID3D12Device_GUIDTextureLayout_CreateCommittedResource2(This,pHeapProperties,HeapFlags,pDesc,pOptimizedClearValue,pProtectedSession,riidResource,ppvResource)    \
    ( (This)->lpVtbl -> CreateCommittedResource2(This,pHeapProperties,HeapFlags,pDesc,pOptimizedClearValue,pProtectedSession,riidResource,ppvResource) ) 

#define ID3D12Device_GUIDTextureLayout_GetResourceDesc1(This,pResource)    \
    ( (This)->lpVtbl -> GetResourceDesc1(This,pResource) ) 

#endif /* COBJMACROS */


#endif     /* C style interface */




#endif     /* __ID3D12Device_GUIDTextureLayout_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_d3d12_1_0000_0015 */
/* [local] */ 

#endif /* WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP | WINAPI_PARTITION_GAMES) */
#pragma endregion
DEFINE_GUID(IID_ID3D12CryptoSession,0xFC7C6C9D,0xC27D,0x4904,0x83,0x5D,0xA5,0xF2,0x09,0x6E,0xC6,0x5F);
DEFINE_GUID(IID_ID3D12CryptoSessionPolicy,0x69FE3108,0x01A4,0x4AC3,0xAB,0x91,0xF5,0x1E,0x37,0x7A,0x62,0xAC);
DEFINE_GUID(IID_ID3D12ContentProtectionDevice,0x59975f53,0xbf5f,0x42f2,0xb8,0x4f,0x5e,0x34,0x7c,0x1e,0x3d,0x43);
DEFINE_GUID(IID_ID3D12TrackedWorkload,0x2C983A52,0xE1A0,0x468E,0xBB,0xE8,0x99,0x1D,0x80,0x90,0x1F,0x57);
DEFINE_GUID(IID_ID3D12VideoDecodeCommandList2,0x589A9607,0xDB1A,0x4573,0x92,0x04,0xA2,0xD9,0xA2,0x5B,0x93,0xBD);
DEFINE_GUID(IID_ID3D12VideoProcessCommandList2,0x6B69BFF8,0xE8C8,0x4F8C,0xB2,0xB4,0x6E,0x2F,0xC2,0x70,0x12,0x74);
DEFINE_GUID(IID_ID3D12VideoMotionEstimator,0x33FDAE0E,0x098B,0x428F,0x87,0xBB,0x34,0xB6,0x95,0xDE,0x08,0xF8);
DEFINE_GUID(IID_ID3D12VideoMotionVectorHeap,0x5BE17987,0x743A,0x4061,0x83,0x4B,0x23,0xD2,0x2D,0xAE,0xA5,0x05);
DEFINE_GUID(IID_ID3D12VideoDevice1,0x1BE98B1F,0x28C7,0x4E3C,0x97,0x4F,0x51,0xC5,0xFF,0x82,0x39,0x7D);
DEFINE_GUID(IID_ID3D12VideoEncodeCommandList,0xDF605487,0x4326,0x4E41,0xBF,0x7A,0x4C,0xEE,0xFA,0xF1,0x86,0x18);
DEFINE_GUID(IID_ID3D12DeviceRaytracingPrototype,0xf52ef3ca,0xf710,0x4ee4,0xb8,0x73,0xa7,0xf5,0x04,0xe4,0x39,0x95);
DEFINE_GUID(IID_ID3D12CommandListRaytracingPrototype,0x3c69787a,0x28fa,0x4701,0x97,0x0a,0x37,0xa1,0xed,0x1f,0x9c,0xab);
DEFINE_GUID(IID_ID3D12GraphicsCommandList5,0xF5E76B80,0xAB90,0x444B,0x92,0x8E,0x13,0xF4,0x73,0x6E,0xB9,0xC2);
DEFINE_GUID(IID_ID3D12DeviceTrackedWorkload,0xAE666F25,0xBBA9,0x4E7C,0xA4,0x72,0xBD,0x28,0x8B,0x15,0x41,0x40);
DEFINE_GUID(IID_ID3D12Device_GUIDTextureLayout,0x1066D110,0x10A2,0x434F,0x8D,0xAD,0xF4,0x00,0x20,0x12,0x20,0xD1);


extern RPC_IF_HANDLE __MIDL_itf_d3d12_1_0000_0015_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_d3d12_1_0000_0015_v0_0_s_ifspec;

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


