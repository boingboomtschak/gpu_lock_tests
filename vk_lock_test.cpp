#include <stdexcept>
#include "easyvk.h"

#ifdef __ANDROID__
#include <android/log.h>
#define APPNAME "GPULockTests"
#endif

using std::vector;
using std::runtime_error;

using easyvk::Instance;
using easyvk::Device;
using easyvk::Buffer;
using easyvk::Program;

void log(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    #ifdef __ANDROID__
    __android_log_vprint(ANDROID_LOG_INFO, APPNAME, fmt, args);
    #else
    vprintf(fmt, args);
    #endif
    va_end(args);
}

extern "C" void run() {
    log("Initializing test...\n");

    Instance instance = Instance(false);
    Device device = instance.devices().at(0);

    log("Using device '%s'\n", device.properties().deviceName);

    Buffer lockBuf = Buffer(device, 1);
    Buffer resultBuf = Buffer(device, 1);
    Buffer itersBuf = Buffer(device, 1);
    vector<Buffer> buffers = { lockBuf, resultBuf, itersBuf };
    std::vector<uint32_t> spvCode = 
    #include "lock_main.cinit"
    ; 
    Program program = Program(device, spvCode, buffers);

    int iters = 1000;
    itersBuf.store(0, iters);

    program.setWorkgroups(8);
    program.setWorkgroupSize(1);
    program.prepare();

    log("Running test...\n");

    program.run();
    log("Result: %d\n", resultBuf.load(0));

    log("Cleaning up...\n");

    program.teardown();

    itersBuf.teardown();
    resultBuf.teardown();
    lockBuf.teardown();
        
    device.teardown();
    instance.teardown();
}

int main() {
    log("Logging 'vk_lock_test' results...\n");

    run();

    return 0;
}