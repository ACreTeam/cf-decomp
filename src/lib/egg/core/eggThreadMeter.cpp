#include <lib/egg/core/eggThreadMeter.h>

namespace EGG {

int ThreadMeter::findIndex(OSThread *thread) const {
    for (int i = 0; i < mThreadCount; i++) {
        if (mpThreads[i].mpThread == thread) {
            return i;
        }
    }
    return -1;
}

int ThreadMeter::add(OSThread *thread) {
    if (mThreadCount != mMaxThreads) {
        int index = mThreadCount;
        mpThreads[index].mpThread = thread;
        mpThreads[index].mColor = mDefaultColor;
        mpThreads[index].mVisible = true;
        mpThreads[index]._09 = false;
        mpThreads[index]._2C = mDefaultParam;
        mThreadCount++;

        if (mSortEnabled) {
            sortByPriority();
        }
        return findIndex(thread);
    }
    return -1;
}

void ThreadMeter::sortByPriority() {
    for (int i = 0; i < mThreadCount - 1; i++) {
        int minPriority = 31;
        int minIndex = -1;
        for (int j = i; j < mThreadCount; j++) {
            int priority = OSGetThreadPriority(mpThreads[j].mpThread);
            if (priority <= minPriority) {
                minPriority = priority;
                minIndex = j;
            }
        }

        if (i != minIndex) {
            ThreadEntry tmp = mpThreads[i];
            mpThreads[i] = mpThreads[minIndex];
            mpThreads[minIndex] = tmp;
        }
    }
}

void ThreadMeter::setThreadColor(OSThread *thread, GXColor color) {
    int index = findIndex(thread);
    if (index == -1) {
        index = add(thread);
    }
    if (index != -1) {
        mpThreads[index].mColor = color;
    }
}

} // namespace EGG
