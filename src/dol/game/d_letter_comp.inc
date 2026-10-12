// Included at the end of src/dol/game/d_letter.cpp. With -sym on, MWCC puts functions from an included file
// after the static initializer, which is where dLetter::Comp_c's destructor sits (800CCC54). Same layout as
// d_string_comp.inc.

// 800CCC54
dLetter::Comp_c::~Comp_c() {}
