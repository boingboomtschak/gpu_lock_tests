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

    uint32_t workgroups = 128;
    uint32_t lock_iters = 1000;
    uint32_t test_iters = 8;

    Buffer lockBuf = Buffer(device, 1);
    Buffer resultBuf = Buffer(device, 1);
    Buffer lockItersBuf = Buffer(device, 1);
    vector<Buffer> buffers = { lockBuf, resultBuf, lockItersBuf };
    lockItersBuf.store(0, lock_iters);

    // -------------- TAS LOCK --------------

    log("Testing TAS lock...\n");
    log("%d workgroups, %d locks per thread, tests run %d times.\n", workgroups, lock_iters, test_iters);
    vector<uint32_t> tasSpvCode =
    #include "tas_lock.cinit"
    ;
    Program tasProgram = Program(device, tasSpvCode, buffers);
    tasProgram.setWorkgroups(workgroups);
    tasProgram.setWorkgroupSize(1);
    tasProgram.prepare();

    for (int i = 1; i <= test_iters; i++) {
        log("  Running test %d: ", i);
        lockBuf.clear();
        resultBuf.clear();

        tasProgram.run();

        uint32_t result = resultBuf.load(0);
        const char* resultStr = result == (lock_iters * workgroups) ? "\u001b[33mCORRECT\u001b[0m" : "\u001b[31mINCORRECT\u001b[0m";
        log("%s\n", resultStr);
    }

    // -------------- TTAS LOCK --------------

    log("Testing TTAS lock...\n");
    log("%d workgroups, %d locks per thread, tests run %d times.\n", workgroups, lock_iters, test_iters);
    vector<uint32_t> ttasSpvCode =
    #include "ttas_lock.cinit"
    ;
    Program ttasProgram = Program(device, ttasSpvCode, buffers);
    ttasProgram.setWorkgroups(workgroups);
    ttasProgram.setWorkgroupSize(1);
    ttasProgram.prepare();

    for (int i = 1; i <= test_iters; i++) {
        log("  Running test %d: ", i);
        lockBuf.clear();
        resultBuf.clear();

        ttasProgram.run();

        uint32_t result = resultBuf.load(0);
        const char* resultStr = result == (lock_iters * workgroups) ? "\u001b[33mCORRECT\u001b[0m" : "\u001b[31mINCORRECT\u001b[0m";
        log("%s\n", resultStr);
    }

    // -------------- CAS LOCK --------------

    log("Testing CAS lock...\n");
    log("%d workgroups, %d locks per thread, tests run %d times.\n", workgroups, lock_iters, test_iters);
    vector<uint32_t> casSpvCode =
    #include "cas_lock.cinit"
    ;
    Program casProgram = Program(device, casSpvCode, buffers);
    casProgram.setWorkgroups(workgroups);
    casProgram.setWorkgroupSize(1);
    casProgram.prepare();

    for (int i = 1; i <= test_iters; i++) {
        log("  Running test %d: ", i);
        lockBuf.clear();
        resultBuf.clear();

        casProgram.run();

        uint32_t result = resultBuf.load(0);
        const char* resultStr = result == (lock_iters * workgroups) ? "\u001b[33mCORRECT\u001b[0m" : "\u001b[31mINCORRECT\u001b[0m";
        log("%s\n", resultStr);
    }

    // save results to json string here

    log("Cleaning up...\n");

    tasProgram.teardown();
    ttasProgram.teardown();
    casProgram.teardown();

    lockItersBuf.teardown();
    resultBuf.teardown();
    lockBuf.teardown();
        
    device.teardown();
    instance.teardown();

    // return json string, eventually
}

int main() {
    log("Logging 'vk_lock_test' results...\n");

    run();

    return 0;
}