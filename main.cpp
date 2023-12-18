#include "interface/usermod.hpp"

int main(int argc, char **argv, __unused char **envp)
{
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <module_path>.ko <addr>" << std::endl;
        return -1;
    }

    int buffer;
    void* addr = (void *)(std::uintptr_t)std::strtoul(argv[2], NULL, 16);

    try {
        KMemory kmodule(argv[1]);
        std::cout << "Kernel Module loaded successfully!" << std::endl;

        // Example of reading with ioctl
        kmodule.read(addr, &buffer, sizeof(buffer));
        std::cout << "Value read from module : " << buffer << std::endl;

        // Example of writing with ioctl
        buffer = 12345;
        kmodule.write(addr, &buffer, sizeof(buffer));
        std::cout << "Value written to module : " << buffer << std::endl;

        // Example of reading back the value with ioctl
        kmodule.read(addr, &buffer, sizeof(buffer));
        std::cout << "New Value read from module : " << buffer << std::endl;

    } catch (const std::exception &err) {
        std::cerr << "Error: " << err.what() << std::endl;
        return -1;
    }

    return 0;
}
