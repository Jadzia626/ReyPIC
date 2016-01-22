/**
 * ReyPIC – ReyPIC Core Source
 */

#include "clsReyPIC.hpp"

using namespace reypic;

ReyPIC::ReyPIC() {

    // Default Values
    RunMode = RUN_MODE_FULL;

    return;
}

/**
 * Setters
 */

bool ReyPIC::SetInputFile(char* cFile) {

    InputFile = cFile;

    return true;
}

bool ReyPIC::SetRunMode(int iRunMode) {

    switch(iRunMode) {
        case RUN_MODE_TEST:
            RunMode = RUN_MODE_TEST;
            return true;
            break;
        case RUN_MODE_FULL:
            RunMode = RUN_MODE_FULL;
            return true;
            break;
    }

    return false;
}
