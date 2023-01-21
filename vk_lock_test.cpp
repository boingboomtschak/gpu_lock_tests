#include <stdexcept>
#include "easyvk.h"

using std::vector;
using std::runtime_error;

using easyvk::Instance;
using easyvk::Device;
using easyvk::Buffer;
using easyvk::Program;

void run(FILE *ofp) {
    Instance instance = Instance(true);
    Device device = instance.devices().at(0);

    fprintf(ofp, "Using device '%s'\n", device.properties().deviceName);

    device.teardown();
    instance.teardown();
}

int main() {
    FILE *ofp = fopen("output.txt", "w");
    if (ofp == NULL) 
        throw runtime_error("Failed to open output file!");
    fprintf(ofp, "Logging 'vk_lock_test' results...\n");

    run(ofp);

    fclose(ofp);
    return 0;
}