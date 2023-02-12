#include <stdexcept>
#include <stdarg.h>

#include "easyvk.h"
#include "json.h"

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
    uint32_t test_total = workgroups * lock_iters;
    uint32_t total_locks = test_total * test_iters;
    uint32_t total_failures = 0;

    Buffer lockBuf = Buffer(device, 1);
    Buffer resultBuf = Buffer(device, 1);
    Buffer lockItersBuf = Buffer(device, 1);
    vector<Buffer> buffers = { lockBuf, resultBuf, lockItersBuf };
    lockItersBuf.store(0, lock_iters);

    // -------------- TAS LOCK --------------

    log("----------------------------------------------------------\n");
    log("Testing TAS lock...\n");
    log("%d workgroups, %d locks per thread, tests run %d times.\n", workgroups, lock_iters, test_iters);
    vector<uint32_t> tasSpvCode =
    #include "tas_lock.cinit"
    ;
    Program tasProgram = Program(device, tasSpvCode, buffers);
    tasProgram.setWorkgroups(workgroups);
    tasProgram.setWorkgroupSize(1);
    tasProgram.prepare();
    total_failures = 0;

    for (int i = 1; i <= test_iters; i++) {
        log("  Test %d: ", i);
        lockBuf.clear();
        resultBuf.clear();

        tasProgram.run();

        uint32_t result = resultBuf.load(0);
        uint32_t test_failures = (lock_iters * workgroups) - result;
        float test_percent = (float)test_failures / (float)test_total * 100;

        if (test_percent > 10.0)
            log("\u001b[31m");
        else if (test_percent > 5.0)
            log("\u001b[33m");
        else
            log("\u001b[32m");
        log("%d / %d, %.2f%%\u001b[0m\n", test_failures, test_total, test_percent);
        total_failures += test_failures;
    }
    log("%d / %d failures, about %.2f%%\n", total_failures, total_locks, (float)total_failures / (float)total_locks * 100);

    // -------------- TTAS LOCK --------------

    log("----------------------------------------------------------\n");
    log("Testing TTAS lock...\n");
    log("%d workgroups, %d locks per thread, tests run %d times.\n", workgroups, lock_iters, test_iters);
    vector<uint32_t> ttasSpvCode =
    #include "ttas_lock.cinit"
    ;
    Program ttasProgram = Program(device, ttasSpvCode, buffers);
    ttasProgram.setWorkgroups(workgroups);
    ttasProgram.setWorkgroupSize(1);
    ttasProgram.prepare();
    total_failures = 0;

    for (int i = 1; i <= test_iters; i++) {
        log("  Test %d: ", i);
        lockBuf.clear();
        resultBuf.clear();

        ttasProgram.run();

        uint32_t result = resultBuf.load(0);
        uint32_t test_failures = (lock_iters * workgroups) - result;
        float test_percent = (float)test_failures / (float)test_total * 100;

        if (test_percent > 10.0)
            log("\u001b[31m");
        else if (test_percent > 5.0)
            log("\u001b[33m");
        else
            log("\u001b[32m");
        log("%d / %d, %.2f%%\u001b[0m\n", test_failures, test_total, test_percent);
        total_failures += test_failures;
    }
    log("%d / %d failures, about %.2f%%\n", total_failures, total_locks, (float)total_failures / (float)total_locks * 100);

    // -------------- CAS LOCK --------------

    log("----------------------------------------------------------\n");
    log("Testing CAS lock...\n");
    log("%d workgroups, %d locks per thread, tests run %d times.\n", workgroups, lock_iters, test_iters);
    vector<uint32_t> casSpvCode =
    #include "cas_lock.cinit"
    ;
    Program casProgram = Program(device, casSpvCode, buffers);
    casProgram.setWorkgroups(workgroups);
    casProgram.setWorkgroupSize(1);
    casProgram.prepare();
    total_failures = 0;

    for (int i = 1; i <= test_iters; i++) {
        log("  Test %d: ", i);
        lockBuf.clear();
        resultBuf.clear();

        casProgram.run();

        uint32_t result = resultBuf.load(0);
        uint32_t test_failures = (lock_iters * workgroups) - result;
        float test_percent = (float)test_failures / (float)test_total * 100;

        if (test_percent > 10.0)
            log("\u001b[31m");
        else if (test_percent > 5.0)
            log("\u001b[33m");
        else
            log("\u001b[32m");
        log("%d / %d, %.2f%%\u001b[0m\n", test_failures, test_total, test_percent);
        total_failures += test_failures;
    }
    log("%d / %d failures, about %.2f%%\n", total_failures, total_locks, (float)total_failures / (float)total_locks * 100);

    // save results to json string here

    log("----------------------------------------------------------\n");
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
    run();
    return 0;
}