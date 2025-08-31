#pragma once
#include <filesystem>
#include "..\Definitions\Execution.hpp"

namespace ELScript 
{
	struct Script 
	{
		std::string name;
		std::filesystem::path scriptPath;
		ExecutionChain execution_chain;
	};
}