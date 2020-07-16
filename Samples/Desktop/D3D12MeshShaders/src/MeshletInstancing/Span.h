//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
#pragma once

#include <cstdint>

// Encapsulates a typed array view into a blob of data.
template <typename T>
class Span
{
public:
    Span()
        : m_data(nullptr)
        , m_count(0)
    { }

    Span(T* data, uint32_t count)
        : m_data(data)
        , m_count(count)
    { }

    // std library container interface
    T* data() { return m_data; }
    const T* data() const { return m_data; }

    T& back() { return *(m_data + m_count - 1); }
    const T& back() const { return *(m_data + m_count - 1); }

    size_t size() const { return m_count; }

    // Iterator interface
    T* begin() { return m_data; }
    T* end() { return m_data + m_count; }

    T& operator[](uint32_t i) { return *(m_data + i); }
    const T& operator[](uint32_t i) const { return *(m_data + i); }

private:
    T* m_data;
    uint32_t m_count;
};

template <typename T>
Span<T> MakeSpan(T* data, uint32_t size) { return Span<T>(data, size); }
