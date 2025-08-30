#pragma once
#include <filesystem>
#include <fstream>
#include "..\Logger\Logger.hpp"

namespace ELScript 
{
    class Loader {
    public:
        static Loader& Get() {
            static Loader instance;
            return instance;
        }

        static std::filesystem::path GetProgramPath() 
        {
            return std::filesystem::current_path();
        }

        bool IsFileExists(std::filesystem::path path)
        {
            return std::filesystem::exists(path);
        }

        std::string Load(const std::string& path) {
            // Проверка существования файла
            if (!std::filesystem::exists(path)) {
                Logger::Get().Log("[Loader] File not found: " + path);
                return "";
            }

            // Проверка, что это файл (а не директория)
            if (!std::filesystem::is_regular_file(path)) {
                Logger::Get().Log("[Loader] Path is not a file: " + path);
                return "";
            }

            // Проверка размера файла (опционально)
            if (std::filesystem::file_size(path) == 0) {
                Logger::Get().Log("[Loader] File is empty: " + path);
                return "";
            }

            // Чтение файла
            std::ifstream in(path, std::ios::binary);
            if (!in.is_open()) {
                Logger::Get().Log("[Loader] Failed to open file: " + path);
                return "";
            }

            std::ostringstream contentStream;
            contentStream << in.rdbuf();
            return contentStream.str();
        }

    private:
        Loader() = default;  // Запрещаем создание экземпляров
    };
}


