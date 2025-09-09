#pragma once
#include "..\Definitions\Execution.hpp"
#include "..\Definitions\StringOperations.hpp"
#include "..\Logger\Logger.hpp"
#include "..\Interpretation\FunctionTable.hpp"
#include "..\ErrorHandling\ErrorHandler.hpp"

namespace ELScript 
{
	class CommandHandlers 
	{
	public:
		CommandHandlers() = delete; // Нельзя создать объект

		static Value GetStackTop(ExecutionChain& chain) 
		{
			if (chain.stack.empty()) 
			{
				ErrorHandlerManager::RaiseError(EHMessage(chain.id,chain.commands[chain.current_rip], chain.current_rip, EHMessageType::Error, "[VM] stack underflow."));
				return Value();
			}
			Value v = chain.stack.top();
			chain.stack.pop();
			return v;
		}


		static void H_PRINT(Command& command, ExecutionChain& chain)
		{
			if (chain.stack.empty())
			{
				ErrorHandlerManager::RaiseError(EHMessage(chain.id, command,chain.current_rip, EHMessageType::Error, "[VM] PRINT: not enough arguments."));
                return;
			}
			Value val1 = GetStackTop(chain);
			if (val1.type == ValueType::NUMBER)Logger::Get().Log(std::to_string(val1.numberVal));
			if (val1.type == ValueType::BOOL)Logger::Get().Log(val1.boolVal ? "true" : "false");
			if (val1.type == ValueType::STRING)Logger::Get().Log(val1.strVal);
			
		}
		static void H_PUSH(Command& command, ExecutionChain& chain)
		{
			chain.stack.push(command.operand);
			
		}
        static void H_SWAP(Command& command, ExecutionChain& chain)
        {
            if (chain.stack.size() < 2)
            {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] SWAP: not enough arguments."));
                return;
            }
            auto t1 = GetStackTop(chain);
            auto t2 = GetStackTop(chain);
            chain.stack.push(t1);
            chain.stack.push(t2);
        }
		static void H_POP(Command& command, ExecutionChain& chain)
		{
			if (chain.stack.empty())
			{
				ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] POP: stack underflow."));
                return;
			}
			chain.stack.pop();
			
		}
		static void H_ADD(Command& command, ExecutionChain& chain)
		{
			Value val1 = GetStackTop(chain);	//Автоматически удаляет элемент из стека
			Value val2 = GetStackTop(chain);
			if (val1.type != val2.type) {ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] ADD: arguments must same type.")); return;
            }
			if (val1.type == ValueType::VOID) { ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] ADD: arguments have void type.")); return;
            }
			if (val1.type == ValueType::NUMBER)chain.stack.push(val2.numberVal + val1.numberVal);
			if (val1.type == ValueType::STRING)chain.stack.push(val2.strVal + val1.strVal);
			if (val1.type == ValueType::BOOL)chain.stack.push(val2.boolVal + val1.boolVal);
			
		}
		static void H_SUB(Command& command, ExecutionChain& chain) {
			if (chain.stack.size() < 2) {
				ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] SUB: not enough arguments."));
                return;
			}
			Value right = GetStackTop(chain); // Верхний элемент (правый операнд)
			Value left = GetStackTop(chain);  // Следующий элемент (левый операнд)

			if (left.type != ValueType::NUMBER || right.type != ValueType::NUMBER) {
				ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] SUB: arguments must be numbers."));
                return;
			}

			chain.stack.push(left.numberVal - right.numberVal); // left - right
			
		}

		static void H_NEG(Command& command, ExecutionChain& chain) {
			if (chain.stack.size() < 1) {
				ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] NEG: not enough arguments."));
                return;
			}
			Value right = GetStackTop(chain); // Верхний элемент (правый операнд)

			if (right.type != ValueType::NUMBER) {
				ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] NEG: argument must be number."));
                return;
			}
			chain.stack.push(-right.numberVal); //  right
			
		}

		static void H_MUL(Command& command, ExecutionChain& chain)
		{
			if (chain.stack.size() < 2) {
				ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] MUL: not enough arguments."));
                return;
			}
			Value val1 = GetStackTop(chain);
			Value val2 = GetStackTop(chain);
			if (val1.type != ValueType::NUMBER || val2.type != ValueType::NUMBER) {
				ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] MUL: arguments must be numbers."));
                return;
			}
			chain.stack.push(val1.numberVal * val2.numberVal);
			
		}

		static void H_DIV(Command& command, ExecutionChain& chain) {
			if (chain.stack.size() < 2) {
				ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] DIV: not enough arguments."));
                return;
			}
			Value right = GetStackTop(chain); // Верхний элемент (правый операнд)
			Value left = GetStackTop(chain);  // Следующий элемент (левый операнд)

			if (left.type != ValueType::NUMBER || right.type != ValueType::NUMBER) {
				ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] DIV: arguments must be numbers."));
                return;
			}
			if (right.numberVal == 0)
			{
				ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] DIV: division on zero."));
                return;
			}
			chain.stack.push(left.numberVal / right.numberVal);
			
		}

		static void H_MOD(Command& command, ExecutionChain& chain) {
			if (chain.stack.size() < 2) {
				ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] MOD: not enough arguments."));
                return;
			}
			Value right = GetStackTop(chain); // Верхний элемент (правый операнд)
			Value left = GetStackTop(chain);  // Следующий элемент (левый операнд)

			if (left.type != ValueType::NUMBER || right.type != ValueType::NUMBER) {
				ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] MOD: arguments must be numbers."));
                return;
			}
			if (right.numberVal == 0)
			{
				ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] MOD: division on zero."));
                return;
			}
			chain.stack.push((int)left.numberVal % (int)right.numberVal);
			
		}

		static void H_LOAD(Command& command, ExecutionChain& chain)
		{
			int index = -1;

			for (int i = 0; i < chain.variables.size(); i++) {
				if (chain.variables[i].count(command.operand.strVal)) index = i;
			}
			if (index == -1) {
				ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] LOAD: undefined variable " + command.operand.strVal + "."));
                return;
			}
			Value val = chain.variables[index][command.operand.strVal];
			chain.stack.push(val); // Кладём копию значения
			
		}
        static void H_LOADM(Command& command, ExecutionChain& chain)
        {       
            if (!chain.meta_variables.count(command.operand.strVal)) {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] LOADM: undefined meta variable " + command.operand.strVal + "."));
                return;
            }
            Value val = chain.meta_variables[command.operand.strVal];

            if (val.type == ValueType::VOID) {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] LOADM: meta variable with type VOID " + command.operand.strVal + "."));
                return;
            }
            chain.stack.push(val); // Кладём копию значения
        }
		static void H_STORE(Command& command, ExecutionChain& chain) {
			if (chain.stack.empty()) {
				ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] STORE: stack underflow."));
                return;
			}
			int index = -1;
			for (int i = 0; i < chain.variables.size(); i++) {
				if (chain.variables[i].count(command.operand.strVal)) index = i;
			}
			if (index == -1) {
				ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] STORE: undefined variable " + command.operand.strVal + "."));
                return;
			}
			Value val = GetStackTop(chain);
			chain.variables[index][command.operand.strVal] = val;
			
		}

		static void H_DECLARE(Command& command, ExecutionChain& chain) {
			if (chain.variables.back().count(command.operand.strVal))
			{
				ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] DECLARE: variable " + command.operand.strVal + " has already declared."));
                return;
			}
			chain.variables.back()[command.operand.strVal] = Value();
			
		}

		static void H_DECLARED(Command& command, ExecutionChain& chain) {
			if (chain.variables.back().count(command.operand.strVal))
			{
				chain.stack.push(true);
			}
			else chain.stack.push(false);
			
		}

		static void H_JMP(Command& command, ExecutionChain& chain) {
			int target;
			try {
				target = (int)command.operand.numberVal;
			}
			catch (...) {
				ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] Invalid jump target: " + std::to_string((int)command.operand.numberVal)));
                return;
			}
			chain.current_rip = target;   // Аналогично для абсолютного перехода

			if (chain.current_rip < 0) chain.current_rip = 0;
			
		}

        static void H_JMP_IF(Command& command, ExecutionChain& chain) {
            if (chain.stack.empty()) {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] JMP_IF: stack underflow."));
                return;
            }
            Value operand = GetStackTop(chain);

            bool shouldJump = (operand.type == ValueType::BOOL && operand.boolVal) ||
                (operand.type == ValueType::NUMBER && operand.numberVal != 0.0f);

            if (shouldJump) {
                Command c(OpCode::JMPA, command.operand);
                return H_JMP(c, chain);  // Используем общую логику прыжка
            }
            
        }

        static void H_JMP_IF_N(Command& command, ExecutionChain& chain) {
            if (chain.stack.empty()) {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] JMP_IF_N: stack underflow."));
                return;
            }
            Value operand = GetStackTop(chain);

            bool shouldJump = (operand.type == ValueType::BOOL && !operand.boolVal) ||
                (operand.type == ValueType::NUMBER && operand.numberVal == 0.0f);

            if (shouldJump) {
                Command c(OpCode::JMPA, command.operand);
                return H_JMP(c, chain);  // Используем общую логику прыжка
            }
            
        }

        static void H_EQUAL(Command& command, ExecutionChain& chain) {
            if (chain.stack.size() < 2) {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] EQUAL: stack underflow."));
                return;
            }

            Value right = GetStackTop(chain);
            Value left = GetStackTop(chain);

            if (left.type != right.type) {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] EQUAL: different types."));
                return;
            }

            bool result;
            switch (left.type) {
            case ValueType::BOOL:   result = (left.boolVal == right.boolVal); break;
            case ValueType::NUMBER: result = (left.numberVal == right.numberVal); break;
            case ValueType::STRING: result = (left.strVal == right.strVal); break;
            default: ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] EQUAL: unsupported type."));
                return;
            }
            chain.stack.push(Value(result)); // Явно создаём bool
            
        }

        static void H_EQUAL_N(Command& command, ExecutionChain& chain) {
            if (chain.stack.size() < 2) {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] EQUAL_N: stack underflow."));
                return;
            }

            Value right = GetStackTop(chain);
            Value left = GetStackTop(chain);

            if (left.type != right.type) {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] EQUAL_N: different types."));
                return;
            }

            bool result;
            switch (left.type) {
            case ValueType::BOOL:   result = (left.boolVal != right.boolVal); break;
            case ValueType::NUMBER: result = (left.numberVal != right.numberVal); break;
            case ValueType::STRING: result = (left.strVal != right.strVal); break;
            default: 
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] EQUAL_N: unsupported type."));
                return;
            }
            chain.stack.push(result); // Явно создаём bool
            
        }

        static void H_AND(Command& command, ExecutionChain& chain) {
            if (chain.stack.size() < 2) {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] AND: stack underflow."));
                return;
            }

            auto val1 = GetStackTop(chain);
            auto val2 = GetStackTop(chain);

            if (val1.type != val2.type) {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] AND: different types of values."));
                return;
            }
            if (val2.type == ValueType::BOOL) chain.stack.push(val2.boolVal && val1.boolVal);
            else {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] AND: values must have BOOL type."));
                return;
            }
            
        }

        static void H_OR(Command& command, ExecutionChain& chain) {
            if (chain.stack.size() < 2) {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] OR: stack underflow."));
                return;
            }

            auto val1 = GetStackTop(chain); // RIGHT
            auto val2 = GetStackTop(chain); // LEFT

            if (val1.type != val2.type) {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] OR: different types of values."));
                return;
            }
            if (val2.type == ValueType::BOOL) chain.stack.push(val2.boolVal || val1.boolVal);
            else {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] OR: values must have BOOL type."));
                return;
            }
            
        }

        static void H_NOT(Command& command, ExecutionChain& chain) {
            if (chain.stack.empty()) {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] NOT: stack underflow."));
                return;
            }

            auto val1 = GetStackTop(chain); // right

            if (val1.type == ValueType::NUMBER) {
                chain.stack.push(val1.numberVal != 0.0f);
            }
            else if (val1.type == ValueType::BOOL) {
                chain.stack.push(!val1.boolVal);
            }
            else {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] NOT: value must be number or boolean."));
                return;
            }
            
        }

        static void H_LESS(Command& command, ExecutionChain& chain) {
            if (chain.stack.size() < 2) {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] LESS: stack underflow."));
                return;
            }

            auto val1 = GetStackTop(chain);
            auto val2 = GetStackTop(chain);

            if (val1.type != val2.type) {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] LESS: different types of values."));
                return;
            }
            if (val2.type == ValueType::NUMBER) chain.stack.push(val2.numberVal < val1.numberVal);
            else {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] LESS: values must be numbers."));
                return;
            }
            
        }

        static void H_GREATER(Command& command, ExecutionChain& chain) {
            if (chain.stack.size() < 2) {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] GREATER: stack underflow."));
                return;
            }

            auto val1 = GetStackTop(chain);
            auto val2 = GetStackTop(chain);

            if (val1.type != val2.type) {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] GREATER: different types of values."));
                return;
            }
            if (val2.type == ValueType::NUMBER) chain.stack.push(val2.numberVal > val1.numberVal);
            else {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] GREATER: values must be numbers."));
                return;
            }
            
        }

        static void H_ELESS(Command& command, ExecutionChain& chain) {
            if (chain.stack.size() < 2) {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] ELESS: stack underflow."));
                return;
            }

            Value right = GetStackTop(chain);
            Value left = GetStackTop(chain);

            if (left.type != ValueType::NUMBER || right.type != ValueType::NUMBER) {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] ELESS: values must be numbers."));
                return;
            }

            chain.stack.push(Value(left.numberVal <= right.numberVal));
            
        }

        static void H_EGREATER(Command& command, ExecutionChain& chain) {
            if (chain.stack.size() < 2) {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] EGREATER: stack underflow."));
                return;
            }

            Value right = GetStackTop(chain);
            Value left = GetStackTop(chain);

            if (left.type != ValueType::NUMBER || right.type != ValueType::NUMBER) {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] EGREATER: values must be numbers."));
                return;
            }

            chain.stack.push(Value(left.numberVal >= right.numberVal));
            
        }

        static void H_CALL(Command& command, ExecutionChain& chain) {
            if (!chain.function_markers.count(command.operand.strVal)) {
                if (FunctionTable::Exists(command.operand.strVal)) { // В приоритете внутренние функции!
                    // Вызываем внешнюю функцию
                    size_t size = FunctionTable::GetArity(command.operand.strVal);
                    if (chain.stack.size() < size) {
                        ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] CALL: stack underflow."));
                        return;
                    }
                    std::vector<Value> values;

                    for (int i = 0; i < size; i++) {
                        values.push_back(GetStackTop(chain));
                    }

                    auto result = FunctionTable::Call(command.operand.strVal, chain.id, values);
                    //if (result.type != ValueType::VOID) chain.stack.push(result);
                    chain.stack.push(result);   //TODO Добавить очистку стека
                    return;
                }
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] CALL: function " + command.operand.strVal + " isnt declared in global scope."));
                return;
            }
            if (chain.call_stack.size() >= MaxCallStackDepth) {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] CALL: call stack overflow (" + std::to_string(chain.call_stack.size()) + ")."));
                return;
            }

            chain.call_stack.push(chain.current_rip);
            chain.depth_call_stack.push(chain.variables.size() - 1);
            chain.current_rip = chain.function_markers.at(command.operand.strVal).start_rip;
        }

        static void H_RET(Command& command, ExecutionChain& chain) {
            chain.current_rip = chain.call_stack.top(); // +1 так как надо перейти на следующую инструкцию
            chain.call_stack.pop();
            chain.depth_call_stack.pop();
             // SUCCESS, а не RIP_INC_FREEZE, так как нам надо перейти на следующую после CALL инструкцию, иначе ошибка переполнения стека
        }
        static void H_GET_TYPE_STR(Command& command, ExecutionChain& chain) {
            Value val = GetStackTop(chain);
            chain.stack.push(Value::GetTypeString(val.type));
        }
        

        static void H_GET_BY(Command& command, ExecutionChain& chain) {
            if (chain.stack.size() < 2) {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] GET_BY: stack underflow."));
                return;
            }
            Value array = GetStackTop(chain);
            Value index = GetStackTop(chain);

            if (array.type != ValueType::ARRAY && array.type != ValueType::DICT) {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] GET_BY: indexing is only possible in array or dict."));
                return;
            }
            if (array.type == ValueType::ARRAY) 
            {
                if (index.type != ValueType::NUMBER) {
                    ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] GET_BY: index must be a number."));
                    return;
                }
                int idx = static_cast<int>(index.numberVal);

                // Проверяем границы
                if (idx < 0 || idx >= array.arrayVal->size()) {
                    ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] GET_BY: array index out of bounds. Index: " + std::to_string(idx) + ", Size: " + std::to_string(array.arrayVal->size())));
                    return;
                }
                chain.stack.push(array.arrayVal->at(idx));
            }
            else
            {
                if (index.type != ValueType::STRING) {
                    ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] GET_BY: key must be a string."));
                    return;
                }
                // Проверяем границы
                if (!array.dictVal->count(index.strVal)) {
                    ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] GET_BY: invalid key. Key: " + index.strVal + "."));
                    return;
                }
                chain.stack.push(array.dictVal->at(index.strVal));
            }
        }

        static void H_SET_BY(Command& command, ExecutionChain& chain) {
            if (chain.stack.size() < 3) {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] SET_BY: stack underflow."));
                return;
            }
            Value array = GetStackTop(chain);
            Value index = GetStackTop(chain);
            Value value = GetStackTop(chain);
            if (array.type != ValueType::ARRAY && array.type != ValueType::DICT) {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] SET_BY: indexing is only possible in array or dict."));
                return;
            }
            if (array.type == ValueType::ARRAY)
            {
                if (index.type != ValueType::NUMBER) {
                    ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] SET_BY: index must be a number."));
                    return;
                }
                int idx = static_cast<int>(index.numberVal);

                // Проверяем границы
                if (idx < 0 || idx >= array.arrayVal->size()) {
                    ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] SET_BY: array index out of bounds. Index: " + std::to_string(idx) + ", Size: " + std::to_string(array.arrayVal->size())));
                    return;
                }
                (*array.arrayVal.get())[idx] = value;
            }
            else
            {
                if (index.type != ValueType::STRING) {
                    ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] SET_BY: key must be a string."));
                    return;
                }
                // Проверяем границы
                if (!array.dictVal->count(index.strVal)) {
                    ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] SET_BY: invalid key. Key: " + index.strVal + "."));
                    return;
                }
                (*array.dictVal.get())[index.strVal] = value;
            }
        }
        static void H_ARRAY_MAKE(Command& command, ExecutionChain& chain)
        {
            int count = command.operand.numberVal;

            if (chain.stack.size() < count)
            {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] ARRAY_MAKE: stack underflow."));
                return;
            }
            std::vector<Value> temp;
            for (int i = 0; i < count; i++)
            {
                auto value = GetStackTop(chain);
                temp.push_back(value);
            }

            chain.stack.push(Value(std::make_shared<std::vector<Value>>(temp)));

        }
        static void H_ARRAY_OPERATIONS(Command& command, ExecutionChain& chain) {
            Value array = GetStackTop(chain);

            if (array.type != ValueType::ARRAY || array.arrayVal == nullptr) {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] ARRAY_OPERATIONS: invalid array."));
                return;
            }
            Value index;
            Value value;
            switch (command.code) {
            case OpCode::ARRAY_PUSH_BACK:
                if (chain.stack.size() < 1) { // 1 так как array уже вытащили
                    ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] ARRAY_PUSH_BACK: stack underflow."));
                    return;
                }
                value = GetStackTop(chain);
                array.arrayVal->push_back(value);
                break;
            case OpCode::ARRAY_POP_BACK:
                if (array.arrayVal->size() < 1) { // 1 так как array уже вытащили
                    ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] ARRAY_POP_BACK: array is empty."));
                    return;
                }
                array.arrayVal->pop_back();
                break;
            case OpCode::ARRAY_INSERT_INDEX:
                if (chain.stack.size() < 2) {
                    ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] ARRAY_INSERT_INDEX: stack underflow."));
                    return;
                }
                index = GetStackTop(chain);
                value = GetStackTop(chain);
                if (index.numberVal >= array.arrayVal->size()) {
                    ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] ARRAY_INSERT_INDEX: array index out of bounds. Index: " + std::to_string((int)index.numberVal) + ", Size: " + std::to_string(array.arrayVal->size())));
                    return;
                }
                array.arrayVal->insert(array.arrayVal->begin() + (int)index.numberVal, value);
                break;
            case OpCode::ARRAY_ERASE_INDEX:
                if (chain.stack.size() < 1) {
                    ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] ARRAY_ERASE_INDEX: stack underflow."));
                    return;
                }
                index = GetStackTop(chain);
                if (index.numberVal >= array.arrayVal->size()) {
                    ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] ARRAY_ERASE_INDEX: array index out of bounds. Index: " + std::to_string((int)index.numberVal) + ", Size: " + std::to_string(array.arrayVal->size())));
                    return;
                }
                array.arrayVal->erase(array.arrayVal->begin() + (int)index.numberVal);
                break;
            case OpCode::ARRAY_SIZE:
                chain.stack.push((int)array.arrayVal->size());
                break;
            }
            
        }
        static void H_DICT_MAKE(Command& command, ExecutionChain& chain) 
        {
            int count = command.operand.numberVal;

            if (chain.stack.size() < count * 2) 
            {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] DICT_MAKE: stack underflow."));
                return;
            }
            std::unordered_map<std::string, Value> temp;
            for (int i = 0; i < count; i++)
            {
                auto value = GetStackTop(chain);
                auto key = GetStackTop(chain);
                if (key.type != ValueType::STRING)
                {
                    ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] DICT_MAKE: key must be string."));
                    return;
                }
                temp.insert({ key.strVal, value });
            }

            chain.stack.push(Value(std::make_shared<std::unordered_map<std::string, Value>>(temp)));

        }
        static void H_DICT_OPERATIONS(Command& command, ExecutionChain& chain) {
            Value dict = GetStackTop(chain);

            if (dict.type != ValueType::DICT || dict.dictVal == nullptr) {
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] DICT_OPERATIONS: invalid dictionary."));
                return;
            }
            Value key;
            Value value;
            switch (command.code) {
            case OpCode::DICT_ERASE:
                if (chain.stack.size() < 1) { // 2 так как dict уже вытащили, а 1 - это ключ
                    ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] DICT_ERASE: stack underflow."));
                    return;
                }
                key = GetStackTop(chain);
                if (!dict.dictVal->count(key.strVal)) {
                    ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] DICT_ERASE: invalid key."));
                    return;
                }
                dict.dictVal->erase(key.strVal);
                break;
            case OpCode::DICT_SIZE:
                chain.stack.push((int)dict.dictVal->size());
                break;
            }
        }
        static void H_CONVERT_TYPE(Command& command, ExecutionChain& chain) {
            ValueType type = OpCodeMap::GetStringType(command.operand.strVal);

            Value v1 = GetStackTop(chain);
            if (v1.type == type) return;

            switch (type) {
            case ELScript::VOID:
                chain.stack.push(Value());
                return;
            case ELScript::NUMBER:
                if (v1.type == VOID) chain.stack.push(0);
                if (v1.type == BOOL) chain.stack.push((int)v1.boolVal);
                if (v1.type == STRING) {
                    if (!StringOperations::IsNumber(v1.strVal)) {
                        ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] CONVERT_TYPE: string doesnt be converted to number."));
                        return;
                    }
                    chain.stack.push(std::stod(v1.strVal));
                }
                return;
            case ELScript::STRING:
                if (v1.type == VOID) chain.stack.push("");
                if (v1.type == BOOL) chain.stack.push(v1.boolVal ? "true" : "false");
                if (v1.type == NUMBER) chain.stack.push(std::to_string(v1.numberVal));
                return;
            case ELScript::BOOL:
                if (v1.type == VOID) chain.stack.push(false);
                if (v1.type == STRING) chain.stack.push(v1.strVal == "true" ? true : false);
                if (v1.type == NUMBER) chain.stack.push(v1.numberVal != 0);
            case ELScript::ARRAY:
                if (v1.type == VOID) chain.stack.push(std::make_shared<std::vector<Value>>());
                if (v1.type == STRING) 
                {
                    std::vector<Value> literals_;
                    for (auto chr : v1.strVal) 
                    {
                        std::string str = std::string(1,chr);
                        literals_.push_back(Value(str));
                    }
                    chain.stack.push(Value(std::make_shared<std::vector<Value>>(literals_)));
                };
                if (v1.type == DICT) {
                    auto result_array = std::make_shared<std::vector<Value>>(); // Главный массив
                    for (auto& [key, value] : *v1.dictVal) {
                        // Создаём массив для пары [key, value]
                        auto pair_array = std::make_shared<std::vector<Value>>();
                        pair_array->push_back(Value(key));    // Ключ как строка
                        pair_array->push_back(value);         // Значение как есть
                        result_array->push_back(Value(pair_array)); // Добавляем пару в результат
                    }
                    chain.stack.push(Value(result_array));
                }
                return;
            default:
                ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] CONVERT_TYPE: invalid type." + Value::GetTypeString(v1.type) + " to " + Value::GetTypeString(type) + "."));
                return;
            }
            ErrorHandlerManager::RaiseError(EHMessage(chain.id, command, chain.current_rip, EHMessageType::Error, "[VM] CONVERT_TYPE: invalid type." + Value::GetTypeString(v1.type) + " to " + Value::GetTypeString(type) + "."));
        }
	};
}