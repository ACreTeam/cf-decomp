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
        playSe(SE_pupupu);
    }
}

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
void setItemName(int slot, const dItem::Item *item) {
    dItem::name_c name(*item);
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
