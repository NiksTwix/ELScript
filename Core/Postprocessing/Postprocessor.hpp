#pragma once
#include "..\Definitions\CommandsInfo.hpp"


namespace ELScript 
{
	class Postprocessor 
	{
	public:
		static std::unordered_map<std::string, FunctionData> GetFunctionTable(std::vector<Command>& commands)
		{
			std::unordered_map<std::string, FunctionData> function_name_rip;

			for (int rip = 0; rip < commands.size(); rip++) 
			{
				Command& c = commands[rip];

				if (c.code == OpCode::CALL) 
				{
					if (rip < commands.size() - 1 && commands[rip + 1].operand.strVal == "#op_end") 
					{
						commands[rip + 1].code = OpCode::POP;
						commands[rip + 1].operand.strVal = "";
						continue;
					}
				}

				if (c.code == OpCode::FUNC_DECLARE) 
				{
					if (function_name_rip.count(c.operand.strVal)) 
					{
						Logger::Get().Log("[Postprocessor] Function with name '"+ c.operand.strVal + "' already exists. RIP: " + std::to_string(rip));
						continue;
					}
					function_name_rip[c.operand.strVal].identifier = c.operand.strVal;
					function_name_rip[c.operand.strVal].start_rip = rip;

					std::vector<std::string> args;

					for (int urip = rip; urip < commands.size(); urip++) 
					{
						if (commands[urip].code == OpCode::NOP && commands[urip].operand.strVal == "#args_end") 
						{
							rip = urip;
							function_name_rip[c.operand.strVal].arguments = args;
							break;
						}
						if (commands[urip].code == OpCode::DECLARE) 
						{
							args.push_back(commands[urip].operand.strVal);
						}
					}
				}
			}

			return function_name_rip;
		}
		static void CalculateJMPs(std::vector<Command>& commands)
		{
			std::unordered_map<std::string, int> target_name_rip;
			
			for (int rip = 0; rip < commands.size(); rip++)	//Собираем все метки
			{
				Command c = commands[rip];

				if (c.code == OpCode::LABEL) 
				{
					if (target_name_rip.count(c.operand.strVal))
					{
						Logger::Get().Log("[Postprocessor] Label '" + c.operand.strVal + "' already exists. RIP: " + std::to_string(rip));
						continue;
					}
					target_name_rip[c.operand.strVal] = rip;
				}
			}

			for (int rip = 0; rip < commands.size(); rip++)	//Собираем все метки
			{
				Command c = commands[rip];

				if (c.code == OpCode::JMPA || c.code == OpCode::JMPA_IF || c.code == OpCode::JMPA_IF_N)
				{
					if (!target_name_rip.count(c.operand.strVal)) 
					{
						Logger::Get().Log("[Postprocessor] invalid label. RIP: " + std::to_string(rip));
						continue;
					}
					commands[rip].operand = target_name_rip[c.operand.strVal];
				}
			}
		}
	};

}