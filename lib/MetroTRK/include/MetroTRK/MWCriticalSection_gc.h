#ifndef METROTRK_MWCRITICALSECTION_GC_H
#define METROTRK_MWCRITICALSECTION_GC_H

#ifdef __cplusplus
extern "C"
{
#endif

    void MWInitializeCriticalSection(unsigned int* section);
    void MWEnterCriticalSection(unsigned int* section);
    void MWExitCriticalSection(unsigned int* section);

#ifdef __cplusplus
}
#endif

#endif // METROTRK_MWCRITICALSECTION_GC_H
