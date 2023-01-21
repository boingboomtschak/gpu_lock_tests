#include <stdexcept>
#include "easyvk.h"

using std::vector;
using std::runtime_error;

using easyvk::Instance;
using easyvk::Device;
using easyvk::Buffer;
using easyvk::Program;

void run() {
    Instance instance = Instance(true);

}

int main() {
    FILE *ofp = fopen("output.txt", "w");
    if (ofp == NULL) 
        throw runtime_error("Failed to open output file!");

    run();

    fclose(ofp);
    return 0;
}