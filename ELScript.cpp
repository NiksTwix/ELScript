#include <iostream>
#include "Core/Scripting/Interpreter.hpp"


int main()
{
    auto& inter_ = ELScript::Interpreter::Get();
    auto id = inter_.LoadScript("D:\\CPP\\MyLibs\\ELScript\\Tests\\test1.els");
    inter_.SetMetaVariable(id, "test", ELScript::Value(100));
    inter_.Execute(id);
}

