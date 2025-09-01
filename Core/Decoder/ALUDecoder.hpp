#pragma once
#include "..\Definitions\CommandsInfo.hpp"
#include "..\ErrorHandling\ErrorHandler.hpp"

namespace ELScript
{
	class ALUDecoder
	{
		private:
			bool IsAssignOperator(const Token& t) {
				if (t.type != TokenType::OPERATOR) return false;
				return OpCodeMap::IsAssign(t.value.strVal);
			}

			void ProcessDelayedAssignment(std::stack<Token>& operatorStack,
				std::vector<Command>& commands) {
				if (operatorStack.size() < 2) return;

				Token opToken = operatorStack.top();
				operatorStack.pop();
				Token varToken = operatorStack.top();
				operatorStack.pop();

				if (opToken.value.strVal == "=") {
					// ������� ������������
					commands.push_back(Command(OpCode::STORE, varToken.value.strVal, opToken.line));
				}
				else {
					std::string baseOp = opToken.value.strVal.substr(0, opToken.value.strVal.size() - 1);
					if (baseOp == "+") commands.push_back(Command(OpCode::ADD, Value(), opToken.line));
					else if (baseOp == "-") commands.push_back(Command(OpCode::SUB, Value(), opToken.line));
					else if (baseOp == "*") commands.push_back(Command(OpCode::MUL, Value(), opToken.line));
					else if (baseOp == "/") commands.push_back(Command(OpCode::DIV, Value(), opToken.line));
					else if (baseOp == "%") commands.push_back(Command(OpCode::MOD, Value(), opToken.line));

					commands.push_back(Command(OpCode::STORE, varToken.value.strVal, opToken.line));
				}
			}

			void AssignHandler(std::stack<Token>& operatorStack, std::vector<Command>& commands,
				const std::vector<Token>& operators) {
				if (operators.size() < 2) return;

				const Token& varToken = operators[0];
				const Token& opToken = operators[1];

				if (opToken.value.strVal == "=") {
					// ������� ������������ - �����������
					operatorStack.push(varToken);  // ������� ����������
					operatorStack.push(opToken);   // ����� ��������
				}
				else {
					// ��������� ������������ - ���� �����������
					operatorStack.push(varToken);
					operatorStack.push(opToken);
				}
			}
			void ApplyOperator(const Token& opToken, std::vector<Command>& commands,
				const std::string& varName = "") {
				std::string op = opToken.value.strVal;
				// ������ ������� ���������, �� ������������!
				if (OpCodeMap::HasOpCode(op)) {
					commands.push_back(Command(OpCodeMap::GetOpCode(op), Value(), opToken.line));
				}
			}
			void ProcessParenthesis(std::stack<Token>& operatorStack, std::vector<Command>& commands) {
				// ����������� ��� ��������� �� ����������� ������
				while (!operatorStack.empty()) {
					Token top = operatorStack.top();
					operatorStack.pop();

					if (top.type == TokenType::DELIMITER && top.value.strVal == "(") {
						break; // ����� ����������� ������
					}

					ApplyOperator(top, commands);
				}
			}
			void ProcessOperator(const Token& opToken, std::stack<Token>& operatorStack,
				std::vector<Command>& commands) {

				static const std::unordered_map<std::string, int> precedence = {
					{"||", 2}, {"&&", 2}, {"!", 2}, // �������� ��������� ! ��� ���������� ������
					{"==", 3}, {"!=", 3}, {"<", 3}, {">", 3}, {"<=", 3}, {">=", 3},
					{"+", 4}, {"-", 4},
					{"*", 5}, {"/", 5}, {"%", 5},
					{"(", 0} // ������ ���������
				};

				std::string currentOp = opToken.value.strVal;

				// ���������� ��������� ������������ - ��� �������������� ��������!
				if (IsAssignOperator(opToken)) {
					return;
				}

				// ����������� ��������� � higher or equal precedence
				while (!operatorStack.empty()) {
					Token top = operatorStack.top();

					// ��������������� �� ����������� ������
					if (top.type == TokenType::DELIMITER && top.value.strVal == "(") {
						break;
					}

					// ���������, ���� �� �������� � ������� �����������
					if (precedence.count(top.value.strVal) > 0 &&
						precedence.count(currentOp) > 0 &&
						precedence.at(currentOp) <= precedence.at(top.value.strVal)) {
						ApplyOperator(top, commands);
						operatorStack.pop();
					}
					else {
						break;
					}
				}

				operatorStack.push(opToken);
			}
			bool IsUnaryOperator(const Token& token, const std::vector<Token>& tokens, size_t index) {
				if (index < tokens.size() - 1 && (token.value.strVal == "-" || token.value.strVal == "!") && tokens[index + 1].value.strVal != "=") {
					// ������� ����: ������ ��������� ��� ����� ���������/������
					return index == 0 ||
						tokens[index - 1].type == TokenType::OPERATOR ||
						(tokens[index - 1].type == TokenType::DELIMITER &&
							tokens[index - 1].value.strVal == "(");
				}
				return false;
			}

			bool IsArrayAssign(std::vector<Token>& tokens, int index) 
			{
				for (size_t i = index; i < tokens.size(); i++) {
					Token& t = tokens[i];
					if (i + 1 < tokens.size() && t.type == TokenType::IDENTIFIER && tokens[i + 1].value.strVal == "[")
					{
						for (int j = i; j < tokens.size(); j++) 
						{
							if (j + 1 < tokens.size() && tokens[j].value.strVal == "]" && tokens[j + 1].value.strVal == "=") return true;
						}
					}
				}
				return false;
			}

	public:
		std::vector<Command> ExpressionHandler(std::vector<Token> tokens) {
			std::vector<Command> commands;
			std::stack<Token> operatorStack;

			std::vector<Value> array_of_values;
			bool array_mode = false;

			for (size_t i = 0; i < tokens.size(); i++) {
				Token& t = tokens[i];

				// ���������, �� �������� �� ��� �����������: �������, ���� �� ����� �������������� '['
				if (t.type == TokenType::IDENTIFIER && IsArrayAssign(tokens,i)) 
				{
					// ��� ������������ �������� �������! a[index] = value
					std::string arrayName = t.value.strVal;
					Token assignOp;
					int assign_index = -1;
					for (int j = i; j < tokens.size(); j++)
					{
						if (IsAssignOperator(tokens[j]))
						{
							assignOp = tokens[j];
							assign_index = j;
						}
					}
					// ���������� ���, �������� ������������ � '['
					

					// �������� ��������� ������� (���������� ������)
					std::vector<Token> indexExpr;
					int bracketDepth = 0;

					while (i < tokens.size()) {
						if (tokens[i].value.strVal == "[") bracketDepth++;
						else if (tokens[i].value.strVal == "]") bracketDepth--;
						else if (bracketDepth > 0){
							indexExpr.push_back(tokens[i]);
						}
						if (IsAssignOperator(tokens[i]))
						{
							break;
						}
						i++;
					}
					// i ������ �� ������ ����� ']'

					// �������� ��������� ������ ����� (��������)
					std::vector<Token> rightExpr;

					while (i < tokens.size()) {
						rightExpr.push_back(tokens[i]);
						i++;
					}

					auto indexCommands = ExpressionHandler(indexExpr);
					auto valueCommands = ExpressionHandler(rightExpr);

					// ���������� ��� ��� ������������ �� �������:
					// 1. ��������� �������� (������ �����)
					commands.insert(commands.end(), valueCommands.begin(), valueCommands.end());
					// 2. ��������� ������
					commands.insert(commands.end(), indexCommands.begin(), indexCommands.end());
					// 3. ��������� ������
					commands.push_back(Command(OpCode::LOAD, arrayName, t.line));
					// 4. �����������: stack -> [value, index, array] -> (������)
					commands.push_back(Command(OpCode::SET_INDEX, assignOp.value.strVal, t.line)); // ������� �������� ��� ��������� += � �.�.

					continue;

				}

				// ��������� ������������
				if (t.type == TokenType::IDENTIFIER &&
					i + 1 < tokens.size() && IsAssignOperator(tokens[i + 1])) {
					

					// ��� ��������� ����������: ����� ��������� ����������
					if (tokens[i + 1].value.strVal != "=")commands.push_back(Command(OpCode::LOAD, t.value.strVal, t.line));
					// ����������� �������� �� �����
					operatorStack.push(t);
					operatorStack.push(tokens[i + 1]);
					i++;

					continue;
				}
				else if (t.type == TokenType::IDENTIFIER) {

					if (i + 1 < tokens.size() && tokens[i + 1].value.strVal == "[") 
					{
						std::string arrayName = t.value.strVal;
						i += 2;// ���������� ��� ������� � ����������� ������ [
						// �������� ��������� ������ ������ (������) �� ����������� ]
						std::vector<Token> indexExpr;
						int bracketDepth = 1;

						while (i < tokens.size() && bracketDepth > 0) {
							if (tokens[i].value.strVal == "[") bracketDepth++;
							else if (tokens[i].value.strVal == "]") bracketDepth--;
							else {
								indexExpr.push_back(tokens[i]);
							}
							i++;
						}
						i--; // ������������ ������, �.�. ���� while �������� ����� ']'
						// ���������� ������������ ��������� �������
						auto indexCommands = ExpressionHandler(indexExpr);
						// ���������� ���:
						commands.insert(commands.end(), indexCommands.begin(), indexCommands.end()); // ��������� ������, ����� �� ����
						commands.push_back(Command(OpCode::LOAD, arrayName, t.line)); // ��������� ������ (��� ��� �����)
						commands.push_back(Command(OpCode::GET_INDEX, Value(), t.line)); // GET_INDEX: stack -> [array, index] -> value

						continue;
					}

					// ���������, �� �������� �� ��������� ����� ����������� ������� - ������� ������ �������
					else if (i + 1 < tokens.size() && tokens[i + 1].value.strVal == "(") {
						// ��� ����� �������!
						std::string funcName = t.value.strVal;
						// ���������� ��� ������� � ����������� ������
						i += 2;

						// �������� ��������� �� ����������� ������
						std::vector<Token> args;
						std::vector<Token> currentArg;
						std::vector<std::vector<Command>> inversed_args;
						int parenDepth = 1; // ��������� ��������� ������

						while (i < tokens.size() && parenDepth > 0) {
							if (tokens[i].value.strVal == "(") parenDepth++;
							else if (tokens[i].value.strVal == ")") parenDepth--;
							else if (tokens[i].value.strVal == "," && parenDepth == 1) {
								// ����������� ���������� - ������������ ��������� ��������
								if (!currentArg.empty()) {
									auto argCommands = ExpressionHandler(currentArg);
									inversed_args.push_back(argCommands);
									currentArg.clear();
								}
							}
							else {
								currentArg.push_back(tokens[i]);
							}
							i++;
						}
						i-=1;	//������������ ������ ��������
						// ������������ ��������� ��������
						if (!currentArg.empty()) {
							auto argCommands = ExpressionHandler(currentArg);
							inversed_args.push_back(argCommands);
						}

						for (int i = inversed_args.size() - 1; i >= 0; i--) 
						{
							commands.insert(commands.end(), inversed_args[i].begin(), inversed_args[i].end());
						}
						// ������ ��������� � ����� � �������: [argN, argN-1, ..., arg1]
						// ��������� ����� �������
						commands.push_back(Command(OpCode::CALL, funcName, t.line));
						continue;
					}
					else {
						// ������� ����������
						commands.push_back(Command(OpCode::LOAD, t.value, t.line));
					}
				}
				// ��������
				if (t.type == TokenType::LITERAL) {
					if (!array_mode)commands.push_back(Command(OpCode::PUSH, t.value, t.line));
					else array_of_values.push_back(t.value);
				}
				// ���������
				else if (t.type == TokenType::OPERATOR) {
					if (IsUnaryOperator(t, tokens, i)) {
						Token unaryToken = t;
						if (unaryToken.value.strVal == "-") unaryToken.value.strVal = "-u";
						ProcessOperator(unaryToken, operatorStack, commands);
					}
					else {
						ProcessOperator(t, operatorStack, commands);
					}
				}

				// ������
				else if (t.type == TokenType::DELIMITER) {
					if (t.value.strVal == "(") {
						operatorStack.push(t);
					}
					else if (t.value.strVal == ")") {
						ProcessParenthesis(operatorStack, commands);
					}
					else if (t.value.strVal == "[") 
					{
						array_mode = true;
					}
					else if (t.value.strVal == "]")
					{
						array_mode = false;
						auto v = Value(std::make_shared<std::vector<Value>>(array_of_values));
						commands.push_back(Command(OpCode::PUSH, v, t.line));
						array_of_values.clear();
					}
				}
				else if (t.type == TokenType::KEYWORD) 
				{
					if (t.value.strVal == "meta")
					{
						if (i < tokens.size() - 2 && tokens[i + 1].value.strVal == ":" && tokens[i + 2].type == TokenType::IDENTIFIER) 
						{
							commands.push_back(Command(OpCode::LOADM, tokens[i + 2].value, t.line));
							i += 2;
						}
						else 
						{
							ErrorHandlerManager::RaiseError(EHMessage(EHMessageType::Error, "[ALUDecoder] invalid meta variable."));
						}
					}
					
				}
			}
			while (!operatorStack.empty()) {
				Token top = operatorStack.top();

				// ���� ��� �������� ������������ - ������������ ��������
				if (IsAssignOperator(top)) {
					ProcessDelayedAssignment(operatorStack, commands);
				}
				// ���� ��� ������� ��������
				else if (top.type == TokenType::OPERATOR && !IsAssignOperator(top)) {
					ApplyOperator(top, commands);
					operatorStack.pop();
				}
				// ���� ��� ����������� ������ - ������, �� ����������
				else if (top.type == TokenType::DELIMITER && top.value.strVal == "(") {
					operatorStack.pop(); // ���������� ������ - ����������
				}
				else {
					// ���-�� unexpected - ������ �������
					operatorStack.pop();
				}
			}
			return commands;
		}
	};
}