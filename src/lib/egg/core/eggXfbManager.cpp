#include <lib/egg/core/eggXfbManager.h>

#include <revolution/gx.h>
#include <revolution/os.h>
#include <revolution/vi.h>

namespace EGG {

bool XfbManager::isAttached(Xfb *xfb) const {
    Xfb *head = m_00;
    Xfb *it = head;
    if (head != NULL) {
        do {
            if (it == xfb) {
                return true;
            }
            it = it->mNext;
        } while (it != head);
    }

    return false;
}

bool XfbManager::attach(Xfb *xfb) {
    BOOL enabled = OSDisableInterrupts();
    bool success = false;

    if (xfb != NULL && !isAttached(xfb)) {
        xfb->mState = 0;

        if (m_00 == NULL) {
            m_00 = xfb;
            m_04 = xfb;
            xfb->mNext = xfb;
            xfb->mPrev = xfb;
        } else {
            m_00->mPrev->mNext = xfb;
            xfb->mPrev = m_00->mPrev;
            m_00->mPrev = xfb;
            xfb->mNext = m_00;
        }

        m_0c++;
        m_0d++;
        success = true;
    }

    OSRestoreInterrupts(enabled);
    return success;
}

void XfbManager::copyEFB(bool clearEfb) {
    if (m_0c == 1 && m_04 == NULL) {
        m_04 = m_00;
    }

    if (m_04 != NULL) {
        if (clearEfb) {
            GXSetZMode(GX_TRUE, GX_ALWAYS, GX_TRUE);
            GXSetAlphaUpdate(GX_TRUE);
            GXSetColorUpdate(GX_TRUE);
        }

        GXCopyDisp(m_04->mBuffer, clearEfb);
        GXFlush();
        GXDrawDone();

        BOOL enabled = OSDisableInterrupts();
        m_04->mState = 1;
        if (m_08 == NULL) {
            m_08 = m_04;
        }

        Xfb *next = m_04->mNext;
        m_04 = (next->mState == 0) ? next : NULL;
        OSRestoreInterrupts(enabled);
    }
}

void XfbManager::setNextFrameBuffer() {
    if (m_08 != NULL) {
        if (m_0d > 2) {
            m_0d--;
        } else {
            VISetNextFrameBuffer(m_08->mBuffer);
            VIFlush();

            if (m_0c > 1) {
                m_00->mState = 0;
                if (m_04 == NULL) {
                    m_04 = m_00;
                }

                m_08->mState = 2;
                m_00 = m_08;

                Xfb *next = m_08->mNext;
                m_08 = (next->mState == 1) ? next : NULL;
            } else {
                m_08->mState = 0;
                if (m_04 == NULL) {
                    m_04 = m_08;
                }
                m_08 = NULL;
            }
        }
    } else if (m_0c == 1 && m_00 != NULL) {
        m_00->mState = 0;
        m_04 = m_00;
    }
}

} // namespace EGG
