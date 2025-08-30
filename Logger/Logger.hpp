#pragma once
#include <iostream>


namespace ELScript
{
	class Logger
	{
	public:
		static Logger& Get()
		{
			static Logger logger;
			return logger;
		}

		void Log(std::string message)
		{
			std::cout << "ELScript: " + message << "\n";
		}
	};
}