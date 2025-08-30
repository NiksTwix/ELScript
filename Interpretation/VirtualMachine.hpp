#pragma once
#include <Interpretation\CommandHandlers.hpp>
#include <Definitions\Execution.hpp>
#include <functional>


namespace ELScript 
{
	class VirtualMachine 
	{
	private:
		void ProcessCommand(ExecutionChain& chain)
		{
			Command command = chain.commands[chain.current_rip];
			if (command.code == OpCode::PUSH)			CommandHandlers::H_PUSH(command, chain);
			else if (command.code == OpCode::POP)		CommandHandlers::H_POP(command, chain);
			else if (command.code == OpCode::ADD)		CommandHandlers::H_ADD(command, chain);
			else if (command.code == OpCode::SUB)		CommandHandlers::H_SUB(command, chain);
			else if (command.code == OpCode::MUL)		CommandHandlers::H_MUL(command, chain);
			else if (command.code == OpCode::DIV)		CommandHandlers::H_DIV(command, chain);
			else if (command.code == OpCode::NEG)		CommandHandlers::H_NEG(command, chain);
			else if (command.code == OpCode::MOD)		CommandHandlers::H_MOD(command, chain);
			else if (command.code == OpCode::PRINT)		CommandHandlers::H_PRINT(command, chain);
			else if (command.code == OpCode::LOAD)		CommandHandlers::H_LOAD(command, chain);
			else if (command.code == OpCode::STORE)		CommandHandlers::H_STORE(command, chain);
			else if (command.code == OpCode::DECLARE)	CommandHandlers::H_DECLARE(command, chain);
			else if (command.code == OpCode::DECLARED)	CommandHandlers::H_DECLARED(command, chain);
			else if (command.code == OpCode::JMPR_IF || command.code == OpCode::JMPA_IF)		CommandHandlers::H_JMP_IF(command, chain);
			else if (command.code == OpCode::JMPR_IF_N || command.code == OpCode::JMPA_IF_N)	CommandHandlers::H_JMP_IF_N(command, chain);
			else if (command.code == OpCode::JMPR || command.code == OpCode::JMPA)				CommandHandlers::H_JMP(command, chain);
			else if (command.code == OpCode::GREATER)	CommandHandlers::H_GREATER(command, chain);
			else if (command.code == OpCode::LESS)		CommandHandlers::H_LESS(command, chain);
			else if (command.code == OpCode::EQUAL)		CommandHandlers::H_EQUAL(command, chain);
			else if (command.code == OpCode::EGREATER)  CommandHandlers::H_EGREATER(command, chain);
			else if (command.code == OpCode::ELESS)		CommandHandlers::H_ELESS(command, chain);
			else if (command.code == OpCode::EQUAL_N)	CommandHandlers::H_EQUAL_N(command, chain);
			else if (command.code == OpCode::AND)		CommandHandlers::H_AND(command, chain);
			else if (command.code == OpCode::OR)		CommandHandlers::H_OR(command, chain);
			else if (command.code == OpCode::NOT)		CommandHandlers::H_NOT(command, chain);
			else if (command.code == OpCode::CALL)		CommandHandlers::H_CALL(command, chain);
			else if (command.code == OpCode::RET)		CommandHandlers::H_RET(command, chain);
			else if (command.code == OpCode::CONVERT_TYPE)		CommandHandlers::H_CONVERT_TYPE(command, chain);
			else if (command.code == OpCode::GET_INDEX)			CommandHandlers::H_GET_INDEX(command, chain);
			else if (command.code == OpCode::SET_INDEX)			CommandHandlers::H_SET_INDEX(command, chain);
			else if (command.code == OpCode::ARRAY_SIZE || command.code == OpCode::ARRAY_POP_BACK || command.code == OpCode::ARRAY_PUSH_BACK || command.code == OpCode::ARRAY_INSERT_INDEX || command.code == OpCode::ARRAY_ERASE_INDEX)  CommandHandlers::H_ARRAY_OPERATIONS(command, chain);
			else if (command.code == OpCode::SCOPESTR)
			{
				chain.variables.push_back({});
			}
			else if (command.code == OpCode::SCOPEEND) {
				int target_depth = (int)command.operand.numberVal + (chain.depth_call_stack.empty() ? 0: chain.depth_call_stack.top());
				// Удаляем все scope'ы, пока не дойдём до целевой глубины
				while (chain.variables.size() > target_depth && chain.variables.size() > 1) {
					chain.variables.pop_back();
				}
			}
		}

		bool error_catch = false;
		EHID eh_id = InvalidEHID;
		ExecutionChain* current_echain;
	public:
		VirtualMachine() 
		{
			eh_id = ErrorHandlerManager::Register([&](const EHMessage message)
				{
					if (current_echain != nullptr && current_echain->GetID() == message.script) 
					{
						Logger::Get().Log(message.description);
						error_catch = true;
					}
				});
		}
	
		void Execute(ExecutionChain& chain)
		{
			ExecuteFrom(chain, 0);
		}

		void ExecuteFrom(ExecutionChain& chain, int start_rip)
		{
			int rip = start_rip;
			error_catch = false;
			chain.state = ECState::ACTIVE;
			current_echain = &chain;
			while (rip < chain.commands.size())
			{
				if (rip < 0)
				{
					rip = 0;
					ErrorHandlerManager::RaiseError(EHMessage(chain.id, chain.commands[rip], rip, EHMessageType::Warning, "[VM] Warning: rip was moved to position less than zero."));
				}
				if (chain.commands[rip].code == OpCode::EXIT)
				{
					chain.state = ECState::ENDED;
					break;
				}
				chain.current_rip = rip;
				ProcessCommand(chain);
				// Проверяем, изменился ли rip внутри ProcessCommand

				if (error_catch) 
				{
					chain.state = ECState::ERROR;
					break;
				}
				if (chain.current_rip != rip)
				{
					rip = chain.current_rip;
					continue;
				}
				rip++;
			}
		}
	};

}