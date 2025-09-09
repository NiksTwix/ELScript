#pragma once
#include "..\Definitions/CommandsInfo.hpp"
#include "..\Logger/Logger.hpp"
#include "ALUDecoder.hpp"

namespace ELScript 
{
	class KeywordDecoder 
	{
	private:

		ALUDecoder aluDecoder;

		std::vector<Command> functions_section;

		void Handler_op_code(CommandNode& node, std::vector<Command>& chain) 
		{	
			for (auto child : node.children) 
			{
				if (child.tokens.size() == 1) 
				{
					chain.push_back(Command(OpCodeMap::GetStringOpCode(child.tokens[0].value.strVal),Value(), child.tokens[0].line));
				}
				if (child.tokens.size() >= 2)
				{
					chain.push_back(Command(OpCodeMap::GetStringOpCode(child.tokens[0].value.strVal), child.tokens[1].value, child.tokens[0].line));
				}
			}
		}



		void Handler_if(CommandNode& node, std::vector<Command>& chain)
		{
			//Обрезаем условие
			std::vector<Token> tokens_of_if;
			for (int j = 1; j < node.tokens.size(); j++)tokens_of_if.push_back(node.tokens[j]);

			auto result_if = aluDecoder.ExpressionHandler(tokens_of_if);
			chain.insert(chain.end(), result_if.begin(), result_if.end());

			std::vector<Command> if_chain;
			for (CommandNode& child : node.children) {
				auto childChain = BuildExecutionChain(child);
				if_chain.insert(if_chain.end(), childChain.begin(), childChain.end());
			}
			std::string label = StringOperations::GenerateLabel("#if_end");
			chain.push_back(Command(OpCode::JMPA_IF_N, label));	
			chain.insert(chain.end(), if_chain.begin(), if_chain.end());
			chain.push_back(Command(OpCode::LABEL, label));
		}

		void Handler_while(CommandNode& node, std::vector<Command>& chain)
		{
			//Обрезаем условие
			std::vector<Token> tokens_of_if;

			std::string start_label = StringOperations::GenerateLabel("#while_start");
			chain.push_back(Command(OpCode::LABEL, start_label));
			for (int j = 1; j < node.tokens.size(); j++)tokens_of_if.push_back(node.tokens[j]);

			auto result_if = aluDecoder.ExpressionHandler(tokens_of_if);
			chain.insert(chain.end(), result_if.begin(), result_if.end());

			std::vector<Command> while_chain;
			for (CommandNode& child : node.children) {
				auto childChain = BuildExecutionChain(child);
				while_chain.insert(while_chain.end(), childChain.begin(), childChain.end());
			}
			std::string end_label = StringOperations::GenerateLabel("#while_end");
			chain.push_back(Command(OpCode::JMPA_IF_N, end_label, chain.back().line)); //Пропускаем тело, если условие ложно

			
			for (int i = 0; i < while_chain.size(); i++)
			{
				//Проходим по детям и ищём необработанные ключевые слова break и continue
				Command& c = while_chain[i];
				if (c.code == OpCode::EXCEPT_HANDLING)
				{
					if (c.operand.strVal == "break")
					{
						c.code = OpCode::JMPA;
						c.operand = end_label; //Прыжок в конец на N, учитываем смещение i, так как оно в другом массиве!
					}
					if (c.operand.strVal == "continue")
					{
						c.code = OpCode::JMPA;
						c.operand = start_label; //Прыжок в начало на N
					}
				}
			}

			chain.insert(chain.end(), while_chain.begin(), while_chain.end());
			chain.push_back(Command(OpCode::JMPA, start_label, chain.back().line));	//Прыгаем обратно
			chain.push_back(Command(OpCode::LABEL, end_label));
		}

		void Handler_for(CommandNode& node, std::vector<Command>& chain)
		{
			if (node.tokens.size() < 2)
			{
				Logger::Get().Log("[Decoder] Invalid 'for' cycle.");
				return;
			}
			std::string check_label = StringOperations::GenerateLabel("#for_check");
			std::string add_label = StringOperations::GenerateLabel("#for_add");
			std::string end_label = StringOperations::GenerateLabel("#for_end");
			bool in_brathes = false;

			std::vector<std::vector<Token>> args;
			args.push_back({});
			std::string variable = "";

			int arg_index = 0;

			for (int i = 0; i < node.tokens.size(); i++) //Предподготовка
			{
				Token t = node.tokens[i];
				if (t.type == TokenType::DELIMITER && t.value.strVal == "(") in_brathes = true;
				if (t.type == TokenType::DELIMITER && t.value.strVal == ")") in_brathes = false;
				if (t.type == TokenType::IDENTIFIER && !in_brathes)
				{
					variable = t.value.strVal;
				}
				if (in_brathes && t.value.strVal != ",")
				{
					if (t.type == TokenType::IDENTIFIER && t.value.strVal == variable)
					{
						Logger::Get().Log("[Decoder] Invalid for cycle. For variable is in args." + std::to_string(node.tokens[0].line)); return;
					}
					args[arg_index].push_back(t);
				}
				if (in_brathes && t.value.strVal == ",")
				{
					args.push_back({});
					arg_index++;
				}
			}
			if (args.size() < 2) { Logger::Get().Log("[Decoder] Invalid for cycle. Invalid cycle's args. Line: " + std::to_string(node.tokens[0].line)); return; }
			if (args.size() == 2) args.push_back({ Token(TokenType::LITERAL, 1) });	//Ставим шаг по умолчанию
			if (variable == "") { Logger::Get().Log("[Decoder] Invalid for cycle. Invalid cycle's variable name. Line: " + std::to_string(node.tokens[0].line)); return; }
			//Declaring

			std::string declare_label = StringOperations::GenerateLabel("#for_declare");

			chain.push_back(Command(OpCode::DECLARED, variable, node.tokens[0].line));
			chain.push_back(Command(OpCode::JMPA_IF, declare_label, node.tokens[0].line));
			chain.push_back(Command(OpCode::DECLARE, variable, node.tokens[0].line));
			chain.push_back(Command(OpCode::LABEL, declare_label, node.tokens[0].line));
			auto start = aluDecoder.ExpressionHandler(args[0]);
			auto end = aluDecoder.ExpressionHandler(args[1]);
			auto step = aluDecoder.ExpressionHandler(args[2]);

			chain.insert(chain.end(), start.begin(), start.end());

			chain.push_back(Command(OpCode::STORE, variable, node.tokens[0].line));

			//проверка
			chain.push_back(Command(OpCode::LABEL, check_label));
			chain.insert(chain.end(), end.begin(), end.end());
			chain.push_back(Command(OpCode::LOAD, variable, node.tokens[0].line));
			chain.push_back(Command(OpCode::EQUAL, Value(), node.tokens[0].line));

			std::vector<Command> for_chain;
			for (CommandNode& child : node.children) {
				auto childChain = BuildExecutionChain(child);
				for_chain.insert(for_chain.end(), childChain.begin(), childChain.end());
			}
			chain.push_back(Command(OpCode::JMPA_IF, end_label, node.tokens[0].line));

			for (int i = 0; i < for_chain.size(); i++)
			{
				//Проходим по детям и ищём необработанные ключевые слова break и continue
				Command& c = for_chain[i];
				if (c.code == OpCode::EXCEPT_HANDLING)
				{
					if (c.operand.strVal == "break")
					{
						c.code = OpCode::JMPA;
						c.operand = end_label; //Прыжок в конец на N, учитываем смещение i, так как оно в другом массиве!
					}
					if (c.operand.strVal == "continue")
					{
						c.code = OpCode::JMPA;
						c.operand = add_label; //Прыжок вниз на N, так как нам надо увеличивать i
					}
				}
			}


			chain.insert(chain.end(), for_chain.begin(), for_chain.end());

			//Увеличение переменной на шаг
			chain.push_back(Command(OpCode::LABEL, add_label, node.tokens[0].line));
			chain.push_back(Command(OpCode::LOAD, variable, node.tokens[0].line));
			chain.insert(chain.end(), step.begin(), step.end());
			chain.push_back(Command(OpCode::ADD, Value(), node.tokens[0].line));
			chain.push_back(Command(OpCode::STORE, variable, node.tokens[0].line));
			chain.push_back(Command(OpCode::JMPA, check_label, node.tokens[0].line));	//Прыгаем обратно
			chain.push_back(Command(OpCode::LABEL, end_label, node.tokens[0].line));
		}

		void Handler_print_value(CommandNode& node, std::vector<Command>& chain, int current_index)
		{
			if (current_index + 1 < node.tokens.size())
			{
				Token& t0 = node.tokens[current_index];
				Token& t = node.tokens[current_index + 1];
				if (t.type == TokenType::IDENTIFIER)
				{
					chain.push_back(Command(OpCode::LOAD, t.value, t.line));
				}
				else if (t.type == TokenType::LITERAL) {
					chain.push_back(Command(OpCode::PUSH, t.value, t.line));
				}
				chain.push_back(Command(OpCode::PRINT, Value(), t.line));
			}
		}

		void Handler_var(CommandNode& node, std::vector<Command>& chain, int current_index)
		{
			if (current_index + 1 < node.tokens.size() && node.tokens[current_index + 1].type == TokenType::IDENTIFIER)
			{
				std::string label = StringOperations::GenerateLabel("#var_declared");
				chain.push_back(Command(OpCode::DECLARED, node.tokens[current_index + 1].value, node.tokens[current_index].line));
				chain.push_back(Command(OpCode::JMPA_IF, label, node.tokens[current_index].line));
				chain.push_back(Command(OpCode::DECLARE, node.tokens[current_index + 1].value, node.tokens[current_index].line));
				chain.push_back(Command(OpCode::LABEL, label, node.tokens[current_index].line));
				std::vector<Token> tokens_of;
				for (int j = 1; j < node.tokens.size(); j++)tokens_of.push_back(node.tokens[j]);
				auto result = aluDecoder.ExpressionHandler(tokens_of);
				chain.insert(chain.end(), result.begin(), result.end());
			}
			else
			{
				Logger::Get().Log("[Decoder] var declaration error. Line: " + std::to_string(node.tokens[current_index].line));
			}
		}

		void Handler_func(CommandNode& node, std::vector<Command>& chain, int current_index)
		{
			if (current_index + 1 < node.tokens.size() && node.tokens[current_index + 1].type == TokenType::IDENTIFIER)
			{
				//chain.push_back(Command(OpCode::FUNC_DECLARED, node.tokens[current_index + 1].value, node.tokens[current_index].line));
				//chain.push_back(Command(OpCode::JMPR_IF, 2, node.tokens[current_index].line));
				chain.push_back(Command(OpCode::FUNC_DECLARE, node.tokens[current_index + 1].value, node.tokens[current_index].line));
				chain.push_back(Command(OpCode::SCOPESTR, "#scope_start", node.tokens[current_index].depth));

				int exit_depth = node.tokens[current_index].depth + 1;

				//Обрабатываем параметры
				bool skobki = false;
				for (int i = 2; i < node.tokens.size(); i++) 
				{
					Token t = node.tokens[i];
					if (t.value.strVal == "(") skobki = true;
					if (t.value.strVal == ")") break;
					if (skobki && t.type == TokenType::IDENTIFIER) 
					{
						chain.push_back(Command(OpCode::DECLARE, t.value, t.line));	//                                           2,1,0
						chain.push_back(Command(OpCode::STORE, t.value, t.line));	//Загружаем значение из стека справа налево (x,y,z) -> x(top),y,z(bottom)
					}
				}
				chain.push_back(Command(OpCode::NOP, "#args_end", node.tokens[current_index].line));
				std::vector<Command> func_chain;
				for (CommandNode& child : node.children) {
					auto childChain = BuildExecutionChain(child);
					func_chain.insert(func_chain.end(), childChain.begin(), childChain.end());
				}
				for (int i = 0; i < func_chain.size(); i++) //проходим по всем токенам
				{
					Command c = func_chain[i];
					if (c.code == OpCode::EXCEPT_HANDLING && c.operand.strVal == "return") 
					{
						if (chain.back().operand.strVal == "#op_end") chain.push_back(Command(OpCode::PUSH, Value(), c.line));
						chain.push_back(Command(OpCode::SCOPEEND, exit_depth, c.line));	//depth+1 так как токен func находится на более высоком уровне
						chain.push_back(Command(OpCode::RET,Value(), c.line)); //Говорим виртуальной машине вернутся по стеку обратно
					}
					else 
					{
						chain.push_back(c);
					}
				}

				if (chain[chain.size()-1].code != OpCode::RET && (chain.size() > 0 && chain[chain.size()-2].code != OpCode::RET)) //Нам же нужно как-то закончить выполнение функции? Второе условие из-за того, что return #op_end может завершиться
				{
					chain.push_back(Command(OpCode::PUSH, Value(), node.tokens[current_index].line));	//Значение VOID 
					chain.push_back(Command(OpCode::SCOPEEND, exit_depth, func_chain.size() > 0 ? func_chain.back().line: node.tokens[current_index].line));	//depth+1 так как токен func находится на более высоком уровне
					chain.push_back(Command(OpCode::RET, Value(), func_chain.size() > 0 ? func_chain.back().line : node.tokens[current_index].line)); //Говорим виртуальной машине вернутся по стеку обратно
				}
			}
			else
			{
				Logger::Get().Log("[Decoder] func declaration error. Line: " + std::to_string(node.tokens[current_index].line));
			}
		}

		void DirectiveHandler(std::vector<Command>& chain, const Token& token) 
		{
			if (token.value.strVal == "#op_end") 
			{
				chain.push_back(Command(OpCode::NOP, "#op_end"));
			}
			if (token.type == TokenType::SCOPE_START) { chain.push_back(Command(OpCode::SCOPESTR, token.value, token.line));}
			if (token.type == TokenType::SCOPE_END) { chain.push_back(Command(OpCode::SCOPEEND, (int)token.depth, token.line));}
		}

	public:
		
		void ClearState()
		{
			functions_section.clear();
		}
		std::vector<Command> GetFunctionsSection() 
		{
			return functions_section;
		}

		std::vector<Command> BuildExecutionChain(CommandNode& node)
		{
			std::vector<Command> chain;

			bool skip_children = false;

			if (node.tokens.size() != 0)
			{
				for (int i = 0; i < node.tokens.size(); i++)	//Нахрен тут цикл?
				{	
					Token& token = node.tokens[i];
					if (token.value.strVal == "if")
					{
						Handler_if(node, chain);
						skip_children = true;
						break;
					}
					if (token.value.strVal == "while")
					{
						Handler_while(node, chain);
						skip_children = true;
						break;
					}
					if (token.value.strVal == "for")
					{
						Handler_for(node, chain);
						skip_children = true;
						break;
					}
					if (token.value.strVal == "print_value")
					{
						Handler_print_value(node, chain, i);
						break;
					}
					if (token.value.strVal == "var")
					{
						Handler_var(node, chain, i);
						break;
					}
					if (token.value.strVal == "func")
					{
						Handler_func(node, functions_section, i);
						skip_children = true;
						break;
					}
					if (token.value.strVal == "op_code")
					{
						Handler_op_code(node, chain);
						skip_children = true;
						break;
					}
					if (token.value.strVal == "break" || token.value.strVal == "continue")
					{
						chain.push_back(Command(OpCode::EXCEPT_HANDLING, token.value, token.line));
						break;
					}
					if (token.value.strVal == "return")
					{
						auto result = aluDecoder.ExpressionHandler(StringOperations::TrimVector<Token>(node.tokens,1,node.tokens.size()-1));	//обрезаем return
						chain.insert(chain.end(), result.begin(), result.end());
						chain.push_back(Command(OpCode::EXCEPT_HANDLING, "return", token.line));	//вставляем return после значения
						break;
					}
					auto result = aluDecoder.ExpressionHandler(node.tokens);
					chain.insert(chain.end(), result.begin(), result.end());
					break;
				}
				for (int i = 0; i < node.tokens.size(); i++) //Отдельно проверяем директивы
				{
					Token& token = node.tokens[i];
					if (token.value.strVal.size() > 0 && token.value.strVal.front() == '#')
					{
						DirectiveHandler(chain, token);
					}
				}
			}
			// Добавляем дочерние ноды (например, тело if/while)
			if (!skip_children)
			{
				for (CommandNode& child : node.children) {
					auto childChain = BuildExecutionChain(child);
					chain.insert(chain.end(), childChain.begin(), childChain.end());
				}
			}
			return chain;
		}
	};

}