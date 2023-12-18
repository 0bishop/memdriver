# MemDriver : Little Library for Read & Write Mem from Linux Kernel Module

### ⚠️ This is still experimental, its a minimal and half viable.

## Why I've made this project :
I wanna avoid the usage of ptrace that logs when attaching, writing or reading memory from process.
I've made the kernel module and use IOCTL to communicate with it from usermod.
The kernel module can also rw from kernel memory as shown in the `make test` rule.

## Usermod usage (single header) :
```cpp
try {
    // Load Module in memory
    KMemory kmodule("my_kernel_mod.ko");

    // Read memory
    kmodule.read(addr, &buffer, sizeof(buffer));

    // Write memory
    kmodule.write(addr, &buffer, sizeof(buffer));

} catch (const std::exception &err) {
    std::cerr << "Error: " << err.what() << std::endl;
    return -1;
}
```

## Building on an Alpine VM :
```bash
setup-alpine
apk update
apk upgrade
# => Reboot VM
apk add alpine-sdk linux-virt-dev
```
[How to build Kernel Module on Debian](https://gist.github.com/Josua-SR/3ee497179b75e8e164e508f98b12d810)  

## Running it :
```bash
localhost:~/memdriver# make test
...
[ 1548.969609] Execute This:
[ 1548.969609] ./usermod target/driver.ko 0xffffffffc0600000
```

```bash
# The kernel driver is automatically loaded and freed in memory during runtime
localhost:~/memdriver# ./usermod target/driver.ko 0xffffffffc0600000
Kernel Module loaded successfully!
Value read from module : 42
Value written to module : 12345
New Value read from module : 12345
```

```bash
# Will clean and unload the test module
localhost:~/memdriver# make cleantest
```

```bash
# Will only build the Usermod and the Kernel Module
localhost:~/memdriver# make
```

## Please read that :
[How to build Kernel Module](https://stackoverflow.com/questions/76495059/what-are-linux-headers-and-why-do-we-need-them)  
[The Linux Kernel Module Programming Guide](https://sysprog21.github.io/lkmpg/)
