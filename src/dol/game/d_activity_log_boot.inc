// Included at the end of d_activity_log.cpp. With -sym on, code from an #included file gets its own .text
// section after the TU's __sinit, which is where the target has isBusy (800C7B9C, between __sinit and the
// weak Log_c destructor). Name inferred: d_s_boot_static polls it with the other systems' isBusy checks.

namespace dActivityLog {

// 800C7B9C
BOOL isBusy() {
    return FALSE;
}

} // namespace dActivityLog
