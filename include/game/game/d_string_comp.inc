// Included at the end of src/dol/game/d_string.cpp. With -sym on, MWCC puts
// functions from an included file after the static initializer, which is
// where dString::Comp_c's destructor sits (8016B820).

// TODO: fix the weak function order; this .inc is a hack (see d_string.cpp).

// 8016B820
__declspec(weak) dString::Comp_c::~Comp_c() {}
