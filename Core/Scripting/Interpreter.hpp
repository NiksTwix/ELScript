#pragma once
#include "..\Interpretation/VirtualMachine.hpp"
#include "..\Loader/Loader.hpp"
#include "..\Lexer/Lexer.hpp"
#include "..\Decoder/Decoder.hpp"
#include "..\Scripting\Script.hpp"
#include "..\Definitions\StringOperations.hpp"
#include "..\Postprocessing\Postprocessor.hpp"

namespace ELScript 
{
	class Interpreter 
	{
	private:
		VirtualMachine machine;
		
		std::unordered_map<ECID, std::shared_ptr<Script>> scripts;
		std::vector<std::filesystem::path> paths_to_global_files;	//Пути к папкам c глобальными файлами

		std::unordered_set<std::string> processed_files;

        ECID next_id = 1;

        std::vector<Token> GetTokens(const std::filesystem::path& scriptPath) {
            if (!Loader::Get().IsFileExists(scriptPath)) {
                Logger::Get().Log("[Interpreter] ERROR: File not found - " + scriptPath.string());
                return {};
            }
            std::filesystem::path currentDir = scriptPath.parent_path(); // <-- Важно!
            std::string text = Loader::Get().Load(scriptPath.string());
            std::vector<Token> tokens = Lexer::Get().ToTokens(text);
            std::vector<Token> result;

            std::vector<Token> main_script_buffer;
            bool expectImportFilename = false;

            for (size_t i = 0; i < tokens.size(); ++i) {
                Token t = tokens[i];

                if (expectImportFilename) {
                    if (t.type == TokenType::LITERAL && t.value.type == ValueType::STRING) {
                        std::filesystem::path importPath = ResolveImportPath(t.value.strVal, currentDir);

                        if (importPath.empty()) {
                            Logger::Get().Log("[Interpreter] ERROR: Cannot resolve import - " + t.value.strVal);
                        }
                        else if (!processed_files.count(importPath.string())) {
                            processed_files.insert(importPath.string());
                            auto importedTokens = GetTokens(importPath); // Рекурсия!
                            result.insert(result.end(), importedTokens.begin(), importedTokens.end());
                        }
                    }
                    else {
                        Logger::Get().Log("[Interpreter] ERROR: Expected string after import");
                    }
                    expectImportFilename = false;
                    continue;
                }

                if (t.type == TokenType::KEYWORD && t.value.strVal == "import") {
                    expectImportFilename = true;
                }
                else {
                    main_script_buffer.push_back(t); // Добавляем обычные токены
                }
            }
            result.insert(result.end(), main_script_buffer.begin(), main_script_buffer.end());
            return result;
        }

        // Вынесем разрешение пути в отдельную функцию
        std::filesystem::path ResolveImportPath(const std::string& importPath,
            const std::filesystem::path& currentScriptDir) {
            // 1. Абсолютный путь
            if (importPath.starts_with("/") || importPath.starts_with("C:\\")) {
                if (Loader::Get().IsFileExists(importPath)) return importPath;
            }

            // 2. Относительно текущего скрипта (САМОЕ ВАЖНОЕ!)
            std::filesystem::path relativeToCurrent = currentScriptDir / importPath;
            if (Loader::Get().IsFileExists(relativeToCurrent)) {
                return relativeToCurrent;
            }

            // 3. Относительно программы
            std::filesystem::path relativeToProgram = Loader::GetProgramPath() / importPath;
            if (Loader::Get().IsFileExists(relativeToProgram)) {
                return relativeToProgram;
            }

            // 4. Глобальные пути
            for (const auto& globalPath : paths_to_global_files) {
                std::filesystem::path fullPath = globalPath / importPath;
                if (Loader::Get().IsFileExists(fullPath)) {
                    return fullPath;
                }
            }

            return {};
        }
        Interpreter() 
        {
            FunctionTable::Register("get_id", 0, [&](ECID id, const std::vector<Value>& args)   //ID собственного скрипта
                {
                    return Value((int)id);
                });
            FunctionTable::Register("get_id_of", 1, [&](ECID id, const std::vector<Value>& args)    //ID другого скрипта
                {
                    if (args.size() < 1 || args[0].type != ValueType::STRING) return -1;
                    for (auto& [id, scr] : scripts) 
                    {
                        if (scr->name == args[0].strVal) return (int)id;
                    }
                    return -1;
                });
            FunctionTable::Register("call_script", 3, [&](ECID id, const std::vector<Value>& args)      //ID, function_name, args(array)
                {
                    if (args.size() < 3) return Value();
                    auto script = args[0];
                    auto func_name = args[1];
                    auto args_ = args[2];
                    if (script.type != ValueType::NUMBER || func_name.type != ValueType::STRING || args_.type != ValueType::ARRAY) return Value();
                    return CallFunction(script.numberVal, func_name.strVal, *args_.arrayVal);
                   
                });
            eh_id = ErrorHandlerManager::Register([&](const EHMessage message)
                {
                    Logger::Get().Log(message.description);
                });
        }
        //TODO добавить обработчик
        EHID eh_id = InvalidEHID;
    public:

        static Interpreter& Get() 
        {
            static Interpreter interpreter;
            return interpreter;
        }

        ECID LoadScript(std::filesystem::path script_path)
        {
            processed_files.clear();
            auto tokens = GetTokens(script_path);

            auto raw_ec = Decoder::Get().DecodeTokens(tokens);
            

            auto id = next_id++;
            scripts[id] = std::make_shared<Script>();
            scripts[id]->execution_chain.id = id;
            scripts[id]->name = script_path.filename().string();
            scripts[id]->scriptPath = script_path;
            scripts[id]->execution_chain.function_markers = Postprocessor::GetFunctionTable(raw_ec.commands);
            Postprocessor::CalculateJMPs(raw_ec.commands);      //В будущем будет удалять метки 
            scripts[id]->execution_chain.commands = raw_ec.commands;
            scripts[id]->execution_chain.exit_rip = raw_ec.exit_rip;
            return id;
        }

        bool DestroyScript(ECID id) 
        {
            if (!scripts.count(id)) return false;
            scripts.erase(id);
            return true;
        }

        std::shared_ptr<Script> GetScript(ECID script)
        {
            if (!scripts.count(script)) return nullptr;
            return scripts[script];
        }

        void Execute(ECID script)
        {
            if (scripts.count(script) && scripts.at(script)->execution_chain.state != ECState::ACTIVE)
            {
                machine.Execute(scripts[script]->execution_chain);
            }
            else if (scripts.count(script) && scripts.at(script)->execution_chain.state == ECState::ACTIVE) 
            {
                ErrorHandlerManager::RaiseError(EHMessage(EHMessageType::Error, "[Interpreter] ERROR: script with id " + std::to_string(script) + " is active."));
            }
            else 
            {
                ErrorHandlerManager::RaiseError(EHMessage(EHMessageType::Error, "[Interpreter] ERROR: script with id " + std::to_string(script) + " isnt defined."));
            }
        }
        Value CallFunction(ECID script, std::string function_name, std::vector<Value> parameters) {
            auto script_ = GetScript(script);
            if (!script_) {
                ErrorHandlerManager::RaiseError(EHMessage(EHMessageType::Error, "[Interpreter] ERROR: script with id " + std::to_string(script) + " isnt defined."));
                return Value();
            }

            if (script_->execution_chain.state == ECState::ACTIVE) {
                ErrorHandlerManager::RaiseError(EHMessage(EHMessageType::Error, "[Interpreter] ERROR: script with id " + std::to_string(script) + " is active."));
                return Value();
            }

            if (!script_->execution_chain.function_markers.count(function_name)) {
                ErrorHandlerManager::RaiseError(EHMessage(EHMessageType::Error, "[Interpreter] ERROR: function " + function_name + " not found."));
                return Value();
            }

            auto before_stack_size = script_->execution_chain.stack.size();

            if (parameters.size() != script_->execution_chain.function_markers[function_name].arguments.size()) 
            {
                ErrorHandlerManager::RaiseError(EHMessage(EHMessageType::Error, "[Interpreter] ERROR: the number of parameters passed does not match the function " + function_name + " parameters."));
                return Value();
            }

            for (int i = parameters.size() - 1; i >= 0; i--)  
            {
                script_->execution_chain.stack.push(parameters[i]); //Да-да, нарушение инкапсуляции, но у нас Interpreter - это божественный класс, он всё должен видеть, но не всё контролировать
            }
            script_->execution_chain.call_stack.push(script_->execution_chain.exit_rip - 1);
            script_->execution_chain.depth_call_stack.push(script_->execution_chain.variables.size() - 1);

            machine.ExecuteFrom(script_->execution_chain, script_->execution_chain.function_markers[function_name].start_rip);

            auto after_stack_size = script_->execution_chain.stack.size();

            if (before_stack_size < after_stack_size) 
            {
                auto value = script_->execution_chain.stack.top();
                script_->execution_chain.stack.pop();
                return value;
            }
            return Value();
        }

        void SetMetaVariable(ECID script, std::string name, Value value) 
        {
            auto script_ = GetScript(script);
            if (!script_) {
                ErrorHandlerManager::RaiseError(EHMessage(EHMessageType::Error, "[Interpreter] ERROR: script with id " + std::to_string(script) + " isnt defined."));
                return;
            }
            script_->execution_chain.meta_variables[name] = value;
        }
        Value GetMetaVariable(ECID script, std::string name) 
        {
            auto script_ = GetScript(script);
            if (!script_) {
                ErrorHandlerManager::RaiseError(EHMessage(EHMessageType::Error, "[Interpreter] ERROR: script with id " + std::to_string(script) + " isnt defined."));
                return Value();
            }
            if (!script_->execution_chain.meta_variables.count(name)) 
            {
                ErrorHandlerManager::RaiseError(EHMessage(EHMessageType::Error, "[Interpreter] ERROR: meta variable " + name + " isnt defined."));
                return Value();
            }
            return script_->execution_chain.meta_variables[name];
        }
	};
} 