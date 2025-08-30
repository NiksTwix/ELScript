#pragma once
#include "..\Definitions\Execution.hpp"
#include <functional>
/*
	FunctionTable - это класс-посредник между нативным кодом и ELScript

*/

namespace ELScript 
{
    class FunctionTable {
    private:
        static FunctionTable& Get() { static FunctionTable instance; return instance; }

        std::unordered_map<std::string, std::function<Value(ECID, const std::vector<Value>&)>> nativeFunctions;
        std::unordered_map<std::string, size_t> arityMap;   // карта количества аргументов
    public:
        static void Register(const std::string& name, int arg_counts,
            std::function<Value(ECID, const std::vector<Value>&)> func) {
            if (Get().nativeFunctions.count(name))
            {
                //TODO Обработка ошибок
                return;
            }
            Get().nativeFunctions[name] = func;
            Get().arityMap[name] = arg_counts;
        }

        static size_t GetArity(const std::string& name)
        {
            if (!Get().arityMap.count(name))
            {
                //TODO Обработка ошибок
                return 0;
            }
            return Get().arityMap[name];
        }

        static bool Exists(const std::string& name) 
        {
            return Get().nativeFunctions.count(name);
        }

        static Value Call(const std::string& name, ECID scriptId, const std::vector<Value>& args) {
            if (Get().nativeFunctions.count(name)) {
                return Get().nativeFunctions[name](scriptId, args);
            }
            return Value(); // или кидать исключение
        }
    };
}