#include <stdexcept>
#include "easyvk.h"

using std::vector;
using std::runtime_error;

using easyvk::Instance;
using easyvk::Device;
using easyvk::Buffer;
using easyvk::Program;

void run() {
    printf("Initializing test...\n");

    Instance instance = Instance(false);
    Device device = instance.devices().at(0);

    printf("Using device '%s'\n", device.properties().deviceName);

    Buffer lockBuf = Buffer(device, 1);
    Buffer resultBuf = Buffer(device, 1);
    Buffer itersBuf = Buffer(device, 1);
    vector<Buffer> buffers = { lockBuf, resultBuf, itersBuf };
    Program program = Program(device, "lock_main.spv", buffers);

    int iters = 1000;
    itersBuf.store(0, iters);

    program.setWorkgroups(100);
    program.setWorkgroupSize(1);
    program.prepare();

    printf("Running test...\n");

    program.run();
    printf("Result: %d\n", resultBuf.load(0));

    printf("Cleaning up...\n");

    program.teardown();

    itersBuf.teardown();
    resultBuf.teardown();
    lockBuf.teardown();
        
    device.teardown();
    instance.teardown();
}

int main() {
    printf("Logging 'vk_lock_test' results...\n");

    run();

    return 0;
}