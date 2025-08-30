#include <iostream>

#include <Core/Scripting/Interpreter.hpp>

int main()
{
    auto& inter_ = ELScript::Interpreter::Get();
    auto id = inter_.LoadScript("E:\\CPP\\MyLibs\\ELScript\\Tests\\test1.els");
    inter_.Execute(id);
}

