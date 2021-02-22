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
// Author:   James Stanard
//

#include "Model.h"
#include "Renderer.h"
#include "ConstantBuffers.h"

using namespace Math;
using namespace Renderer;

void Model::Destroy()
{
    m_BoundingSphere = BoundingSphere(kZero);
    m_DataBuffer.Destroy();
    m_MaterialConstants.Destroy();
    m_NumNodes = 0;
    m_NumMeshes = 0;
    m_MeshData = nullptr;
    m_SceneGraph = nullptr;
}

void Model::Render(
    MeshSorter& sorter,
    const GpuBuffer& meshConstants,
    const ScaleAndTranslation sphereTransforms[],
    const Joint* skeleton ) const
{
    // Pointer to current mesh
    const uint8_t* pMesh = m_MeshData.get();

    const Frustum& frustum = sorter.GetViewFrustum();
    const AffineTransform& viewMat = (const AffineTransform&)sorter.GetViewMatrix();

    for (uint32_t i = 0; i < m_NumMeshes; ++i)
    {
        const Mesh& mesh = *(const Mesh*)pMesh;

        const ScaleAndTranslation& sphereXform = sphereTransforms[mesh.meshCBV];
        BoundingSphere sphereLS((const XMFLOAT4*)mesh.bounds);
        BoundingSphere sphereWS = sphereXform * sphereLS;
        BoundingSphere sphereVS = BoundingSphere(viewMat * sphereWS.GetCenter(), sphereWS.GetRadius());

        if (frustum.IntersectSphere(sphereVS))
        {
            float distance = -sphereVS.GetCenter().GetZ() - sphereVS.GetRadius();
            sorter.AddMesh(mesh, distance,
                meshConstants.GetGpuVirtualAddress() + sizeof(MeshConstants) * mesh.meshCBV,
                m_MaterialConstants.GetGpuVirtualAddress() + sizeof(MaterialConstants) * mesh.materialCBV,
                m_DataBuffer.GetGpuVirtualAddress(), skeleton);
        }

        pMesh += sizeof(Mesh) + (mesh.numDraws - 1) * sizeof(Mesh::Draw);
    }
}

void ModelInstance::Render(MeshSorter& sorter) const
{
    if (m_Model != nullptr)
    {
        //const Frustum& frustum = sorter.GetWorldFrustum();
        m_Model->Render(sorter, m_MeshConstantsGPU, (const ScaleAndTranslation*)m_BoundingSphereTransforms.get(),
            m_Skeleton.get());
    }
}

ModelInstance::ModelInstance( std::shared_ptr<const Model> sourceModel )
    : m_Model(sourceModel), m_Locator(kIdentity)
{
    static_assert((_alignof(MeshConstants) & 255) == 0, "CBVs need 256 byte alignment");
    if (sourceModel == nullptr)
    {
        m_MeshConstantsCPU.Destroy();
        m_MeshConstantsGPU.Destroy();
        m_BoundingSphereTransforms = nullptr;
        m_AnimGraph = nullptr;
        m_AnimState.clear();
        m_Skeleton = nullptr;
    }
    else
    {
        m_MeshConstantsCPU.Create(L"Mesh Constant Upload Buffer", sourceModel->m_NumNodes * sizeof(MeshConstants));
        m_MeshConstantsGPU.Create(L"Mesh Constant GPU Buffer", sourceModel->m_NumNodes, sizeof(MeshConstants));
        m_BoundingSphereTransforms.reset(new __m128[sourceModel->m_NumNodes]);
        m_Skeleton.reset(new Joint[sourceModel->m_NumJoints]);

        if (sourceModel->m_NumAnimations > 0)
        {
            m_AnimGraph.reset(new GraphNode[sourceModel->m_NumNodes]);
            std::memcpy(m_AnimGraph.get(), sourceModel->m_SceneGraph.get(), sourceModel->m_NumNodes * sizeof(GraphNode));
            m_AnimState.resize(sourceModel->m_NumAnimations);
        }
        else
        {
            m_AnimGraph = nullptr;
            m_AnimState.clear();
        }
    }
}

ModelInstance::ModelInstance( const ModelInstance& modelInstance )
    : ModelInstance(modelInstance.m_Model)
{
}

ModelInstance& ModelInstance::operator=( std::shared_ptr<const Model> sourceModel )
{
    m_Model = sourceModel;
    m_Locator = UniformTransform(kIdentity);
    if (sourceModel == nullptr)
    {
        m_MeshConstantsCPU.Destroy();
        m_MeshConstantsGPU.Destroy();
        m_BoundingSphereTransforms = nullptr;
        m_AnimGraph = nullptr;
        m_AnimState.clear();
        m_Skeleton = nullptr;
    }
    else
    {
        m_MeshConstantsCPU.Create(L"Mesh Constant Upload Buffer", sourceModel->m_NumNodes * sizeof(MeshConstants));
        m_MeshConstantsGPU.Create(L"Mesh Constant GPU Buffer", sourceModel->m_NumNodes, sizeof(MeshConstants));
        m_BoundingSphereTransforms.reset(new __m128[sourceModel->m_NumNodes]);
        m_Skeleton.reset(new Joint[sourceModel->m_NumJoints]);

        if (sourceModel->m_NumAnimations > 0)
        {
            m_AnimGraph.reset(new GraphNode[sourceModel->m_NumNodes]);
            std::memcpy(m_AnimGraph.get(), sourceModel->m_SceneGraph.get(), sourceModel->m_NumNodes * sizeof(GraphNode));
            m_AnimState.resize(sourceModel->m_NumAnimations);
        }
        else
        {
            m_AnimGraph = nullptr;
            m_AnimState.clear();
        }
    }
    return *this;
}

void ModelInstance::Update(GraphicsContext& gfxContext, float deltaTime)
{
    if (m_Model == nullptr)
        return;

    static const size_t kMaxStackDepth = 32;

    size_t stackIdx = 0;
    Matrix4 matrixStack[kMaxStackDepth];
    Matrix4 ParentMatrix = Matrix4((AffineTransform)m_Locator);

    ScaleAndTranslation* boundingSphereTransforms = (ScaleAndTranslation*)m_BoundingSphereTransforms.get();
    MeshConstants* cb = (MeshConstants*)m_MeshConstantsCPU.Map();

    if (m_AnimGraph)
    {
        UpdateAnimations(deltaTime);

        for (uint32_t i = 0; i < m_Model->m_NumNodes; ++i)
        {
            GraphNode& node = m_AnimGraph[i];

            // Regenerate the 3x3 matrix if it has scale or rotation
            if (node.staleMatrix)
            {
                node.staleMatrix = false;
                node.xform.Set3x3(Matrix3(node.rotation) * Matrix3::MakeScale(node.scale));
            }
        }
    }

    const GraphNode* sceneGraph = m_AnimGraph ? m_AnimGraph.get() : m_Model->m_SceneGraph.get();

    // Traverse the scene graph in depth first order.  This is the same as linear order
    // for how the nodes are stored in memory.  Uses a matrix stack instead of recursion.
    for (const GraphNode* Node = sceneGraph; ; ++Node)
    {
        Matrix4 xform = Node->xform;
        if (!Node->skeletonRoot)
            xform = ParentMatrix * xform;

        // Concatenate the transform with the parent's matrix and update the matrix list
        {
            // Scoped so that I don't forget that I'm pointing to write-combined memory and
            // should not read from it.
            MeshConstants& cbv = cb[Node->matrixIdx];
            cbv.World = xform;
            cbv.WorldIT = InverseTranspose(xform.Get3x3());

            Scalar scaleXSqr = LengthSquare((Vector3)xform.GetX());
            Scalar scaleYSqr = LengthSquare((Vector3)xform.GetY());
            Scalar scaleZSqr = LengthSquare((Vector3)xform.GetZ());
            Scalar sphereScale = Sqrt(Max(Max(scaleXSqr, scaleYSqr), scaleZSqr));
            boundingSphereTransforms[Node->matrixIdx] = ScaleAndTranslation((Vector3)xform.GetW(), sphereScale);
        }

        // If the next node will be a descendent, replace the parent matrix with our new matrix
        if (Node->hasChildren)
        {
            // ...but if we have siblings, make sure to backup our current parent matrix on the stack
            if (Node->hasSibling)
            {
                ASSERT(stackIdx < kMaxStackDepth, "Overflowed the matrix stack");
                matrixStack[stackIdx++] = ParentMatrix;
            }
            ParentMatrix = xform;
        }
        else if (!Node->hasSibling)
        {
            // There are no more siblings.  If the stack is empty, we are done.  Otherwise, pop
            // a matrix off the stack and continue.
            if (stackIdx == 0)
                break;

            ParentMatrix = matrixStack[--stackIdx];
        }
    }

    // Update skeletal joints
    for (uint32_t i = 0; i < m_Model->m_NumJoints; ++i)
    {
        Joint& joint = m_Skeleton[i];
        joint.posXform = cb[m_Model->m_JointIndices[i]].World * m_Model->m_JointIBMs[i];
        joint.nrmXform = InverseTranspose(joint.posXform.Get3x3());
    }

    m_MeshConstantsCPU.Unmap();

    gfxContext.TransitionResource(m_MeshConstantsGPU, D3D12_RESOURCE_STATE_COPY_DEST, true);
    gfxContext.GetCommandList()->CopyBufferRegion(m_MeshConstantsGPU.GetResource(), 0, m_MeshConstantsCPU.GetResource(), 0, m_MeshConstantsCPU.GetBufferSize());
    gfxContext.TransitionResource(m_MeshConstantsGPU, D3D12_RESOURCE_STATE_GENERIC_READ);
}

void ModelInstance::Resize( float newRadius )
{
    if (m_Model == nullptr)
        return;

    m_Locator.SetScale(newRadius / m_Model->m_BoundingSphere.GetRadius());
}

Vector3 ModelInstance::GetCenter() const
{
    if (m_Model == nullptr)
        return Vector3(kOrigin);

    return m_Locator * m_Model->m_BoundingSphere.GetCenter();
}

Scalar ModelInstance::GetRadius() const
{
    if (m_Model == nullptr)
        return Scalar(kZero);

    return m_Locator.GetScale() * m_Model->m_BoundingSphere.GetRadius();
}

Math::BoundingSphere ModelInstance::GetBoundingSphere() const
{
    if (m_Model == nullptr)
        return BoundingSphere(kZero);

    return m_Locator * m_Model->m_BoundingSphere;
}

Math::OrientedBox ModelInstance::GetBoundingBox() const
{
    if (m_Model == nullptr)
        return AxisAlignedBox(Vector3(kZero), Vector3(kZero));

    return m_Locator * m_Model->m_BoundingBox;
}
