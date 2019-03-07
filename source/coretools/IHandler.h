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
	class ITextHandler : public IHandler<bool, EEcstr>
	{
	public:
		virtual bool Handle(EEcstr) override = 0;

		::CORETOOLS::CmdList GetCommands() const { return i_commands; }

	protected:
		::CORETOOLS::CmdList i_commands;
	};
}