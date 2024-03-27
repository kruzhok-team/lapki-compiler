#include <iostream>
#include "foo.hpp" 

int main(void)
{
    int y = foo(3);
    std::cout << "Result: " << y << std::endl;
    
    return 0;
}