#include <mutex>
#include <vector>
#include "VSEPR.h"

// Model shared between threads
// This variable is only available in the scope
// of this file so it cannot be accessed by the render/model threads
// through any non-thread-safe means.
std::vector<BondedElement> sharedModel;
// Mutex to manage thread permissions
std::mutex accessMutex;

/**
 * Get/set the vector representing the chemical model in
 * a position shared between threads.
 * Uses a mutex to ensure thread-safe read/write.
 *
 * @param model an updated version of the model if it's being set (optional for read)
*/
std::vector<BondedElement> mutateModel(std::vector<BondedElement> model) {
    std::vector<BondedElement> returnVec;
    accessMutex.lock();
        if(model.size() > 0) {
            sharedModel = model;
        }
        returnVec = sharedModel;
    accessMutex.unlock();
    return returnVec;
}
