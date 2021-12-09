#include <mutex>
#include <vector>
#include <condition_variable>
#include "VSEPR.h"

// Model shared between threads
// This variable is only available in the scope
// of this file so it cannot be accessed by the render/model threads
// through any non-thread-safe means.
std::vector<BondedElement> sharedModel;
// Mutex to manage thread permissions
std::mutex accessMutex;

bool ready = false;
std::mutex frameMutex;
std::condition_variable frameWait;

/**
 * Get/set the vector representing the chemical model in
 * a position shared between threads.
 * Uses a mutex to ensure thread-safe read/write.
 * Also prohibits writing to data while drawing a frame
 *
 * @param model an updated version of the model if it's being set (optional for read)
*/
std::vector<BondedElement> mutateModel(std::vector<BondedElement> model) {
    std::vector<BondedElement> returnVec;
    accessMutex.lock();
        if(model.size() > 0) {
            std::unique_lock<std::mutex> lck(frameMutex);
            while (!ready) frameWait.wait(lck);
            sharedModel = model;
        }
        returnVec = sharedModel;
    accessMutex.unlock();
    return returnVec;
}

/**
 * Called by render thread to allow mutation of model variable.
 * Mutation is prohibited during a frame draw to avoid errors with
 * unsupported representations.
*/
void readyFrameUpdate() {
    // Tell the model thread it's safe to write data
    {
        std::lock_guard<std::mutex> lock(frameMutex);
        ready = true;
    }
    frameWait.notify_one();
}
