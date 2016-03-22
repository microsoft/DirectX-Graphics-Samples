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

#include "stdafx.h"
#include <stdio.h>

#define DISABLE_LOGGING 0

static LPCSTR g_SeverityStrings[_ELS_COUNT] =
{
	"Message",
	"Warning",
	"Error",
	"Critical Error"
};

_Use_decl_annotations_
void LogMessage(LogSeverity Severity, LPCSTR pFormat, ...)
{
	assert(Severity < _ELS_COUNT);

#if(!DISABLE_LOGGING)
	va_list Args;
	va_start(Args, pFormat);

	printf("%s: ", g_SeverityStrings[Severity]);
	vprintf(pFormat, Args);
	printf("\n");
#endif

#if(_DEBUG)
	if (Severity >= ELS_Error)
	{
		__debugbreak();
	}
#endif

	if (Severity == ELS_Critical)
	{
		ExitProcess(0);
	}
}
