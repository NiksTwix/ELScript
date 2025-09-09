#pragma once
#include "CommandHandlers.hpp"
#include "..\Definitions\Execution.hpp"
#include <functional>


namespace ELScript
{
	class VirtualMachine
	{
		using handler = void(*)(Command&, ExecutionChain&);
		std::unordered_map<OpCode, handler> handlers;
	private:

		void HandlersMapInitialization()
		{
			// Заполняем все элементы карты обработчиком по умолчанию на случай неизвестных opcode

			// Инициализируем конкретные обработчики
			handlers[OpCode::NOP] = [](Command&, ExecutionChain&) {};
			handlers[OpCode::LABEL] = [](Command&, ExecutionChain&) {};
			handlers[OpCode::PUSH] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_PUSH(command, chain); };
			handlers[OpCode::SWAP] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_SWAP(command, chain); };
			handlers[OpCode::POP] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_POP(command, chain); };
			handlers[OpCode::ADD] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_ADD(command, chain); };
			handlers[OpCode::SUB] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_SUB(command, chain); };
			handlers[OpCode::MUL] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_MUL(command, chain); };
			handlers[OpCode::DIV] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_DIV(command, chain); };
			handlers[OpCode::NEG] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_NEG(command, chain); };
			handlers[OpCode::MOD] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_MOD(command, chain); };
			handlers[OpCode::PRINT] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_PRINT(command, chain); };
			handlers[OpCode::LOAD] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_LOAD(command, chain); };
			handlers[OpCode::LOADM] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_LOADM(command, chain); };
			handlers[OpCode::STORE] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_STORE(command, chain); };
			handlers[OpCode::DECLARE] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_DECLARE(command, chain); };
			handlers[OpCode::DECLARED] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_DECLARED(command, chain); };
			handlers[OpCode::JMPA_IF] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_JMP_IF(command, chain); };
			handlers[OpCode::JMPA_IF_N] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_JMP_IF_N(command, chain); };
			handlers[OpCode::JMPA] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_JMP(command, chain); };
			handlers[OpCode::GREATER] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_GREATER(command, chain); };
			handlers[OpCode::LESS] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_LESS(command, chain); };
			handlers[OpCode::EQUAL] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_EQUAL(command, chain); };
			handlers[OpCode::EGREATER] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_EGREATER(command, chain); };
			handlers[OpCode::ELESS] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_ELESS(command, chain); };
			handlers[OpCode::EQUAL_N] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_EQUAL_N(command, chain); };
			handlers[OpCode::AND] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_AND(command, chain); };
			handlers[OpCode::OR] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_OR(command, chain); };
			handlers[OpCode::NOT] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_NOT(command, chain); };
			handlers[OpCode::CALL] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_CALL(command, chain); };
			handlers[OpCode::RET] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_RET(command, chain); };
			handlers[OpCode::CONVERT_TYPE] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_CONVERT_TYPE(command, chain); };
			handlers[OpCode::GET_TYPE_STR] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_GET_TYPE_STR(command, chain); };
			handlers[OpCode::GET_BY] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_GET_BY(command, chain); };
			handlers[OpCode::SET_BY] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_SET_BY(command, chain); };
			handlers[OpCode::ARRAY_MAKE] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_ARRAY_MAKE(command, chain); };
			handlers[OpCode::DICT_MAKE] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_DICT_MAKE(command, chain); };
			handlers[OpCode::ARRAY_SIZE] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_ARRAY_OPERATIONS(command, chain); };
			handlers[OpCode::ARRAY_POP_BACK] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_ARRAY_OPERATIONS(command, chain); };
			handlers[OpCode::ARRAY_PUSH_BACK] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_ARRAY_OPERATIONS(command, chain); };
			handlers[OpCode::ARRAY_INSERT_INDEX] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_ARRAY_OPERATIONS(command, chain); };
			handlers[OpCode::ARRAY_ERASE_INDEX] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_ARRAY_OPERATIONS(command, chain); };
			handlers[OpCode::DICT_SIZE] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_DICT_OPERATIONS(command, chain); };
			handlers[OpCode::DICT_ERASE] = [](Command& command, ExecutionChain& chain) { CommandHandlers::H_DICT_OPERATIONS(command, chain); };

			// Обработчики, которые были прямо в условии if-else
			handlers[OpCode::SCOPESTR] = [](Command& command, ExecutionChain& chain) {
				chain.variables.push_back({});
				};
			handlers[OpCode::SCOPEEND] = [](Command& command, ExecutionChain& chain) {
				int target_depth = (int)command.operand.numberVal + (chain.depth_call_stack.empty() ? 0 : chain.depth_call_stack.top());
				while (chain.variables.size() > target_depth && chain.variables.size() > 1) {
					chain.variables.pop_back();
				}
				};
		}


		void ProcessCommand(ExecutionChain& chain)
		{
			Command& command = chain.commands[chain.current_rip];
			if (!handlers.count(command.code)) return;
			handlers[command.code](command, chain);
		}

		bool error_catch = false;
		EHID eh_id = InvalidEHID;
		ExecutionChain* current_echain;
	public:
		VirtualMachine()
		{
			HandlersMapInitialization();
			eh_id = ErrorHandlerManager::Register([&](const EHMessage message)
				{
					if (current_echain != nullptr && current_echain->GetID() == message.script)
					{
						Logger::Get().Log(message.description + " RIP: " + std::to_string(message.rip));
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
				if (error_catch)
				{
					chain.state = ECState::ERROR;
					break;
				}
				if (chain.current_rip != rip)// Проверяем, изменился ли rip внутри ProcessCommand
				{
					rip = chain.current_rip;
					rip++;
					continue;
				}
				rip++;
			}
		}
	};

}