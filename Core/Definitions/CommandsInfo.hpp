#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <stack>
#include <unordered_set>

namespace ELScript
{
    constexpr int MaxCallStackDepth = 65535;
    enum ValueType {VOID, NUMBER, STRING, BOOL, ARRAY, DICT};

    struct Value {
        ValueType type;

        union {
            double numberVal;
            bool boolVal; 
        };
        std::string strVal;   // Для строк 
        std::shared_ptr<std::vector<Value>> arrayVal; // Для массивов
        std::shared_ptr<std::unordered_map<std::string,Value>> dictVal; // Для массивов
        // Конструкторы
        Value() : type(ValueType::VOID), numberVal(0) {  } 

        Value(double v) : type(ValueType::NUMBER), numberVal(v) {}
        Value(int v) : type(ValueType::NUMBER), numberVal(v) {}

        Value(bool v) : type(ValueType::BOOL), boolVal(v) {}

        Value(const std::string& s) : type(ValueType::STRING) {
            new (&strVal) std::string(s); 
        }

        Value(const char* s) : type(ValueType::STRING) {
            new (&strVal) std::string(s);
        }

        Value(std::shared_ptr<std::vector<Value>> arr) : type(ValueType::ARRAY), arrayVal(arr) {}
        Value(std::shared_ptr<std::unordered_map<std::string, Value>> map) : type(ValueType::DICT), dictVal(map) {}
        // ПРАВИЛО ПЯТИ (очень важно!)
        // Конструктор копирования
        Value(const Value& other) : type(other.type) {
            switch (type) {
            case ValueType::NUMBER: numberVal = other.numberVal; break;
            case ValueType::BOOL:   boolVal = other.boolVal; break;
            case ValueType::STRING: new (&strVal) std::string(other.strVal); break; // Копируем строку
            case ValueType::ARRAY:  arrayVal = other.arrayVal; break; // Копируем shared_ptr
            case ValueType::DICT:  dictVal = other.dictVal; break; // Копируем shared_ptr
            case ValueType::VOID:   break; // Ничего не делаем
            }
        }

        // Оператор присваивания
        Value& operator=(const Value& other) {
            // Если текущий объект уже хранит строку, её нужно уничтожить
            if (this == &other) return *this; // Защита от самоприсваивания
            destroyCurrent();

            type = other.type;
            switch (type) {
            case ValueType::NUMBER: numberVal = other.numberVal; break;
            case ValueType::BOOL:   boolVal = other.boolVal; break;
            case ValueType::STRING: new (&strVal) std::string(other.strVal); break;
            case ValueType::ARRAY:  arrayVal = other.arrayVal; break;
            case ValueType::DICT:  dictVal = other.dictVal; break;
            case ValueType::VOID:   break;
            }
            return *this;
        }
        Value(Value&& other) noexcept : type(other.type) {
            switch (type) {
            case ValueType::NUMBER: numberVal = other.numberVal; break;
            case ValueType::BOOL:   boolVal = other.boolVal; break;
            case ValueType::STRING: new (&strVal) std::string(std::move(other.strVal)); break;
            case ValueType::ARRAY:  arrayVal = std::move(other.arrayVal); break; // Важно: перемещаем shared_ptr
            case ValueType::DICT:  dictVal = std::move(other.dictVal); break;
            case ValueType::VOID:   break;
            }
            // После перемещения нужно привести исходный объект в безопасное состояние!
            other.type = ValueType::VOID; // Чтобы деструктор other ничего не делал
        }

        // Оператор перемещающего присваивания
        Value& operator=(Value&& other) noexcept {
            if (this != &other) {
                destroyCurrent(); // Уничтожаем текущие данные
                type = other.type;
                switch (type) {
                case ValueType::NUMBER: numberVal = other.numberVal; break;
                case ValueType::BOOL:   boolVal = other.boolVal; break;
                case ValueType::STRING: new (&strVal) std::string(std::move(other.strVal)); break;
                case ValueType::ARRAY:  arrayVal = std::move(other.arrayVal); break;
                case ValueType::DICT:  dictVal = std::move(other.dictVal); break;
                case ValueType::VOID:   break;
                }
                other.type = ValueType::VOID;
            }
            return *this;
        }

        std::string ToString() const 
        {
            std::string result;
            switch (type)
            {
            case ELScript::VOID:
                return "";
            case ELScript::NUMBER:
                return std::to_string(numberVal);
            case ELScript::STRING:
                return strVal;
            case ELScript::BOOL:
                return boolVal ? "true" : "false";
            case ELScript::ARRAY:
                for (auto l : *arrayVal)
                {
                    result += l.ToString() + " ";
                }
                return result;
            case ELScript::DICT:
                for (auto l : *dictVal)
                {
                    result += l.first + ":" + l.second.ToString() + " ";
                }
                return result;
            default:
                return "";
            }
        }
        static std::string GetTypeString(ValueType type) 
        {
            std::string result;
            switch (type)
            {
            case ELScript::VOID:
                return "void";
            case ELScript::NUMBER:
                return "number";
            case ELScript::STRING:
                return "string";
            case ELScript::BOOL:
                return "bool";
            case ELScript::ARRAY:
                return "array";
            case ELScript::DICT:
                return "dict";
            default:
                return "void";
            }
        }
        // Деструктор
        ~Value() {
            destroyCurrent();
        }

    private:
        void destroyCurrent() {
            // Явно вызываем деструктор только для строки
            if (type == ValueType::STRING) {
                strVal.~basic_string();
            }
            // Для shared_ptr и примитивов деструктор вызывать не нужно
        }
    };

    // Типы токенов (упрощённо)
    enum class TokenType {
        NOP,
        IDENTIFIER,// Переменные, ключевые слова
        OPERATOR,  // +, -, =, *, /, >, <
        LITERAL,   // Числа, строки, true/false
        DELIMITER,
        OP_END, //Токен конца операции
        SCOPE_START,
        SCOPE_END,
        TYPE_MARKER,
        KEYWORD
    };

    // Структура токена
    struct Token {
        TokenType type;
        Value value;
        size_t line; // Для ошибок
        size_t depth;  // Уровень вложенности
    };

    struct CommandNode
    {
        std::vector<Token> tokens;
        std::vector<CommandNode> children;
        int line;
    };

    enum class OpCode
    {
        NOP = 1,
        LABEL, //Метка
        EXIT,   //Выход из программы

        PUSH,
        POP,
        SWAP,
        CALL,
        RET,
        ADD,
        SUB,
        MUL,
        DIV,
        MOD,//%
        NEG,//- NEGATIVE
        //Logic
        EQUAL,
        EQUAL_N,
        GREATER,
        LESS,
        EGREATER,
        ELESS,
        AND,
        OR,
        NOT,
        //Variables
        STORE,
        LOAD,
        LOADM,  //LOAD META
        //IF A- Absolute, R-relative
        JMPA,
        JMPA_IF_N,
        JMPA_IF,

        PRINT,

        //Special
        EXCEPT_HANDLING, // Особый маркер для ключевых слов, которые не могут сразу быть полностью обработаны, value в Command хранит в себе ключевое слово
        DECLARE,
        DECLARED,   //Проверка на наличие переменной в current scope
        FUNC_DECLARE,
        FUNC_DECLARED,
        SCOPESTR,   //Начало новой области видимости переменных
        SCOPEEND,    //Конец области видимости переменных

        GET_TYPE_STR,
        CONVERT_TYPE,        //конвертация значения в number,string,bool

        //Работа с массивами
        ARRAY_MAKE,
        GET_BY,
        SET_BY,
        ARRAY_PUSH_BACK,
        ARRAY_POP_BACK,
        ARRAY_INSERT_INDEX,
        ARRAY_ERASE_INDEX,
        ARRAY_SIZE,

        //Работа с хеш-картой
        DICT_MAKE,
        DICT_SIZE,
        DICT_ERASE,
    };
    
    struct Command 
    {
        OpCode code = OpCode::NOP;
        Value operand;  //Константное значение команды, например, для определения числа и тд
        int line; // строка исходного кода
        Command(OpCode code, Value operand)
        {
            this->code = code;
            this->operand = operand;
        }
        Command(OpCode code)
        {
            this->code = code;
        }
        Command(OpCode code, Value operand,int line)
        {
            this->code = code;
            this->operand = operand;
            this->line = line;
        }
        Command() 
        {
            this->code = OpCode::NOP;
        }
    };

   
    class OpCodeMap 
    {
    private:
        static std::unordered_map<std::string, OpCode> GetOpCodeMap() 
        {
            static const std::unordered_map<std::string, OpCode> opCodeMap = {
                // Арифметические
                {"+", OpCode::ADD},
                {"-", OpCode::SUB},
                {"*", OpCode::MUL},
                {"/", OpCode::DIV},
                {"%", OpCode::MOD},
                {"-u", OpCode::NEG}, // унарный минус

                // Логические
                {"==", OpCode::EQUAL},
                {"!=", OpCode::EQUAL_N}, // + NOT после
                {">", OpCode::GREATER},
                {"<", OpCode::LESS},
                {">=", OpCode::EGREATER},
                {"<=", OpCode::ELESS},
                {"&&", OpCode::AND},
                {"||", OpCode::OR},
                {"!", OpCode::NOT},

                // Присваивания (обрабатываются отдельно)
                {"=", OpCode::STORE},
                {"+=", OpCode::ADD},
                {"-=", OpCode::SUB},
                {"*=", OpCode::MUL},
                {"/=", OpCode::DIV},
                {"%=", OpCode::MOD}
            };
            return opCodeMap;
        }
        static std::unordered_set<std::string> GetAssignOpCodeMap()
        {
            static const std::unordered_set<std::string> assignOps = {
                     "=", "+=", "-=", "*=", "/=", "%="
            };
            return assignOps;
        }

        static std::unordered_map<std::string, OpCode> GetStringOpCodeMap() 
        {
            static const std::unordered_map<std::string, OpCode> opcodeMap = {
                {"nop", OpCode::NOP},
                {"label", OpCode::LABEL},
                {"exit", OpCode::EXIT},
                {"push", OpCode::PUSH},
                {"pop", OpCode::POP},
                {"call", OpCode::CALL},
                {"ret", OpCode::RET},
                {"add", OpCode::ADD},
                {"sub", OpCode::SUB},
                {"mul", OpCode::MUL},
                {"div", OpCode::DIV},
                {"mod", OpCode::MOD},
                {"neg", OpCode::NEG},
                {"equal", OpCode::EQUAL},
                {"equal_n", OpCode::EQUAL_N},
                {"greater", OpCode::GREATER},
                {"less", OpCode::LESS},
                {"egreater", OpCode::EGREATER},
                {"eless", OpCode::ELESS},
                {"and", OpCode::AND},
                {"or", OpCode::OR},
                {"not", OpCode::NOT},
                {"store", OpCode::STORE},
                {"load", OpCode::LOAD},
                {"loadm", OpCode::LOADM},
                {"jmpa", OpCode::JMPA},
                {"jmpa_if_n", OpCode::JMPA_IF_N},
                {"jmpa_if", OpCode::JMPA_IF},
                {"print", OpCode::PRINT},
                {"except_handling", OpCode::EXCEPT_HANDLING},
                {"declare", OpCode::DECLARE},
                {"declared", OpCode::DECLARED},
                {"func_declare", OpCode::FUNC_DECLARE},
                {"func_declared", OpCode::FUNC_DECLARED},
                {"scopestr", OpCode::SCOPESTR},
                {"scopeend", OpCode::SCOPEEND},

                {"convert_type", OpCode::CONVERT_TYPE},
                {"get_type_str", OpCode::GET_TYPE_STR},
                {"get_by", OpCode::GET_BY},
                {"set_by", OpCode::SET_BY},
                {"array_push_back", OpCode::ARRAY_PUSH_BACK},
                {"array_pop_back", OpCode::ARRAY_POP_BACK},
                {"array_insert_index", OpCode::ARRAY_INSERT_INDEX},
                {"array_erase_index", OpCode::ARRAY_ERASE_INDEX},
                {"array_size", OpCode::ARRAY_SIZE},
                {"dict_size", OpCode::DICT_SIZE},
                {"dict_erase", OpCode::DICT_ERASE},
            };
            return opcodeMap;
        }
        static std::unordered_map<std::string, ValueType> GetStringTypeMap()
        {
            static const std::unordered_map<std::string, ValueType> typeMap = {
                {"string",ValueType::STRING},
                {"bool",ValueType::BOOL},
                {"number",ValueType::NUMBER},
                {"array",ValueType::ARRAY},
                {"void",ValueType::VOID},
            };
            return typeMap;
        }
    public:
        static OpCode GetOpCode(std::string token) 
        {
            if (GetOpCodeMap().count(token) > 0) return GetOpCodeMap().at(token);
            return OpCode::NOP;
        }
        static OpCode GetStringOpCode(std::string token)
        {
            if (GetStringOpCodeMap().count(token) > 0) return GetStringOpCodeMap().at(token);
            return OpCode::NOP;
        }
        static ValueType GetStringType(std::string token)
        {
            if (GetStringTypeMap().count(token) > 0) return GetStringTypeMap().at(token);
            return ValueType::VOID;
        }
        static bool HasOpCode(std::string token) 
        {
            return GetOpCodeMap().count(token) > 0;
        }
        static bool IsAssign(std::string token) 
        {
            return GetAssignOpCodeMap().count(token) > 0;
        }
    };
}