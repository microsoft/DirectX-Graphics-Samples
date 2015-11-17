//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once

inline void InitializeListHead(LIST_ENTRY* pHead)
{
	pHead->Flink = pHead->Blink = pHead;
}

inline void InsertHeadList(LIST_ENTRY* pHead, LIST_ENTRY* pEntry)
{
	pEntry->Blink = pHead;
	pEntry->Flink = pHead->Flink;

	pHead->Flink->Blink = pEntry;
	pHead->Flink = pEntry;
}

inline void InsertTailList(LIST_ENTRY* pHead, LIST_ENTRY* pEntry)
{
	pEntry->Flink = pHead;
	pEntry->Blink = pHead->Blink;

	pHead->Blink->Flink = pEntry;
	pHead->Blink = pEntry;
}

inline void RemoveEntryList(LIST_ENTRY* pEntry)
{
	pEntry->Blink->Flink = pEntry->Flink;
	pEntry->Flink->Blink = pEntry->Blink;
}

inline LIST_ENTRY* RemoveHeadList(LIST_ENTRY* pHead)
{
	LIST_ENTRY* pEntry = pHead->Flink;
	RemoveEntryList(pEntry);
	return pEntry;
}

inline LIST_ENTRY* RemoveTailList(LIST_ENTRY* pHead)
{
	LIST_ENTRY* pEntry = pHead->Blink;
	RemoveEntryList(pEntry);
	return pEntry;
}

inline bool IsListEmpty(LIST_ENTRY* pEntry)
{
	return pEntry->Flink == pEntry;
}
