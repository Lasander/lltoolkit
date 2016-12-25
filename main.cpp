#include "LLToolkitConfig.hpp"
#include "common/Semaphore.hpp"
#include <iostream>

int main(int argc, char** argv)
{
    std::cout << "LLToolkit v" << LLToolkit_VERSION_MAJOR << "." << LLToolkit_VERSION_MINOR << std::endl;

    Common::Semaphore sem;
    sem.getCount();
    std::cout << "Sem " << sem.getCount() << std::endl;

    return 0;
}
