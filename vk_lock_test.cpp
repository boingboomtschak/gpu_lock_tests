#include <stdexcept>
#include <stdarg.h>

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
    log("MaxComputeWorkGroupInvocations: %d\n", device.properties().limits.maxComputeWorkGroupInvocations);

    Buffer lockBuf = Buffer(device, 1);
    Buffer resultBuf = Buffer(device, 1);
    Buffer itersBuf = Buffer(device, 1);
    vector<Buffer> buffers = { lockBuf, resultBuf, itersBuf };
    std::vector<uint32_t> spvCode = 
    #include "lock_main.cinit"
    ; 
    Program program = Program(device, spvCode, buffers);

    uint32_t iters = 1000;
    uint32_t workgroups = 8;
    lockBuf.clear();
    resultBuf.clear();
    itersBuf.store(0, iters);

    program.setWorkgroups(workgroups);
    program.setWorkgroupSize(1);
    program.prepare();

    log("Running test...\n");

    program.run();

    uint32_t result = resultBuf.load(0);
    const char* ans = result == (iters * workgroups) ? "CORRECT" : "INCORRECT";
    log("Result: %d...%s\n", result, ans);

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