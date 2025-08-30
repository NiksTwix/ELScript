#include <iostream>
#include <Scripting\Interpreter.hpp>

int main()
{
    auto& inter_ = ELScript::Interpreter::Get();
    auto id1 = inter_.LoadScript("E:\\CPP\\MyLibs\\ELScript\\Tests\\math.els");
    inter_.Execute(id1);
    auto id = inter_.LoadScript("E:\\CPP\\MyLibs\\ELScript\\Tests\\test1.els");
    inter_.Execute(id);
}

