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
					// Простое присваивание
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
					// Простое присваивание - откладываем
					operatorStack.push(varToken);  // Сначала переменная
					operatorStack.push(opToken);   // Затем оператор
				}
				else {
					// Составное присваивание - тоже откладываем
					operatorStack.push(varToken);
					operatorStack.push(opToken);
				}
			}
			void ApplyOperator(const Token& opToken, std::vector<Command>& commands,
				const std::string& varName = "") {
				std::string op = opToken.value.strVal;
				// Только обычные операторы, не присваивания!
				if (OpCodeMap::HasOpCode(op)) {
					commands.push_back(Command(OpCodeMap::GetOpCode(op), Value(), opToken.line));
				}
			}
			void ProcessParenthesis(std::stack<Token>& operatorStack, std::vector<Command>& commands) {
				// Выталкиваем все операторы до открывающей скобки
				while (!operatorStack.empty()) {
					Token top = operatorStack.top();
					operatorStack.pop();

					if (top.type == TokenType::DELIMITER && top.value.strVal == "(") {
						break; // Нашли открывающую скобку
					}

					ApplyOperator(top, commands);
				}
			}
			void ProcessOperator(const Token& opToken, std::stack<Token>& operatorStack,
				std::vector<Command>& commands) {

				static const std::unordered_map<std::string, int> precedence = {
					{"||", 2}, {"&&", 2}, {"!", 2}, // Увеличил приоритет ! для правильной работы
					{"==", 3}, {"!=", 3}, {"<", 3}, {">", 3}, {"<=", 3}, {">=", 3},
					{"+", 4}, {"-", 4},
					{"*", 5}, {"/", 5}, {"%", 5},
					{"(", 0} // низший приоритет
				};

				std::string currentOp = opToken.value.strVal;

				// Пропускаем операторы присваивания - они обрабатываются отдельно!
				if (IsAssignOperator(opToken)) {
					return;
				}

				// Выталкиваем операторы с higher or equal precedence
				while (!operatorStack.empty()) {
					Token top = operatorStack.top();

					// Останавливаемся на открывающей скобке
					if (top.type == TokenType::DELIMITER && top.value.strVal == "(") {
						break;
					}

					// Проверяем, есть ли оператор в таблице приоритетов
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
					// Унарный если: начало выражения или после оператора/скобки
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

			void ParseIndexAssign(std::vector<Token>& tokens, size_t& i, std::vector<Command>& commands) 
			{
				Token t = tokens[i];
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
				// Пропускаем имя, оператор присваивания и '['
				// Собираем выражение индекса (аналогично чтению)
				std::vector<Token> indexExpr;
				int bracketDepth = 0;

				while (i < tokens.size()) {
					if (tokens[i].value.strVal == "[") bracketDepth++;
					else if (tokens[i].value.strVal == "]") bracketDepth--;
					else if (bracketDepth > 0) {
						indexExpr.push_back(tokens[i]);
					}
					if (IsAssignOperator(tokens[i]))
					{
						break;
					}
					i++;
				}
				// i теперь на токене после ']'

				// Собираем выражение правой части (значение)
				std::vector<Token> rightExpr;

				while (i < tokens.size()) {
					rightExpr.push_back(tokens[i]);
					i++;
				}

				auto indexCommands = ExpressionHandler(indexExpr);
				auto valueCommands = ExpressionHandler(rightExpr);

				// Генерируем код для присваивания по индексу:
				// 1. Вычисляем значение (правая часть)
				commands.insert(commands.end(), valueCommands.begin(), valueCommands.end());
				// 2. Вычисляем индекс
				commands.insert(commands.end(), indexCommands.begin(), indexCommands.end());
				// 3. Загружаем массив
				commands.push_back(Command(OpCode::LOAD, arrayName, t.line));
				// 4. Присваиваем: stack -> [value, index, array] -> (ничего)
				commands.push_back(Command(OpCode::SET_BY, assignOp.value.strVal, t.line)); // Передаём оператор для составных += и т.д.

			}
			
			std::shared_ptr<std::unordered_map<std::string, Value>> ParseDict(std::vector<Token>& tokens, size_t& i, size_t line) {
				std::unordered_map<std::string, Value> dict;
				return std::make_shared<std::unordered_map<std::string, Value>>(dict);
			}
			std::vector<Command> ParseArray(std::vector<Token>& tokens, size_t& i, size_t line) {
				std::vector<std::vector<Token>> expressions;
				i++; //Скипаем [

				std::vector<Token> temp_tokens;

				std::vector<Command> commands;

				for (; i < tokens.size(); i++) 
				{
					Token& t = tokens[i];

					if (t.value.strVal == ",") 
					{
						if (!temp_tokens.empty()) expressions.push_back(temp_tokens);
						temp_tokens.clear();
						continue;
					}
					if (t.value.strVal == "]") 
					{
						if (!temp_tokens.empty())expressions.push_back(temp_tokens);
						temp_tokens.clear();
						break;
					}
					temp_tokens.push_back(t);
				}

				for (int j = expressions.size() - 1; j >= 0; j--) 
				{
					auto commands_ = ExpressionHandler(expressions[j]);
					commands.insert(commands.end(), commands_.begin(), commands_.end());
				}
				commands.push_back(Command(OpCode::ARRAY_MAKE, (int)expressions.size()));
				return commands;
			}

			std::vector<Command> ParseDictionary(std::vector<Token>& tokens, size_t& i, size_t line) {
				std::vector<Command> commands;
				i++; // Пропускаем '['

				std::vector<std::pair<std::vector<Token>, std::vector<Token>>> pairs; // пары [ключ, значение]

				// Парсим пары ключ:значение
				while (i < tokens.size() && tokens[i].value.strVal != "]") {
					// Парсим ключ (до двоеточия)
					std::vector<Token> key_tokens;
					while (i < tokens.size() && tokens[i].value.strVal != ":") {
						key_tokens.push_back(tokens[i]);
						i++;
					}

					if (i >= tokens.size()) 
					{
						ErrorHandlerManager::RaiseError(EHMessage(EHMessageType::Error, "[ALUDecoder] Expected ':'"));
					}  
					i++; // Пропускаем ':'   

					// Парсим значение (до запятой или ])
					std::vector<Token> value_tokens;
					while (i < tokens.size() && tokens[i].value.strVal != "," && tokens[i].value.strVal != "]") {
						value_tokens.push_back(tokens[i]);
						i++;
					}

					pairs.push_back({ key_tokens, value_tokens });

					if (i < tokens.size() && tokens[i].value.strVal == ",") {
						i++; // Пропускаем ','
					}
				}

				if (i >= tokens.size() || tokens[i].value.strVal != "]") {
					throw std::runtime_error("Expected ']'");
				}
				i++; // Пропускаем ']'

				// Генерируем код в обратном порядке
				for (auto it = pairs.rbegin(); it != pairs.rend(); ++it) {
					// Ключ
					auto key_cmds = ExpressionHandler(it->first);
					commands.insert(commands.end(), key_cmds.begin(), key_cmds.end());
					// Значение
					auto value_cmds = ExpressionHandler(it->second);
					commands.insert(commands.end(), value_cmds.begin(), value_cmds.end());

					
				}

				commands.push_back(Command(OpCode::DICT_MAKE, Value((int)pairs.size()), line));
				return commands;
			}
			bool IsLikelyDictionary(std::vector<Token>& tokens, size_t start_index) {
				size_t j = start_index;
				while (j < tokens.size() && tokens[j].value.strVal != "]") {
					// Если находим двоеточие ':' — это скорее всего словарь
					if (tokens[j].value.strVal == ":") {
						return true;
					}
					// Если находим запятую до любого двоеточия — это скорее массив
					if (tokens[j].value.strVal == ",") {
						return false;
					}
					j++;
				}
				return false;
			}
	public:
		std::vector<Command> ExpressionHandler(std::vector<Token> tokens) {
			std::vector<Command> commands;
			std::stack<Token> operatorStack;

			

			for (size_t i = 0; i < tokens.size(); i++) {
				Token& t = tokens[i];

				// Проверяем, не является ли это индексацией: смотрим, есть ли после идентификатора '['
				if (t.type == TokenType::IDENTIFIER && IsArrayAssign(tokens,i)) 
				{
					// Это присваивание элементу массива! a[index] = value
					ParseIndexAssign(tokens, i, commands);		//Для словаря тоже работает, ну для обращений по индексу или ключу
					continue;
				}

				// Обработка присваивания
				if (t.type == TokenType::IDENTIFIER &&
					i + 1 < tokens.size() && IsAssignOperator(tokens[i + 1])) {
					

					// Для составных операторов: сразу загружаем переменную
					if (tokens[i + 1].value.strVal != "=")commands.push_back(Command(OpCode::LOAD, t.value.strVal, t.line));
					// Откладываем операцию на потом
					operatorStack.push(t);
					operatorStack.push(tokens[i + 1]);
					i++;

					continue;
				}
				else if (t.type == TokenType::IDENTIFIER) {

					if (i + 1 < tokens.size() && tokens[i + 1].value.strVal == "[") //работает и для словарей
					{
						std::string arrayName = t.value.strVal;
						i += 2;// Пропускаем имя массива и открывающую скобку [
						// Собираем выражение внутри скобок (индекс) до закрывающей ]
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
						i--; // Корректируем индекс, т.к. цикл while перебрал токен ']'
						// Рекурсивно обрабатываем выражение индекса
						auto indexCommands = ExpressionHandler(indexExpr);
						// Генерируем код:

						commands.insert(commands.end(), indexCommands.begin(), indexCommands.end()); // Вычисляем индекс, кладём на стек
						commands.push_back(Command(OpCode::LOAD, arrayName, t.line)); // Загружаем массив (или его адрес)

						commands.push_back(Command(OpCode::GET_BY, Value(), t.line)); // GET_INDEX: stack -> [array, index] -> value
						continue;
					}

					// Проверяем, не является ли следующий токен открывающей скобкой - признак вызова функции
					else if (i + 1 < tokens.size() && tokens[i + 1].value.strVal == "(") {
						// Это вызов функции!
						std::string funcName = t.value.strVal;
						// Пропускаем имя функции и открывающую скобку
						i += 2;

						// Собираем аргументы до закрывающей скобки
						std::vector<Token> args;
						std::vector<Token> currentArg;
						std::vector<std::vector<Command>> inversed_args;
						int parenDepth = 1; // Учитываем вложенные скобки

						while (i < tokens.size() && parenDepth > 0) {
							if (i < tokens.size() - 1 && tokens[i].type == TokenType::IDENTIFIER && tokens[i + 1].value.strVal == "(") 
							{
								std::vector<Token> inner_function_call;
								inner_function_call.push_back(tokens[i]);
								inner_function_call.push_back(tokens[i+1]);
								i += 2;
								int start = parenDepth;
								parenDepth++;
								for (; i < tokens.size(); i++) 
								{
									Token t = tokens[i];
									if (tokens[i].value.strVal == "(") parenDepth++;
									else if (tokens[i].value.strVal == ")" && parenDepth  == start + 1)
									{
										inner_function_call.push_back(tokens[i]);
										parenDepth--;
										break;
									}
									else if (tokens[i].value.strVal == ")")parenDepth--;
									inner_function_call.push_back(tokens[i]);
								}
								auto f_call = ExpressionHandler(inner_function_call);
								inversed_args.push_back(f_call);
								i++;
							}
							if (tokens[i].value.strVal == "(") parenDepth++;
							else if (tokens[i].value.strVal == ")") parenDepth--;
							else if (tokens[i].value.strVal == "," && parenDepth == 1) {
								// Разделитель аргументов - обрабатываем собранный аргумент
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
						i-=1;	//компенсируем лишнюю операцию
						// Обрабатываем последний аргумент
						if (!currentArg.empty()) {
							auto argCommands = ExpressionHandler(currentArg);
							inversed_args.push_back(argCommands);
						}

						for (int i = inversed_args.size() - 1; i >= 0; i--) 
						{
							commands.insert(commands.end(), inversed_args[i].begin(), inversed_args[i].end());
						}
						// Теперь аргументы в стеке в порядке: [argN, argN-1, ..., arg1]
						// Добавляем вызов функции
						commands.push_back(Command(OpCode::CALL, funcName, t.line));
						continue;
					}
					else {
						// Обычная переменная
						commands.push_back(Command(OpCode::LOAD, t.value, t.line));
					}
				}
				// Литералы
				if (t.type == TokenType::LITERAL) {
					commands.push_back(Command(OpCode::PUSH, t.value, t.line));
				}
				// Операторы
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

				// Скобки
				else if (t.type == TokenType::DELIMITER) {
					if (t.value.strVal == "(") {
						operatorStack.push(t);
					}
					else if (t.value.strVal == ")") {
						ProcessParenthesis(operatorStack, commands);
					}
					else if (t.value.strVal == "[")
					{
						bool is_dict = IsLikelyDictionary(tokens, i + 1);

						std::vector<Command> commands1;
						if (is_dict) {
							commands1 = ParseDictionary(tokens, i, t.line);
						}
						else {
							commands1 = ParseArray(tokens, i, t.line);
						}

						commands.insert(commands.end(), commands1.begin(), commands1.end());
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

				// Если это оператор присваивания - обрабатываем отдельно
				if (IsAssignOperator(top)) {
					ProcessDelayedAssignment(operatorStack, commands);
				}
				// Если это обычный оператор
				else if (top.type == TokenType::OPERATOR && !IsAssignOperator(top)) {
					ApplyOperator(top, commands);
					operatorStack.pop();
				}
				// Если это открывающая скобка - ошибка, но пропускаем
				else if (top.type == TokenType::DELIMITER && top.value.strVal == "(") {
					operatorStack.pop(); // Незакрытая скобка - пропускаем
				}
				else {
					// Что-то unexpected - просто удаляем
					operatorStack.pop();
				}
			}
			return commands;
		}
	};
}