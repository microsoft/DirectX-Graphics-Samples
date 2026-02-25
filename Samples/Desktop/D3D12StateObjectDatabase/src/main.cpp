//
//  Sample using ID3D12StateObjectDatabase to write an SODB
//

#include "RayTracing.h"
#include "RootSignatureVS.h"
#include "RootSignaturePS.h"
#include "NoRootSignatureVS.h"
#include "NoRootSignaturePS.h"
#include "PositionColorVS.h"
#include "PositionColorPS.h"

#include <initguid.h> 
#include <d3d12.h>
#include <D3dx12.h>
#include <wil/result.h>
#include <string>

// Enable D3D12 agility SDK.
extern "C" { _declspec(dllexport) extern const UINT D3D12SDKVersion = D3D12_SDK_VERSION; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = u8".\\D3D12\\"; }

// Example for setting the application description
void SetApplicationDescription(ID3D12StateObjectDatabase* pStateObjectDatabase);

// Example of storing a Pipeline State Object where the root signature is embeded in the shaders.
void StorePSO(ID3D12StateObjectDatabase* pStateObjectDatabase);

// Example of storing a Pipeline StateObject where the root signatures are defined external to the shaders.
void StorePSOExternalSerializedRootSignature(ID3D12StateObjectDatabase* pStateObjectDatabase);

// Store a ray tracing object that uses AddToStateObject
void StoreAddToStateObjectRaytracing(ID3D12StateObjectDatabase* pStateObjectDatabase);

// State object with generic program using an existing collection for shaders
void StoreGenericProgramWithExistingCollection(ID3D12StateObjectDatabase* pStateObjectDatabase);

//----------------------------------------------------------------------------------------------------------------------------------
int main() try
{
    // Obtain a State Object Database Factory instance
    Microsoft::WRL::ComPtr<ID3D12StateObjectDatabaseFactory> spStateObjectDatabaseFactory;
    THROW_IF_FAILED(D3D12GetInterface(CLSID_D3D12StateObjectFactory, IID_PPV_ARGS(&spStateObjectDatabaseFactory)));

    // Define an output file.
    std::wstring outputFile(L"output.sodb");

    // Delete the file if it already exists.
    DeleteFile(outputFile.c_str());

    // Open the State Object Database (SODB)
    Microsoft::WRL::ComPtr<ID3D12StateObjectDatabase> spStateObjectDatabase;
    THROW_IF_FAILED(spStateObjectDatabaseFactory->CreateStateObjectDatabaseFromFile(outputFile.c_str(), D3D12_STATE_OBJECT_DATABASE_FLAG_NONE, IID_PPV_ARGS(&spStateObjectDatabase)));

    // Set the Application Description
    SetApplicationDescription(spStateObjectDatabase.Get());

    // Examples for storing Pipeline State Objects (PSOs).  Input is a D3D12_PIPELINE_STATE_STREAM_DESC, 
    // similar to ID3D12Device2::CreatePipelineState can be stored.  The one difference is root 
    // signatures.  Root signatures must use a serialized form instead of ID3D12RootSignature.

    // Example storing a PSO where the serialized root signature is embedded within the shaders.
    StorePSO(spStateObjectDatabase.Get());

    // Sample storing a PSO where the serialized root signature is defined external to the shaders.
    StorePSOExternalSerializedRootSignature(spStateObjectDatabase.Get());

    // Examples for storing State Objects (SOs). Input is similar to StoreStateObject and AddToStateObject
    // Except that:
    //     - Root Signatures must be Serialized
    //     - Referring to parent SOs or Existing SOs is done by key instead of by API object

    // Sample storing a ray tracing SO including using AddToStateObject.
    StoreAddToStateObjectRaytracing(spStateObjectDatabase.Get());

    // Sample storing a generic program that references a collection
    StoreGenericProgramWithExistingCollection(spStateObjectDatabase.Get());
}
CATCH_RETURN();

// Example for setting the application description
void SetApplicationDescription(ID3D12StateObjectDatabase* pStateObjectDatabase)
{
    // Initialize a sample D3D12_APPLICATION_DESC.  This information is used to identify the application during offline compilation.
    D3D12_APPLICATION_DESC applicationDesc = {};
    applicationDesc.pExeFilename = L"Game.exe";
    applicationDesc.pName = L"Game Title";
    applicationDesc.Version.Version = 0x0001000000000000; // 1.0.0.0
    applicationDesc.pEngineName = L"Test Game Engine";
    applicationDesc.EngineVersion.Version = 0x0005000000000000; // 5.0.0.0

    THROW_IF_FAILED(pStateObjectDatabase->SetApplicationDesc(&applicationDesc));
}

//----------------------------------------------------------------------------------------------------------------------------------
void StorePSO(ID3D12StateObjectDatabase* pStateObjectDatabase)
{
    // Define the input layout
    const D3D12_INPUT_ELEMENT_DESC InputElementDescsDescs[1] =
    {
        { "POS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    const D3D12_INPUT_LAYOUT_DESC inputLayout = { InputElementDescsDescs, _countof(InputElementDescsDescs) };

    // Declare the stream desc for the pipeline state.
    struct PSO_STREAM
    {
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_VS VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS PS;
    }
    PSOStream =
    {
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
        inputLayout,
        CD3DX12_SHADER_BYTECODE(g_RootSignatureVS, _countof(g_RootSignatureVS)),
        CD3DX12_SHADER_BYTECODE(g_RootSignaturePS, _countof(g_RootSignaturePS))
    };

    D3D12_PIPELINE_STATE_STREAM_DESC StreamDesc{ sizeof(PSOStream), &PSOStream };

    // Define a unique identifier for the pipeline state object.
    const char keyName[] = "StateObjectDatabase_Dxil_VSPS";
    const UINT keySize = sizeof(keyName);
    UINT version = 1u;

    // Store the pipeline state in the state object database.
    THROW_IF_FAILED(pStateObjectDatabase->StorePipelineStateDesc(keyName, keySize, version, &StreamDesc));
}

//----------------------------------------------------------------------------------------------------------------------------------
void StorePSOExternalSerializedRootSignature(ID3D12StateObjectDatabase* pStateObjectDatabase)
{
    // Initialize the serialized root signature
    CD3DX12_DESCRIPTOR_RANGE1 DescRange[] =
    {
        {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0},    // t0
        {D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 2, 0},    // u0-u1
        {D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 2, 0} // s0-s1
    };

    CD3DX12_ROOT_PARAMETER1 rootParameters[_countof(DescRange)];
    for (UINT i = 0; i < _countof(rootParameters); ++i)
    {
        rootParameters[i].InitAsDescriptorTable(1u, &DescRange[i], D3D12_SHADER_VISIBILITY_ALL);
    }

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc(_countof(rootParameters), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT);

    Microsoft::WRL::ComPtr <ID3DBlob> signature;
    Microsoft::WRL::ComPtr <ID3DBlob> error;
    THROW_IF_FAILED(D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error));

    D3D12_SERIALIZED_ROOT_SIGNATURE_DESC serializedRootSignatureDesc = {};
    serializedRootSignatureDesc.pSerializedBlob = signature->GetBufferPointer();
    serializedRootSignatureDesc.SerializedBlobSizeInBytes = signature->GetBufferSize();

    // Define the input layout
    const D3D12_INPUT_ELEMENT_DESC InputElementDescsDescs[1] =
    {
        { "POS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    const D3D12_INPUT_LAYOUT_DESC inputLayout = { InputElementDescsDescs, _countof(InputElementDescsDescs) };

    // Declare the stream desc for the pipeline state.
    struct PSO_STREAM
    {
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_VS VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS PS;
        CD3DX12_PIPELINE_STATE_STREAM_SERIALIZED_ROOT_SIGNATURE RootSigDesc;

    } PSOStream =
    {
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
        inputLayout,
        CD3DX12_SHADER_BYTECODE(g_NoRootSignatureVS, _countof(g_NoRootSignatureVS)),
        CD3DX12_SHADER_BYTECODE(g_NoRootSignaturePS, _countof(g_NoRootSignaturePS)),
        CD3DX12_SERIALIZED_ROOT_SIGNATURE_DESC{signature->GetBufferPointer(), signature->GetBufferSize()}
    };

    D3D12_PIPELINE_STATE_STREAM_DESC StreamDesc{ sizeof(PSOStream), &PSOStream };

    // Define a unique identifier for the pipeline state object.
    const char keyName[] = "StateObjectDatabase_Dxil_VSPS_ExternalRootSignature";
    const UINT keySize = sizeof(keyName);
    UINT version = 1u;

    // Store the pipeline state in the state object database.
    THROW_IF_FAILED(pStateObjectDatabase->StorePipelineStateDesc(keyName, keySize, version, &StreamDesc));
}

Microsoft::WRL::ComPtr<ID3DBlob> CreateRootSig(bool bLocalRootSig, UINT space)
{
    CD3DX12_DESCRIPTOR_RANGE dr[4];
    dr[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 5, 0, space);
    dr[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0, space);
    dr[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 5, 0, space);
    dr[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 5, 0, space);
    CD3DX12_ROOT_PARAMETER rp[5];
    rp[0].InitAsDescriptorTable(3, dr);
    rp[1].InitAsDescriptorTable(1, &dr[3]);
    rp[2].InitAsConstantBufferView(5, space);
    rp[3].InitAsUnorderedAccessView(5, space);
    rp[4].InitAsShaderResourceView(5, space);
    CD3DX12_STATIC_SAMPLER_DESC ss(5 + (bLocalRootSig ? 1 : 0));
    CD3DX12_ROOT_SIGNATURE_DESC rs;
    rs.Init(5, rp, 1, &ss, bLocalRootSig ? D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE : D3D12_ROOT_SIGNATURE_FLAG_NONE);

    Microsoft::WRL::ComPtr<ID3DBlob> spRSBlob;
    THROW_IF_FAILED(D3D12SerializeRootSignature(&rs, D3D_ROOT_SIGNATURE_VERSION_1, &spRSBlob, NULL));
    return spRSBlob;
}

//----------------------------------------------------------------------------------------------------------------------------------
void StoreAddToStateObjectRaytracing(ID3D12StateObjectDatabase* pStateObjectDatabase)
{
    // Store a parent Raytracing Pipeline
    CD3DX12_STATE_OBJECT_DESC StartedEmpty(D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE);
    auto Flags = StartedEmpty.CreateSubobject<CD3DX12_STATE_OBJECT_CONFIG_SUBOBJECT>();
    Flags->SetFlags(D3D12_STATE_OBJECT_FLAG_ALLOW_STATE_OBJECT_ADDITIONS);

    auto ShaderConfig = StartedEmpty.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
    ShaderConfig->Config(256, 32);

    auto RaytracingConfig = StartedEmpty.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
    RaytracingConfig->Config(8);

    const char StartedEmptyObjectName[] = "SODB_StartedEmptyRaytracingPipeline";
    UINT StartedEmptyKeySize = sizeof(StartedEmptyObjectName);
    UINT StartedEmptyObjectVersion = 1u;
    const D3D12_STATE_OBJECT_DESC* pStartedEmpty = StartedEmpty;
    THROW_IF_FAILED(pStateObjectDatabase->StoreStateObjectDesc(StartedEmptyObjectName, StartedEmptyKeySize, StartedEmptyObjectVersion, pStartedEmpty, nullptr, 0u));

    // Store an Addition derived from the parent above.
    Microsoft::WRL::ComPtr<ID3DBlob> spLocalRootSigBlob = CreateRootSig(true, 0);
    Microsoft::WRL::ComPtr<ID3DBlob> spGlobalRootSigBlob = CreateRootSig(false, 1);

    CD3DX12_STATE_OBJECT_DESC Addition(D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE);
    auto Flags2 = Addition.CreateSubobject<CD3DX12_STATE_OBJECT_CONFIG_SUBOBJECT>();
    Flags2->SetFlags(D3D12_STATE_OBJECT_FLAG_ALLOW_STATE_OBJECT_ADDITIONS);

    D3D12_SHADER_BYTECODE MyAppDxilLib =
        CD3DX12_SHADER_BYTECODE(g_RayTracingLib, sizeof(g_RayTracingLib));

    auto Lib0 = Addition.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
    Lib0->SetDXILLibrary(&MyAppDxilLib);
    Lib0->DefineExport(L"raygen_mainColNoAdds", L"raygen_main");

    D3D12_SERIALIZED_ROOT_SIGNATURE_DESC serializedGlobalRootSignatureDesc = {};
    serializedGlobalRootSignatureDesc.pSerializedBlob = spGlobalRootSigBlob->GetBufferPointer();
    serializedGlobalRootSignatureDesc.SerializedBlobSizeInBytes = spGlobalRootSigBlob->GetBufferSize();
    auto GlobalRootSig = Addition.CreateSubobject<CD3DX12_GLOBAL_SERIALIZED_ROOT_SIGNATURE_SUBOBJECT>();
    GlobalRootSig->SetRootSignature(&serializedGlobalRootSignatureDesc);

    D3D12_SERIALIZED_ROOT_SIGNATURE_DESC serializedLocalRootSignatureDesc = {};
    serializedLocalRootSignatureDesc.pSerializedBlob = spLocalRootSigBlob->GetBufferPointer();
    serializedLocalRootSignatureDesc.SerializedBlobSizeInBytes = spLocalRootSigBlob->GetBufferSize();
    auto LocalRootSig = Addition.CreateSubobject<CD3DX12_LOCAL_SERIALIZED_ROOT_SIGNATURE_SUBOBJECT>();
    LocalRootSig->SetRootSignature(&serializedLocalRootSignatureDesc);

    auto ShaderConfig2 = Addition.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
    ShaderConfig2->Config(256, 32);

    auto RaytracingConfig2 = Addition.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
    RaytracingConfig2->Config(8);

    // Add the derived state object.  The parent object must be indicated by its key.
    const char AddToEmptyObjectName[] = "SODB_AddToEmptyRaytracingPipeline";
    UINT AddToKeySize = sizeof(StartedEmptyObjectName);
    UINT AddToObjectVersion = 1u;
    const D3D12_STATE_OBJECT_DESC* pAddToEmpty = Addition;
    THROW_IF_FAILED(pStateObjectDatabase->StoreStateObjectDesc(AddToEmptyObjectName, AddToKeySize, AddToObjectVersion, pStartedEmpty, StartedEmptyObjectName, StartedEmptyKeySize));
}

//----------------------------------------------------------------------------------------------------------------------------------
void StoreGenericProgramWithExistingCollection(ID3D12StateObjectDatabase* pStateObjectDatabase)
{
    // Shaders use root signatures specified in the shader code.

    // Define vertex input layout
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // Create collection
    CD3DX12_STATE_OBJECT_DESC collectionStateObjectDesc(D3D12_STATE_OBJECT_TYPE_COLLECTION);

    // Shaders use root signatures specified in the shader code.
    auto vsBytecode = CD3DX12_SHADER_BYTECODE(g_PositionColorVS, sizeof(g_PositionColorVS));
    auto pVS = collectionStateObjectDesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
    pVS->SetDXILLibrary(&vsBytecode);
    pVS->DefineExport(L"VSMain", L"*");

    auto psBytecode = CD3DX12_SHADER_BYTECODE(g_PositionColorPS, sizeof(g_PositionColorPS));
    auto pPS = collectionStateObjectDesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
    pPS->SetDXILLibrary(&psBytecode);
    pPS->DefineExport(L"PSMain", L"*");

    const char collectionKey[] = "SODB_CreateStateObjectCollection";
    UINT collectionKeySize = sizeof(collectionKey);
    UINT collectionVersion = 1u; 
    THROW_IF_FAILED(pStateObjectDatabase->StoreStateObjectDesc(collectionKey, collectionKeySize, collectionVersion, collectionStateObjectDesc, nullptr, 0u));

    // Create executable state object with programs using the collection.
    CD3DX12_STATE_OBJECT_DESC executableStateObjectDesc(D3D12_STATE_OBJECT_TYPE_EXECUTABLE);

    auto pPrimitiveTopology = executableStateObjectDesc.CreateSubobject<CD3DX12_PRIMITIVE_TOPOLOGY_SUBOBJECT>();
    pPrimitiveTopology->SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

    auto pIL = executableStateObjectDesc.CreateSubobject<CD3DX12_INPUT_LAYOUT_SUBOBJECT>();

    for (UINT i = 0; i < _countof(inputElementDescs); i++)
    {
        pIL->AddInputLayoutElementDesc(inputElementDescs[i]);
    }

    auto pBlendState = executableStateObjectDesc.CreateSubobject<CD3DX12_BLEND_SUBOBJECT>();
    pBlendState->SetAlphaToCoverageEnable(true);

    auto pRTFormats = executableStateObjectDesc.CreateSubobject<CD3DX12_RENDER_TARGET_FORMATS_SUBOBJECT>();
    pRTFormats->SetNumRenderTargets(1);
    pRTFormats->SetRenderTargetFormat(0, DXGI_FORMAT_R8G8B8A8_UNORM);

    // Refer to the existing collection by key instead of API object.
    auto pCollection = executableStateObjectDesc.CreateSubobject<CD3DX12_EXISTING_COLLECTION_BY_KEY_SUBOBJECT>();
    pCollection->SetExistingCollection(collectionKey, collectionKeySize);

    auto pGenericProgram = executableStateObjectDesc.CreateSubobject<CD3DX12_GENERIC_PROGRAM_SUBOBJECT>();
    pGenericProgram->SetProgramName(L"testProgram1");
    pGenericProgram->AddExport(L"VSMain");
    pGenericProgram->AddExport(L"PSMain");
    pGenericProgram->AddSubobject(*pPrimitiveTopology);
    pGenericProgram->AddSubobject(*pBlendState);
    pGenericProgram->AddSubobject(*pIL);
    pGenericProgram->AddSubobject(*pRTFormats);

    const char objectKey[] = "SODB_CreateStateObjectGP";
    UINT objectKeySize = sizeof(objectKey);
    UINT Version = 1u;
    THROW_IF_FAILED(pStateObjectDatabase->StoreStateObjectDesc(objectKey, objectKeySize, Version, executableStateObjectDesc, nullptr, 0));
}