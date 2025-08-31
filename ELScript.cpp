#include <iostream>

#include <Core/Scripting/Interpreter.hpp>

int main()
{
    auto& inter_ = ELScript::Interpreter::Get();
    auto id = inter_.LoadScript("E:\\CPP\\MyLibs\\ELScript\\Tests\\test1.els");
    inter_.Execute(id);
    for (int i = 0; i < 10; i++) 
    {
        inter_.CallFunction(id, "process", std::vector<ELScript::Value>({ ELScript::Value(i) }));
    }
    std::cout << " ";
}

