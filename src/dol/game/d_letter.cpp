// d_letter: letter text building and the keyboard key sound effects. .text 800CB0BC..800CCCD8.
// See include/game/game/d_letter.hpp.
#include <game/game/d_letter.hpp>
#include <game/game/d_kbd.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_land.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_region.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_snd_util.hpp>
#include <cstdio>
#include <cstring>
#include <game/snd/snd_id.hpp>

// Key SE tables, per SE set (getKeySeSet). Digits: three rows of ten (playDigitKeySe); letters: A..Z.
#if VERSION == VERSION_RUUP01_00 || VERSION == VERSION_RUUP01_01
static const int sDigitSeEn[] = {
    SE_Keyboard_ENG_Boy_s00, SE_Keyboard_ENG_Boy_s01, SE_Keyboard_ENG_Boy_s02, SE_Keyboard_ENG_Boy_s03,
    SE_Keyboard_ENG_Boy_s04, SE_Keyboard_ENG_Boy_s05, SE_Keyboard_ENG_Boy_s06, SE_Keyboard_ENG_Boy_s07,
    SE_Keyboard_ENG_Boy_s08, SE_Keyboard_ENG_Boy_s09,
    SE_Keyboard_ENG_Boy_00, SE_Keyboard_ENG_Boy_01, SE_Keyboard_ENG_Boy_02, SE_Keyboard_ENG_Boy_03,
    SE_Keyboard_ENG_Boy_04, SE_Keyboard_ENG_Boy_05, SE_Keyboard_ENG_Boy_06, SE_Keyboard_ENG_Boy_07,
    SE_Keyboard_ENG_Boy_08, SE_Keyboard_ENG_Boy_09,
    SE_Keyboard_ENG_Boy_00_fix, SE_Keyboard_ENG_Boy_01_fix, SE_Keyboard_ENG_Boy_02_fix,
    SE_Keyboard_ENG_Boy_03_fix, SE_Keyboard_ENG_Boy_04_fix, SE_Keyboard_ENG_Boy_05_fix,
    SE_Keyboard_ENG_Boy_06_fix, SE_Keyboard_ENG_Boy_07_fix, SE_Keyboard_ENG_Boy_08_fix,
    SE_Keyboard_ENG_Boy_09_fix,
};

static const int sUpperSeEn[] = {
    SE_Keyboard_ENG_Boy_1a, SE_Keyboard_ENG_Boy_1b, SE_Keyboard_ENG_Boy_1c, SE_Keyboard_ENG_Boy_1d,
    SE_Keyboard_ENG_Boy_1e, SE_Keyboard_ENG_Boy_1f, SE_Keyboard_ENG_Boy_1g, SE_Keyboard_ENG_Boy_1h,
    SE_Keyboard_ENG_Boy_1i, SE_Keyboard_ENG_Boy_1j,
    SE_Keyboard_ENG_Boy_1k, SE_Keyboard_ENG_Boy_1l, SE_Keyboard_ENG_Boy_1m, SE_Keyboard_ENG_Boy_1n,
    SE_Keyboard_ENG_Boy_1o, SE_Keyboard_ENG_Boy_1p, SE_Keyboard_ENG_Boy_1q, SE_Keyboard_ENG_Boy_1r,
    SE_Keyboard_ENG_Boy_1s, SE_Keyboard_ENG_Boy_1t,
    SE_Keyboard_ENG_Boy_1u, SE_Keyboard_ENG_Boy_1v, SE_Keyboard_ENG_Boy_1w, SE_Keyboard_ENG_Boy_1x,
    SE_Keyboard_ENG_Boy_1y, SE_Keyboard_ENG_Boy_1z,
};

static const int sLowerSeEn[] = {
    SE_Keyboard_ENG_Boy_2a, SE_Keyboard_ENG_Boy_2b, SE_Keyboard_ENG_Boy_2c, SE_Keyboard_ENG_Boy_2d,
    SE_Keyboard_ENG_Boy_2e, SE_Keyboard_ENG_Boy_2f, SE_Keyboard_ENG_Boy_2g, SE_Keyboard_ENG_Boy_2h,
    SE_Keyboard_ENG_Boy_2i, SE_Keyboard_ENG_Boy_2j,
    SE_Keyboard_ENG_Boy_2k, SE_Keyboard_ENG_Boy_2l, SE_Keyboard_ENG_Boy_2m, SE_Keyboard_ENG_Boy_2n,
    SE_Keyboard_ENG_Boy_2o, SE_Keyboard_ENG_Boy_2p, SE_Keyboard_ENG_Boy_2q, SE_Keyboard_ENG_Boy_2r,
    SE_Keyboard_ENG_Boy_2s, SE_Keyboard_ENG_Boy_2t,
    SE_Keyboard_ENG_Boy_2u, SE_Keyboard_ENG_Boy_2v, SE_Keyboard_ENG_Boy_2w, SE_Keyboard_ENG_Boy_2x,
    SE_Keyboard_ENG_Boy_2y, SE_Keyboard_ENG_Boy_2z,
};

static const int sAltSeEn[] = {
    SE_Keyboard_ENG_Boy_1a_fix, SE_Keyboard_ENG_Boy_1b_fix, SE_Keyboard_ENG_Boy_1c_fix,
    SE_Keyboard_ENG_Boy_1d_fix, SE_Keyboard_ENG_Boy_1e_fix, SE_Keyboard_ENG_Boy_1f_fix,
    SE_Keyboard_ENG_Boy_1g_fix, SE_Keyboard_ENG_Boy_1h_fix, SE_Keyboard_ENG_Boy_1i_fix,
    SE_Keyboard_ENG_Boy_1j_fix,
    SE_Keyboard_ENG_Boy_1k_fix, SE_Keyboard_ENG_Boy_1l_fix, SE_Keyboard_ENG_Boy_1m_fix,
    SE_Keyboard_ENG_Boy_1n_fix, SE_Keyboard_ENG_Boy_1o_fix, SE_Keyboard_ENG_Boy_1p_fix,
    SE_Keyboard_ENG_Boy_1q_fix, SE_Keyboard_ENG_Boy_1r_fix, SE_Keyboard_ENG_Boy_1s_fix,
    SE_Keyboard_ENG_Boy_1t_fix,
    SE_Keyboard_ENG_Boy_1u_fix, SE_Keyboard_ENG_Boy_1v_fix, SE_Keyboard_ENG_Boy_1w_fix,
    SE_Keyboard_ENG_Boy_1x_fix, SE_Keyboard_ENG_Boy_1y_fix, SE_Keyboard_ENG_Boy_1z_fix,
};

static const int sDigitSeEnF[] = {
    SE_Keyboard_ENG_Gal_s00, SE_Keyboard_ENG_Gal_s01, SE_Keyboard_ENG_Gal_s02, SE_Keyboard_ENG_Gal_s03,
    SE_Keyboard_ENG_Gal_s04, SE_Keyboard_ENG_Gal_s05, SE_Keyboard_ENG_Gal_s06, SE_Keyboard_ENG_Gal_s07,
    SE_Keyboard_ENG_Gal_s08, SE_Keyboard_ENG_Gal_s09,
    SE_Keyboard_ENG_Gal_00, SE_Keyboard_ENG_Gal_01, SE_Keyboard_ENG_Gal_02, SE_Keyboard_ENG_Gal_03,
    SE_Keyboard_ENG_Gal_04, SE_Keyboard_ENG_Gal_05, SE_Keyboard_ENG_Gal_06, SE_Keyboard_ENG_Gal_07,
    SE_Keyboard_ENG_Gal_08, SE_Keyboard_ENG_Gal_09,
    SE_Keyboard_ENG_Gal_00_fix, SE_Keyboard_ENG_Gal_01_fix, SE_Keyboard_ENG_Gal_02_fix,
    SE_Keyboard_ENG_Gal_03_fix, SE_Keyboard_ENG_Gal_04_fix, SE_Keyboard_ENG_Gal_05_fix,
    SE_Keyboard_ENG_Gal_06_fix, SE_Keyboard_ENG_Gal_07_fix, SE_Keyboard_ENG_Gal_08_fix,
    SE_Keyboard_ENG_Gal_09_fix,
};

static const int sUpperSeEnF[] = {
    SE_Keyboard_ENG_Gal_1a, SE_Keyboard_ENG_Gal_1b, SE_Keyboard_ENG_Gal_1c, SE_Keyboard_ENG_Gal_1d,
    SE_Keyboard_ENG_Gal_1e, SE_Keyboard_ENG_Gal_1f, SE_Keyboard_ENG_Gal_1g, SE_Keyboard_ENG_Gal_1h,
    SE_Keyboard_ENG_Gal_1i, SE_Keyboard_ENG_Gal_1j,
    SE_Keyboard_ENG_Gal_1k, SE_Keyboard_ENG_Gal_1l, SE_Keyboard_ENG_Gal_1m, SE_Keyboard_ENG_Gal_1n,
    SE_Keyboard_ENG_Gal_1o, SE_Keyboard_ENG_Gal_1p, SE_Keyboard_ENG_Gal_1q, SE_Keyboard_ENG_Gal_1r,
    SE_Keyboard_ENG_Gal_1s, SE_Keyboard_ENG_Gal_1t,
    SE_Keyboard_ENG_Gal_1u, SE_Keyboard_ENG_Gal_1v, SE_Keyboard_ENG_Gal_1w, SE_Keyboard_ENG_Gal_1x,
    SE_Keyboard_ENG_Gal_1y, SE_Keyboard_ENG_Gal_1z,
};

static const int sAltSeEnF[] = {
    SE_Keyboard_ENG_Gal_1a_fix, SE_Keyboard_ENG_Gal_1b_fix, SE_Keyboard_ENG_Gal_1c_fix,
    SE_Keyboard_ENG_Gal_1d_fix, SE_Keyboard_ENG_Gal_1e_fix, SE_Keyboard_ENG_Gal_1f_fix,
    SE_Keyboard_ENG_Gal_1g_fix, SE_Keyboard_ENG_Gal_1h_fix, SE_Keyboard_ENG_Gal_1i_fix,
    SE_Keyboard_ENG_Gal_1j_fix,
    SE_Keyboard_ENG_Gal_1k_fix, SE_Keyboard_ENG_Gal_1l_fix, SE_Keyboard_ENG_Gal_1m_fix,
    SE_Keyboard_ENG_Gal_1n_fix, SE_Keyboard_ENG_Gal_1o_fix, SE_Keyboard_ENG_Gal_1p_fix,
    SE_Keyboard_ENG_Gal_1q_fix, SE_Keyboard_ENG_Gal_1r_fix, SE_Keyboard_ENG_Gal_1s_fix,
    SE_Keyboard_ENG_Gal_1t_fix,
    SE_Keyboard_ENG_Gal_1u_fix, SE_Keyboard_ENG_Gal_1v_fix, SE_Keyboard_ENG_Gal_1w_fix,
    SE_Keyboard_ENG_Gal_1x_fix, SE_Keyboard_ENG_Gal_1y_fix, SE_Keyboard_ENG_Gal_1z_fix,
};

static const int sLowerSeEnF[] = {
    SE_Keyboard_ENG_Gal_2a, SE_Keyboard_ENG_Gal_2b, SE_Keyboard_ENG_Gal_2c, SE_Keyboard_ENG_Gal_2d,
    SE_Keyboard_ENG_Gal_2e, SE_Keyboard_ENG_Gal_2f, SE_Keyboard_ENG_Gal_2g, SE_Keyboard_ENG_Gal_2h,
    SE_Keyboard_ENG_Gal_2i, SE_Keyboard_ENG_Gal_2j,
    SE_Keyboard_ENG_Gal_2k, SE_Keyboard_ENG_Gal_2l, SE_Keyboard_ENG_Gal_2m, SE_Keyboard_ENG_Gal_2n,
    SE_Keyboard_ENG_Gal_2o, SE_Keyboard_ENG_Gal_2p, SE_Keyboard_ENG_Gal_2q, SE_Keyboard_ENG_Gal_2r,
    SE_Keyboard_ENG_Gal_2s, SE_Keyboard_ENG_Gal_2t,
    SE_Keyboard_ENG_Gal_2u, SE_Keyboard_ENG_Gal_2v, SE_Keyboard_ENG_Gal_2w, SE_Keyboard_ENG_Gal_2x,
    SE_Keyboard_ENG_Gal_2y, SE_Keyboard_ENG_Gal_2z,
};

static const int sDigitSeFr[] = {
    SE_Keyboard_FRN_Boy_00, SE_Keyboard_FRN_Boy_01, SE_Keyboard_FRN_Boy_02, SE_Keyboard_FRN_Boy_03,
    SE_Keyboard_FRN_Boy_04, SE_Keyboard_FRN_Boy_05, SE_Keyboard_FRN_Boy_06, SE_Keyboard_FRN_Boy_07,
    SE_Keyboard_FRN_Boy_08, SE_Keyboard_FRN_Boy_09,
    SE_Keyboard_FRN_Boy_00, SE_Keyboard_FRN_Boy_01, SE_Keyboard_FRN_Boy_02, SE_Keyboard_FRN_Boy_03,
    SE_Keyboard_FRN_Boy_04, SE_Keyboard_FRN_Boy_05, SE_Keyboard_FRN_Boy_06, SE_Keyboard_FRN_Boy_07,
    SE_Keyboard_FRN_Boy_08, SE_Keyboard_FRN_Boy_09,
    SE_Keyboard_FRN_Boy_00_fix, SE_Keyboard_FRN_Boy_01_fix, SE_Keyboard_FRN_Boy_02_fix,
    SE_Keyboard_FRN_Boy_03_fix, SE_Keyboard_FRN_Boy_04_fix, SE_Keyboard_FRN_Boy_05_fix,
    SE_Keyboard_FRN_Boy_06_fix, SE_Keyboard_FRN_Boy_07_fix, SE_Keyboard_FRN_Boy_08_fix,
    SE_Keyboard_FRN_Boy_09_fix,
};

static const int sUpperSeFr[] = {
    SE_Keyboard_FRN_Boy_1a, SE_Keyboard_FRN_Boy_1b, SE_Keyboard_FRN_Boy_1c, SE_Keyboard_FRN_Boy_1d,
    SE_Keyboard_FRN_Boy_1e, SE_Keyboard_FRN_Boy_1f, SE_Keyboard_FRN_Boy_1g, SE_Keyboard_FRN_Boy_1h,
    SE_Keyboard_FRN_Boy_1i, SE_Keyboard_FRN_Boy_1j,
    SE_Keyboard_FRN_Boy_1k, SE_Keyboard_FRN_Boy_1l, SE_Keyboard_FRN_Boy_1m, SE_Keyboard_FRN_Boy_1n,
    SE_Keyboard_FRN_Boy_1o, SE_Keyboard_FRN_Boy_1p, SE_Keyboard_FRN_Boy_1q, SE_Keyboard_FRN_Boy_1r,
    SE_Keyboard_FRN_Boy_1s, SE_Keyboard_FRN_Boy_1t,
    SE_Keyboard_FRN_Boy_1u, SE_Keyboard_FRN_Boy_1v, SE_Keyboard_FRN_Boy_1w, SE_Keyboard_FRN_Boy_1x,
    SE_Keyboard_FRN_Boy_1y, SE_Keyboard_FRN_Boy_1z,
};

static const int sAltSeFr[] = {
    SE_Keyboard_FRN_Boy_1a_fix, SE_Keyboard_FRN_Boy_1b_fix, SE_Keyboard_FRN_Boy_1c_fix,
    SE_Keyboard_FRN_Boy_1d_fix, SE_Keyboard_FRN_Boy_1e_fix, SE_Keyboard_FRN_Boy_1f_fix,
    SE_Keyboard_FRN_Boy_1g_fix, SE_Keyboard_FRN_Boy_1h_fix, SE_Keyboard_FRN_Boy_1i_fix,
    SE_Keyboard_FRN_Boy_1j_fix,
    SE_Keyboard_FRN_Boy_1k_fix, SE_Keyboard_FRN_Boy_1l_fix, SE_Keyboard_FRN_Boy_1m_fix,
    SE_Keyboard_FRN_Boy_1n_fix, SE_Keyboard_FRN_Boy_1o_fix, SE_Keyboard_FRN_Boy_1p_fix,
    SE_Keyboard_FRN_Boy_1q_fix, SE_Keyboard_FRN_Boy_1r_fix, SE_Keyboard_FRN_Boy_1s_fix,
    SE_Keyboard_FRN_Boy_1t_fix,
    SE_Keyboard_FRN_Boy_1u_fix, SE_Keyboard_FRN_Boy_1v_fix, SE_Keyboard_FRN_Boy_1w_fix,
    SE_Keyboard_FRN_Boy_1x_fix, SE_Keyboard_FRN_Boy_1y_fix, SE_Keyboard_FRN_Boy_1z_fix,
};

static const int sLowerSeFr[] = {
    SE_Keyboard_FRN_Boy_2a, SE_Keyboard_FRN_Boy_2b, SE_Keyboard_FRN_Boy_2c, SE_Keyboard_FRN_Boy_2d,
    SE_Keyboard_FRN_Boy_2e, SE_Keyboard_FRN_Boy_2f, SE_Keyboard_FRN_Boy_2g, SE_Keyboard_FRN_Boy_2h,
    SE_Keyboard_FRN_Boy_2i, SE_Keyboard_FRN_Boy_2j,
    SE_Keyboard_FRN_Boy_2k, SE_Keyboard_FRN_Boy_2l, SE_Keyboard_FRN_Boy_2m, SE_Keyboard_FRN_Boy_2n,
    SE_Keyboard_FRN_Boy_2o, SE_Keyboard_FRN_Boy_2p, SE_Keyboard_FRN_Boy_2q, SE_Keyboard_FRN_Boy_2r,
    SE_Keyboard_FRN_Boy_2s, SE_Keyboard_FRN_Boy_2t,
    SE_Keyboard_FRN_Boy_2u, SE_Keyboard_FRN_Boy_2v, SE_Keyboard_FRN_Boy_2w, SE_Keyboard_FRN_Boy_2x,
    SE_Keyboard_FRN_Boy_2y, SE_Keyboard_FRN_Boy_2z,
};

static const int sDigitSeFrF[] = {
    SE_Keyboard_FRN_Gal_00, SE_Keyboard_FRN_Gal_01, SE_Keyboard_FRN_Gal_02, SE_Keyboard_FRN_Gal_03,
    SE_Keyboard_FRN_Gal_04, SE_Keyboard_FRN_Gal_05, SE_Keyboard_FRN_Gal_06, SE_Keyboard_FRN_Gal_07,
    SE_Keyboard_FRN_Gal_08, SE_Keyboard_FRN_Gal_09,
    SE_Keyboard_FRN_Gal_00, SE_Keyboard_FRN_Gal_01, SE_Keyboard_FRN_Gal_02, SE_Keyboard_FRN_Gal_03,
    SE_Keyboard_FRN_Gal_04, SE_Keyboard_FRN_Gal_05, SE_Keyboard_FRN_Gal_06, SE_Keyboard_FRN_Gal_07,
    SE_Keyboard_FRN_Gal_08, SE_Keyboard_FRN_Gal_09,
    SE_Keyboard_FRN_Gal_00_fix, SE_Keyboard_FRN_Gal_01_fix, SE_Keyboard_FRN_Gal_02_fix,
    SE_Keyboard_FRN_Gal_03_fix, SE_Keyboard_FRN_Gal_04_fix, SE_Keyboard_FRN_Gal_05_fix,
    SE_Keyboard_FRN_Gal_06_fix, SE_Keyboard_FRN_Gal_07_fix, SE_Keyboard_FRN_Gal_08_fix,
    SE_Keyboard_FRN_Gal_09_fix,
};

static const int sUpperSeFrF[] = {
    SE_Keyboard_FRN_Gal_1a, SE_Keyboard_FRN_Gal_1b, SE_Keyboard_FRN_Gal_1c, SE_Keyboard_FRN_Gal_1d,
    SE_Keyboard_FRN_Gal_1e, SE_Keyboard_FRN_Gal_1f, SE_Keyboard_FRN_Gal_1g, SE_Keyboard_FRN_Gal_1h,
    SE_Keyboard_FRN_Gal_1i, SE_Keyboard_FRN_Gal_1j,
    SE_Keyboard_FRN_Gal_1k, SE_Keyboard_FRN_Gal_1l, SE_Keyboard_FRN_Gal_1m, SE_Keyboard_FRN_Gal_1n,
    SE_Keyboard_FRN_Gal_1o, SE_Keyboard_FRN_Gal_1p, SE_Keyboard_FRN_Gal_1q, SE_Keyboard_FRN_Gal_1r,
    SE_Keyboard_FRN_Gal_1s, SE_Keyboard_FRN_Gal_1t,
    SE_Keyboard_FRN_Gal_1u, SE_Keyboard_FRN_Gal_1v, SE_Keyboard_FRN_Gal_1w, SE_Keyboard_FRN_Gal_1x,
    SE_Keyboard_FRN_Gal_1y, SE_Keyboard_FRN_Gal_1z,
};

static const int sAltSeFrF[] = {
    SE_Keyboard_FRN_Gal_1a_fix, SE_Keyboard_FRN_Gal_1b_fix, SE_Keyboard_FRN_Gal_1c_fix,
    SE_Keyboard_FRN_Gal_1d_fix, SE_Keyboard_FRN_Gal_1e_fix, SE_Keyboard_FRN_Gal_1f_fix,
    SE_Keyboard_FRN_Gal_1g_fix, SE_Keyboard_FRN_Gal_1h_fix, SE_Keyboard_FRN_Gal_1i_fix,
    SE_Keyboard_FRN_Gal_1j_fix,
    SE_Keyboard_FRN_Gal_1k_fix, SE_Keyboard_FRN_Gal_1l_fix, SE_Keyboard_FRN_Gal_1m_fix,
    SE_Keyboard_FRN_Gal_1n_fix, SE_Keyboard_FRN_Gal_1o_fix, SE_Keyboard_FRN_Gal_1p_fix,
    SE_Keyboard_FRN_Gal_1q_fix, SE_Keyboard_FRN_Gal_1r_fix, SE_Keyboard_FRN_Gal_1s_fix,
    SE_Keyboard_FRN_Gal_1t_fix,
    SE_Keyboard_FRN_Gal_1u_fix, SE_Keyboard_FRN_Gal_1v_fix, SE_Keyboard_FRN_Gal_1w_fix,
    SE_Keyboard_FRN_Gal_1x_fix, SE_Keyboard_FRN_Gal_1y_fix, SE_Keyboard_FRN_Gal_1z_fix,
};

static const int sLowerSeFrF[] = {
    SE_Keyboard_FRN_Gal_2a, SE_Keyboard_FRN_Gal_2b, SE_Keyboard_FRN_Gal_2c, SE_Keyboard_FRN_Gal_2d,
    SE_Keyboard_FRN_Gal_2e, SE_Keyboard_FRN_Gal_2f, SE_Keyboard_FRN_Gal_2g, SE_Keyboard_FRN_Gal_2h,
    SE_Keyboard_FRN_Gal_2i, SE_Keyboard_FRN_Gal_2j,
    SE_Keyboard_FRN_Gal_2k, SE_Keyboard_FRN_Gal_2l, SE_Keyboard_FRN_Gal_2m, SE_Keyboard_FRN_Gal_2n,
    SE_Keyboard_FRN_Gal_2o, SE_Keyboard_FRN_Gal_2p, SE_Keyboard_FRN_Gal_2q, SE_Keyboard_FRN_Gal_2r,
    SE_Keyboard_FRN_Gal_2s, SE_Keyboard_FRN_Gal_2t,
    SE_Keyboard_FRN_Gal_2u, SE_Keyboard_FRN_Gal_2v, SE_Keyboard_FRN_Gal_2w, SE_Keyboard_FRN_Gal_2x,
    SE_Keyboard_FRN_Gal_2y, SE_Keyboard_FRN_Gal_2z,
};

static const int sDigitSeEs[] = {
    SE_Keyboard_SPN_Boy_s00, SE_Keyboard_SPN_Boy_s01, SE_Keyboard_SPN_Boy_s02, SE_Keyboard_SPN_Boy_s03,
    SE_Keyboard_SPN_Boy_s04, SE_Keyboard_SPN_Boy_s05, SE_Keyboard_SPN_Boy_s06, SE_Keyboard_SPN_Boy_s07,
    SE_Keyboard_SPN_Boy_s08, SE_Keyboard_SPN_Boy_s09,
    SE_Keyboard_SPN_Boy_00, SE_Keyboard_SPN_Boy_01, SE_Keyboard_SPN_Boy_02, SE_Keyboard_SPN_Boy_03,
    SE_Keyboard_SPN_Boy_04, SE_Keyboard_SPN_Boy_05, SE_Keyboard_SPN_Boy_06, SE_Keyboard_SPN_Boy_07,
    SE_Keyboard_SPN_Boy_08, SE_Keyboard_SPN_Boy_09,
    SE_Keyboard_SPN_Boy_00_fix, SE_Keyboard_SPN_Boy_01_fix, SE_Keyboard_SPN_Boy_02_fix,
    SE_Keyboard_SPN_Boy_03_fix, SE_Keyboard_SPN_Boy_04_fix, SE_Keyboard_SPN_Boy_05_fix,
    SE_Keyboard_SPN_Boy_06_fix, SE_Keyboard_SPN_Boy_07_fix, SE_Keyboard_SPN_Boy_08_fix,
    SE_Keyboard_SPN_Boy_09_fix,
};

static const int sUpperSeEs[] = {
    SE_Keyboard_SPN_Boy_1a, SE_Keyboard_SPN_Boy_1b, SE_Keyboard_SPN_Boy_1c, SE_Keyboard_SPN_Boy_1d,
    SE_Keyboard_SPN_Boy_1e, SE_Keyboard_SPN_Boy_1f, SE_Keyboard_SPN_Boy_1g, SE_Keyboard_SPN_Boy_1h,
    SE_Keyboard_SPN_Boy_1i, SE_Keyboard_SPN_Boy_1j,
    SE_Keyboard_SPN_Boy_1k, SE_Keyboard_SPN_Boy_1l, SE_Keyboard_SPN_Boy_1m, SE_Keyboard_SPN_Boy_1n,
    SE_Keyboard_SPN_Boy_1o, SE_Keyboard_SPN_Boy_1p, SE_Keyboard_SPN_Boy_1q, SE_Keyboard_SPN_Boy_1r,
    SE_Keyboard_SPN_Boy_1s, SE_Keyboard_SPN_Boy_1t,
    SE_Keyboard_SPN_Boy_1u, SE_Keyboard_SPN_Boy_1v, SE_Keyboard_SPN_Boy_1w, SE_Keyboard_SPN_Boy_1x,
    SE_Keyboard_SPN_Boy_1y, SE_Keyboard_SPN_Boy_1z,
};

static const int sAltSeEs[] = {
    SE_Keyboard_SPN_Boy_1a_fix, SE_Keyboard_SPN_Boy_1b_fix, SE_Keyboard_SPN_Boy_1c_fix,
    SE_Keyboard_SPN_Boy_1d_fix, SE_Keyboard_SPN_Boy_1e_fix, SE_Keyboard_SPN_Boy_1f_fix,
    SE_Keyboard_SPN_Boy_1g_fix, SE_Keyboard_SPN_Boy_1h_fix, SE_Keyboard_SPN_Boy_1i_fix,
    SE_Keyboard_SPN_Boy_1j_fix,
    SE_Keyboard_SPN_Boy_1k_fix, SE_Keyboard_SPN_Boy_1l_fix, SE_Keyboard_SPN_Boy_1m_fix,
    SE_Keyboard_SPN_Boy_1n_fix, SE_Keyboard_SPN_Boy_1o_fix, SE_Keyboard_SPN_Boy_1p_fix,
    SE_Keyboard_SPN_Boy_1q_fix, SE_Keyboard_SPN_Boy_1r_fix, SE_Keyboard_SPN_Boy_1s_fix,
    SE_Keyboard_SPN_Boy_1t_fix,
    SE_Keyboard_SPN_Boy_1u_fix, SE_Keyboard_SPN_Boy_1v_fix, SE_Keyboard_SPN_Boy_1w_fix,
    SE_Keyboard_SPN_Boy_1x_fix, SE_Keyboard_SPN_Boy_1y_fix, SE_Keyboard_SPN_Boy_1z_fix,
};

static const int sLowerSeEs[] = {
    SE_Keyboard_SPN_Boy_2a, SE_Keyboard_SPN_Boy_2b, SE_Keyboard_SPN_Boy_2c, SE_Keyboard_SPN_Boy_2d,
    SE_Keyboard_SPN_Boy_2e, SE_Keyboard_SPN_Boy_2f, SE_Keyboard_SPN_Boy_2g, SE_Keyboard_SPN_Boy_2h,
    SE_Keyboard_SPN_Boy_2i, SE_Keyboard_SPN_Boy_2j,
    SE_Keyboard_SPN_Boy_2k, SE_Keyboard_SPN_Boy_2l, SE_Keyboard_SPN_Boy_2m, SE_Keyboard_SPN_Boy_2n,
    SE_Keyboard_SPN_Boy_2o, SE_Keyboard_SPN_Boy_2p, SE_Keyboard_SPN_Boy_2q, SE_Keyboard_SPN_Boy_2r,
    SE_Keyboard_SPN_Boy_2s, SE_Keyboard_SPN_Boy_2t,
    SE_Keyboard_SPN_Boy_2u, SE_Keyboard_SPN_Boy_2v, SE_Keyboard_SPN_Boy_2w, SE_Keyboard_SPN_Boy_2x,
    SE_Keyboard_SPN_Boy_2y, SE_Keyboard_SPN_Boy_2z,
};

static const int sDigitSeEsF[] = {
    SE_Keyboard_SPN_Gal_s00, SE_Keyboard_SPN_Gal_s01, SE_Keyboard_SPN_Gal_s02, SE_Keyboard_SPN_Gal_s03,
    SE_Keyboard_SPN_Gal_s04, SE_Keyboard_SPN_Gal_s05, SE_Keyboard_SPN_Gal_s06, SE_Keyboard_SPN_Gal_s07,
    SE_Keyboard_SPN_Gal_s08, SE_Keyboard_SPN_Gal_s09,
    SE_Keyboard_SPN_Gal_00, SE_Keyboard_SPN_Gal_01, SE_Keyboard_SPN_Gal_02, SE_Keyboard_SPN_Gal_03,
    SE_Keyboard_SPN_Gal_04, SE_Keyboard_SPN_Gal_05, SE_Keyboard_SPN_Gal_06, SE_Keyboard_SPN_Gal_07,
    SE_Keyboard_SPN_Gal_08, SE_Keyboard_SPN_Gal_09,
    SE_Keyboard_SPN_Gal_00_fix, SE_Keyboard_SPN_Gal_01_fix, SE_Keyboard_SPN_Gal_02_fix,
    SE_Keyboard_SPN_Gal_03_fix, SE_Keyboard_SPN_Gal_04_fix, SE_Keyboard_SPN_Gal_05_fix,
    SE_Keyboard_SPN_Gal_06_fix, SE_Keyboard_SPN_Gal_07_fix, SE_Keyboard_SPN_Gal_08_fix,
    SE_Keyboard_SPN_Gal_09_fix,
};

static const int sUpperSeEsF[] = {
    SE_Keyboard_SPN_Gal_1a, SE_Keyboard_SPN_Gal_1b, SE_Keyboard_SPN_Gal_1c, SE_Keyboard_SPN_Gal_1d,
    SE_Keyboard_SPN_Gal_1e, SE_Keyboard_SPN_Gal_1f, SE_Keyboard_SPN_Gal_1g, SE_Keyboard_SPN_Gal_1h,
    SE_Keyboard_SPN_Gal_1i, SE_Keyboard_SPN_Gal_1j,
    SE_Keyboard_SPN_Gal_1k, SE_Keyboard_SPN_Gal_1l, SE_Keyboard_SPN_Gal_1m, SE_Keyboard_SPN_Gal_1n,
    SE_Keyboard_SPN_Gal_1o, SE_Keyboard_SPN_Gal_1p, SE_Keyboard_SPN_Gal_1q, SE_Keyboard_SPN_Gal_1r,
    SE_Keyboard_SPN_Gal_1s, SE_Keyboard_SPN_Gal_1t,
    SE_Keyboard_SPN_Gal_1u, SE_Keyboard_SPN_Gal_1v, SE_Keyboard_SPN_Gal_1w, SE_Keyboard_SPN_Gal_1x,
    SE_Keyboard_SPN_Gal_1y, SE_Keyboard_SPN_Gal_1z,
};

static const int sAltSeEsF[] = {
    SE_Keyboard_SPN_Gal_1a_fix, SE_Keyboard_SPN_Gal_1b_fix, SE_Keyboard_SPN_Gal_1c_fix,
    SE_Keyboard_SPN_Gal_1d_fix, SE_Keyboard_SPN_Gal_1e_fix, SE_Keyboard_SPN_Gal_1f_fix,
    SE_Keyboard_SPN_Gal_1g_fix, SE_Keyboard_SPN_Gal_1h_fix, SE_Keyboard_SPN_Gal_1i_fix,
    SE_Keyboard_SPN_Gal_1j_fix,
    SE_Keyboard_SPN_Gal_1k_fix, SE_Keyboard_SPN_Gal_1l_fix, SE_Keyboard_SPN_Gal_1m_fix,
    SE_Keyboard_SPN_Gal_1n_fix, SE_Keyboard_SPN_Gal_1o_fix, SE_Keyboard_SPN_Gal_1p_fix,
    SE_Keyboard_SPN_Gal_1q_fix, SE_Keyboard_SPN_Gal_1r_fix, SE_Keyboard_SPN_Gal_1s_fix,
    SE_Keyboard_SPN_Gal_1t_fix,
    SE_Keyboard_SPN_Gal_1u_fix, SE_Keyboard_SPN_Gal_1v_fix, SE_Keyboard_SPN_Gal_1w_fix,
    SE_Keyboard_SPN_Gal_1x_fix, SE_Keyboard_SPN_Gal_1y_fix, SE_Keyboard_SPN_Gal_1z_fix,
};

static const int sLowerSeEsF[] = {
    SE_Keyboard_SPN_Gal_2a, SE_Keyboard_SPN_Gal_2b, SE_Keyboard_SPN_Gal_2c, SE_Keyboard_SPN_Gal_2d,
    SE_Keyboard_SPN_Gal_2e, SE_Keyboard_SPN_Gal_2f, SE_Keyboard_SPN_Gal_2g, SE_Keyboard_SPN_Gal_2h,
    SE_Keyboard_SPN_Gal_2i, SE_Keyboard_SPN_Gal_2j,
    SE_Keyboard_SPN_Gal_2k, SE_Keyboard_SPN_Gal_2l, SE_Keyboard_SPN_Gal_2m, SE_Keyboard_SPN_Gal_2n,
    SE_Keyboard_SPN_Gal_2o, SE_Keyboard_SPN_Gal_2p, SE_Keyboard_SPN_Gal_2q, SE_Keyboard_SPN_Gal_2r,
    SE_Keyboard_SPN_Gal_2s, SE_Keyboard_SPN_Gal_2t,
    SE_Keyboard_SPN_Gal_2u, SE_Keyboard_SPN_Gal_2v, SE_Keyboard_SPN_Gal_2w, SE_Keyboard_SPN_Gal_2x,
    SE_Keyboard_SPN_Gal_2y, SE_Keyboard_SPN_Gal_2z,
};

static const int sDigitSeDe[] = {
    SE_Keyboard_GER_Boy_s00, SE_Keyboard_GER_Boy_s01, SE_Keyboard_GER_Boy_s02, SE_Keyboard_GER_Boy_s03,
    SE_Keyboard_GER_Boy_s04, SE_Keyboard_GER_Boy_s05, SE_Keyboard_GER_Boy_s06, SE_Keyboard_GER_Boy_s07,
    SE_Keyboard_GER_Boy_s08, SE_Keyboard_GER_Boy_s09,
    SE_Keyboard_GER_Boy_00, SE_Keyboard_GER_Boy_01, SE_Keyboard_GER_Boy_02, SE_Keyboard_GER_Boy_03,
    SE_Keyboard_GER_Boy_04, SE_Keyboard_GER_Boy_05, SE_Keyboard_GER_Boy_06, SE_Keyboard_GER_Boy_07,
    SE_Keyboard_GER_Boy_08, SE_Keyboard_GER_Boy_09,
    SE_Keyboard_GER_Boy_00_fix, SE_Keyboard_GER_Boy_01_fix, SE_Keyboard_GER_Boy_02_fix,
    SE_Keyboard_GER_Boy_03_fix, SE_Keyboard_GER_Boy_04_fix, SE_Keyboard_GER_Boy_05_fix,
    SE_Keyboard_GER_Boy_06_fix, SE_Keyboard_GER_Boy_07_fix, SE_Keyboard_GER_Boy_08_fix,
    SE_Keyboard_GER_Boy_09_fix,
};

static const int sUpperSeDe[] = {
    SE_Keyboard_GER_Boy_1a, SE_Keyboard_GER_Boy_1b, SE_Keyboard_GER_Boy_1c, SE_Keyboard_GER_Boy_1d,
    SE_Keyboard_GER_Boy_1e, SE_Keyboard_GER_Boy_1f, SE_Keyboard_GER_Boy_1g, SE_Keyboard_GER_Boy_1h,
    SE_Keyboard_GER_Boy_1i, SE_Keyboard_GER_Boy_1j,
    SE_Keyboard_GER_Boy_1k, SE_Keyboard_GER_Boy_1l, SE_Keyboard_GER_Boy_1m, SE_Keyboard_GER_Boy_1n,
    SE_Keyboard_GER_Boy_1o, SE_Keyboard_GER_Boy_1p, SE_Keyboard_GER_Boy_1q, SE_Keyboard_GER_Boy_1r,
    SE_Keyboard_GER_Boy_1s, SE_Keyboard_GER_Boy_1t,
    SE_Keyboard_GER_Boy_1u, SE_Keyboard_GER_Boy_1v, SE_Keyboard_GER_Boy_1w, SE_Keyboard_GER_Boy_1x,
    SE_Keyboard_GER_Boy_1y, SE_Keyboard_GER_Boy_1z,
};

static const int sAltSeDe[] = {
    SE_Keyboard_GER_Boy_1a_fix, SE_Keyboard_GER_Boy_1b_fix, SE_Keyboard_GER_Boy_1c_fix,
    SE_Keyboard_GER_Boy_1d_fix, SE_Keyboard_GER_Boy_1e_fix, SE_Keyboard_GER_Boy_1f_fix,
    SE_Keyboard_GER_Boy_1g_fix, SE_Keyboard_GER_Boy_1h_fix, SE_Keyboard_GER_Boy_1i_fix,
    SE_Keyboard_GER_Boy_1j_fix,
    SE_Keyboard_GER_Boy_1k_fix, SE_Keyboard_GER_Boy_1l_fix, SE_Keyboard_GER_Boy_1m_fix,
    SE_Keyboard_GER_Boy_1n_fix, SE_Keyboard_GER_Boy_1o_fix, SE_Keyboard_GER_Boy_1p_fix,
    SE_Keyboard_GER_Boy_1q_fix, SE_Keyboard_GER_Boy_1r_fix, SE_Keyboard_GER_Boy_1s_fix,
    SE_Keyboard_GER_Boy_1t_fix,
    SE_Keyboard_GER_Boy_1u_fix, SE_Keyboard_GER_Boy_1v_fix, SE_Keyboard_GER_Boy_1w_fix,
    SE_Keyboard_GER_Boy_1x_fix, SE_Keyboard_GER_Boy_1y_fix, SE_Keyboard_GER_Boy_1z_fix,
};

static const int sLowerSeDe[] = {
    SE_Keyboard_GER_Boy_2a, SE_Keyboard_GER_Boy_2b, SE_Keyboard_GER_Boy_2c, SE_Keyboard_GER_Boy_2d,
    SE_Keyboard_GER_Boy_2e, SE_Keyboard_GER_Boy_2f, SE_Keyboard_GER_Boy_2g, SE_Keyboard_GER_Boy_2h,
    SE_Keyboard_GER_Boy_2i, SE_Keyboard_GER_Boy_2j,
    SE_Keyboard_GER_Boy_2k, SE_Keyboard_GER_Boy_2l, SE_Keyboard_GER_Boy_2m, SE_Keyboard_GER_Boy_2n,
    SE_Keyboard_GER_Boy_2o, SE_Keyboard_GER_Boy_2p, SE_Keyboard_GER_Boy_2q, SE_Keyboard_GER_Boy_2r,
    SE_Keyboard_GER_Boy_2s, SE_Keyboard_GER_Boy_2t,
    SE_Keyboard_GER_Boy_2u, SE_Keyboard_GER_Boy_2v, SE_Keyboard_GER_Boy_2w, SE_Keyboard_GER_Boy_2x,
    SE_Keyboard_GER_Boy_2y, SE_Keyboard_GER_Boy_2z,
};

static const int sUmlautSeDe[] = {
    SE_Keyboard_GER_Boy_2_umlaut_a, SE_Keyboard_GER_Boy_2_umlaut_o, SE_Keyboard_GER_Boy_2_umlaut_u,
    SE_Keyboard_GER_Boy_2_umlaut_eszett,
    SE_Keyboard_GER_Boy_1_umlaut_a, SE_Keyboard_GER_Boy_1_umlaut_o, SE_Keyboard_GER_Boy_1_umlaut_u,
    SE_Keyboard_GER_Boy_1_umlaut_a_fix, SE_Keyboard_GER_Boy_1_umlaut_o_fix,
    SE_Keyboard_GER_Boy_1_umlaut_u_fix, SE_Keyboard_GER_Boy_1_umlaut_eszett_fix,
    SE_Keyboard_GER_Boy_1_umlaut_a_fix, SE_Keyboard_GER_Boy_1_umlaut_o_fix,
    SE_Keyboard_GER_Boy_1_umlaut_u_fix,
};

static const int sDigitSeDeF[] = {
    SE_Keyboard_GER_Gal_s00, SE_Keyboard_GER_Gal_s01, SE_Keyboard_GER_Gal_s02, SE_Keyboard_GER_Gal_s03,
    SE_Keyboard_GER_Gal_s04, SE_Keyboard_GER_Gal_s05, SE_Keyboard_GER_Gal_s06, SE_Keyboard_GER_Gal_s07,
    SE_Keyboard_GER_Gal_s08, SE_Keyboard_GER_Gal_s09,
    SE_Keyboard_GER_Gal_00, SE_Keyboard_GER_Gal_01, SE_Keyboard_GER_Gal_02, SE_Keyboard_GER_Gal_03,
    SE_Keyboard_GER_Gal_04, SE_Keyboard_GER_Gal_05, SE_Keyboard_GER_Gal_06, SE_Keyboard_GER_Gal_07,
    SE_Keyboard_GER_Gal_08, SE_Keyboard_GER_Gal_09,
    SE_Keyboard_GER_Gal_00_fix, SE_Keyboard_GER_Gal_01_fix, SE_Keyboard_GER_Gal_02_fix,
    SE_Keyboard_GER_Gal_03_fix, SE_Keyboard_GER_Gal_04_fix, SE_Keyboard_GER_Gal_05_fix,
    SE_Keyboard_GER_Gal_06_fix, SE_Keyboard_GER_Gal_07_fix, SE_Keyboard_GER_Gal_08_fix,
    SE_Keyboard_GER_Gal_09_fix,
};

static const int sUpperSeDeF[] = {
    SE_Keyboard_GER_Gal_1a, SE_Keyboard_GER_Gal_1b, SE_Keyboard_GER_Gal_1c, SE_Keyboard_GER_Gal_1d,
    SE_Keyboard_GER_Gal_1e, SE_Keyboard_GER_Gal_1f, SE_Keyboard_GER_Gal_1g, SE_Keyboard_GER_Gal_1h,
    SE_Keyboard_GER_Gal_1i, SE_Keyboard_GER_Gal_1j,
    SE_Keyboard_GER_Gal_1k, SE_Keyboard_GER_Gal_1l, SE_Keyboard_GER_Gal_1m, SE_Keyboard_GER_Gal_1n,
    SE_Keyboard_GER_Gal_1o, SE_Keyboard_GER_Gal_1p, SE_Keyboard_GER_Gal_1q, SE_Keyboard_GER_Gal_1r,
    SE_Keyboard_GER_Gal_1s, SE_Keyboard_GER_Gal_1t,
    SE_Keyboard_GER_Gal_1u, SE_Keyboard_GER_Gal_1v, SE_Keyboard_GER_Gal_1w, SE_Keyboard_GER_Gal_1x,
    SE_Keyboard_GER_Gal_1y, SE_Keyboard_GER_Gal_1z,
};

static const int sAltSeDeF[] = {
    SE_Keyboard_GER_Gal_1a_fix, SE_Keyboard_GER_Gal_1b_fix, SE_Keyboard_GER_Gal_1c_fix,
    SE_Keyboard_GER_Gal_1d_fix, SE_Keyboard_GER_Gal_1e_fix, SE_Keyboard_GER_Gal_1f_fix,
    SE_Keyboard_GER_Gal_1g_fix, SE_Keyboard_GER_Gal_1h_fix, SE_Keyboard_GER_Gal_1i_fix,
    SE_Keyboard_GER_Gal_1j_fix,
    SE_Keyboard_GER_Gal_1k_fix, SE_Keyboard_GER_Gal_1l_fix, SE_Keyboard_GER_Gal_1m_fix,
    SE_Keyboard_GER_Gal_1n_fix, SE_Keyboard_GER_Gal_1o_fix, SE_Keyboard_GER_Gal_1p_fix,
    SE_Keyboard_GER_Gal_1q_fix, SE_Keyboard_GER_Gal_1r_fix, SE_Keyboard_GER_Gal_1s_fix,
    SE_Keyboard_GER_Gal_1t_fix,
    SE_Keyboard_GER_Gal_1u_fix, SE_Keyboard_GER_Gal_1v_fix, SE_Keyboard_GER_Gal_1w_fix,
    SE_Keyboard_GER_Gal_1x_fix, SE_Keyboard_GER_Gal_1y_fix, SE_Keyboard_GER_Gal_1z_fix,
};

static const int sLowerSeDeF[] = {
    SE_Keyboard_GER_Gal_2a, SE_Keyboard_GER_Gal_2b, SE_Keyboard_GER_Gal_2c, SE_Keyboard_GER_Gal_2d,
    SE_Keyboard_GER_Gal_2e, SE_Keyboard_GER_Gal_2f, SE_Keyboard_GER_Gal_2g, SE_Keyboard_GER_Gal_2h,
    SE_Keyboard_GER_Gal_2i, SE_Keyboard_GER_Gal_2j,
    SE_Keyboard_GER_Gal_2k, SE_Keyboard_GER_Gal_2l, SE_Keyboard_GER_Gal_2m, SE_Keyboard_GER_Gal_2n,
    SE_Keyboard_GER_Gal_2o, SE_Keyboard_GER_Gal_2p, SE_Keyboard_GER_Gal_2q, SE_Keyboard_GER_Gal_2r,
    SE_Keyboard_GER_Gal_2s, SE_Keyboard_GER_Gal_2t,
    SE_Keyboard_GER_Gal_2u, SE_Keyboard_GER_Gal_2v, SE_Keyboard_GER_Gal_2w, SE_Keyboard_GER_Gal_2x,
    SE_Keyboard_GER_Gal_2y, SE_Keyboard_GER_Gal_2z,
};

static const int sUmlautSeDeF[] = {
    SE_Keyboard_GER_Gal_2_umlaut_a, SE_Keyboard_GER_Gal_2_umlaut_o, SE_Keyboard_GER_Gal_2_umlaut_u,
    SE_Keyboard_GER_Gal_2_umlaut_eszett,
    SE_Keyboard_GER_Gal_1_umlaut_a, SE_Keyboard_GER_Gal_1_umlaut_o, SE_Keyboard_GER_Gal_1_umlaut_u,
    SE_Keyboard_GER_Gal_1_umlaut_a_fix, SE_Keyboard_GER_Gal_1_umlaut_o_fix,
    SE_Keyboard_GER_Gal_1_umlaut_u_fix, SE_Keyboard_GER_Gal_1_umlaut_eszett_fix,
    SE_Keyboard_GER_Gal_1_umlaut_a_fix, SE_Keyboard_GER_Gal_1_umlaut_o_fix,
    SE_Keyboard_GER_Gal_1_umlaut_u_fix,
};

static const int sDigitSeIt[] = {
    SE_Keyboard_ITL_Boy_s00, SE_Keyboard_ITL_Boy_s01, SE_Keyboard_ITL_Boy_s02, SE_Keyboard_ITL_Boy_s03,
    SE_Keyboard_ITL_Boy_s04, SE_Keyboard_ITL_Boy_s05, SE_Keyboard_ITL_Boy_s06, SE_Keyboard_ITL_Boy_s07,
    SE_Keyboard_ITL_Boy_s08, SE_Keyboard_ITL_Boy_s09,
    SE_Keyboard_ITL_Boy_00, SE_Keyboard_ITL_Boy_01, SE_Keyboard_ITL_Boy_02, SE_Keyboard_ITL_Boy_03,
    SE_Keyboard_ITL_Boy_04, SE_Keyboard_ITL_Boy_05, SE_Keyboard_ITL_Boy_06, SE_Keyboard_ITL_Boy_07,
    SE_Keyboard_ITL_Boy_08, SE_Keyboard_ITL_Boy_09,
    SE_Keyboard_ITL_Boy_00_fix, SE_Keyboard_ITL_Boy_01_fix, SE_Keyboard_ITL_Boy_02_fix,
    SE_Keyboard_ITL_Boy_03_fix, SE_Keyboard_ITL_Boy_04_fix, SE_Keyboard_ITL_Boy_05_fix,
    SE_Keyboard_ITL_Boy_06_fix, SE_Keyboard_ITL_Boy_07_fix, SE_Keyboard_ITL_Boy_08_fix,
    SE_Keyboard_ITL_Boy_09_fix,
};

static const int sUpperSeIt[] = {
    SE_Keyboard_ITL_Boy_1a, SE_Keyboard_ITL_Boy_1b, SE_Keyboard_ITL_Boy_1c, SE_Keyboard_ITL_Boy_1d,
    SE_Keyboard_ITL_Boy_1e, SE_Keyboard_ITL_Boy_1f, SE_Keyboard_ITL_Boy_1g, SE_Keyboard_ITL_Boy_1h,
    SE_Keyboard_ITL_Boy_1i, SE_Keyboard_ITL_Boy_1j,
    SE_Keyboard_ITL_Boy_1k, SE_Keyboard_ITL_Boy_1l, SE_Keyboard_ITL_Boy_1m, SE_Keyboard_ITL_Boy_1n,
    SE_Keyboard_ITL_Boy_1o, SE_Keyboard_ITL_Boy_1p, SE_Keyboard_ITL_Boy_1q, SE_Keyboard_ITL_Boy_1r,
    SE_Keyboard_ITL_Boy_1s, SE_Keyboard_ITL_Boy_1t,
    SE_Keyboard_ITL_Boy_1u, SE_Keyboard_ITL_Boy_1v, SE_Keyboard_ITL_Boy_1w, SE_Keyboard_ITL_Boy_1x,
    SE_Keyboard_ITL_Boy_1y, SE_Keyboard_ITL_Boy_1z,
};

static const int sAltSeIt[] = {
    SE_Keyboard_ITL_Boy_1a_fix, SE_Keyboard_ITL_Boy_1b_fix, SE_Keyboard_ITL_Boy_1c_fix,
    SE_Keyboard_ITL_Boy_1d_fix, SE_Keyboard_ITL_Boy_1e_fix, SE_Keyboard_ITL_Boy_1f_fix,
    SE_Keyboard_ITL_Boy_1g_fix, SE_Keyboard_ITL_Boy_1h_fix, SE_Keyboard_ITL_Boy_1i_fix,
    SE_Keyboard_ITL_Boy_1j_fix,
    SE_Keyboard_ITL_Boy_1k_fix, SE_Keyboard_ITL_Boy_1l_fix, SE_Keyboard_ITL_Boy_1m_fix,
    SE_Keyboard_ITL_Boy_1n_fix, SE_Keyboard_ITL_Boy_1o_fix, SE_Keyboard_ITL_Boy_1p_fix,
    SE_Keyboard_ITL_Boy_1q_fix, SE_Keyboard_ITL_Boy_1r_fix, SE_Keyboard_ITL_Boy_1s_fix,
    SE_Keyboard_ITL_Boy_1t_fix,
    SE_Keyboard_ITL_Boy_1u_fix, SE_Keyboard_ITL_Boy_1v_fix, SE_Keyboard_ITL_Boy_1w_fix,
    SE_Keyboard_ITL_Boy_1x_fix, SE_Keyboard_ITL_Boy_1y_fix, SE_Keyboard_ITL_Boy_1z_fix,
};

static const int sLowerSeIt[] = {
    SE_Keyboard_ITL_Boy_2a, SE_Keyboard_ITL_Boy_2b, SE_Keyboard_ITL_Boy_2c, SE_Keyboard_ITL_Boy_2d,
    SE_Keyboard_ITL_Boy_2e, SE_Keyboard_ITL_Boy_2f, SE_Keyboard_ITL_Boy_2g, SE_Keyboard_ITL_Boy_2h,
    SE_Keyboard_ITL_Boy_2i, SE_Keyboard_ITL_Boy_2j,
    SE_Keyboard_ITL_Boy_2k, SE_Keyboard_ITL_Boy_2l, SE_Keyboard_ITL_Boy_2m, SE_Keyboard_ITL_Boy_2n,
    SE_Keyboard_ITL_Boy_2o, SE_Keyboard_ITL_Boy_2p, SE_Keyboard_ITL_Boy_2q, SE_Keyboard_ITL_Boy_2r,
    SE_Keyboard_ITL_Boy_2s, SE_Keyboard_ITL_Boy_2t,
    SE_Keyboard_ITL_Boy_2u, SE_Keyboard_ITL_Boy_2v, SE_Keyboard_ITL_Boy_2w, SE_Keyboard_ITL_Boy_2x,
    SE_Keyboard_ITL_Boy_2y, SE_Keyboard_ITL_Boy_2z,
};

static const int sDigitSeItF[] = {
    SE_Keyboard_ITL_Gal_s00, SE_Keyboard_ITL_Gal_s01, SE_Keyboard_ITL_Gal_s02, SE_Keyboard_ITL_Gal_s03,
    SE_Keyboard_ITL_Gal_s04, SE_Keyboard_ITL_Gal_s05, SE_Keyboard_ITL_Gal_s06, SE_Keyboard_ITL_Gal_s07,
    SE_Keyboard_ITL_Gal_s08, SE_Keyboard_ITL_Gal_s09,
    SE_Keyboard_ITL_Gal_00, SE_Keyboard_ITL_Gal_01, SE_Keyboard_ITL_Gal_02, SE_Keyboard_ITL_Gal_03,
    SE_Keyboard_ITL_Gal_04, SE_Keyboard_ITL_Gal_05, SE_Keyboard_ITL_Gal_06, SE_Keyboard_ITL_Gal_07,
    SE_Keyboard_ITL_Gal_08, SE_Keyboard_ITL_Gal_09,
    SE_Keyboard_ITL_Gal_00_fix, SE_Keyboard_ITL_Gal_01_fix, SE_Keyboard_ITL_Gal_02_fix,
    SE_Keyboard_ITL_Gal_03_fix, SE_Keyboard_ITL_Gal_04_fix, SE_Keyboard_ITL_Gal_05_fix,
    SE_Keyboard_ITL_Gal_06_fix, SE_Keyboard_ITL_Gal_07_fix, SE_Keyboard_ITL_Gal_08_fix,
    SE_Keyboard_ITL_Gal_09_fix,
};

static const int sUpperSeItF[] = {
    SE_Keyboard_ITL_Gal_1a, SE_Keyboard_ITL_Gal_1b, SE_Keyboard_ITL_Gal_1c, SE_Keyboard_ITL_Gal_1d,
    SE_Keyboard_ITL_Gal_1e, SE_Keyboard_ITL_Gal_1f, SE_Keyboard_ITL_Gal_1g, SE_Keyboard_ITL_Gal_1h,
    SE_Keyboard_ITL_Gal_1i, SE_Keyboard_ITL_Gal_1j,
    SE_Keyboard_ITL_Gal_1k, SE_Keyboard_ITL_Gal_1l, SE_Keyboard_ITL_Gal_1m, SE_Keyboard_ITL_Gal_1n,
    SE_Keyboard_ITL_Gal_1o, SE_Keyboard_ITL_Gal_1p, SE_Keyboard_ITL_Gal_1q, SE_Keyboard_ITL_Gal_1r,
    SE_Keyboard_ITL_Gal_1s, SE_Keyboard_ITL_Gal_1t,
    SE_Keyboard_ITL_Gal_1u, SE_Keyboard_ITL_Gal_1v, SE_Keyboard_ITL_Gal_1w, SE_Keyboard_ITL_Gal_1x,
    SE_Keyboard_ITL_Gal_1y, SE_Keyboard_ITL_Gal_1z,
};

static const int sAltSeItF[] = {
    SE_Keyboard_ITL_Gal_1a_fix, SE_Keyboard_ITL_Gal_1b_fix, SE_Keyboard_ITL_Gal_1c_fix,
    SE_Keyboard_ITL_Gal_1d_fix, SE_Keyboard_ITL_Gal_1e_fix, SE_Keyboard_ITL_Gal_1f_fix,
    SE_Keyboard_ITL_Gal_1g_fix, SE_Keyboard_ITL_Gal_1h_fix, SE_Keyboard_ITL_Gal_1i_fix,
    SE_Keyboard_ITL_Gal_1j_fix,
    SE_Keyboard_ITL_Gal_1k_fix, SE_Keyboard_ITL_Gal_1l_fix, SE_Keyboard_ITL_Gal_1m_fix,
    SE_Keyboard_ITL_Gal_1n_fix, SE_Keyboard_ITL_Gal_1o_fix, SE_Keyboard_ITL_Gal_1p_fix,
    SE_Keyboard_ITL_Gal_1q_fix, SE_Keyboard_ITL_Gal_1r_fix, SE_Keyboard_ITL_Gal_1s_fix,
    SE_Keyboard_ITL_Gal_1t_fix,
    SE_Keyboard_ITL_Gal_1u_fix, SE_Keyboard_ITL_Gal_1v_fix, SE_Keyboard_ITL_Gal_1w_fix,
    SE_Keyboard_ITL_Gal_1x_fix, SE_Keyboard_ITL_Gal_1y_fix, SE_Keyboard_ITL_Gal_1z_fix,
};

static const int sLowerSeItF[] = {
    SE_Keyboard_ITL_Gal_2a, SE_Keyboard_ITL_Gal_2b, SE_Keyboard_ITL_Gal_2c, SE_Keyboard_ITL_Gal_2d,
    SE_Keyboard_ITL_Gal_2e, SE_Keyboard_ITL_Gal_2f, SE_Keyboard_ITL_Gal_2g, SE_Keyboard_ITL_Gal_2h,
    SE_Keyboard_ITL_Gal_2i, SE_Keyboard_ITL_Gal_2j,
    SE_Keyboard_ITL_Gal_2k, SE_Keyboard_ITL_Gal_2l, SE_Keyboard_ITL_Gal_2m, SE_Keyboard_ITL_Gal_2n,
    SE_Keyboard_ITL_Gal_2o, SE_Keyboard_ITL_Gal_2p, SE_Keyboard_ITL_Gal_2q, SE_Keyboard_ITL_Gal_2r,
    SE_Keyboard_ITL_Gal_2s, SE_Keyboard_ITL_Gal_2t,
    SE_Keyboard_ITL_Gal_2u, SE_Keyboard_ITL_Gal_2v, SE_Keyboard_ITL_Gal_2w, SE_Keyboard_ITL_Gal_2x,
    SE_Keyboard_ITL_Gal_2y, SE_Keyboard_ITL_Gal_2z,
};

static const wchar_t sUmlautChars[] = {0x00E4, 0x00F6, 0x00FC, 0x00DF, 0x00C4, 0x00D6, 0x00DC};

static const int *sDigitSe[] = {sDigitSeEn, sDigitSeFr, sDigitSeEs, sDigitSeIt, sDigitSeDe, sDigitSeEnF, sDigitSeFrF, sDigitSeEsF, sDigitSeItF, sDigitSeDeF};
static const int *sUpperSe[] = {sUpperSeEn, sUpperSeFr, sUpperSeEs, sUpperSeIt, sUpperSeDe, sUpperSeEnF, sUpperSeFrF, sUpperSeEsF, sUpperSeItF, sUpperSeDeF};
static const int *sAltSe[] = {sAltSeEn, sAltSeFr, sAltSeEs, sAltSeIt, sAltSeDe, sAltSeEnF, sAltSeFrF, sAltSeEsF, sAltSeItF, sAltSeDeF};
static const int *sLowerSe[] = {sLowerSeEn, sLowerSeFr, sLowerSeEs, sLowerSeIt, sLowerSeDe, sLowerSeEnF, sLowerSeFrF, sLowerSeEsF, sLowerSeItF, sLowerSeDeF};
#elif VERSION == VERSION_RUUJ01_01 || VERSION == VERSION_RUUJ01_02
static const int sKanaSeJp[] = {
    SE_Keyboard_JPN_Boy_30a, SE_Keyboard_JPN_Boy_0a, SE_Keyboard_JPN_Boy_30i, SE_Keyboard_JPN_Boy_0i,
    SE_Keyboard_JPN_Boy_30u, SE_Keyboard_JPN_Boy_0u, SE_Keyboard_JPN_Boy_30e, SE_Keyboard_JPN_Boy_0e,
    SE_Keyboard_JPN_Boy_30o, SE_Keyboard_JPN_Boy_0o, SE_Keyboard_JPN_Boy_ka, SE_Keyboard_JPN_Boy_ga,
    SE_Keyboard_JPN_Boy_ki, SE_Keyboard_JPN_Boy_gi, SE_Keyboard_JPN_Boy_ku, SE_Keyboard_JPN_Boy_gu,
    SE_Keyboard_JPN_Boy_ke, SE_Keyboard_JPN_Boy_ge, SE_Keyboard_JPN_Boy_ko, SE_Keyboard_JPN_Boy_go,
    SE_Keyboard_JPN_Boy_sa, SE_Keyboard_JPN_Boy_za, SE_Keyboard_JPN_Boy_si, SE_Keyboard_JPN_Boy_zi,
    SE_Keyboard_JPN_Boy_su, SE_Keyboard_JPN_Boy_zu, SE_Keyboard_JPN_Boy_se, SE_Keyboard_JPN_Boy_ze,
    SE_Keyboard_JPN_Boy_so, SE_Keyboard_JPN_Boy_zo, SE_Keyboard_JPN_Boy_ta, SE_Keyboard_JPN_Boy_da,
    SE_Keyboard_JPN_Boy_ti, SE_Keyboard_JPN_Boy_zi, SE_Keyboard_JPN_Boy_3tu, SE_Keyboard_JPN_Boy_tu,
    SE_Keyboard_JPN_Boy_zu, SE_Keyboard_JPN_Boy_te, SE_Keyboard_JPN_Boy_de, SE_Keyboard_JPN_Boy_to,
    SE_Keyboard_JPN_Boy_do, SE_Keyboard_JPN_Boy_na, SE_Keyboard_JPN_Boy_ni, SE_Keyboard_JPN_Boy_nu,
    SE_Keyboard_JPN_Boy_ne, SE_Keyboard_JPN_Boy_no, SE_Keyboard_JPN_Boy_ha, SE_Keyboard_JPN_Boy_ba,
    SE_Keyboard_JPN_Boy_pa, SE_Keyboard_JPN_Boy_hi, SE_Keyboard_JPN_Boy_bi, SE_Keyboard_JPN_Boy_pi,
    SE_Keyboard_JPN_Boy_fu, SE_Keyboard_JPN_Boy_bu, SE_Keyboard_JPN_Boy_pu, SE_Keyboard_JPN_Boy_he,
    SE_Keyboard_JPN_Boy_be, SE_Keyboard_JPN_Boy_pe, SE_Keyboard_JPN_Boy_ho, SE_Keyboard_JPN_Boy_bo,
    SE_Keyboard_JPN_Boy_po, SE_Keyboard_JPN_Boy_ma, SE_Keyboard_JPN_Boy_mi, SE_Keyboard_JPN_Boy_mu,
    SE_Keyboard_JPN_Boy_me, SE_Keyboard_JPN_Boy_mo, SE_Keyboard_JPN_Boy_3ya, SE_Keyboard_JPN_Boy_ya,
    SE_Keyboard_JPN_Boy_3yu, SE_Keyboard_JPN_Boy_yu, SE_Keyboard_JPN_Boy_3yo, SE_Keyboard_JPN_Boy_yo,
    SE_Keyboard_JPN_Boy_ra, SE_Keyboard_JPN_Boy_ri, SE_Keyboard_JPN_Boy_ru, SE_Keyboard_JPN_Boy_re,
    SE_Keyboard_JPN_Boy_ro, SE_Keyboard_JPN_Boy_3wa, SE_Keyboard_JPN_Boy_wa, SE_pupupu, SE_pupupu,
    SE_Keyboard_JPN_Boy_0o, SE_Keyboard_JPN_Boy_0n, SE_pupupu, SE_Keyboard_JPN_Boy_3ka,
    SE_Keyboard_JPN_Boy_3ke,
    SE_Keyboard_JPN_Boy_30a_fix, SE_Keyboard_JPN_Boy_0a_fix, SE_Keyboard_JPN_Boy_30i_fix,
    SE_Keyboard_JPN_Boy_0i_fix, SE_Keyboard_JPN_Boy_30u_fix, SE_Keyboard_JPN_Boy_0u_fix,
    SE_Keyboard_JPN_Boy_30e_fix, SE_Keyboard_JPN_Boy_0e_fix, SE_Keyboard_JPN_Boy_30o_fix,
    SE_Keyboard_JPN_Boy_0o_fix, SE_Keyboard_JPN_Boy_ka_fix, SE_Keyboard_JPN_Boy_ga_fix,
    SE_Keyboard_JPN_Boy_ki_fix, SE_Keyboard_JPN_Boy_gi_fix, SE_Keyboard_JPN_Boy_ku_fix,
    SE_Keyboard_JPN_Boy_gu_fix, SE_Keyboard_JPN_Boy_ke_fix, SE_Keyboard_JPN_Boy_ge_fix,
    SE_Keyboard_JPN_Boy_ko_fix, SE_Keyboard_JPN_Boy_go_fix, SE_Keyboard_JPN_Boy_sa_fix,
    SE_Keyboard_JPN_Boy_za_fix, SE_Keyboard_JPN_Boy_si_fix, SE_Keyboard_JPN_Boy_zi_fix,
    SE_Keyboard_JPN_Boy_su_fix, SE_Keyboard_JPN_Boy_zu_fix, SE_Keyboard_JPN_Boy_se_fix,
    SE_Keyboard_JPN_Boy_ze_fix, SE_Keyboard_JPN_Boy_so_fix, SE_Keyboard_JPN_Boy_zo_fix,
    SE_Keyboard_JPN_Boy_ta_fix, SE_Keyboard_JPN_Boy_da_fix, SE_Keyboard_JPN_Boy_ti_fix,
    SE_Keyboard_JPN_Boy_zi_fix, SE_Keyboard_JPN_Boy_3tu_fix, SE_Keyboard_JPN_Boy_tu_fix,
    SE_Keyboard_JPN_Boy_zu_fix, SE_Keyboard_JPN_Boy_te_fix, SE_Keyboard_JPN_Boy_de_fix,
    SE_Keyboard_JPN_Boy_to_fix, SE_Keyboard_JPN_Boy_do_fix, SE_Keyboard_JPN_Boy_na_fix,
    SE_Keyboard_JPN_Boy_ni_fix, SE_Keyboard_JPN_Boy_nu_fix, SE_Keyboard_JPN_Boy_ne_fix,
    SE_Keyboard_JPN_Boy_no_fix, SE_Keyboard_JPN_Boy_ha_fix, SE_Keyboard_JPN_Boy_ba_fix,
    SE_Keyboard_JPN_Boy_pa_fix, SE_Keyboard_JPN_Boy_hi_fix, SE_Keyboard_JPN_Boy_bi_fix,
    SE_Keyboard_JPN_Boy_pi_fix, SE_Keyboard_JPN_Boy_fu_fix, SE_Keyboard_JPN_Boy_bu_fix,
    SE_Keyboard_JPN_Boy_pu_fix, SE_Keyboard_JPN_Boy_he_fix, SE_Keyboard_JPN_Boy_be_fix,
    SE_Keyboard_JPN_Boy_pe_fix, SE_Keyboard_JPN_Boy_ho_fix, SE_Keyboard_JPN_Boy_bo_fix,
    SE_Keyboard_JPN_Boy_po_fix, SE_Keyboard_JPN_Boy_ma_fix, SE_Keyboard_JPN_Boy_mi_fix,
    SE_Keyboard_JPN_Boy_mu_fix, SE_Keyboard_JPN_Boy_me_fix, SE_Keyboard_JPN_Boy_mo_fix,
    SE_Keyboard_JPN_Boy_3ya_fix, SE_Keyboard_JPN_Boy_ya_fix, SE_Keyboard_JPN_Boy_3yu_fix,
    SE_Keyboard_JPN_Boy_yu_fix, SE_Keyboard_JPN_Boy_3yo_fix, SE_Keyboard_JPN_Boy_yo_fix,
    SE_Keyboard_JPN_Boy_ra_fix, SE_Keyboard_JPN_Boy_ri_fix, SE_Keyboard_JPN_Boy_ru_fix,
    SE_Keyboard_JPN_Boy_re_fix, SE_Keyboard_JPN_Boy_ro_fix, SE_Keyboard_JPN_Boy_3wa_fix,
    SE_Keyboard_JPN_Boy_wa_fix, SE_pupupu, SE_pupupu, SE_Keyboard_JPN_Boy_0o_fix, SE_Keyboard_JPN_Boy_0n_fix,
    SE_pupupu, SE_Keyboard_JPN_Boy_3ka_fix, SE_Keyboard_JPN_Boy_3ke_fix,
};

static const int sDigitSeJp[] = {
    SE_Keyboard_JPN_Boy_s00, SE_Keyboard_JPN_Boy_s01, SE_Keyboard_JPN_Boy_s02, SE_Keyboard_JPN_Boy_s03,
    SE_Keyboard_JPN_Boy_s04, SE_Keyboard_JPN_Boy_s05, SE_Keyboard_JPN_Boy_s06, SE_Keyboard_JPN_Boy_s07,
    SE_Keyboard_JPN_Boy_s08, SE_Keyboard_JPN_Boy_s09,
    SE_Keyboard_JPN_Boy_00, SE_Keyboard_JPN_Boy_01, SE_Keyboard_JPN_Boy_02, SE_Keyboard_JPN_Boy_03,
    SE_Keyboard_JPN_Boy_04, SE_Keyboard_JPN_Boy_05, SE_Keyboard_JPN_Boy_06, SE_Keyboard_JPN_Boy_07,
    SE_Keyboard_JPN_Boy_08, SE_Keyboard_JPN_Boy_09,
    SE_Keyboard_JPN_Boy_00_fix, SE_Keyboard_JPN_Boy_01_fix, SE_Keyboard_JPN_Boy_02_fix,
    SE_Keyboard_JPN_Boy_03_fix, SE_Keyboard_JPN_Boy_04_fix, SE_Keyboard_JPN_Boy_05_fix,
    SE_Keyboard_JPN_Boy_06_fix, SE_Keyboard_JPN_Boy_07_fix, SE_Keyboard_JPN_Boy_08_fix,
    SE_Keyboard_JPN_Boy_09_fix,
};

static const int sUpperSeJp[] = {
    SE_Keyboard_JPN_Boy_1a, SE_Keyboard_JPN_Boy_1b, SE_Keyboard_JPN_Boy_1c, SE_Keyboard_JPN_Boy_1d,
    SE_Keyboard_JPN_Boy_1e, SE_Keyboard_JPN_Boy_1f, SE_Keyboard_JPN_Boy_1g, SE_Keyboard_JPN_Boy_1h,
    SE_Keyboard_JPN_Boy_1i, SE_Keyboard_JPN_Boy_1j,
    SE_Keyboard_JPN_Boy_1k, SE_Keyboard_JPN_Boy_1l, SE_Keyboard_JPN_Boy_1m, SE_Keyboard_JPN_Boy_1n,
    SE_Keyboard_JPN_Boy_1o, SE_Keyboard_JPN_Boy_1p, SE_Keyboard_JPN_Boy_1q, SE_Keyboard_JPN_Boy_1r,
    SE_Keyboard_JPN_Boy_1s, SE_Keyboard_JPN_Boy_1t,
    SE_Keyboard_JPN_Boy_1u, SE_Keyboard_JPN_Boy_1v, SE_Keyboard_JPN_Boy_1w, SE_Keyboard_JPN_Boy_1x,
    SE_Keyboard_JPN_Boy_1y, SE_Keyboard_JPN_Boy_1z,
};

static const int sAltSeJp[] = {
    SE_Keyboard_JPN_Boy_1a_fix, SE_Keyboard_JPN_Boy_1b_fix, SE_Keyboard_JPN_Boy_1c_fix,
    SE_Keyboard_JPN_Boy_1d_fix, SE_Keyboard_JPN_Boy_1e_fix, SE_Keyboard_JPN_Boy_1f_fix,
    SE_Keyboard_JPN_Boy_1g_fix, SE_Keyboard_JPN_Boy_1h_fix, SE_Keyboard_JPN_Boy_1i_fix,
    SE_Keyboard_JPN_Boy_1j_fix,
    SE_Keyboard_JPN_Boy_1k_fix, SE_Keyboard_JPN_Boy_1l_fix, SE_Keyboard_JPN_Boy_1m_fix,
    SE_Keyboard_JPN_Boy_1n_fix, SE_Keyboard_JPN_Boy_1o_fix, SE_Keyboard_JPN_Boy_1p_fix,
    SE_Keyboard_JPN_Boy_1q_fix, SE_Keyboard_JPN_Boy_1r_fix, SE_Keyboard_JPN_Boy_1s_fix,
    SE_Keyboard_JPN_Boy_1t_fix,
    SE_Keyboard_JPN_Boy_1u_fix, SE_Keyboard_JPN_Boy_1v_fix, SE_Keyboard_JPN_Boy_1w_fix,
    SE_Keyboard_JPN_Boy_1x_fix, SE_Keyboard_JPN_Boy_1y_fix, SE_Keyboard_JPN_Boy_1z_fix,
};

static const int sUnfixSeJp[] = {
    SE_Keyboard_JPN_Boy_a_unfix, SE_Keyboard_JPN_Boy_b_unfix, SE_Keyboard_JPN_Boy_c_unfix,
    SE_Keyboard_JPN_Boy_d_unfix, SE_Keyboard_JPN_Boy_e_unfix, SE_Keyboard_JPN_Boy_f_unfix,
    SE_Keyboard_JPN_Boy_g_unfix, SE_Keyboard_JPN_Boy_h_unfix, SE_Keyboard_JPN_Boy_i_unfix,
    SE_Keyboard_JPN_Boy_j_unfix,
    SE_Keyboard_JPN_Boy_k_unfix, SE_Keyboard_JPN_Boy_l_unfix, SE_Keyboard_JPN_Boy_m_unfix,
    SE_Keyboard_JPN_Boy_n_unfix, SE_Keyboard_JPN_Boy_o_unfix, SE_Keyboard_JPN_Boy_p_unfix,
    SE_Keyboard_JPN_Boy_q_unfix, SE_Keyboard_JPN_Boy_r_unfix, SE_Keyboard_JPN_Boy_s_unfix,
    SE_Keyboard_JPN_Boy_t_unfix,
    SE_Keyboard_JPN_Boy_u_unfix, SE_Keyboard_JPN_Boy_v_unfix, SE_Keyboard_JPN_Boy_w_unfix,
    SE_Keyboard_JPN_Boy_x_unfix, SE_Keyboard_JPN_Boy_y_unfix, SE_Keyboard_JPN_Boy_z_unfix,
};

static const int sLowerSeJp[] = {
    SE_Keyboard_JPN_BOY_2a, SE_Keyboard_JPN_BOY_2b, SE_Keyboard_JPN_BOY_2c, SE_Keyboard_JPN_BOY_2d,
    SE_Keyboard_JPN_BOY_2e, SE_Keyboard_JPN_BOY_2f, SE_Keyboard_JPN_BOY_2g, SE_Keyboard_JPN_BOY_2h,
    SE_Keyboard_JPN_BOY_2i, SE_Keyboard_JPN_BOY_2j,
    SE_Keyboard_JPN_BOY_2k, SE_Keyboard_JPN_BOY_2l, SE_Keyboard_JPN_BOY_2m, SE_Keyboard_JPN_BOY_2n,
    SE_Keyboard_JPN_BOY_2o, SE_Keyboard_JPN_BOY_2p, SE_Keyboard_JPN_BOY_2q, SE_Keyboard_JPN_BOY_2r,
    SE_Keyboard_JPN_BOY_2s, SE_Keyboard_JPN_BOY_2t,
    SE_Keyboard_JPN_BOY_2u, SE_Keyboard_JPN_BOY_2v, SE_Keyboard_JPN_BOY_2w, SE_Keyboard_JPN_BOY_2x,
    SE_Keyboard_JPN_BOY_2y, SE_Keyboard_JPN_BOY_2z,
};

static const int sKanaSeJpF[] = {
    SE_Keyboard_JPN_Gal_30a, SE_Keyboard_JPN_Gal_0a, SE_Keyboard_JPN_Gal_30i, SE_Keyboard_JPN_Gal_0i,
    SE_Keyboard_JPN_Gal_30u, SE_Keyboard_JPN_Gal_0u, SE_Keyboard_JPN_Gal_30e, SE_Keyboard_JPN_Gal_0e,
    SE_Keyboard_JPN_Gal_30o, SE_Keyboard_JPN_Gal_0o, SE_Keyboard_JPN_Gal_ka, SE_Keyboard_JPN_Gal_ga,
    SE_Keyboard_JPN_Gal_ki, SE_Keyboard_JPN_Gal_gi, SE_Keyboard_JPN_Gal_ku, SE_Keyboard_JPN_Gal_gu,
    SE_Keyboard_JPN_Gal_ke, SE_Keyboard_JPN_Gal_ge, SE_Keyboard_JPN_Gal_ko, SE_Keyboard_JPN_Gal_go,
    SE_Keyboard_JPN_Gal_sa, SE_Keyboard_JPN_Gal_za, SE_Keyboard_JPN_Gal_si, SE_Keyboard_JPN_Gal_zi,
    SE_Keyboard_JPN_Gal_su, SE_Keyboard_JPN_Gal_zu, SE_Keyboard_JPN_Gal_se, SE_Keyboard_JPN_Gal_ze,
    SE_Keyboard_JPN_Gal_so, SE_Keyboard_JPN_Gal_zo, SE_Keyboard_JPN_Gal_ta, SE_Keyboard_JPN_Gal_da,
    SE_Keyboard_JPN_Gal_ti, SE_Keyboard_JPN_Gal_zi, SE_Keyboard_JPN_Gal_3tu, SE_Keyboard_JPN_Gal_tu,
    SE_Keyboard_JPN_Gal_zu, SE_Keyboard_JPN_Gal_te, SE_Keyboard_JPN_Gal_de, SE_Keyboard_JPN_Gal_to,
    SE_Keyboard_JPN_Gal_do, SE_Keyboard_JPN_Gal_na, SE_Keyboard_JPN_Gal_ni, SE_Keyboard_JPN_Gal_nu,
    SE_Keyboard_JPN_Gal_ne, SE_Keyboard_JPN_Gal_no, SE_Keyboard_JPN_Gal_ha, SE_Keyboard_JPN_Gal_ba,
    SE_Keyboard_JPN_Gal_pa, SE_Keyboard_JPN_Gal_hi, SE_Keyboard_JPN_Gal_bi, SE_Keyboard_JPN_Gal_pi,
    SE_Keyboard_JPN_Gal_fu, SE_Keyboard_JPN_Gal_bu, SE_Keyboard_JPN_Gal_pu, SE_Keyboard_JPN_Gal_he,
    SE_Keyboard_JPN_Gal_be, SE_Keyboard_JPN_Gal_pe, SE_Keyboard_JPN_Gal_ho, SE_Keyboard_JPN_Gal_bo,
    SE_Keyboard_JPN_Gal_po, SE_Keyboard_JPN_Gal_ma, SE_Keyboard_JPN_Gal_mi, SE_Keyboard_JPN_Gal_mu,
    SE_Keyboard_JPN_Gal_me, SE_Keyboard_JPN_Gal_mo, SE_Keyboard_JPN_Gal_3ya, SE_Keyboard_JPN_Gal_ya,
    SE_Keyboard_JPN_Gal_3yu, SE_Keyboard_JPN_Gal_yu, SE_Keyboard_JPN_Gal_3yo, SE_Keyboard_JPN_Gal_yo,
    SE_Keyboard_JPN_Gal_ra, SE_Keyboard_JPN_Gal_ri, SE_Keyboard_JPN_Gal_ru, SE_Keyboard_JPN_Gal_re,
    SE_Keyboard_JPN_Gal_ro, SE_Keyboard_JPN_Gal_3wa, SE_Keyboard_JPN_Gal_wa, SE_pupupu, SE_pupupu,
    SE_Keyboard_JPN_Gal_0o, SE_Keyboard_JPN_Gal_0n, SE_pupupu, SE_Keyboard_JPN_Gal_3ka,
    SE_Keyboard_JPN_Gal_3ke,
    SE_Keyboard_JPN_Gal_30a_fix, SE_Keyboard_JPN_Gal_0a_fix, SE_Keyboard_JPN_Gal_30i_fix,
    SE_Keyboard_JPN_Gal_0i_fix, SE_Keyboard_JPN_Gal_30u_fix, SE_Keyboard_JPN_Gal_0u_fix,
    SE_Keyboard_JPN_Gal_30e_fix, SE_Keyboard_JPN_Gal_0e_fix, SE_Keyboard_JPN_Gal_30o_fix,
    SE_Keyboard_JPN_Gal_0o_fix, SE_Keyboard_JPN_Gal_ka_fix, SE_Keyboard_JPN_Gal_ga_fix,
    SE_Keyboard_JPN_Gal_ki_fix, SE_Keyboard_JPN_Gal_gi_fix, SE_Keyboard_JPN_Gal_ku_fix,
    SE_Keyboard_JPN_Gal_gu_fix, SE_Keyboard_JPN_Gal_ke_fix, SE_Keyboard_JPN_Gal_ge_fix,
    SE_Keyboard_JPN_Gal_ko_fix, SE_Keyboard_JPN_Gal_go_fix, SE_Keyboard_JPN_Gal_sa_fix,
    SE_Keyboard_JPN_Gal_za_fix, SE_Keyboard_JPN_Gal_si_fix, SE_Keyboard_JPN_Gal_zi_fix,
    SE_Keyboard_JPN_Gal_su_fix, SE_Keyboard_JPN_Gal_zu_fix, SE_Keyboard_JPN_Gal_se_fix,
    SE_Keyboard_JPN_Gal_ze_fix, SE_Keyboard_JPN_Gal_so_fix, SE_Keyboard_JPN_Gal_zo_fix,
    SE_Keyboard_JPN_Gal_ta_fix, SE_Keyboard_JPN_Gal_da_fix, SE_Keyboard_JPN_Gal_ti_fix,
    SE_Keyboard_JPN_Gal_zi_fix, SE_Keyboard_JPN_Gal_3tu_fix, SE_Keyboard_JPN_Gal_tu_fix,
    SE_Keyboard_JPN_Gal_zu_fix, SE_Keyboard_JPN_Gal_te_fix, SE_Keyboard_JPN_Gal_de_fix,
    SE_Keyboard_JPN_Gal_to_fix, SE_Keyboard_JPN_Gal_do_fix, SE_Keyboard_JPN_Gal_na_fix,
    SE_Keyboard_JPN_Gal_ni_fix, SE_Keyboard_JPN_Gal_nu_fix, SE_Keyboard_JPN_Gal_ne_fix,
    SE_Keyboard_JPN_Gal_no_fix, SE_Keyboard_JPN_Gal_ha_fix, SE_Keyboard_JPN_Gal_ba_fix,
    SE_Keyboard_JPN_Gal_pa_fix, SE_Keyboard_JPN_Gal_hi_fix, SE_Keyboard_JPN_Gal_bi_fix,
    SE_Keyboard_JPN_Gal_pi_fix, SE_Keyboard_JPN_Gal_fu_fix, SE_Keyboard_JPN_Gal_bu_fix,
    SE_Keyboard_JPN_Gal_pu_fix, SE_Keyboard_JPN_Gal_he_fix, SE_Keyboard_JPN_Gal_be_fix,
    SE_Keyboard_JPN_Gal_pe_fix, SE_Keyboard_JPN_Gal_ho_fix, SE_Keyboard_JPN_Gal_bo_fix,
    SE_Keyboard_JPN_Gal_po_fix, SE_Keyboard_JPN_Gal_ma_fix, SE_Keyboard_JPN_Gal_mi_fix,
    SE_Keyboard_JPN_Gal_mu_fix, SE_Keyboard_JPN_Gal_me_fix, SE_Keyboard_JPN_Gal_mo_fix,
    SE_Keyboard_JPN_Gal_3ya_fix, SE_Keyboard_JPN_Gal_ya_fix, SE_Keyboard_JPN_Gal_3yu_fix,
    SE_Keyboard_JPN_Gal_yu_fix, SE_Keyboard_JPN_Gal_3yo_fix, SE_Keyboard_JPN_Gal_yo_fix,
    SE_Keyboard_JPN_Gal_ra_fix, SE_Keyboard_JPN_Gal_ri_fix, SE_Keyboard_JPN_Gal_ru_fix,
    SE_Keyboard_JPN_Gal_re_fix, SE_Keyboard_JPN_Gal_ro_fix, SE_Keyboard_JPN_Gal_3wa_fix,
    SE_Keyboard_JPN_Gal_wa_fix, SE_pupupu, SE_pupupu, SE_Keyboard_JPN_Gal_0o_fix, SE_Keyboard_JPN_Gal_0n_fix,
    SE_pupupu, SE_Keyboard_JPN_Gal_3ka_fix, SE_Keyboard_JPN_Gal_3ke_fix,
};

static const int sDigitSeJpF[] = {
    SE_Keyboard_JPN_Gal_s00, SE_Keyboard_JPN_Gal_s01, SE_Keyboard_JPN_Gal_s02, SE_Keyboard_JPN_Gal_s03,
    SE_Keyboard_JPN_Gal_s04, SE_Keyboard_JPN_Gal_s05, SE_Keyboard_JPN_Gal_s06, SE_Keyboard_JPN_Gal_s07,
    SE_Keyboard_JPN_Gal_s08, SE_Keyboard_JPN_Gal_s09,
    SE_Keyboard_JPN_Gal_00, SE_Keyboard_JPN_Gal_01, SE_Keyboard_JPN_Gal_02, SE_Keyboard_JPN_Gal_03,
    SE_Keyboard_JPN_Gal_04, SE_Keyboard_JPN_Gal_05, SE_Keyboard_JPN_Gal_06, SE_Keyboard_JPN_Gal_07,
    SE_Keyboard_JPN_Gal_08, SE_Keyboard_JPN_Gal_09,
    SE_Keyboard_JPN_Gal_00_fix, SE_Keyboard_JPN_Gal_01_fix, SE_Keyboard_JPN_Gal_02_fix,
    SE_Keyboard_JPN_Gal_03_fix, SE_Keyboard_JPN_Gal_04_fix, SE_Keyboard_JPN_Gal_05_fix,
    SE_Keyboard_JPN_Gal_06_fix, SE_Keyboard_JPN_Gal_07_fix, SE_Keyboard_JPN_Gal_08_fix,
    SE_Keyboard_JPN_Gal_09_fix,
};

static const int sUpperSeJpF[] = {
    SE_Keyboard_JPN_Gal_1a, SE_Keyboard_JPN_Gal_1b, SE_Keyboard_JPN_Gal_1c, SE_Keyboard_JPN_Gal_1d,
    SE_Keyboard_JPN_Gal_1e, SE_Keyboard_JPN_Gal_1f, SE_Keyboard_JPN_Gal_1g, SE_Keyboard_JPN_Gal_1h,
    SE_Keyboard_JPN_Gal_1i, SE_Keyboard_JPN_Gal_1j,
    SE_Keyboard_JPN_Gal_1k, SE_Keyboard_JPN_Gal_1l, SE_Keyboard_JPN_Gal_1m, SE_Keyboard_JPN_Gal_1n,
    SE_Keyboard_JPN_Gal_1o, SE_Keyboard_JPN_Gal_1p, SE_Keyboard_JPN_Gal_1q, SE_Keyboard_JPN_Gal_1r,
    SE_Keyboard_JPN_Gal_1s, SE_Keyboard_JPN_Gal_1t,
    SE_Keyboard_JPN_Gal_1u, SE_Keyboard_JPN_Gal_1v, SE_Keyboard_JPN_Gal_1w, SE_Keyboard_JPN_Gal_1x,
    SE_Keyboard_JPN_Gal_1y, SE_Keyboard_JPN_Gal_1z,
};

static const int sAltSeJpF[] = {
    SE_Keyboard_JPN_Gal_1a_fix, SE_Keyboard_JPN_Gal_1b_fix, SE_Keyboard_JPN_Gal_1c_fix,
    SE_Keyboard_JPN_Gal_1d_fix, SE_Keyboard_JPN_Gal_1e_fix, SE_Keyboard_JPN_Gal_1f_fix,
    SE_Keyboard_JPN_Gal_1g_fix, SE_Keyboard_JPN_Gal_1h_fix, SE_Keyboard_JPN_Gal_1i_fix,
    SE_Keyboard_JPN_Gal_1j_fix,
    SE_Keyboard_JPN_Gal_1k_fix, SE_Keyboard_JPN_Gal_1l_fix, SE_Keyboard_JPN_Gal_1m_fix,
    SE_Keyboard_JPN_Gal_1n_fix, SE_Keyboard_JPN_Gal_1o_fix, SE_Keyboard_JPN_Gal_1p_fix,
    SE_Keyboard_JPN_Gal_1q_fix, SE_Keyboard_JPN_Gal_1r_fix, SE_Keyboard_JPN_Gal_1s_fix,
    SE_Keyboard_JPN_Gal_1t_fix,
    SE_Keyboard_JPN_Gal_1u_fix, SE_Keyboard_JPN_Gal_1v_fix, SE_Keyboard_JPN_Gal_1w_fix,
    SE_Keyboard_JPN_Gal_1x_fix, SE_Keyboard_JPN_Gal_1y_fix, SE_Keyboard_JPN_Gal_1z_fix,
};

static const int sUnfixSeJpF[] = {
    SE_Keyboard_JPN_Gal_a_unfix, SE_Keyboard_JPN_Gal_b_unfix, SE_Keyboard_JPN_Gal_c_unfix,
    SE_Keyboard_JPN_Gal_d_unfix, SE_Keyboard_JPN_Gal_e_unfix, SE_Keyboard_JPN_Gal_f_unfix,
    SE_Keyboard_JPN_Gal_g_unfix, SE_Keyboard_JPN_Gal_h_unfix, SE_Keyboard_JPN_Gal_i_unfix,
    SE_Keyboard_JPN_Gal_j_unfix,
    SE_Keyboard_JPN_Gal_k_unfix, SE_Keyboard_JPN_Gal_l_unfix, SE_Keyboard_JPN_Gal_m_unfix,
    SE_Keyboard_JPN_Gal_n_unfix, SE_Keyboard_JPN_Gal_o_unfix, SE_Keyboard_JPN_Gal_p_unfix,
    SE_Keyboard_JPN_Gal_q_unfix, SE_Keyboard_JPN_Gal_r_unfix, SE_Keyboard_JPN_Gal_s_unfix,
    SE_Keyboard_JPN_Gal_t_unfix,
    SE_Keyboard_JPN_Gal_u_unfix, SE_Keyboard_JPN_Gal_v_unfix, SE_Keyboard_JPN_Gal_w_unfix,
    SE_Keyboard_JPN_Gal_x_unfix, SE_Keyboard_JPN_Gal_y_unfix, SE_Keyboard_JPN_Gal_z_unfix,
};

static const int sLowerSeJpF[] = {
    SE_Keyboard_JPN_Gal_2a, SE_Keyboard_JPN_Gal_2b, SE_Keyboard_JPN_Gal_2c, SE_Keyboard_JPN_Gal_2d,
    SE_Keyboard_JPN_Gal_2e, SE_Keyboard_JPN_Gal_2f, SE_Keyboard_JPN_Gal_2g, SE_Keyboard_JPN_Gal_2h,
    SE_Keyboard_JPN_Gal_2i, SE_Keyboard_JPN_Gal_2j,
    SE_Keyboard_JPN_Gal_2k, SE_Keyboard_JPN_Gal_2l, SE_Keyboard_JPN_Gal_2m, SE_Keyboard_JPN_Gal_2n,
    SE_Keyboard_JPN_Gal_2o, SE_Keyboard_JPN_Gal_2p, SE_Keyboard_JPN_Gal_2q, SE_Keyboard_JPN_Gal_2r,
    SE_Keyboard_JPN_Gal_2s, SE_Keyboard_JPN_Gal_2t,
    SE_Keyboard_JPN_Gal_2u, SE_Keyboard_JPN_Gal_2v, SE_Keyboard_JPN_Gal_2w, SE_Keyboard_JPN_Gal_2x,
    SE_Keyboard_JPN_Gal_2y, SE_Keyboard_JPN_Gal_2z,
};
#else
// 80472E50
static const int sDigitSeUs[] = {
    SE_Keyboard_ENG_Boy_s00, SE_Keyboard_ENG_Boy_s01, SE_Keyboard_ENG_Boy_s02, SE_Keyboard_ENG_Boy_s03,
    SE_Keyboard_ENG_Boy_s04, SE_Keyboard_ENG_Boy_s05, SE_Keyboard_ENG_Boy_s06, SE_Keyboard_ENG_Boy_s07,
    SE_Keyboard_ENG_Boy_s08, SE_Keyboard_ENG_Boy_s09,
    SE_Keyboard_ENG_Boy_00, SE_Keyboard_ENG_Boy_01, SE_Keyboard_ENG_Boy_02, SE_Keyboard_ENG_Boy_03,
    SE_Keyboard_ENG_Boy_04, SE_Keyboard_ENG_Boy_05, SE_Keyboard_ENG_Boy_06, SE_Keyboard_ENG_Boy_07,
    SE_Keyboard_ENG_Boy_08, SE_Keyboard_ENG_Boy_09,
    SE_Keyboard_ENG_Boy_00_fix, SE_Keyboard_ENG_Boy_01_fix, SE_Keyboard_ENG_Boy_02_fix,
    SE_Keyboard_ENG_Boy_03_fix, SE_Keyboard_ENG_Boy_04_fix, SE_Keyboard_ENG_Boy_05_fix,
    SE_Keyboard_ENG_Boy_06_fix, SE_Keyboard_ENG_Boy_07_fix, SE_Keyboard_ENG_Boy_08_fix,
    SE_Keyboard_ENG_Boy_09_fix,
};

static const int sUpperSeUs[] = {
    SE_Keyboard_ENG_Boy_1a, SE_Keyboard_ENG_Boy_1b, SE_Keyboard_ENG_Boy_1c, SE_Keyboard_ENG_Boy_1d,
    SE_Keyboard_ENG_Boy_1e, SE_Keyboard_ENG_Boy_1f, SE_Keyboard_ENG_Boy_1g, SE_Keyboard_ENG_Boy_1h,
    SE_Keyboard_ENG_Boy_1i, SE_Keyboard_ENG_Boy_1j,
    SE_Keyboard_ENG_Boy_1k, SE_Keyboard_ENG_Boy_1l, SE_Keyboard_ENG_Boy_1m, SE_Keyboard_ENG_Boy_1n,
    SE_Keyboard_ENG_Boy_1o, SE_Keyboard_ENG_Boy_1p, SE_Keyboard_ENG_Boy_1q, SE_Keyboard_ENG_Boy_1r,
    SE_Keyboard_ENG_Boy_1s, SE_Keyboard_ENG_Boy_1t,
    SE_Keyboard_ENG_Boy_1u, SE_Keyboard_ENG_Boy_1v, SE_Keyboard_ENG_Boy_1w, SE_Keyboard_ENG_Boy_1x,
    SE_Keyboard_ENG_Boy_1y, SE_Keyboard_ENG_Boy_1z,
};

static const int sLowerSeUs[] = {
    SE_Keyboard_ENG_Boy_2a, SE_Keyboard_ENG_Boy_2b, SE_Keyboard_ENG_Boy_2c, SE_Keyboard_ENG_Boy_2d,
    SE_Keyboard_ENG_Boy_2e, SE_Keyboard_ENG_Boy_2f, SE_Keyboard_ENG_Boy_2g, SE_Keyboard_ENG_Boy_2h,
    SE_Keyboard_ENG_Boy_2i, SE_Keyboard_ENG_Boy_2j,
    SE_Keyboard_ENG_Boy_2k, SE_Keyboard_ENG_Boy_2l, SE_Keyboard_ENG_Boy_2m, SE_Keyboard_ENG_Boy_2n,
    SE_Keyboard_ENG_Boy_2o, SE_Keyboard_ENG_Boy_2p, SE_Keyboard_ENG_Boy_2q, SE_Keyboard_ENG_Boy_2r,
    SE_Keyboard_ENG_Boy_2s, SE_Keyboard_ENG_Boy_2t,
    SE_Keyboard_ENG_Boy_2u, SE_Keyboard_ENG_Boy_2v, SE_Keyboard_ENG_Boy_2w, SE_Keyboard_ENG_Boy_2x,
    SE_Keyboard_ENG_Boy_2y, SE_Keyboard_ENG_Boy_2z,
};

static const int sAltSeUs[] = {
    SE_Keyboard_ENG_Boy_1a_fix, SE_Keyboard_ENG_Boy_1b_fix, SE_Keyboard_ENG_Boy_1c_fix,
    SE_Keyboard_ENG_Boy_1d_fix, SE_Keyboard_ENG_Boy_1e_fix, SE_Keyboard_ENG_Boy_1f_fix,
    SE_Keyboard_ENG_Boy_1g_fix, SE_Keyboard_ENG_Boy_1h_fix, SE_Keyboard_ENG_Boy_1i_fix,
    SE_Keyboard_ENG_Boy_1j_fix,
    SE_Keyboard_ENG_Boy_1k_fix, SE_Keyboard_ENG_Boy_1l_fix, SE_Keyboard_ENG_Boy_1m_fix,
    SE_Keyboard_ENG_Boy_1n_fix, SE_Keyboard_ENG_Boy_1o_fix, SE_Keyboard_ENG_Boy_1p_fix,
    SE_Keyboard_ENG_Boy_1q_fix, SE_Keyboard_ENG_Boy_1r_fix, SE_Keyboard_ENG_Boy_1s_fix,
    SE_Keyboard_ENG_Boy_1t_fix,
    SE_Keyboard_ENG_Boy_1u_fix, SE_Keyboard_ENG_Boy_1v_fix, SE_Keyboard_ENG_Boy_1w_fix,
    SE_Keyboard_ENG_Boy_1x_fix, SE_Keyboard_ENG_Boy_1y_fix, SE_Keyboard_ENG_Boy_1z_fix,
};

// 80473000
static const int sDigitSeUsF[] = {
    SE_Keyboard_ENG_Gal_s00, SE_Keyboard_ENG_Gal_s01, SE_Keyboard_ENG_Gal_s02, SE_Keyboard_ENG_Gal_s03,
    SE_Keyboard_ENG_Gal_s04, SE_Keyboard_ENG_Gal_s05, SE_Keyboard_ENG_Gal_s06, SE_Keyboard_ENG_Gal_s07,
    SE_Keyboard_ENG_Gal_s08, SE_Keyboard_ENG_Gal_s09,
    SE_Keyboard_ENG_Gal_00, SE_Keyboard_ENG_Gal_01, SE_Keyboard_ENG_Gal_02, SE_Keyboard_ENG_Gal_03,
    SE_Keyboard_ENG_Gal_04, SE_Keyboard_ENG_Gal_05, SE_Keyboard_ENG_Gal_06, SE_Keyboard_ENG_Gal_07,
    SE_Keyboard_ENG_Gal_08, SE_Keyboard_ENG_Gal_09,
    SE_Keyboard_ENG_Gal_00_fix, SE_Keyboard_ENG_Gal_01_fix, SE_Keyboard_ENG_Gal_02_fix,
    SE_Keyboard_ENG_Gal_03_fix, SE_Keyboard_ENG_Gal_04_fix, SE_Keyboard_ENG_Gal_05_fix,
    SE_Keyboard_ENG_Gal_06_fix, SE_Keyboard_ENG_Gal_07_fix, SE_Keyboard_ENG_Gal_08_fix,
    SE_Keyboard_ENG_Gal_09_fix,
};

static const int sUpperSeUsF[] = {
    SE_Keyboard_ENG_Gal_1a, SE_Keyboard_ENG_Gal_1b, SE_Keyboard_ENG_Gal_1c, SE_Keyboard_ENG_Gal_1d,
    SE_Keyboard_ENG_Gal_1e, SE_Keyboard_ENG_Gal_1f, SE_Keyboard_ENG_Gal_1g, SE_Keyboard_ENG_Gal_1h,
    SE_Keyboard_ENG_Gal_1i, SE_Keyboard_ENG_Gal_1j,
    SE_Keyboard_ENG_Gal_1k, SE_Keyboard_ENG_Gal_1l, SE_Keyboard_ENG_Gal_1m, SE_Keyboard_ENG_Gal_1n,
    SE_Keyboard_ENG_Gal_1o, SE_Keyboard_ENG_Gal_1p, SE_Keyboard_ENG_Gal_1q, SE_Keyboard_ENG_Gal_1r,
    SE_Keyboard_ENG_Gal_1s, SE_Keyboard_ENG_Gal_1t,
    SE_Keyboard_ENG_Gal_1u, SE_Keyboard_ENG_Gal_1v, SE_Keyboard_ENG_Gal_1w, SE_Keyboard_ENG_Gal_1x,
    SE_Keyboard_ENG_Gal_1y, SE_Keyboard_ENG_Gal_1z,
};

static const int sAltSeUsF[] = {
    SE_Keyboard_ENG_Gal_1a_fix, SE_Keyboard_ENG_Gal_1b_fix, SE_Keyboard_ENG_Gal_1c_fix,
    SE_Keyboard_ENG_Gal_1d_fix, SE_Keyboard_ENG_Gal_1e_fix, SE_Keyboard_ENG_Gal_1f_fix,
    SE_Keyboard_ENG_Gal_1g_fix, SE_Keyboard_ENG_Gal_1h_fix, SE_Keyboard_ENG_Gal_1i_fix,
    SE_Keyboard_ENG_Gal_1j_fix,
    SE_Keyboard_ENG_Gal_1k_fix, SE_Keyboard_ENG_Gal_1l_fix, SE_Keyboard_ENG_Gal_1m_fix,
    SE_Keyboard_ENG_Gal_1n_fix, SE_Keyboard_ENG_Gal_1o_fix, SE_Keyboard_ENG_Gal_1p_fix,
    SE_Keyboard_ENG_Gal_1q_fix, SE_Keyboard_ENG_Gal_1r_fix, SE_Keyboard_ENG_Gal_1s_fix,
    SE_Keyboard_ENG_Gal_1t_fix,
    SE_Keyboard_ENG_Gal_1u_fix, SE_Keyboard_ENG_Gal_1v_fix, SE_Keyboard_ENG_Gal_1w_fix,
    SE_Keyboard_ENG_Gal_1x_fix, SE_Keyboard_ENG_Gal_1y_fix, SE_Keyboard_ENG_Gal_1z_fix,
};

static const int sLowerSeUsF[] = {
    SE_Keyboard_ENG_Gal_2a, SE_Keyboard_ENG_Gal_2b, SE_Keyboard_ENG_Gal_2c, SE_Keyboard_ENG_Gal_2d,
    SE_Keyboard_ENG_Gal_2e, SE_Keyboard_ENG_Gal_2f, SE_Keyboard_ENG_Gal_2g, SE_Keyboard_ENG_Gal_2h,
    SE_Keyboard_ENG_Gal_2i, SE_Keyboard_ENG_Gal_2j,
    SE_Keyboard_ENG_Gal_2k, SE_Keyboard_ENG_Gal_2l, SE_Keyboard_ENG_Gal_2m, SE_Keyboard_ENG_Gal_2n,
    SE_Keyboard_ENG_Gal_2o, SE_Keyboard_ENG_Gal_2p, SE_Keyboard_ENG_Gal_2q, SE_Keyboard_ENG_Gal_2r,
    SE_Keyboard_ENG_Gal_2s, SE_Keyboard_ENG_Gal_2t,
    SE_Keyboard_ENG_Gal_2u, SE_Keyboard_ENG_Gal_2v, SE_Keyboard_ENG_Gal_2w, SE_Keyboard_ENG_Gal_2x,
    SE_Keyboard_ENG_Gal_2y, SE_Keyboard_ENG_Gal_2z,
};

// 804731B0
static const int sDigitSeQc[] = {
    SE_Keyboard_FRN_Boy_00, SE_Keyboard_FRN_Boy_01, SE_Keyboard_FRN_Boy_02, SE_Keyboard_FRN_Boy_03,
    SE_Keyboard_FRN_Boy_04, SE_Keyboard_FRN_Boy_05, SE_Keyboard_FRN_Boy_06, SE_Keyboard_FRN_Boy_07,
    SE_Keyboard_FRN_Boy_08, SE_Keyboard_FRN_Boy_09,
    SE_Keyboard_FRN_Boy_00, SE_Keyboard_FRN_Boy_01, SE_Keyboard_FRN_Boy_02, SE_Keyboard_FRN_Boy_03,
    SE_Keyboard_FRN_Boy_04, SE_Keyboard_FRN_Boy_05, SE_Keyboard_FRN_Boy_06, SE_Keyboard_FRN_Boy_07,
    SE_Keyboard_FRN_Boy_08, SE_Keyboard_FRN_Boy_09,
    SE_Keyboard_FRN_Boy_00_fix, SE_Keyboard_FRN_Boy_01_fix, SE_Keyboard_FRN_Boy_02_fix,
    SE_Keyboard_FRN_Boy_03_fix, SE_Keyboard_FRN_Boy_04_fix, SE_Keyboard_FRN_Boy_05_fix,
    SE_Keyboard_FRN_Boy_06_fix, SE_Keyboard_FRN_Boy_07_fix, SE_Keyboard_FRN_Boy_08_fix,
    SE_Keyboard_FRN_Boy_09_fix,
};

static const int sUpperSeQc[] = {
    SE_Keyboard_FRN_Boy_1a, SE_Keyboard_FRN_Boy_1b, SE_Keyboard_FRN_Boy_1c, SE_Keyboard_FRN_Boy_1d,
    SE_Keyboard_FRN_Boy_1e, SE_Keyboard_FRN_Boy_1f, SE_Keyboard_FRN_Boy_1g, SE_Keyboard_FRN_Boy_1h,
    SE_Keyboard_FRN_Boy_1i, SE_Keyboard_FRN_Boy_1j,
    SE_Keyboard_FRN_Boy_1k, SE_Keyboard_FRN_Boy_1l, SE_Keyboard_FRN_Boy_1m, SE_Keyboard_FRN_Boy_1n,
    SE_Keyboard_FRN_Boy_1o, SE_Keyboard_FRN_Boy_1p, SE_Keyboard_FRN_Boy_1q, SE_Keyboard_FRN_Boy_1r,
    SE_Keyboard_FRN_Boy_1s, SE_Keyboard_FRN_Boy_1t,
    SE_Keyboard_FRN_Boy_1u, SE_Keyboard_FRN_Boy_1v, SE_Keyboard_FRN_Boy_1w, SE_Keyboard_FRN_Boy_1x,
    SE_Keyboard_FRN_Boy_1y, SE_Keyboard_FRN_Boy_1z,
};

static const int sAltSeQc[] = {
    SE_Keyboard_FRN_Boy_1a_fix, SE_Keyboard_FRN_Boy_1b_fix, SE_Keyboard_FRN_Boy_1c_fix,
    SE_Keyboard_FRN_Boy_1d_fix, SE_Keyboard_FRN_Boy_1e_fix, SE_Keyboard_FRN_Boy_1f_fix,
    SE_Keyboard_FRN_Boy_1g_fix, SE_Keyboard_FRN_Boy_1h_fix, SE_Keyboard_FRN_Boy_1i_fix,
    SE_Keyboard_FRN_Boy_1j_fix,
    SE_Keyboard_FRN_Boy_1k_fix, SE_Keyboard_FRN_Boy_1l_fix, SE_Keyboard_FRN_Boy_1m_fix,
    SE_Keyboard_FRN_Boy_1n_fix, SE_Keyboard_FRN_Boy_1o_fix, SE_Keyboard_FRN_Boy_1p_fix,
    SE_Keyboard_FRN_Boy_1q_fix, SE_Keyboard_FRN_Boy_1r_fix, SE_Keyboard_FRN_Boy_1s_fix,
    SE_Keyboard_FRN_Boy_1t_fix,
    SE_Keyboard_FRN_Boy_1u_fix, SE_Keyboard_FRN_Boy_1v_fix, SE_Keyboard_FRN_Boy_1w_fix,
    SE_Keyboard_FRN_Boy_1x_fix, SE_Keyboard_FRN_Boy_1y_fix, SE_Keyboard_FRN_Boy_1z_fix,
};

static const int sLowerSeQc[] = {
    SE_Keyboard_FRN_Boy_2a, SE_Keyboard_FRN_Boy_2b, SE_Keyboard_FRN_Boy_2c, SE_Keyboard_FRN_Boy_2d,
    SE_Keyboard_FRN_Boy_2e, SE_Keyboard_FRN_Boy_2f, SE_Keyboard_FRN_Boy_2g, SE_Keyboard_FRN_Boy_2h,
    SE_Keyboard_FRN_Boy_2i, SE_Keyboard_FRN_Boy_2j,
    SE_Keyboard_FRN_Boy_2k, SE_Keyboard_FRN_Boy_2l, SE_Keyboard_FRN_Boy_2m, SE_Keyboard_FRN_Boy_2n,
    SE_Keyboard_FRN_Boy_2o, SE_Keyboard_FRN_Boy_2p, SE_Keyboard_FRN_Boy_2q, SE_Keyboard_FRN_Boy_2r,
    SE_Keyboard_FRN_Boy_2s, SE_Keyboard_FRN_Boy_2t,
    SE_Keyboard_FRN_Boy_2u, SE_Keyboard_FRN_Boy_2v, SE_Keyboard_FRN_Boy_2w, SE_Keyboard_FRN_Boy_2x,
    SE_Keyboard_FRN_Boy_2y, SE_Keyboard_FRN_Boy_2z,
};

// 80473360
static const int sDigitSeQcF[] = {
    SE_Keyboard_FRN_Gal_00, SE_Keyboard_FRN_Gal_01, SE_Keyboard_FRN_Gal_02, SE_Keyboard_FRN_Gal_03,
    SE_Keyboard_FRN_Gal_04, SE_Keyboard_FRN_Gal_05, SE_Keyboard_FRN_Gal_06, SE_Keyboard_FRN_Gal_07,
    SE_Keyboard_FRN_Gal_08, SE_Keyboard_FRN_Gal_09,
    SE_Keyboard_FRN_Gal_00, SE_Keyboard_FRN_Gal_01, SE_Keyboard_FRN_Gal_02, SE_Keyboard_FRN_Gal_03,
    SE_Keyboard_FRN_Gal_04, SE_Keyboard_FRN_Gal_05, SE_Keyboard_FRN_Gal_06, SE_Keyboard_FRN_Gal_07,
    SE_Keyboard_FRN_Gal_08, SE_Keyboard_FRN_Gal_09,
    SE_Keyboard_FRN_Gal_00_fix, SE_Keyboard_FRN_Gal_01_fix, SE_Keyboard_FRN_Gal_02_fix,
    SE_Keyboard_FRN_Gal_03_fix, SE_Keyboard_FRN_Gal_04_fix, SE_Keyboard_FRN_Gal_05_fix,
    SE_Keyboard_FRN_Gal_06_fix, SE_Keyboard_FRN_Gal_07_fix, SE_Keyboard_FRN_Gal_08_fix,
    SE_Keyboard_FRN_Gal_09_fix,
};

static const int sUpperSeQcF[] = {
    SE_Keyboard_FRN_Gal_1a, SE_Keyboard_FRN_Gal_1b, SE_Keyboard_FRN_Gal_1c, SE_Keyboard_FRN_Gal_1d,
    SE_Keyboard_FRN_Gal_1e, SE_Keyboard_FRN_Gal_1f, SE_Keyboard_FRN_Gal_1g, SE_Keyboard_FRN_Gal_1h,
    SE_Keyboard_FRN_Gal_1i, SE_Keyboard_FRN_Gal_1j,
    SE_Keyboard_FRN_Gal_1k, SE_Keyboard_FRN_Gal_1l, SE_Keyboard_FRN_Gal_1m, SE_Keyboard_FRN_Gal_1n,
    SE_Keyboard_FRN_Gal_1o, SE_Keyboard_FRN_Gal_1p, SE_Keyboard_FRN_Gal_1q, SE_Keyboard_FRN_Gal_1r,
    SE_Keyboard_FRN_Gal_1s, SE_Keyboard_FRN_Gal_1t,
    SE_Keyboard_FRN_Gal_1u, SE_Keyboard_FRN_Gal_1v, SE_Keyboard_FRN_Gal_1w, SE_Keyboard_FRN_Gal_1x,
    SE_Keyboard_FRN_Gal_1y, SE_Keyboard_FRN_Gal_1z,
};

static const int sAltSeQcF[] = {
    SE_Keyboard_FRN_Gal_1a_fix, SE_Keyboard_FRN_Gal_1b_fix, SE_Keyboard_FRN_Gal_1c_fix,
    SE_Keyboard_FRN_Gal_1d_fix, SE_Keyboard_FRN_Gal_1e_fix, SE_Keyboard_FRN_Gal_1f_fix,
    SE_Keyboard_FRN_Gal_1g_fix, SE_Keyboard_FRN_Gal_1h_fix, SE_Keyboard_FRN_Gal_1i_fix,
    SE_Keyboard_FRN_Gal_1j_fix,
    SE_Keyboard_FRN_Gal_1k_fix, SE_Keyboard_FRN_Gal_1l_fix, SE_Keyboard_FRN_Gal_1m_fix,
    SE_Keyboard_FRN_Gal_1n_fix, SE_Keyboard_FRN_Gal_1o_fix, SE_Keyboard_FRN_Gal_1p_fix,
    SE_Keyboard_FRN_Gal_1q_fix, SE_Keyboard_FRN_Gal_1r_fix, SE_Keyboard_FRN_Gal_1s_fix,
    SE_Keyboard_FRN_Gal_1t_fix,
    SE_Keyboard_FRN_Gal_1u_fix, SE_Keyboard_FRN_Gal_1v_fix, SE_Keyboard_FRN_Gal_1w_fix,
    SE_Keyboard_FRN_Gal_1x_fix, SE_Keyboard_FRN_Gal_1y_fix, SE_Keyboard_FRN_Gal_1z_fix,
};

static const int sLowerSeQcF[] = {
    SE_Keyboard_FRN_Gal_2a, SE_Keyboard_FRN_Gal_2b, SE_Keyboard_FRN_Gal_2c, SE_Keyboard_FRN_Gal_2d,
    SE_Keyboard_FRN_Gal_2e, SE_Keyboard_FRN_Gal_2f, SE_Keyboard_FRN_Gal_2g, SE_Keyboard_FRN_Gal_2h,
    SE_Keyboard_FRN_Gal_2i, SE_Keyboard_FRN_Gal_2j,
    SE_Keyboard_FRN_Gal_2k, SE_Keyboard_FRN_Gal_2l, SE_Keyboard_FRN_Gal_2m, SE_Keyboard_FRN_Gal_2n,
    SE_Keyboard_FRN_Gal_2o, SE_Keyboard_FRN_Gal_2p, SE_Keyboard_FRN_Gal_2q, SE_Keyboard_FRN_Gal_2r,
    SE_Keyboard_FRN_Gal_2s, SE_Keyboard_FRN_Gal_2t,
    SE_Keyboard_FRN_Gal_2u, SE_Keyboard_FRN_Gal_2v, SE_Keyboard_FRN_Gal_2w, SE_Keyboard_FRN_Gal_2x,
    SE_Keyboard_FRN_Gal_2y, SE_Keyboard_FRN_Gal_2z,
};

// 80473510
static const int sDigitSeMx[] = {
    SE_Keyboard_SPN_Boy_s00, SE_Keyboard_SPN_Boy_s01, SE_Keyboard_SPN_Boy_s02, SE_Keyboard_SPN_Boy_s03,
    SE_Keyboard_SPN_Boy_s04, SE_Keyboard_SPN_Boy_s05, SE_Keyboard_SPN_Boy_s06, SE_Keyboard_SPN_Boy_s07,
    SE_Keyboard_SPN_Boy_s08, SE_Keyboard_SPN_Boy_s09,
    SE_Keyboard_SPN_Boy_00, SE_Keyboard_SPN_Boy_01, SE_Keyboard_SPN_Boy_02, SE_Keyboard_SPN_Boy_03,
    SE_Keyboard_SPN_Boy_04, SE_Keyboard_SPN_Boy_05, SE_Keyboard_SPN_Boy_06, SE_Keyboard_SPN_Boy_07,
    SE_Keyboard_SPN_Boy_08, SE_Keyboard_SPN_Boy_09,
    SE_Keyboard_SPN_Boy_00_fix, SE_Keyboard_SPN_Boy_01_fix, SE_Keyboard_SPN_Boy_02_fix,
    SE_Keyboard_SPN_Boy_03_fix, SE_Keyboard_SPN_Boy_04_fix, SE_Keyboard_SPN_Boy_05_fix,
    SE_Keyboard_SPN_Boy_06_fix, SE_Keyboard_SPN_Boy_07_fix, SE_Keyboard_SPN_Boy_08_fix,
    SE_Keyboard_SPN_Boy_09_fix,
};

static const int sUpperSeMx[] = {
    SE_Keyboard_SPN_Boy_1a, SE_Keyboard_SPN_Boy_1b, SE_Keyboard_SPN_Boy_1c, SE_Keyboard_SPN_Boy_1d,
    SE_Keyboard_SPN_Boy_1e, SE_Keyboard_SPN_Boy_1f, SE_Keyboard_SPN_Boy_1g, SE_Keyboard_SPN_Boy_1h,
    SE_Keyboard_SPN_Boy_1i, SE_Keyboard_SPN_Boy_1j,
    SE_Keyboard_SPN_Boy_1k, SE_Keyboard_SPN_Boy_1l, SE_Keyboard_SPN_Boy_1m, SE_Keyboard_SPN_Boy_1n,
    SE_Keyboard_SPN_Boy_1o, SE_Keyboard_SPN_Boy_1p, SE_Keyboard_SPN_Boy_1q, SE_Keyboard_SPN_Boy_1r,
    SE_Keyboard_SPN_Boy_1s, SE_Keyboard_SPN_Boy_1t,
    SE_Keyboard_SPN_Boy_1u, SE_Keyboard_SPN_Boy_1v, SE_Keyboard_SPN_Boy_1w, SE_Keyboard_SPN_Boy_1x,
    SE_Keyboard_SPN_Boy_1y, SE_Keyboard_SPN_Boy_1z,
};

static const int sAltSeMx[] = {
    SE_Keyboard_SPN_Boy_1a_fix, SE_Keyboard_SPN_Boy_1b_fix, SE_Keyboard_SPN_Boy_1c_fix,
    SE_Keyboard_SPN_Boy_1d_fix, SE_Keyboard_SPN_Boy_1e_fix, SE_Keyboard_SPN_Boy_1f_fix,
    SE_Keyboard_SPN_Boy_1g_fix, SE_Keyboard_SPN_Boy_1h_fix, SE_Keyboard_SPN_Boy_1i_fix,
    SE_Keyboard_SPN_Boy_1j_fix,
    SE_Keyboard_SPN_Boy_1k_fix, SE_Keyboard_SPN_Boy_1l_fix, SE_Keyboard_SPN_Boy_1m_fix,
    SE_Keyboard_SPN_Boy_1n_fix, SE_Keyboard_SPN_Boy_1o_fix, SE_Keyboard_SPN_Boy_1p_fix,
    SE_Keyboard_SPN_Boy_1q_fix, SE_Keyboard_SPN_Boy_1r_fix, SE_Keyboard_SPN_Boy_1s_fix,
    SE_Keyboard_SPN_Boy_1t_fix,
    SE_Keyboard_SPN_Boy_1u_fix, SE_Keyboard_SPN_Boy_1v_fix, SE_Keyboard_SPN_Boy_1w_fix,
    SE_Keyboard_SPN_Boy_1x_fix, SE_Keyboard_SPN_Boy_1y_fix, SE_Keyboard_SPN_Boy_1z_fix,
};

static const int sLowerSeMx[] = {
    SE_Keyboard_SPN_Boy_2a, SE_Keyboard_SPN_Boy_2b, SE_Keyboard_SPN_Boy_2c, SE_Keyboard_SPN_Boy_2d,
    SE_Keyboard_SPN_Boy_2e, SE_Keyboard_SPN_Boy_2f, SE_Keyboard_SPN_Boy_2g, SE_Keyboard_SPN_Boy_2h,
    SE_Keyboard_SPN_Boy_2i, SE_Keyboard_SPN_Boy_2j,
    SE_Keyboard_SPN_Boy_2k, SE_Keyboard_SPN_Boy_2l, SE_Keyboard_SPN_Boy_2m, SE_Keyboard_SPN_Boy_2n,
    SE_Keyboard_SPN_Boy_2o, SE_Keyboard_SPN_Boy_2p, SE_Keyboard_SPN_Boy_2q, SE_Keyboard_SPN_Boy_2r,
    SE_Keyboard_SPN_Boy_2s, SE_Keyboard_SPN_Boy_2t,
    SE_Keyboard_SPN_Boy_2u, SE_Keyboard_SPN_Boy_2v, SE_Keyboard_SPN_Boy_2w, SE_Keyboard_SPN_Boy_2x,
    SE_Keyboard_SPN_Boy_2y, SE_Keyboard_SPN_Boy_2z,
};

// 804736C0
static const int sDigitSeMxF[] = {
    SE_Keyboard_SPN_Gal_s00, SE_Keyboard_SPN_Gal_s01, SE_Keyboard_SPN_Gal_s02, SE_Keyboard_SPN_Gal_s03,
    SE_Keyboard_SPN_Gal_s04, SE_Keyboard_SPN_Gal_s05, SE_Keyboard_SPN_Gal_s06, SE_Keyboard_SPN_Gal_s07,
    SE_Keyboard_SPN_Gal_s08, SE_Keyboard_SPN_Gal_s09,
    SE_Keyboard_SPN_Gal_00, SE_Keyboard_SPN_Gal_01, SE_Keyboard_SPN_Gal_02, SE_Keyboard_SPN_Gal_03,
    SE_Keyboard_SPN_Gal_04, SE_Keyboard_SPN_Gal_05, SE_Keyboard_SPN_Gal_06, SE_Keyboard_SPN_Gal_07,
    SE_Keyboard_SPN_Gal_08, SE_Keyboard_SPN_Gal_09,
    SE_Keyboard_SPN_Gal_00_fix, SE_Keyboard_SPN_Gal_01_fix, SE_Keyboard_SPN_Gal_02_fix,
    SE_Keyboard_SPN_Gal_03_fix, SE_Keyboard_SPN_Gal_04_fix, SE_Keyboard_SPN_Gal_05_fix,
    SE_Keyboard_SPN_Gal_06_fix, SE_Keyboard_SPN_Gal_07_fix, SE_Keyboard_SPN_Gal_08_fix,
    SE_Keyboard_SPN_Gal_09_fix,
};

static const int sUpperSeMxF[] = {
    SE_Keyboard_SPN_Gal_1a, SE_Keyboard_SPN_Gal_1b, SE_Keyboard_SPN_Gal_1c, SE_Keyboard_SPN_Gal_1d,
    SE_Keyboard_SPN_Gal_1e, SE_Keyboard_SPN_Gal_1f, SE_Keyboard_SPN_Gal_1g, SE_Keyboard_SPN_Gal_1h,
    SE_Keyboard_SPN_Gal_1i, SE_Keyboard_SPN_Gal_1j,
    SE_Keyboard_SPN_Gal_1k, SE_Keyboard_SPN_Gal_1l, SE_Keyboard_SPN_Gal_1m, SE_Keyboard_SPN_Gal_1n,
    SE_Keyboard_SPN_Gal_1o, SE_Keyboard_SPN_Gal_1p, SE_Keyboard_SPN_Gal_1q, SE_Keyboard_SPN_Gal_1r,
    SE_Keyboard_SPN_Gal_1s, SE_Keyboard_SPN_Gal_1t,
    SE_Keyboard_SPN_Gal_1u, SE_Keyboard_SPN_Gal_1v, SE_Keyboard_SPN_Gal_1w, SE_Keyboard_SPN_Gal_1x,
    SE_Keyboard_SPN_Gal_1y, SE_Keyboard_SPN_Gal_1z,
};

static const int sAltSeMxF[] = {
    SE_Keyboard_SPN_Gal_1a_fix, SE_Keyboard_SPN_Gal_1b_fix, SE_Keyboard_SPN_Gal_1c_fix,
    SE_Keyboard_SPN_Gal_1d_fix, SE_Keyboard_SPN_Gal_1e_fix, SE_Keyboard_SPN_Gal_1f_fix,
    SE_Keyboard_SPN_Gal_1g_fix, SE_Keyboard_SPN_Gal_1h_fix, SE_Keyboard_SPN_Gal_1i_fix,
    SE_Keyboard_SPN_Gal_1j_fix,
    SE_Keyboard_SPN_Gal_1k_fix, SE_Keyboard_SPN_Gal_1l_fix, SE_Keyboard_SPN_Gal_1m_fix,
    SE_Keyboard_SPN_Gal_1n_fix, SE_Keyboard_SPN_Gal_1o_fix, SE_Keyboard_SPN_Gal_1p_fix,
    SE_Keyboard_SPN_Gal_1q_fix, SE_Keyboard_SPN_Gal_1r_fix, SE_Keyboard_SPN_Gal_1s_fix,
    SE_Keyboard_SPN_Gal_1t_fix,
    SE_Keyboard_SPN_Gal_1u_fix, SE_Keyboard_SPN_Gal_1v_fix, SE_Keyboard_SPN_Gal_1w_fix,
    SE_Keyboard_SPN_Gal_1x_fix, SE_Keyboard_SPN_Gal_1y_fix, SE_Keyboard_SPN_Gal_1z_fix,
};

static const int sLowerSeMxF[] = {
    SE_Keyboard_SPN_Gal_2a, SE_Keyboard_SPN_Gal_2b, SE_Keyboard_SPN_Gal_2c, SE_Keyboard_SPN_Gal_2d,
    SE_Keyboard_SPN_Gal_2e, SE_Keyboard_SPN_Gal_2f, SE_Keyboard_SPN_Gal_2g, SE_Keyboard_SPN_Gal_2h,
    SE_Keyboard_SPN_Gal_2i, SE_Keyboard_SPN_Gal_2j,
    SE_Keyboard_SPN_Gal_2k, SE_Keyboard_SPN_Gal_2l, SE_Keyboard_SPN_Gal_2m, SE_Keyboard_SPN_Gal_2n,
    SE_Keyboard_SPN_Gal_2o, SE_Keyboard_SPN_Gal_2p, SE_Keyboard_SPN_Gal_2q, SE_Keyboard_SPN_Gal_2r,
    SE_Keyboard_SPN_Gal_2s, SE_Keyboard_SPN_Gal_2t,
    SE_Keyboard_SPN_Gal_2u, SE_Keyboard_SPN_Gal_2v, SE_Keyboard_SPN_Gal_2w, SE_Keyboard_SPN_Gal_2x,
    SE_Keyboard_SPN_Gal_2y, SE_Keyboard_SPN_Gal_2z,
};

static const int *sDigitSe[] = {sDigitSeUs, sDigitSeQc, sDigitSeMx, sDigitSeUsF, sDigitSeQcF, sDigitSeMxF};
static const int *sUpperSe[] = {sUpperSeUs, sUpperSeQc, sUpperSeMx, sUpperSeUsF, sUpperSeQcF, sUpperSeMxF};
static const int *sAltSe[] = {sAltSeUs, sAltSeQc, sAltSeMx, sAltSeUsF, sAltSeQcF, sAltSeMxF};
static const int *sLowerSe[] = {sLowerSeUs, sLowerSeQc, sLowerSeMx, sLowerSeUsF, sLowerSeQcF, sLowerSeMxF};
#endif

// The mail BMGs, by MailKind_e.
static const char *sMailDirs[dLetter::MAIL_KIND_NUM] = {
    "sys_MAIL1/TO/",     "sys_MAIL1/MSG/",    "sys_MAIL1/FROM/",   "sys_BBS/",         "sys_MAIL2/TO2/",
    "sys_MAIL2/MSG2_A/", "sys_MAIL2/MSG2_B/", "sys_MAIL2/MSG2_C/", "sys_MAIL2/FROM2/",
};

// The keyboard SEs of d_kbd (declared in d_kbd.hpp) open this TU's .text.
namespace dKbd {

// 800CB0BC
void playSeCanNotInput() {
    playSe(SE_keyboard_CanNotInputWord);
}

// 800CB0C4
void playSeSubmit() {
    playSe(SE_text_submit);
}

} // namespace dKbd

namespace dLetter {

// 805A02D0 (static initializer 800CCBBC)
Comp_c g_comp;

#if VERSION == VERSION_RUUJ01_01 || VERSION == VERSION_RUUJ01_02
int getKeySeSet() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
    if (player == NULL) {
        return TRUE;
    }
    return player->mPID.player.mGender != GENDER_FEMALE;
}

void playKanaKeySe(int idx, BOOL fix) {
    if (fix) {
        idx += 86;
    }
    playSe(getKeySeSet() ? sKanaSeJp[idx] : sKanaSeJpF[idx]);
}

void playDigitKeySe(int idx, BOOL a, BOOL b) {
    if (b) {
        idx += 20;
    } else if (a) {
        idx += 10;
    }
    playSe(getKeySeSet() ? sDigitSeJp[idx] : sDigitSeJpF[idx]);
}

void playAltKeySe(int idx) {
    playSe(getKeySeSet() ? sAltSeJp[idx] : sAltSeJpF[idx]);
}

void playUnfixKeySe(int idx) {
    playSe(getKeySeSet() ? sUnfixSeJp[idx] : sUnfixSeJpF[idx]);
}

void playUpperKeySe(int idx) {
    playSe(getKeySeSet() ? sUpperSeJp[idx] : sUpperSeJpF[idx]);
}

void playLowerKeySe(int idx) {
    if (dSaveOption::getKeySe()) {
        playUpperKeySe(idx);
    } else {
        playSe(getKeySeSet() ? sLowerSeJp[idx] : sLowerSeJpF[idx]);
    }
}

void playKeySe(wchar_t c, BOOL a, BOOL b, BOOL pcKeyboard) {
    if (c >= 0x3041 && c <= 0x3093) {
        playKanaKeySe(c - 0x3041, b);
    } else if (c >= 0x30A1 && c <= 0x30F6) {
        playKanaKeySe(c - 0x30A1, b);
    } else if (c >= L'0' && c <= L'9') {
        playDigitKeySe(c - L'0', a, b);
    } else if (c >= L'A' && c <= L'Z') {
        if (b) {
            playAltKeySe(c - L'A');
        } else if (pcKeyboard) {
            playUnfixKeySe(c - L'A');
        } else {
            playUpperKeySe(c - L'A');
        }
    } else if (c >= L'a' && c <= L'z') {
        if (b) {
            playAltKeySe(c - L'a');
        } else if (pcKeyboard) {
            playUnfixKeySe(c - L'a');
        } else {
            playLowerKeySe(c - L'a');
        }
    } else {
        playSe(SE_pupupu);
    }
}

#else
// 800CB0CC
BOOL isMalePlayer() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
    if (player == NULL) {
        return TRUE;
    }
    return player->mPID.player.mGender != GENDER_FEMALE;
}

// 800CB110
int getKeySeSet() {
    int set = 0;
#if VERSION == VERSION_RUUP01_00 || VERSION == VERSION_RUUP01_01
    switch (getLanguage()) {
    case LANGUAGE_EN:
        set = 0;
        break;
    case LANGUAGE_FR:
        set = 1;
        break;
    case LANGUAGE_ES:
        set = 2;
        break;
    case LANGUAGE_IT:
        set = 3;
        break;
    case LANGUAGE_DE:
        set = 4;
        break;
    }
    if (!isMalePlayer()) {
        set += 5;
    }
#else
    switch (getLanguage()) {
    case LANGUAGE_US:
        set = 0;
        break;
    case LANGUAGE_QC:
        set = 1;
        break;
    case LANGUAGE_MX:
        set = 2;
        break;
    }
    if (!isMalePlayer()) {
        set += 3;
    }
#endif
    return set;
}

// 800CB188
void playDigitKeySe(int idx, BOOL a, BOOL b) {
    if (b) {
        idx += 20;
    } else if (a) {
        idx += 10;
    }
    playSe(sDigitSe[getKeySeSet()][idx]);
}

// 800CB1EC
void playUpperKeySe(int idx) {
    playSe(sUpperSe[getKeySeSet()][idx]);
}

// 800CB234
void playAltKeySe(int idx) {
    playSe(sAltSe[getKeySeSet()][idx]);
}

// 800CB27C
void playLowerKeySe(int idx) {
    if (dSaveOption::getKeySe()) {
        playUpperKeySe(idx);
    } else {
        playSe(sLowerSe[getKeySeSet()][idx]);
    }
}

#if VERSION == VERSION_RUUP01_00 || VERSION == VERSION_RUUP01_01
int getUmlautIndex(wchar_t c, BOOL alt) {
    for (int i = 0; i < 7; i++) {
        if (c == sUmlautChars[i]) {
            if (alt) {
                i += 7;
            }
            return i;
        }
    }
    return -1;
}

BOOL playUmlautKeySe(wchar_t c, BOOL alt) {
    if (getLanguage() != LANGUAGE_DE) {
        return FALSE;
    }
    int idx = getUmlautIndex(c, alt);
    if (idx == -1) {
        return FALSE;
    }
    playSe(isMalePlayer() ? sUmlautSeDe[idx] : sUmlautSeDeF[idx]);
    return TRUE;
}
#endif

// 800CB2DC
void playKeySe(wchar_t c, BOOL a, BOOL b, BOOL pcKeyboard) {
    if (c >= L'0' && c <= L'9') {
        playDigitKeySe(c - L'0', a, b);
    } else if (c >= L'A' && c <= L'Z') {
        if (b) {
            playAltKeySe(c - L'A');
        } else {
            playUpperKeySe(c - L'A');
        }
    } else if (c >= L'a' && c <= L'z') {
        if (b) {
            playAltKeySe(c - L'a');
        } else {
            playLowerKeySe(c - L'a');
        }
    } else {
#if VERSION == VERSION_RUUP01_00 || VERSION == VERSION_RUUP01_01
        if (!playUmlautKeySe(c, b)) {
            playSe(SE_pupupu);
        }
#else
        playSe(SE_pupupu);
#endif
    }
}

#endif

// 800CB34C
int getEntryNum(int kind, const char *label) {
    char path[0x40];
    sprintf(path, "%s%s", sMailDirs[kind], label);
    dScript::Res_c res(dScript::getBmgFile(dScript::getBank(), path));
    int num = res.getCount();
    return num;
}

// 800CB3C8
BOOL loadEntry(dScript::Word_c *word, int kind, u16 idx, const char *label) {
    u32 max = word->getCapacity() - 1;
    char path[0x40];
    sprintf(path, "%s%s", sMailDirs[kind], label);
    void *bmg = dScript::getBmgFile(dScript::getBank(), path);
    if (bmg == NULL) {
        return FALSE;
    }
    dScript::Res_c res(bmg);
    if (idx >= res.getCount()) {
        return FALSE;
    }
    const wchar_t *msg = res.getMessage(idx);
    if (dScript::getStringLength(msg, 0, 0) > max) {
        return FALSE;
    }
    dLetter::Word_c text;
    if (!text.set(msg, 0)) {
        return FALSE;
    }
    if (text.getLength(0) > max) {
        return FALSE;
    }
    if (!word->set(text.getText(), 0)) {
        return FALSE;
    }
    return TRUE;
}

// 800CB594
void reset() {
    g_comp.mTo.clear();
    g_comp.mFrom.clear();
    for (int i = 0; i < LETTER_WORD_NUM; i++) {
        g_comp.mWords[i].clear();
    }
    for (int i = 0; i < LETTER_NUM_WORD_NUM; i++) {
        g_comp.mNumWords[i].clear();
    }
    memset(g_comp.mRandomWords, 0, sizeof(g_comp.mRandomWords));
}

// 800CB638
BOOL loadText(dScript::Word_c *header, dScript::Word_c *body, dScript::Word_c *footer, u16 kind, const char *label) {
    return loadTextParts(header, body, footer, kind, kind, kind, label, label, label);
}

// 800CB66C
BOOL loadTextParts(dScript::Word_c *header, dScript::Word_c *body, dScript::Word_c *footer, u16 headerIdx,
                 u16 bodyIdx, u16 footerIdx, const char *headerLabel, const char *bodyLabel, const char *footerLabel) {
    if (!loadEntry(header, MAIL_KIND_TO, headerIdx, headerLabel)) {
        return FALSE;
    }
    if (!loadEntry(body, MAIL_KIND_MSG, bodyIdx, bodyLabel)) {
        return FALSE;
    }
    if (!loadEntry(footer, MAIL_KIND_FROM, footerIdx, footerLabel)) {
        return FALSE;
    }
    reset();
    return TRUE;
}

// 800CB720
BOOL loadBbsText(dScript::Word_c *word, u16 msgId, const char *group) {
    BOOL ok = loadEntry(word, MAIL_KIND_BBS, msgId, group);
    reset();
    return ok;
}

// 800CB760
BOOL loadText2(dScript::Word_c *header, dScript::Word_c *body, dScript::Word_c *footer, u16 headerIdx, u16 bodyA,
                 u16 bodyB, u16 bodyC, u16 footerIdx, const char *bodyLabel, const char *label) {
    if (!loadEntry(header, MAIL_KIND_TO2, headerIdx, label)) {
        return FALSE;
    }
    if (!loadEntry(footer, MAIL_KIND_FROM2, footerIdx, label)) {
        return FALSE;
    }
    dLetter::Word_c a;
    dLetter::Word_c b;
    dLetter::Word_c c;
    if (!loadEntry(&a, MAIL_KIND_MSG2_A, bodyA, bodyLabel)) {
        return FALSE;
    }
    if (!loadEntry(&b, MAIL_KIND_MSG2_B, bodyB, bodyLabel)) {
        return FALSE;
    }
    if (!loadEntry(&c, MAIL_KIND_MSG2_C, bodyC, bodyLabel)) {
        return FALSE;
    }
    if (!body->set(a.getText(), 0)) {
        return FALSE;
    }
    if (!body->appendText(b.getText(), 0)) {
        return FALSE;
    }
    if (!body->appendText(c.getText(), 0)) {
        return FALSE;
    }
    reset();
    return TRUE;
}

// 800CBA14
u16 getToNum(const char *label) {
    return getEntryNum(MAIL_KIND_TO, label);
}

// 800CBA40
u16 getMsgNum(const char *label) {
    return getEntryNum(MAIL_KIND_MSG, label);
}

// 800CBA6C
u16 getFromNum(const char *label) {
    return getEntryNum(MAIL_KIND_FROM, label);
}

// 800CBA98
u16 getMsg2ANum(const char *label) {
    return getEntryNum(MAIL_KIND_MSG2_A, label);
}

// 800CBAC4
u16 getTo2Num(const char *label) {
    return getEntryNum(MAIL_KIND_TO2, label);
}

// 800CBAF0
void setMonthName(int slot, int month) {
    dString::Word_c word;
    dScript::setMonthName(&word, month);
    dLetter::setWord(slot, &word);
}

// 800CBB50
void setDayName(int slot, u8 day) {
    dString::Word_c word;
    dScript::setDayName(&word, day);
    dLetter::setWord(slot, &word);
}

// 800CBBB0
void setPersonalName(int slot, const dPersonalID_c *pid) {
    dHmnName::Word_c name;
    pid->setWord(&name);
    dLetter::setWord(slot, &name);
}

// 800CBC10
void setLandName(int slot, const dLandID_c *land) {
    dLandNameWord_c name;
    // dLandID_c::setWord is not const (symbol setWord__9dLandID_cFPQ27dScript6Word_c).
    const_cast<dLandID_c *>(land)->setWord(&name);
    dLetter::setWord(slot, &name);
}

// 800CBC70
void setAnmPersonalName(int slot, const dAnmPersonalID_c *animal) {
    dHmnName::Word_c name;
    const_cast<dAnmPersonalID_c *>(animal)->setWord(&name, LANGUAGE_NUM);
    dLetter::setWord(slot, &name);
}

// 800CBCD4
void setWord(int slot, dScript::Word_c *word) {
    dScript::Word_c *dst = &g_comp.mWords[slot];
    dst->clear();
    dst->copy(word, 0);
}

// 800CBD30
void setStringWord(int slot, u16 index, const char *group) {
    dString::Word_c word;
    dString::loadString(&word, index, group);
    dLetter::setWord(slot, &word);
}

// 800CBDA0
void setItemName(int slot, const dItem::Item &item) {
    dItem::name_c name(item);
    dLetter::setWord(slot, &name);
}

// 800CBDF4
dScript::Word_c *getWord(u8 slot) {
    return &g_comp.mWords[slot];
}

// 800CBE0C
void setNumber(int slot, int value, int digits, dScript::NumberFormat_e format) {
    if (value >= 0) {
        dString::Word_c word;
        dScript::setNumber(&word, value, digits, format);
        dScript::Word_c *dst = &g_comp.mNumWords[slot];
        dst->clear();
        dst->copy(&word, 0);
    }
}

// 800CBEB4
void setNumber4(int slot, int value) {
    dString::Word_c word;
    dScript::setNumber4(&word, value);
    dScript::Word_c *dst = &g_comp.mNumWords[slot];
    dst->clear();
    dst->copy(&word, 0);
}

// 800CBF34
void formatNumber(int slot, int value, int digits, dScript::NumberFormat_e format) {
    if (value >= 0) {
        dString::Word_c word;
        dScript::formatNumber(&word, value, digits, format, 0);
        dScript::Word_c *dst = &g_comp.mNumWords[slot];
        dst->clear();
        dst->copy(&word, 0);
    }
}

// 800CBFE0
void setHour(u8 hour, int slot, int numSlot) {
    int hour12 = dScript::to12Hour(hour);
    dString::Word_c word;
    dScript::setAmPm(&word, hour);
    dLetter::setWord(slot, &word);
    formatNumber(numSlot, hour12, 2, dScript::NUM_FORMAT_REGION);
}

// 800CC070
dScript::Word_c *getNumWord(u8 slot) {
    return &g_comp.mNumWords[slot];
}

// 800CC088
u16 getRandomWord(int kind, int slot) {
    return g_comp.mRandomWords[kind][slot];
}

// 800CC0A8
void setRandomWord(int kind, int slot, u16 msgId) {
    g_comp.mRandomWords[kind][slot] = msgId;
}

// 800CC0C8
void setTo(const dPersonalID_c *to) {
    g_comp.mTo = *to;
}

// 800CC1AC
void setFrom(const dAnmPersonalID_c *sender) {
    g_comp.mFrom = *sender;
}

// 800CC2A4
dPersonalID_c *getTo() {
    return &g_comp.mTo;
}

// 800CC2B4
dAnmPersonalID_c *getFrom() {
    return &g_comp.mFrom;
}

// 800CC2C4
Word_c::Word_c() {
    clear();
}

// 800CC308
Word_c::~Word_c() {}

// 800CC360
u32 Word_c::getBufferSize() {
    return sizeof(mBuffer);
}

// 800CC368
wchar_t *Word_c::getBuffer() {
    return mBuffer;
}

// 800CC370
int Word_c::procNumber(int pos) {
    const wchar_t *tag = getBuffer() + pos;
    dScript::getTagId(tag);
    const u8 *params = dScript::getTagParams(tag);
    u8 slot = 0;
    memcpy(&slot, params, 1);
    dScript::Word_c *word = getNumWord(slot);
    _18 = 1;
    return setTagWord(pos, word);
}

// 800CC418
int Word_c::procWord(int pos) {
    const wchar_t *tag = getBuffer() + pos;
    dScript::getTagId(tag);
    const u8 *params = dScript::getTagParams(tag);
    u8 slot = 0;
    memcpy(&slot, params, 1);
    dScript::Word_c *word = getWord(slot);
    _18 = 0;
    return setTagWord(pos, word);
}

// 800CC4BC
int Word_c::procRandomWordTag(int pos, int kind) {
    const u8 *params = dScript::getTagParams(getBuffer() + pos);
    u8 slot = 0;
    memcpy(&slot, params, 1);
    dAnmPersonalID_c *id = getFrom();
    const char *group = getGenderedGroup(kind, id);
    u16 msgId;
    u16 used[LETTER_WORD_NUM];
    u16 *list = used;
    for (int i = 0; i < LETTER_WORD_NUM; i++) {
        list[i] = getRandomWord(kind, i);
    }
    int n = procRandomWord(pos, kind, id, list);
    msgId = used[slot];
    setRandomWord(kind, slot, msgId);
    setStringWord(slot, msgId, group);
    return n;
}

// 800CC5BC
int Word_c::procDrink(int pos) {
    return procRandomWordTag(pos, 0);
}

// 800CC5C4
int Word_c::procObject(int pos) {
    return procRandomWordTag(pos, 1);
}

// 800CC5CC
int Word_c::procSweets(int pos) {
    return procRandomWordTag(pos, 2);
}

// 800CC5D4
int Word_c::procSports(int pos) {
    return procRandomWordTag(pos, 3);
}

// 800CC5DC
int Word_c::procMusic(int pos) {
    return procRandomWordTag(pos, 4);
}

// 800CC5E4
int Word_c::procFood(int pos) {
    return procRandomWordTag(pos, 5);
}

// 800CC5EC
int Word_c::procHobby(int pos) {
    return procRandomWordTag(pos, 6);
}

// 800CC5F4
int Word_c::procUnit(int pos) {
    dAnmPersonalID_c *from = getFrom();
    return procAnimalUnit(pos, getTo(), from);
}

// 800CC650
int Word_c::procPlayerChar(int pos, u32 index) {
    dHmnName::Word_c name;
    getTo()->setWord(&name);
    wchar_t c = 0;
    if (!name.getChar(&c, index)) {
        return removeTag(pos);
    }
    return insertWord(pos, &c, 1, FALSE);
}

// 800CC714
int Word_c::procPlayerChar0(int pos) {
    return procPlayerChar(pos, 0);
}

// 800CC71C
int Word_c::procPlayerChar1(int pos) {
    return procPlayerChar(pos, 1);
}

// 800CC724
int Word_c::procPlayerChar2(int pos) {
    return procPlayerChar(pos, 2);
}

// 800CC72C
int Word_c::procPlayer(int pos) {
    return procPlayerName(pos, getTo());
}

// 800CC774
int Word_c::procAnimal(int pos) {
    return procAnimalName(pos, getFrom());
}

// 800CC7BC
int Word_c::procAnimalA(int pos) {
    return removeTag(pos);
}

// 800CC7C0
int Word_c::procAnimalB(int pos) {
    return removeTag(pos);
}

// 800CC7C4
int Word_c::procAnimalParam(int pos) {
    return removeTag(pos);
}

// 800CC7C8
int Word_c::procCatchphrase(int pos) {
    return removeTag(pos);
}

// 800CC7CC
int Word_c::procPlayerNickname(int pos) {
    return removeTag(pos);
}

// 800CC7D0
int Word_c::procImpression(int pos) {
    return removeTag(pos);
}

// 800CC7D4
int Word_c::procPlayerStarSign(int pos) {
    return removeTag(pos);
}

// 800CC7D8
int Word_c::procAnimalStarSign(int pos) {
    return removeTag(pos);
}

// 800CC7DC
int Word_c::procPlayerGender(int pos) {
    return procByPlayerGender(pos, getTo());
}

// 800CC824
int Word_c::procAnimalGender(int pos) {
    return procByAnimalGender(pos, getFrom());
}

// 800CC86C
int Word_c::procWordGender(int pos) {
    const u8 *params = dScript::getTagParams(getBuffer() + pos);
    u8 slot = 0;
    memcpy(&slot, params, 1);
    return procByWordGender(pos, getWord(slot));
}

// 800CC8EC
int Word_c::procPlayerElision(int pos) {
    return procElisionPlayer(pos, getTo());
}

// 800CC934
int Word_c::procAnimalElision(int pos) {
    return procElisionAnimal(pos, getFrom());
}

// 800CC97C
int Word_c::procAnimalAElision(int pos) {
    return removeTag(pos);
}

// 800CC980
int Word_c::procAnimalBElision(int pos) {
    return removeTag(pos);
}

// 800CC984
int Word_c::procAnimalParamElision(int pos) {
    return removeTag(pos);
}

// 800CC988
int Word_c::procPlayerNicknameElision(int pos) {
    return removeTag(pos);
}

// 800CC98C
int Word_c::procWordElision(int pos) {
    const u8 *params = dScript::getTagParams(getBuffer() + pos);
    u8 slot = 0;
    memcpy(&slot, params, 1);
    if (slot >= LETTER_WORD_NUM) {
        return removeTag(pos);
    }
    return procElisionWord(pos, 1, getWord(slot));
}

// 800CCA28
int Word_c::procNumberPlural(int pos) {
    const wchar_t *tag = getBuffer() + pos;
    const u8 *params = dScript::getTagParams(tag);
    dScript::getTagLength(tag);
    u8 slot = 0;
    memcpy(&slot, params, 1);
    if (slot >= LETTER_NUM_WORD_NUM) {
        return removeTag(pos);
    }
    return procPlural(pos, getNumWord(slot));
}

// 800CCAE0: Japanese only: the ruby text replaces the tag and its base text
int Word_c::procRubyTag(int pos) {
    if (getLanguage() == LANGUAGE_JP) {
        const wchar_t *tag = getBuffer() + pos;
        const u8 *params = dScript::getTagParams(tag);
        u8 len = dScript::getTagLength(tag);
        u8 base = 0;
        memcpy(&base, params, 1);
        const wchar_t *reading = (const wchar_t *)(params + 1);
        int n = len - 4;
        insert(reading, n, len + pos + base);
        erase(len + base, pos);
        return n;
    }
    return removeTag(pos);
}

} // namespace dLetter

#include "d_letter_comp.inc"
