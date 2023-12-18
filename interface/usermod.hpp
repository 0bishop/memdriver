#ifndef USERMOD_HPP
    #define USERMOD_HPP

    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/ioctl.h>

    #include <iostream>
    #include <fstream>
    #include <vector>

    #define __always_inline  inline __attribute__((__always_inline__))
    #define __unused  __attribute__((unused))

    #define RTErr(msg) std::runtime_error(std::string(__FILE__) + ":" + std::to_string(__LINE__) + " " + msg)

    namespace Sys {
        static inline uint64_t fsyscall(uint64_t syscall_number, ...) {
            __builtin_va_list args;
            __builtin_va_start(args, syscall_number);

            uint64_t ret;
            asm volatile (
                "mov %1, %%rax\n"
                "mov %2, %%rdi\n"
                "mov %3, %%rsi\n"
                "mov %4, %%rdx\n"
                "mov %5, %%r10\n"
                "mov %6, %%r8\n"
                "mov %7, %%r9\n"
                "syscall\n"
                "mov %%rax, %0\n"
                : "=r"(ret)
                : "r"(syscall_number),
                "r"(__builtin_va_arg(args, uint64_t) ?: 0),
                "r"(__builtin_va_arg(args, uint64_t) ?: 0),
                "r"(__builtin_va_arg(args, uint64_t) ?: 0),
                "r"(__builtin_va_arg(args, uint64_t) ?: 0),
                "r"(__builtin_va_arg(args, uint64_t) ?: 0),
                "r"(__builtin_va_arg(args, uint64_t) ?: 0)
                : "rax", "rdi", "rsi", "rdx", "r10", "r8", "r9"
            );

            __builtin_va_end(args);
            return ret;
        }
    }

    class KModule {
        public:
        
            KModule(std::string kmodule_path) {
                this->path = kmodule_path;
                if (this->load(this->path) < 0)
                    throw RTErr(strerror(errno));
            }

            ~KModule() {
                this->unload(this->path);
            }

        private:
            std::string path;
            static __always_inline int load(std::string &module_path);
            static __always_inline int unload(std::string &module_name);

            enum {
                INIT_MODULE_SYSCALL = 175,
                UNLOAD_KMODULE_SYSCALL
            };
    };

    __always_inline int KModule::load(std::string &module_path) {

        if (module_path.empty()) {
            std::perror("Module path is null");
            return -1;
        }

        std::ifstream module_file(module_path, std::ios::binary | std::ios::ate);

        if (!module_file.is_open()) {
            std::perror("Failed to open module file");
            return -1;
        }

        std::streamsize module_size = module_file.tellg();
        module_file.seekg(0, std::ios::beg);

        std::vector<char> module_image(module_size);

        if (!module_file.read(module_image.data(), module_size)) {
            std::perror("Failed to read module file");
            return -1;
        }

        module_file.close();

        // Use syscall to load the module
        int result = Sys::fsyscall(KModule::INIT_MODULE_SYSCALL, module_image.data(), module_size, "");

        if (result == -1) {
            std::perror("Failed to load module");
            return -1;
        }

        return 0;
    }

    __always_inline int KModule::unload(std::string &module_name) {

        if (module_name.empty()) {
            std::perror("Module name is null");
            return -1;
        }

        size_t lastSlash = module_name.find_last_of('/');
        if (lastSlash != std::string::npos) {
            module_name = module_name.substr(lastSlash + 1);
        }
        size_t firstDot = module_name.find_first_of('.');
        if (firstDot != std::string::npos) {
            module_name.resize(firstDot);
        }

        int result = Sys::fsyscall(KModule::UNLOAD_KMODULE_SYSCALL, module_name.c_str(), "");

        if (result < 0) {
            std::perror("Failed to unload module");
            return -1;
        }
        return 0;
    }

    class KMemory : public KModule {
        public:
            KMemory(std::string kmodule_path) : KModule(kmodule_path) {
                if (this->open_device() < 0)
                    throw RTErr(strerror(errno));
            }

            ~KMemory() {
                close(this->devicefd);
            }

            std::int32_t write(void *addr, void *buffer, std::size_t len) {
                if (write_memory(this->devicefd, addr, buffer, len) < 0)
                    throw RTErr(strerror(errno));
                return 0;
            }

            std::int32_t read(void *addr, void *buffer, std::size_t len) {
                if (read_memory(this->devicefd, addr, buffer, len) < 0)
                    throw RTErr(strerror(errno));
                return 0;
            }

        private:
            // IOCTL CODES
            enum {
                RD_MEMORY = _IOR(0x666, 0x667, struct memory_data*),
                WR_MEMORY = _IOW(0x666, 0x668, struct memory_data*)
            };

            typedef struct {
                void* address;
                void* buffer;
                std::size_t len;
            } memory_data;

            __always_inline int open_device(void);
            __always_inline int read_memory(int fd, void *addr, void *buffer, std::size_t len);
            __always_inline int write_memory(int fd, void *addr, void *buffer, std::size_t len);

            int devicefd;
    };

    __always_inline int KMemory::open_device(void) {
        int devicefd = open("/dev/memdriver", O_RDWR);

        if (devicefd < 0) {
            std::perror("Error opening the device");
            return -1;
        }

        this->devicefd = devicefd;
        return 0;
    }

    __always_inline int KMemory::read_memory(int fd, void *addr, void *buffer, std::size_t len) {

        if (fd < 0)
            return -1;

        

        KMemory::memory_data data = { addr, buffer, len };
        if (ioctl(fd, RD_MEMORY, &data) < 0) {
            std::perror("Error during ioctl call for reading memory");
            return -1;
        }

        return 0;
    }

    __always_inline int KMemory::write_memory(int fd, void *addr, void *buffer, std::size_t len) {

        if (fd < 0)
            return -1;

        KMemory::memory_data data = { addr, buffer, len };
        if (ioctl(fd, WR_MEMORY, &data) < 0) {
            std::perror("Error during ioctl call for reading memory");
            return -1;
        }

        return 0;
    }

#endif // USERMOD_HPP