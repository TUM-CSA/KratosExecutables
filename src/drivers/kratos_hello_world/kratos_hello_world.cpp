#include <iostream>
#include <utility>
#include "includes/kernel.h"
#include "containers/model.h"
#include "containers/array_1d.h"

enum class Enum
{
    None = 0,
    Smth = 1
};

using DerivativeTerm = std::pair<Enum, int*>;

void f(Kratos::array_1d<std::pair<Enum, int*>, 2>&& r) {
    std::cout << (int)r[0].first << std::endl;
}

int main()
{
    Kratos::Kernel kernel;
    std::cout << "Hello worldsss" << std::endl;

    int i;
    f(Kratos::array_1d<std::pair<Enum, int*>, 2> {DerivativeTerm(Enum::None, nullptr), DerivativeTerm(Enum::Smth, nullptr)});
    return 0;
}