/////////////////////////////////////////////////////////////////////
// Filename: IHandler.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "Command.h"


namespace CORETOOLS
{
	/// Abstract version of a handler with variadic args
	template<typename TReturn, typename ... TParams>
	class IHandler {
	public:
		virtual TReturn Handle(TParams ...) = 0;
	};

	/// Abstract text handler class definition
	class ITextHandler : public IHandler<bool, wchar_t const*>
	{
	public:
		virtual bool Handle(wchar_t const*) override = 0;

		::CORETOOLS::CmdList GetCommands() const { return i_commands; }

	protected:
		::CORETOOLS::CmdList i_commands;
	};
}