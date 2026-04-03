# Simple Device Driver - Educational Guide

## Overview

This is a **character device driver** for Linux kernel (ARM64/Raspberry Pi). It demonstrates fundamental kernel driver concepts including device registration, file operations, and process tracking.

The driver creates a character device `/dev/mydevice` that accepts `open()` calls and logs them to the kernel log (`dmesg`).

---

## What This Driver Does

### Functionality
- **Registers a character device** with the kernel using `register_chrdev()`
- **Allocates a dynamic major number** (no hardcoded device number)
- **Implements file operations**:
  - `.open()` - called when userspace opens `/dev/mydevice`
  - `.release()` - called when userspace closes the device
- **Logs events** to kernel log with detailed metadata: process PID, command name, inode, major/minor numbers, file flags

### Example Kernel Log Output
```
simple_device_driver: open inode=1234 major=236 minor=0 mode=0100666 flags=0x0 pid=1234 comm=test_simple_module
simple_device_driver: release inode=1234 major=236 minor=0 pid=1234 comm=test_simple_module
```

---

## Project Structure

```
src/simple_device_driver/
├── simple_device_driver.c          # Main driver source code
├── Makefile                        # Linux kernel module build configuration
├── simple_device_driver.ko         # Compiled kernel module (binary)
├── README.md                       # This file
└── test_application/
    ├── test_application.c          # Userspace test program
    ├── test_simple_module.arm64    # Compiled test binary (ARM64)
    ├── Makefile                    # Simple build for test app
    ├── CMakeLists.txt             # CMake configuration
    ├── build.sh                    # Automated build script
    ├── create-mydevice.sh         # Helper: creates /dev/mydevice node
    └── copy-module-to-rpi.sh      # Helper: copies module to RPi via SSH
```

---

## How It Works - Technical Details

### 1. Driver Registration (`simple_driver_init()`)

```c
device_major = register_chrdev(0, "simple_device_driver", &simple_driver_fops);
```

- **`register_chrdev(0, ...)`** - Passes `0` to request a **dynamic major number** (kernel assigns one automatically)
- **`"simple_device_driver"`** - Driver name used in `/proc/devices`
- **`&simple_driver_fops`** - Points to file operations structure containing `.open` and `.release` handlers
- **Return value** - Receives the assigned major number (e.g., 236)

### 2. Device Node Creation (Manual Step)

The driver **does NOT** automatically create `/dev/mydevice`. This must be done manually or via the helper script:

```bash
MAJOR=$(awk '$2=="simple_device_driver" {print $1}' /proc/devices)
sudo mknod -m 666 /dev/mydevice c $MAJOR 0
```

- **`mknod`** - Creates a character device node
- **`c`** - Specifies "character device" (vs `b` for block)
- **`$MAJOR 0`** - Major number from `/proc/devices`, minor number 0
- **`-m 666`** - Permissions (read/write for all users)

### 3. Userspace Interaction

When userspace opens the device:

```c
int dev = open("/dev/mydevice", O_RDONLY);
```

- Kernel matches major/minor numbers to find the driver
- Calls `driver_open()` callback
- Driver logs process information via `pr_info()`
- Returns file descriptor to userspace

---

## Building the Driver

### Prerequisites

The build process assumes precompiled Linux kernel headers are available at:
```
/usr/src/linux-headers-6.12.47+rpt-rpi-v8/
```

These headers are typically:
- Provided by the Docker container (`rpi-build`)
- Downloaded from Raspberry Pi kernel repository
- Pre-extracted during container setup

### Build Methods

#### Method 1: Docker Container (Recommended - Windows/Mac)

```powershell
# Run from Windows (requires Docker Desktop running)
.\scripts\build-simple-driver.ps1
```

This PowerShell script:
1. Copies source files to `work/simple_device_driver/`
2. Starts `rpi-build` Docker container
3. Runs `make` inside container with ARM64 cross-compilation flags
4. Container has headers, aarch64-linux-gnu toolchain, and kernel source

#### Method 2: Direct Make (Linux/RPi Host)

```bash
cd src/simple_device_driver
make
```

This executes:
```bash
make -C /usr/src/linux-headers-6.12.47+rpt-rpi-v8 \
  M=$(pwd) \
  ARCH=arm64 \
  CROSS_COMPILE=aarch64-linux-gnu- \
  modules
```

**Flags explained**:
- **`-C <KDIR>`** - Change to kernel source directory
- **`M=$(pwd)`** - Build module in current directory (not in kernel tree)
- **`ARCH=arm64`** - Target ARM64 architecture
- **`CROSS_COMPILE=aarch64-linux-gnu-`** - Use ARM64 compiler toolchain
- **`modules`** - Target: build modules (not full kernel)

#### Method 3: Clean Build Artifacts

```bash
# Via PowerShell
.\scripts\build-simple-driver.ps1 -Clean

# Via direct make
cd src/simple_device_driver && make clean
```

---

## Driver Build Output Artifacts

After successful build:

```
simple_device_driver.ko           # ← Main binary to load on RPi
simple_device_driver.mod          # Kernel module info
simple_device_driver.mod.c        # Auto-generated module glue code
Module.symvers                    # Symbol version information
modules.order                     # Module load order
```

**Only `simple_device_driver.ko` is needed on the target RPi.**

---

## Installing & Testing on Raspberry Pi

### Step 1: Copy Module to RPi

From Windows:
```powershell
# Using provided script (SSH-based)
.\src\simple_device_driver\test_application\copy-module-to-rpi.sh 192.168.1.105 janusz ./simple_device_driver.ko /tmp
```

Or manual `scp`:
```bash
scp -P 22 work/simple_device_driver/simple_device_driver.ko janusz@192.168.1.105:/tmp/
```

### Step 2: Load Module on RPi

```bash
# SSH into RPi
ssh janusz@192.168.1.105

# Load module
cd /tmp
sudo insmod ./simple_device_driver.ko

# Verify: should show major number (e.g., 236)
grep simple_device_driver /proc/devices
```

### Step 3: Create Device Node

```bash
# Option A: Manual
MAJOR=$(awk '$2=="simple_device_driver" {print $1}' /proc/devices)
sudo mknod -m 666 /dev/mydevice c $MAJOR 0

# Option B: Automated script
./create-mydevice.sh
```

### Step 4: Build & Run Test Application

```bash
cd test_application

# Build (choose one method)
./build.sh                    # Auto CMake
./build.sh --make            # Direct Makefile
make                          # Plain make

# Run test
sudo ./test_simple_module.arm64
```

Expected output:
```
Opening was successfull!
```

If device doesn't exist:
```
open(/dev/mydevice): No such file or directory
Opening was not possible! errno=2
```

### Step 5: View Kernel Logs

```bash
# See driver events in real-time
sudo dmesg -w

# Or view recent entries
dmesg | tail -20
```

Output example:
```
[1234.567890] simple_device_driver: registered with major=236
[1234.567891] simple_device_driver: open inode=1234 major=236 minor=0 mode=0100666 flags=0x0 pid=5678 comm=test_simple_module
[1234.567892] simple_device_driver: release inode=1234 major=236 minor=0 pid=5678 comm=test_simple_module
```

### Cleanup: Unload Module

```bash
sudo rmmod simple_device_driver
sudo rm /dev/mydevice
```

---

## Test Application (`test_application.c`)

### What It Does

```c
int dev = open("/dev/mydevice", O_RDONLY);
if (dev == -1) {
    perror("open(/dev/mydevice)");
    printf("errno=%d\n", errno);
    return -1;
}
printf("Opening was successfull!\n");
close(dev);
```

- Attempts to open `/dev/mydevice` for reading
- Uses `perror()` to print human-readable error messages
- Prints `errno` value for debugging
- Closes file descriptor cleanly
- Returns success/failure to shell

### Building Test Application

The test app needs the **ARM64 GNU toolchain** (`aarch64-linux-gnu-gcc`).

#### Via Makefile (Simplest)

```bash
make
# Output: test_simple_module.arm64
```

#### Via CMake

```bash
mkdir build
cd build
cmake -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=aarch64 ..
make
cd ..
```

#### Via build.sh Script

```bash
chmod +x build.sh
./build.sh              # Uses CMake by default
./build.sh --make      # Uses Makefile
./build.sh --clean     # Removes build artifacts
```

---

## Cross-Compilation Explanation

### Why Cross-Compile?

- **Host**: Windows/Mac running amd64 (x86-64)
- **Target**: Raspberry Pi running ARM64 (aarch64)
- You cannot run ARM binaries on x86 (or vice versa)
- **Solution**: Cross-compiler that runs on x86 but produces ARM binaries

### Toolchain: `aarch64-linux-gnu-`

| Component | Role |
|-----------|------|
| `aarch64-linux-gnu-gcc` | C compiler for ARM64 |
| `aarch64-linux-gnu-ar` | Archiver (creates .a libraries) |
| `aarch64-linux-gnu-ld` | Linker |
| `aarch64-linux-gnu-objdump` | Binary inspector |

### ARCH and CROSS_COMPILE Variables

```bash
ARCH=arm64                           # Kernel build system knows to prepare ARM64 headers
CROSS_COMPILE=aarch64-linux-gnu-    # Prepend this to all tool names
```

When `CROSS_COMPILE=aarch64-linux-gnu-`:
- `gcc` → `aarch64-linux-gnu-gcc`
- `ar` → `aarch64-linux-gnu-ar`
- `ld` → `aarch64-linux-gnu-ld`

---

## Kernel Module Internals

### Module Lifecycle

```
insmod simple_device_driver.ko
          ↓
  kernel calls: module_init()
          ↓
  calls: simple_driver_init()
          ↓
  register_chrdev(0, "simple_device_driver", &simple_driver_fops)
          ↓
  [module loaded, visible in /proc/modules, /proc/devices]
          
------- userspace uses device -------

rmmod simple_device_driver
          ↓
  kernel calls: module_exit()
          ↓
  calls: simple_driver_exit()
          ↓
  unregister_chrdev(device_major, "simple_device_driver")
          ↓
  [module unloaded, /proc/devices updated]
```

### gcc vs Kernel Made

**Userspace app** (`test_application.c`):
```bash
aarch64-linux-gnu-gcc -Wall -Wextra -O2 -o test_simple_module.arm64 test_application.c
```

**Kernel module** (`simple_device_driver.c`):
```bash
make -C /usr/src/linux-headers-... ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- modules
```

Kernel build system:
- Passes special `-D` flags (e.g., `__KERNEL__`)
- Adds kernel include paths
- Handles module metadata (`MODULE_LICENSE`, `MODULE_AUTHOR`, etc.)
- Generates `.mod.c` glue code
- Cannot mix with regular gcc compilation

---

## Troubleshooting

### Issue: "Opening was not possible!"

**Check 1: Module loaded?**
```bash
lsmod | grep simple_device_driver
grep simple_device_driver /proc/devices
```

**Check 2: Device node exists?**
```bash
ls -l /dev/mydevice
```

**Check 3: Wrong major/minor?**
```bash
MAJOR=$(awk '$2=="simple_device_driver" {print $1}' /proc/devices)
sudo rm /dev/mydevice
sudo mknod -m 666 /dev/mydevice c $MAJOR 0
```

**Check 4: Permissions?**
```bash
sudo chmod 666 /dev/mydevice
```

### Issue: "register_chrdev failed"

- Check kernel headers match running kernel: `uname -r` vs header version in Makefile
- Verify kernel source has `chrdev` symbols: `grep -r "register_chrdev" /usr/src/linux-headers-*/`

### Issue: Cross-compiler not found

On target RPi:
```bash
sudo apt-get install build-essential  # For native compilation
```

On build host (Windows/Docker):
```bash
docker compose up  # Provides aarch64-linux-gnu toolchain
```

---

## Helper Scripts Reference

### `copy-module-to-rpi.sh`

Copies `.ko` file to RPi via SSH with directory creation.

```bash
./copy-module-to-rpi.sh <rpi_ip> <user> <local_file> <remote_dir> [ssh_port]
./copy-module-to-rpi.sh 192.168.1.105 janusz ./simple_device_driver.ko /tmp
```

### `create-mydevice.sh`

Creates `/dev/mydevice` with correct major/minor from `/proc/devices`.

```bash
sudo ./create-mydevice.sh
# Detects major number automatically
# Recreates node with 666 permissions
```

### `build.sh`

Automated test app builder with CMake/Makefile options.

```bash
./build.sh              # CMake build (default)
./build.sh --make       # Makefile build
./build.sh --clean      # Remove artifacts
```

---

## Learning Resources

### Key Concepts Demonstrated

1. **Character Device Registration** - `register_chrdev()`
2. **File Operations Structure** - `file_operations` hooks
3. **Kernel Logging** - `pr_info()`, `pr_err()`
4. **Dynamic Major Numbers** - Automatic allocation
5. **Cross-Compilation** - ARM64 toolchain setup
6. **Module Metadata** - `MODULE_LICENSE`, `MODULE_AUTHOR`
7. **Process Context** - Accessing `current->pid`, `current->comm`

### Related Kernel Functions

```c
// Character device management
register_chrdev(major, name, fops)      // Register driver
unregister_chrdev(major, name)          // Unregister driver

// Logging
pr_info(fmt, ...)                       // Info level
pr_err(fmt, ...)                        // Error level
pr_debug(fmt, ...)                      // Debug level (compile-time disabled)

// File operations
struct file_operations {
    .open = handler_open,
    .release = handler_close,
    .read = handler_read,               // (not implemented here)
    .write = handler_write,             // (not implemented here)
}

// Inode/File info
imajor(inode)                           // Extract major number
iminor(inode)                           // Extract minor number
current->pid                            // Current process ID
current->comm                           // Current process command name
```

---

## Next Steps - Extending This Driver

### Add Read/Write Support

```c
static ssize_t driver_read(struct file *f, char __user *buf, size_t len, loff_t *off) {
    // Copy data to userspace
}

static const struct file_operations simple_driver_fops = {
    .open = driver_open,
    .release = driver_release,
    .read = driver_read,     // ← Add this
};
```

### Use Device Classes (Auto Device Node Creation)

```c
static struct class *device_class;
static struct device *device;

device_class = class_create(THIS_MODULE, "simple_device");
device_create(device_class, NULL, MKDEV(device_major, 0), NULL, "mydevice");
// Automatically creates /dev/mydevice!
```

### Add ioctl() for Commands

```c
static long driver_ioctl(struct file *f, unsigned int cmd, unsigned long arg) {
    // Handle device-specific commands
}
```

---

## Summary

| Aspect | Details |
|--------|---------|
| **Type** | Character device driver (kernel module) |
| **Architecture** | ARM64 (Raspberry Pi 4/5) |
| **Build System** | Linux kernel Kbuild (via Makefile) |
| **Cross-Compile** | aarch64-linux-gnu toolchain |
| **Device Node** | `/dev/mydevice` (manual creation) |
| **Major Number** | Dynamic (allocated at runtime) |
| **Operations** | open, release (read/write not implemented) |
| **Logging** | Kernel log accessible via `dmesg` |
| **Test App** | Userspace binary demonstrating open/close |

---

**Author**: Janusz Wolak  
**License**: GPL-2.0 (as specified in driver source)  
**Target**: Raspberry Pi OS 64-bit (ARM64), Kernel 6.12.47+rpt-rpi-v8
