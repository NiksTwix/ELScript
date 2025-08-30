#pragma once
#include "..\Definitions\CommandsInfo.hpp"
#include "..\Definitions\Execution.hpp"
#include <functional>

namespace ELScript
{
    enum class EHMessageType
    {
        Warning,
        Error
    };

    struct EHMessage    //Error Handler Message
    {
        ECID script = InvalidECID;
        int line = 0;
        int rip = 0;
        Command error_command;
        std::stack<Value> stack;    //копии стеков
        std::stack<int> call_stack;
        EHMessageType type;
        std::string description; // Добавим описание ошибки

        EHMessage() = default;
        EHMessage(ECID id, int c_rip, EHMessageType type, std::string desc)
        {
            this->script = id;
            this->description = desc;
            this->rip = c_rip;
            this->type = type;
        }
        EHMessage(EHMessageType type, std::string desc)
        {
            this->description = desc;
            this->type = type;
        }
        EHMessage(ECID id, Command error_command, int c_rip, EHMessageType type, std::string desc)
        {
            this->script = id;
            this->description = desc;
            this->rip = c_rip;
            this->type = type;
            this->line = error_command.line;
            this->error_command = error_command;
        }
    };

    using ErrorHandlerCallback = std::function<void(const EHMessage& message)>; // Переименуем тип колбэка
    using EHID = size_t;    //Error Handler ID
    constexpr EHID InvalidEHID = SIZE_MAX;
    class ErrorHandlerManager 
    {
    private:
        ErrorHandlerManager() = default;
        std::unordered_map<EHID, ErrorHandlerCallback> error_handlers;
        EHID next_ehid = 0;

        static ErrorHandlerManager& Get()
        {
            static ErrorHandlerManager instance;
            return instance;
        }

    public:
        static EHID Register(ErrorHandlerCallback handler)
        {
            auto& instance = Get();
            EHID id = instance.next_ehid++;
            instance.error_handlers[id] = handler;
            return id;
        }

        static bool Unregister(EHID id)
        {
            auto& instance = Get();
            return instance.error_handlers.erase(id) > 0;
        }

        // Главный метод для вызова всех обработчиков
        static void RaiseError(const EHMessage& message)
        {
            auto& instance = Get();
            for (auto& [id, handler] : instance.error_handlers) {
                handler(message); // Вызываем каждый зарегистрированный обработчик
            }
        }
    };
}

