#include "LLToolkitConfig.hpp"
#include "common/Semaphore.hpp"
#include <iostream>

int main(int argc, char** argv)
{
    std::cout << "ll-toolkit v" << LL_TOOLKIT_VERSION_MAJOR << "." << LL_TOOLKIT_VERSION_MINOR << std::endl;

    Common::Semaphore sem;
    sem.getCount();
    std::cout << "Sem " << sem.getCount() << std::endl;

    return 0;
}
