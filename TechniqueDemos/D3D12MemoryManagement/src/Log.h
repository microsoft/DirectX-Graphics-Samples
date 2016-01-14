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

//
// The severity of a message being logged.
//
enum LogSeverity
{
	ELS_Message,
	ELS_Warning,
	ELS_Error,
	ELS_Critical,

	_ELS_COUNT
};

//
// Logs a formatted message to the command prompt with the specified severity.
//
void LogMessage(_In_range_(0, _ELS_COUNT-1) LogSeverity Severity, LPCSTR pFormat, ...);

#define LOG_MESSAGE(Format, ...) LogMessage(ELS_Message, Format, __VA_ARGS__)
#define LOG_WARNING(Format, ...) LogMessage(ELS_Warning, Format, __VA_ARGS__)
#define LOG_ERROR(Format, ...) LogMessage(ELS_Error, Format, __VA_ARGS__)
#define LOG_CRITICAL_ERROR(Format, ...) LogMessage(ELS_Critical, Format, __VA_ARGS__)
