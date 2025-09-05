#pragma once
#include "..\Definitions\CommandsInfo.hpp"
#include "..\Logger\Logger.hpp"
#include <unordered_set>
#include "KeywordDecoder.hpp"
/*
	Decoder разбивает текст на токены и расшифровывает команды
*/
namespace ELScript
{

	struct RawEC //Raw Execution Chain
	{
		int exit_rip;
		std::vector<Command> commands;
	};

	class Decoder
	{
	private:

		struct SkipScope {
			bool flag = false;
			int depth = -1;
		};

		KeywordDecoder kwDecoder;

		std::unordered_set<std::string> scope_exceptions = {"func","op_code"};	//Объявления, в блоках кода которых не надо ставить автоматический SCOPE

		int current_depth = 0;

		std::stack<SkipScope> skip_scopes;		//Для вложенности


		void if_scope_exception(Token& t) 
		{
			if (scope_exceptions.count(t.value.strVal))
			{
				skip_scopes.push({ true,current_depth });
			}
		}

		CommandNode BuildCommandTree(std::vector<Token>& tokens)
		{
			CommandNode parent;

			std::stack<CommandNode> cn_stack;

			cn_stack.push(parent);

			std::vector<Token> command_tokens;

			current_depth = 0;

			for (int i = 0; i < tokens.size(); i++)
			{
				Token t = tokens[i];
				if_scope_exception(t);

				if (t.value.strVal == "{")
				{
					if (command_tokens.size() != 0)
					{
						///Начало области видимости переменных
						if (skip_scopes.empty() || !(skip_scopes.top().flag && skip_scopes.top().depth == current_depth))
						{
							CommandNode scope_start;
							scope_start.tokens.push_back(Token(TokenType::SCOPE_START, "#scope_start", -1, current_depth));	//TODO инрементировать, если надо
							cn_stack.top().children.push_back(scope_start);
						}

						CommandNode node;
						node.tokens = command_tokens;
						node.line = command_tokens.back().line;
						command_tokens.clear();
						cn_stack.push(node);
					}
					current_depth++;
					continue;
				}
				if (t.value.strVal == "}")
				{
					if (cn_stack.size() <= 1) 
					{
						Logger::Get().Log("[Decoder] there are extra ones }.");
						return parent;
					}

					auto node = cn_stack.top();
					cn_stack.pop();
					cn_stack.top().children.push_back(node);

					///Конец области видимости переменных
					if (skip_scopes.empty() || !(skip_scopes.top().flag && skip_scopes.top().depth + 1 == current_depth))
					{
						CommandNode scope_end;
						scope_end.tokens.push_back(Token(TokenType::SCOPE_END, "#scope_end", -1, current_depth));	//TODO инрементировать, если надо
						cn_stack.top().children.push_back(scope_end);
					}
					else if (skip_scopes.top().flag && skip_scopes.top().depth + 1 == current_depth)
					{
						skip_scopes.pop();
					}
					current_depth--;
					continue;
				}
				if (t.type == TokenType::OP_END) 
				{
					if (command_tokens.size() != 0) 
					{
						CommandNode node;
						command_tokens.push_back(t);
						node.tokens = command_tokens;
						node.line = command_tokens.back().line;
						command_tokens.clear();
						cn_stack.top().children.push_back(node);
					}
					continue;
				}
				if (current_depth < 0) 
				{
					Logger::Get().Log("[Decoder] there are extra ones } .");
					return parent;
				}  
				command_tokens.push_back(t);
				
			}

			if (command_tokens.size() != 0) 
			{
				CommandNode node;
				node.tokens = command_tokens;
				node.line = command_tokens.back().line;
				command_tokens.clear();
				cn_stack.top().children.push_back(node);
			}

			if (current_depth > 0)
			{
				Logger::Get().Log("[Decoder] expected } .");
				return parent;
			}

			return cn_stack.top();
		}

		
	public:
		static Decoder Get()
		{
			static Decoder lexer;
			return lexer;
		}

		RawEC DecodeTokens(std::vector<Token>& tokens)
		{
			kwDecoder.ClearState();
			CommandNode nd = BuildCommandTree(tokens);

			auto cd = kwDecoder.BuildExecutionChain(nd);
			cd.push_back(Command(OpCode::EXIT));

			RawEC result;
			result.exit_rip = cd.size() - 1;
			auto f = kwDecoder.GetFunctionsSection();
			cd.insert(cd.end(), f.begin(), f.end());
			result.commands = cd;
			return result;
		}

	};
}