#pragma once
#include "CommandsInfo.hpp"


namespace ELScript 
{
	using ECID = size_t;
	constexpr ECID InvalidECID = SIZE_MAX;
	class Interpreter;
	class VirtualMachine;
	class CommandHandlers;
	enum class ECState 
	{
		CLEAR,
		ACTIVE,
		ENDED,
		ERROR,
	};

	struct FunctionData 
	{
		std::string identifier;
		int start_rip;
		std::vector<std::string> arguments;
	};

	struct ExecutionChain
	{
		friend Interpreter;
		friend VirtualMachine;
		friend CommandHandlers;
	private:	
		std::stack<Value> stack;
		std::vector<std::unordered_map<std::string, Value>> variables = { {} };
		std::unordered_map<std::string, Value> meta_variables;
		std::vector<Command> commands;
		std::unordered_map<std::string, FunctionData> function_markers;
		std::stack<int> call_stack; //rip
		std::stack<int> depth_call_stack;
		ECState state;
		int current_rip = 0;
		int exit_rip = 0;
		ECID id = InvalidECID;
	public:
		
		ECID GetID() { return id; }

		ECState GetState() { return state; }

		void ClearState()
		{
			stack = std::stack<Value>();
			variables.clear();
			while (call_stack.size() > 0) call_stack.pop();
			while (depth_call_stack.size() > 0) depth_call_stack.pop();
			variables.push_back({});	//Глобальная
			state = ECState::CLEAR;
		}
	};

}