#pragma once
#include "..\Definitions\CommandsInfo.hpp"
#include "..\Definitions\StringOperations.hpp"
namespace ELScript
{
	class Lexer
	{
	private:
		std::unordered_map<std::string, TokenType> tokentypes =
		{
			{"+", TokenType::OPERATOR},
			{"-", TokenType::OPERATOR},
			{"/", TokenType::OPERATOR},
			{"*", TokenType::OPERATOR},
			{"%", TokenType::OPERATOR},
			{"=", TokenType::OPERATOR},
			{"+=", TokenType::OPERATOR},
			{"-=", TokenType::OPERATOR},
			{"/=", TokenType::OPERATOR},
			{"*=", TokenType::OPERATOR},
			{"==", TokenType::OPERATOR},
			{"!=", TokenType::OPERATOR},
			{"!", TokenType::OPERATOR},
			{">",TokenType::OPERATOR},
			{"<",TokenType::OPERATOR},
			{">=",TokenType::OPERATOR},
			{"<=",TokenType::OPERATOR},
			{"&&",TokenType::OPERATOR},
			{"||",TokenType::OPERATOR},
			{"false", TokenType::LITERAL},
			{"true", TokenType::LITERAL},
			{"(", TokenType::DELIMITER},
			{")", TokenType::DELIMITER},
			{"{", TokenType::DELIMITER},
			{"}", TokenType::DELIMITER},
			{"[", TokenType::DELIMITER},
			{"]", TokenType::DELIMITER},
			{":", TokenType::DELIMITER},
			{",", TokenType::DELIMITER},
			{";",TokenType::DELIMITER},
			{"bool", TokenType::TYPE_MARKER},
			{"number", TokenType::TYPE_MARKER},
			{"string", TokenType::TYPE_MARKER},
			{"array", TokenType::TYPE_MARKER},
			{"void", TokenType::TYPE_MARKER},
			{"dict", TokenType::TYPE_MARKER},
			{"if", TokenType::KEYWORD},
			{"while", TokenType::KEYWORD},
			{"for", TokenType::KEYWORD},
			{"continue", TokenType::KEYWORD},
			{"break", TokenType::KEYWORD},
			{"print_value", TokenType::KEYWORD},	//Специальное слово
			{"var", TokenType::KEYWORD}, 
			{"func", TokenType::KEYWORD},
			{"return", TokenType::KEYWORD},
			{"op_code",TokenType::KEYWORD},
			{"import",TokenType::KEYWORD},
			{"meta",TokenType::KEYWORD},
		};
		std::string undefined_token = std::string();
		int current_line = 0;
		int current_depth = 0;
		bool was_comment = false;
		bool is_negative_value = false; // -10
		std::string ProcessStringLiteral(const std::string& str) {
			std::string result;
			result.reserve(str.size()); // Оптимизация памяти

			for (size_t i = 0; i < str.size(); ++i) {
				if (str[i] == '\\' && i + 1 < str.size()) {
					switch (str[i + 1]) {
					case 'n': result += '\n'; break;
					case 't': result += '\t'; break;
					case '"': result += '"'; break;  // Это нужно!
					case '\\': result += '\\'; break;
					default:
						// Неизвестная escape-последовательность - оставляем как есть
						result += str[i];
						continue; // не пропускаем следующий символ
					}
					i++; // Пропускаем обработанный символ
				}
				else {
					result += str[i];
				}
			}
			return result;
		}


		TokenType GetTokenType(const std::string& token) //Простой и быстрый способ для получения токенов
		{
			if (tokentypes.count(token) != 0)
			{
				return tokentypes[token];
			}

			if (StringOperations::IsNumber(token)) return TokenType::LITERAL;

			if (token.size() >= 2 && token[0] == '"' && token[token.size() - 1] == '"') return TokenType::LITERAL;

			return TokenType::IDENTIFIER;
		}

		Value GetTokenValue(const std::string& token) //Транслирует в строки/цифры и тд
		{
			if (token == "true") return true;
			else if (token == "false") return false;

			if (StringOperations::IsNumber(token))
			{
				if (is_negative_value)
				{
					is_negative_value = false;
					return -std::stod(token);
				}
				return std::stod(token);
			}
			if (token.size() >= 2 && token[0] == '"' && token.back() == '"') {
				return ProcessStringLiteral(token.substr(1, token.size() - 2));
			}
			return token;
		}


		Token CreateIdentifierOrKeyword(std::string operand, int line) 
		{
			Token t;
			t.line = line;

			//Проверка на ключевое слово в будущем
			
			t.type = GetTokenType(operand);
			t.value = GetTokenValue(operand);
			t.depth = current_depth;
			return t; 
		}

		char GetNearCharWithoutSpaces(const std::string& text, int start_index, int step = 1) //Чтоб можно было идти в двух направлениях 
		{
			int i = start_index + step;
			while (i >= 0 && i < text.size()) 
			{
				if (text[i] != ' ' && text[i] != '\t' && text[i] != '\r' && text[i] != '\n') return text[i];
				i += step;
			}
		}

		Token InsertOpEnd(std::vector<Token>& tokens, int current_index, const std::string& text)
		{
			if (tokens.size() == 0)return Token(TokenType::NOP);
			if (text[current_index] == ';')return Token(TokenType::OP_END, "#op_end", current_line, current_depth);
			if (tokens.back().type == TokenType::OP_END || tokens.back().value.strVal == "{" && undefined_token.empty()) return Token(TokenType::NOP);

			if (tokens.back().type != TokenType::OP_END && tokens.back().value.strVal != "}" && text[current_index] == '}')
			{
				return Token(TokenType::OP_END, "#op_end", current_line, current_depth);
			}

			if (text[current_index] == '\n' && !(GetNearCharWithoutSpaces(text, current_index, 1) == '{' || GetNearCharWithoutSpaces(text, current_index, -1) == '}'))
			{
				return Token(TokenType::OP_END, "#op_end", current_line, current_depth);
			}

			
		}

	public:
		static Lexer Get()
		{
			static Lexer lexer;
			return lexer;
		}


        std::vector<Token> ToTokens(std::string text) {
            std::vector<Token> result;
            current_line = 1;
			current_depth = 0;
			was_comment = false;
			undefined_token = std::string();
			is_negative_value = false;
			bool in_string = false;

			

			for (int i = 0; i < text.size(); i++) 
			{
				char c = text[i];
				if (c == '\r') continue;

				// Пропуск однострочных комментариев
				if (!in_string) 
				{
					if (c == '/' && i + 1 < text.size() && text[i + 1] == '/') {
						while (i < text.size() && text[i] != '\n')
						{
							i++;
						}
						was_comment = true;
						i--;	//чтобы вернуть \n
						continue;
					}

					if ((c == ' ' || c == '\n' || c == '\t'))
					{
						if (!undefined_token.empty())
						{
							result.push_back(CreateIdentifierOrKeyword(undefined_token, current_line));
						}
						undefined_token.clear();
						if (c == '\n')
						{
							current_line++;
						}
					}

					auto t = InsertOpEnd(result, i, text);
					if (t.type != TokenType::NOP) 
					{
						if (!undefined_token.empty())
						{
							result.push_back(CreateIdentifierOrKeyword(undefined_token, current_line));
							undefined_token.clear();
						}
						result.push_back(t);
					}
					if (was_comment) was_comment = false;


					//Односимвольных операторы
					if (GetTokenType(std::string(1, c)) == TokenType::OPERATOR)
					{
						std::string operator_ = std::string({ c });
						if (!undefined_token.empty())	//Сразу добавляем токен перед ним, если есть
						{
							result.push_back(CreateIdentifierOrKeyword(undefined_token, current_line));
							undefined_token.clear();
						}

						if (i < text.size() - 1 && GetTokenType(std::string({ c, text[i + 1] })) == TokenType::LITERAL && (result.size() == 0 || (result.back().type != TokenType::LITERAL && result.back().type != TokenType::IDENTIFIER))) //если число
						{
							is_negative_value = true;
							continue;
						}

						//Проверка на двусимвольных операторы, они обычно состоят из односимвольных операторов
						if (i < text.size() - 1 && GetTokenType(std::string({ c, text[i + 1] })) == TokenType::OPERATOR)
						{
							operator_ = std::string({ c, text[i + 1] });
							i++;
						}
						Token t;
						t.line = current_line;
						t.type = TokenType::OPERATOR;
						t.value = operator_;
						t.depth = current_depth;
						result.push_back(t);
						continue;
					}


					if (GetTokenType(std::string(1, c)) == TokenType::DELIMITER)
					{
						std::string delimiter = std::string({ c });
						if (!undefined_token.empty())	//Сразу добавляем токен перед ним, если есть
						{
							result.push_back(CreateIdentifierOrKeyword(undefined_token, current_line));
							undefined_token.clear();
						}
						Token t;
						t.line = current_line;
						t.type = TokenType::DELIMITER;
						t.value = delimiter;
						t.depth = current_depth;
						if (c == '{') current_depth++;
						if (c == '}')
						{
							current_depth--;
							t.depth = current_depth;
						}

						result.push_back(t);
						continue;
					}
				}
				

				if (c == '\"' && undefined_token.empty()) 
				{
					in_string = true;
					undefined_token.push_back(c);
					continue;
				}
				if (c == '\"' && !undefined_token.empty())
				{
					in_string = false;
					undefined_token.push_back(c);
					result.push_back(CreateIdentifierOrKeyword(undefined_token, current_line));
					undefined_token.clear();
					continue;
				}
				if ((c == ' ' || c == '\n' || c == '\t') && !in_string) continue;
				undefined_token.push_back(c);
			}

			if (!undefined_token.empty()) 
			{
				Token t;
				t.line = current_line;
				t.depth = current_depth;
				t.type = GetTokenType(undefined_token);
				t.value = undefined_token;
				result.push_back(t);
				result.push_back(Token(TokenType::OP_END, "#op_end", current_line, current_depth));
			}
			
			return result;
        }
	};
}