#pragma once
#include "..\Definitions\CommandsInfo.hpp"
#include "..\Definitions\Execution.hpp"
#include <functional>

namespace ELScript
{
    enum class MessageType
    {
        Warning,
        Error,
        Info
    };

    struct Message    //Error Handler Message
    {
        ECID script = InvalidECID;
        int line = 0;
        int rip = 0;
        Command error_command;
        std::stack<Value> stack;    //копии стеков
        std::stack<int> call_stack;
        MessageType type;

        std::string description; // Добавим описание ошибки

        Message(ECID id,std::string desc)
        {
            this->script = id;
            this->script = id;
            this->description = desc;
        }

        Message() = default;
        Message(ECID id, int c_rip, MessageType type, std::string desc)
        {
            this->script = id;
            this->description = desc;
            this->rip = c_rip;
            this->type = type;
        }
        Message(MessageType type, std::string desc)
        {
            this->description = desc;
            this->type = type;
        }
        Message(ECID id, Command error_command, int c_rip, MessageType type, std::string desc)
        {
            this->script = id;
            this->description = desc;
            this->rip = c_rip;
            this->type = type;
            this->line = error_command.line;
            this->error_command = error_command;
        }
    };

    using MessageHandlerCallback = std::function<void(const Message& message)>; // Переименуем тип колбэка
    using MHID = size_t;    //Message Handler ID
    constexpr MHID InvalidMHID = SIZE_MAX;
    class MessageHandlerManager 
    {
    private:
        MessageHandlerManager() = default;
        std::unordered_map<MHID, MessageHandlerCallback> error_handlers;
        std::unordered_map<MHID, MessageHandlerCallback> info_handlers;
        MHID next_ehid = 0;

        static MessageHandlerManager& Get()
        {
            static MessageHandlerManager instance;
            return instance;
        }

    public:
        static MHID Register(MessageHandlerCallback handler, MessageType type = MessageType::Error)
        {
            auto& instance = Get();
            MHID id = instance.next_ehid++;
            switch (type)
            {
            case ELScript::MessageType::Warning:
                break;
            case ELScript::MessageType::Error:
                instance.error_handlers[id] = handler;
                break;
            case ELScript::MessageType::Info:
                instance.info_handlers[id] = handler;
                break;
            default:
                instance.error_handlers[id] = handler;
                break;
            }
           
            return id;
        }

        static bool Unregister(MHID id)
        {
            auto& instance = Get();

            bool result = instance.error_handlers.erase(id) > 0;
            if (result) return true;
            result = instance.info_handlers.erase(id) > 0;
            if (result) return true;
            return result;
        }
        static void SendInfo(const Message& message)
        {
            auto& instance = Get();
            for (auto& [id, handler] : instance.info_handlers) {
                handler(message); // Вызываем каждый зарегистрированный обработчик
            }
        }
        // Главный метод для вызова всех обработчиков
        static void RaiseError(const Message& message)
        {
            auto& instance = Get();
            for (auto& [id, handler] : instance.error_handlers) {
                handler(message); // Вызываем каждый зарегистрированный обработчик
            }
        }
    };
}

