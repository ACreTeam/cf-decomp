// Explicit specializations place each base destructor before its concrete step.
template <>
inline dSvMgr_c::stepBase_c<dSvMgr_c, dSvMgr_c::stepLoad_c>::~stepBase_c() {}
inline dSvMgr_c::stepLoad_c::~stepLoad_c() {}
template <>
inline dSvMgr_c::stepBase_c<dSvMgr_c, dSvMgr_c::stepSaveNormal_c>::~stepBase_c() {}
inline dSvMgr_c::stepSaveNormal_c::~stepSaveNormal_c() {}
template <>
inline dSvMgr_c::stepBase_c<dSvMgr_c, dSvMgr_c::stepSaveRetireNetVst_c>::~stepBase_c() {}
inline dSvMgr_c::stepSaveRetireNetVst_c::~stepSaveRetireNetVst_c() {}
template <>
inline dSvMgr_c::stepBase_c<dSvMgr_c, dSvMgr_c::stepSaveConnectNetHst_c>::~stepBase_c() {}
inline dSvMgr_c::stepSaveConnectNetHst_c::~stepSaveConnectNetHst_c() {}
template <>
inline dSvMgr_c::stepBase_c<dSvMgr_c, dSvMgr_c::stepSaveConnectNetVst_c>::~stepBase_c() {}
inline dSvMgr_c::stepSaveConnectNetVst_c::~stepSaveConnectNetVst_c() {}
template <>
inline dSvMgr_c::stepBase_c<dSvMgr_c, dSvMgr_c::stepSaveContinueNetHst_c>::~stepBase_c() {}
inline dSvMgr_c::stepSaveContinueNetHst_c::~stepSaveContinueNetHst_c() {}
template <>
inline dSvMgr_c::stepBase_c<dSvMgr_c, dSvMgr_c::stepSaveContinueNetVst_c>::~stepBase_c() {}
inline dSvMgr_c::stepSaveContinueNetVst_c::~stepSaveContinueNetVst_c() {}
template <>
inline dSvMgr_c::stepBase_c<dSvMgr_c, dSvMgr_c::stepSaveInterruptNetHst_c>::~stepBase_c() {}
inline dSvMgr_c::stepSaveInterruptNetHst_c::~stepSaveInterruptNetHst_c() {}
template <>
inline dSvMgr_c::stepBase_c<dSvMgr_c, dSvMgr_c::stepSaveInterruptNetVst_c>::~stepBase_c() {}
inline dSvMgr_c::stepSaveInterruptNetVst_c::~stepSaveInterruptNetVst_c() {}
inline dSvMgr_c::~dSvMgr_c() {}
