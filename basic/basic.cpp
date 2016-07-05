#include <iostream>

class SomeClass
{
public:
    SomeClass()
    {
        std::cout << "this is ctr" << std::endl;
    }

    SomeClass s1()
    {
        std::cout << "it's function" << std::endl;
        return *this;
    }

    int operator()()
    {
        std::cout << "this is operator overloading" << std::endl;
        return 0;
    }

    virtual ~SomeClass() {}
};

int main(void)
{
    std::cout << "----------" << std::endl;
    SomeClass s;
    std::cout << "----------" << std::endl;
    SomeClass s1();
    std::cout << "----------" << std::endl;
    return 0;
}
