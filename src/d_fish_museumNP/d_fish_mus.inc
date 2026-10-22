// The included part of d_fish_museum.cpp: the manager process, the tanks, the fish and decoration bases.

namespace dFishMuseum {

dFishMusData_c sFishData[FISH_KIND_NUM] = {
    {dAquarium_c::IDX_RIVER, dFishMusData_c::HIT_CYLINDER, 1.5f, 5.7f, 6.4f, 34.0f, 0.01f, 0.6f, 150, 100, 150, 100, 0x5555, 35.0f, 5.0f, 15, 1.5f, 0.6f, -2.0f, -2.0f, 18.0f, 25.0f, 6.4f}, // 0 fsh_tanago
    {dAquarium_c::IDX_RIVER, dFishMusData_c::HIT_CYLINDER, 1.5f, 3.0f, 5.0f, 36.0f, 0.01f, 0.6f, 150, 100, 150, 100, 0x5555, 35.0f, 5.0f, 45, 1.5f, 0.6f, -2.0f, -2.0f, 18.0f, 25.0f, 5.0f}, // 1 fsh_oikawa
    {dAquarium_c::IDX_RIVER, dFishMusData_c::HIT_CYLINDER, 2.5f, 5.5f, 8.7f, 18.0f, 0.01f, 0.6f, 150, 100, 150, 100, 0x5555, 28.0f, 9.0f, 4, 0.7f, 0.3f, -5.0f, -5.0f, 18.0f, 50.0f, 8.7f}, // 2 fsh_funa
    {dAquarium_c::IDX_RIVER, dFishMusData_c::HIT_CAPSULE, 3.0f, 5.5f, 12.5f, 20.0f, 0.01f, 0.6f, 150, 100, 150, 100, 0x5555, 27.0f, 13.0f, 3, 0.7f, 0.2f, -10.0f, -10.0f, 15.0f, 50.0f, 12.5f}, // 3 fsh_ugui
    {dAquarium_c::IDX_RIVER, dFishMusData_c::HIT_CAPSULE, 3.8f, 8.0f, 10.0f, 7.0f, 0.01f, 0.3f, 150, 100, 150, 100, 0x5555, 19.0f, 11.0f, 3, 1.0f, 0.3f, -5.0f, -5.0f, 15.0f, 50.0f, 10.0f}, // 4 fsh_nigoi
    {dAquarium_c::IDX_RIVER, dFishMusData_c::HIT_CYLINDER, 6.0f, 7.0f, 10.0f, 27.0f, 0.007f, 0.4f, 120, 80, 260, 180, 0x4000, 29.0f, 19.0f, 2, 1.5f, 0.7f, -6.0f, -6.0f, 15.0f, 70.0f, 10.0f}, // 5 fsh_koi
    {dAquarium_c::IDX_RIVER, dFishMusData_c::HIT_CYLINDER, 6.5f, 7.0f, 10.5f, 30.0f, 0.007f, 0.4f, 120, 80, 260, 180, 0x4000, 29.0f, 28.0f, 2, 1.5f, 0.7f, -6.0f, -6.0f, 12.0f, 70.0f, 10.5f}, // 6 fsh_nishiki
    {dAquarium_c::IDX_CYL_LEFT, dFishMusData_c::HIT_CYLINDER, 3.0f, 2.4f, 5.0f, 41.0f, 0.01f, 0.5f, 150, 80, 200, 100, 0x6AAB, 53.0f, 11.0f, 20, 1.0f, 0.3f, 0.0f, -0.7f, 15.0f, 10.0f, 5.0f}, // 7 fsh_kingyo
    {dAquarium_c::IDX_CYL_LEFT, dFishMusData_c::HIT_CYLINDER, 3.0f, 2.4f, 5.0f, 41.0f, 0.01f, 0.5f, 150, 80, 200, 100, 0x6AAB, 53.0f, 11.0f, 20, 1.0f, 0.3f, 0.0f, 0.0f, 15.0f, 10.0f, 5.0f}, // 8 fsh_demekin
    {dAquarium_c::IDX_CYL_LEFT, dFishMusData_c::HIT_CYLINDER, 1.0f, 2.5f, 4.8f, 16.0f, 0.0f, 0.6f, 150, 80, 150, 100, 0x6AAB, 34.0f, 4.0f, 3, 1.0f, 0.35f, -2.0f, -2.0f, 15.0f, 10.0f, 4.5f}, // 9 fsh_medaka
    {dAquarium_c::IDX_POND, dFishMusData_c::HIT_NONE, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.1f, 1000, 800, 600, 550, 0x0000, 0.0f, 0.0f, 0, 0.8f, 0.1f, 0.0f, 0.0f, 0.0f, 120.0f, 10.0f}, // 10 fsh_zarigani
    {dAquarium_c::IDX_RIVER, dFishMusData_c::HIT_NONE, 0.0f, 0.0f, 0.0f, 42.0f, 0.0f, 0.0f, 6, 1, 588, 118, 0x0000, 0.0f, 0.0f, 0, 0.57f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 7.0f}, // 11 m_fsh_kaeru
    {dAquarium_c::IDX_RIVER, dFishMusData_c::HIT_CYLINDER, 3.0f, 4.5f, 9.0f, 0.8f, 0.002f, 0.1f, 1000, 850, 150, 80, 0x8000, 4.0f, 2.0f, 0, 0.4f, 0.2f, -5.5f, -5.5f, 20.0f, 150.0f, 8.5f}, // 12 fsh_donko
    {dAquarium_c::IDX_RIVER, dFishMusData_c::HIT_CYLINDER, 3.0f, 3.0f, 8.0f, 0.0f, 0.002f, 0.1f, 1000, 850, 150, 80, 0x8000, 4.0f, 2.0f, 0, 0.4f, 0.2f, -4.3f, -4.3f, 20.0f, 150.0f, 8.5f}, // 13 fsh_dojou
    {dAquarium_c::IDX_RIVER, dFishMusData_c::HIT_CYLINDER, 6.0f, 9.5f, 11.5f, 1.0f, 0.001f, 0.1f, 150, 100, 200, 100, 0x471C, 0.0f, 0.0f, 0, 0.4f, 0.2f, -6.0f, -6.0f, 13.0f, 150.0f, 12.5f}, // 14 fsh_namazu
    {dAquarium_c::IDX_RIVER, dFishMusData_c::HIT_BOX, 0.0f, 2.0f, 3.5f, -1.0f, 0.0f, 0.35f, 300, 270, 240, 90, 0x0000, 0.0f, 0.0f, 0, 3.0f, 0.2f, -12.0f, -12.0f, 6.0f, 25.0f, 12.0f}, // 15 fsh_unagi
    {dAquarium_c::IDX_RIVER, dFishMusData_c::HIT_CAPSULE, 5.0f, 9.0f, 15.5f, 9.0f, 0.001f, 0.1f, 700, 500, 200, 100, 0x471C, 11.0f, 7.0f, 1, 0.4f, 0.2f, -10.0f, -10.0f, 16.0f, 150.0f, 15.5f}, // 16 fsh_raigyo
    {dAquarium_c::IDX_POND, dFishMusData_c::HIT_CYLINDER, 1.5f, 7.0f, 6.5f, 25.0f, 0.01f, 0.6f, 150, 100, 130, 80, 0x5555, 36.0f, 20.0f, 7, 0.8f, 0.3f, -6.0f, -6.0f, 18.0f, 30.0f, 6.5f}, // 17 fsh_blue
    {dAquarium_c::IDX_POND, dFishMusData_c::HIT_CYLINDER, 3.0f, 6.0f, 7.7f, 26.0f, 0.01f, 0.6f, 150, 100, 130, 80, 0x5555, 37.0f, 20.0f, 7, 0.8f, 0.3f, -6.3f, -6.3f, 15.0f, 30.0f, 11.5f}, // 18 fsh_yellow
    {dAquarium_c::IDX_POND, dFishMusData_c::HIT_CAPSULE, 4.0f, 6.0f, 11.5f, 26.0f, 0.0f, 0.8f, 0, 0, 0, 0, 0x0000, 31.0f, 20.0f, 0, 0.9f, 0.0f, -6.0f, -6.0f, 22.0f, 50.0f, 11.5f}, // 19 fsh_black
    {dAquarium_c::IDX_POND, dFishMusData_c::HIT_CAPSULE, 5.5f, 5.8f, 19.6f, 20.5f, 0.0f, 0.6f, 500, 420, 200, 150, 0x0000, 20.5f, 20.5f, 0, 1.1f, 0.5f, -11.0f, -5.0f, 25.0f, 150.0f, 25.5f}, // 20 fsh_pike
    {dAquarium_c::IDX_POND, dFishMusData_c::HIT_CYLINDER, 1.5f, 3.0f, 5.0f, 37.0f, 0.0f, 0.8f, 300, 180, 40, 5, 0x5555, 39.0f, 35.0f, 2, 1.0f, 0.25f, -7.7f, -4.0f, 12.0f, 10.0f, 5.0f}, // 21 fsh_wakasagi
    {dAquarium_c::IDX_POND, dFishMusData_c::HIT_CAPSULE, 3.0f, 4.5f, 9.3f, 35.0f, 0.01f, 0.6f, 150, 100, 150, 100, 0x5555, 36.5f, 32.0f, 3, 0.7f, 0.2f, -5.0f, -5.0f, 15.0f, 30.0f, 12.0f}, // 22 fsh_ayu
    {dAquarium_c::IDX_RIVER, dFishMusData_c::HIT_CAPSULE, 3.0f, 5.5f, 9.5f, 33.0f, 0.01f, 0.6f, 150, 100, 130, 80, 0x5555, 37.0f, 23.0f, 3, 0.8f, 0.4f, -7.7f, -7.7f, 16.0f, 50.0f, 9.5f}, // 23 fsh_yamame
    {dAquarium_c::IDX_RIVER, dFishMusData_c::HIT_CAPSULE, 3.5f, 5.0f, 11.0f, 13.0f, 0.01f, 0.6f, 250, 150, 130, 80, 0x471C, 23.0f, 5.0f, 3, 0.8f, 0.4f, -6.5f, -6.5f, 18.0f, 50.0f, 11.0f}, // 24 fsh_ooiwana
    {dAquarium_c::IDX_RIVER, dFishMusData_c::HIT_CAPSULE, 4.0f, 4.0f, 14.0f, 24.0f, 0.01f, 0.6f, 250, 150, 150, 100, 0x471C, 33.0f, 11.0f, 3, 0.7f, 0.3f, -8.0f, -8.0f, 17.0f, 50.0f, 14.0f}, // 25 fsh_nijimasu
    {dAquarium_c::IDX_RIVER, dFishMusData_c::HIT_CAPSULE, 4.2f, 5.8f, 16.0f, 27.0f, 0.0f, 0.6f, 500, 420, 200, 150, 0x0000, 32.0f, 17.0f, 0, 0.7f, 0.3f, -14.0f, -7.5f, 20.0f, 150.0f, 25.5f}, // 26 fsh_itou
    {dAquarium_c::IDX_RIVER, dFishMusData_c::HIT_CAPSULE, 4.0f, 7.4f, 14.0f, 20.0f, 0.0f, 0.8f, 0, 0, 0, 0, 0x0000, 22.0f, 18.0f, 0, 1.0f, 0.0f, -13.0f, -13.0f, 33.0f, 90.0f, 17.0f}, // 27 fsh_sake
    {dAquarium_c::IDX_POND, dFishMusData_c::HIT_CAPSULE, 5.0f, 12.0f, 19.7f, 20.0f, 0.0f, 1.0f, 0, 0, 0, 0, 0x0000, 22.0f, 18.0f, 0, 1.0f, 0.0f, -14.0f, -14.0f, 35.0f, 70.0f, 20.0f}, // 28 fsh_king
    {dAquarium_c::IDX_CYL_LEFT, dFishMusData_c::HIT_CYLINDER, 1.0f, 3.0f, 4.5f, 61.0f, 0.0f, 0.8f, 150, 80, 150, 100, 0x6AAB, 84.0f, 21.0f, 7, 1.0f, 0.35f, -0.5f, -1.6f, 15.0f, 10.0f, 4.5f}, // 29 fsh_guppi
    {dAquarium_c::IDX_CYL_LEFT, dFishMusData_c::HIT_CYLINDER, 2.0f, 6.0f, 5.0f, 81.0f, 0.01f, 0.5f, 150, 80, 200, 100, 0x6AAB, 92.0f, 31.0f, 7, 0.7f, 0.25f, -2.0f, -2.0f, 10.0f, 20.0f, 5.0f}, // 30 fsh_angel
    {dAquarium_c::IDX_CYL_LEFT, dFishMusData_c::HIT_CYLINDER, 1.0f, 2.0f, 3.5f, 16.0f, 0.0f, 0.8f, 150, 80, 150, 100, 0x6AAB, 38.0f, 9.0f, 3, 1.0f, 0.35f, -0.7f, -1.0f, 15.0f, 10.0f, 3.5f}, // 31 fsh_neon
    {dAquarium_c::IDX_POND, dFishMusData_c::HIT_CYLINDER, 2.5f, 6.7f, 5.5f, 23.0f, 0.01f, 0.6f, 150, 100, 150, 100, 0x8000, 27.0f, 19.0f, 3, 0.7f, 0.3f, -3.5f, -2.1f, 20.0f, 450.0f, 7.5f}, // 32 fsh_pirania
    {dAquarium_c::IDX_POND, dFishMusData_c::HIT_CYLINDER, 4.4f, 7.0f, 11.0f, 29.0f, 0.0f, 0.1f, 1000, 700, 200, 100, 0x0000, 34.0f, 26.0f, 2, 0.3f, 0.2f, -10.0f, -10.0f, 15.0f, 150.0f, 14.0f}, // 33 fsh_arowana
    {dAquarium_c::IDX_POND, dFishMusData_c::HIT_CAPSULE, 4.7f, 8.9f, 17.2f, 24.0f, 0.0f, 1.0f, 0, 0, 0, 0, 0x0000, 28.0f, 22.0f, 0, 1.2f, 0.0f, -9.0f, -9.1f, 25.0f, 70.0f, 22.0f}, // 34 fsh_dorado
    {dAquarium_c::IDX_POND, dFishMusData_c::HIT_CAPSULE, 3.5f, 5.8f, 9.0f, 2.5f, 0.0f, 0.1f, 1000, 700, 200, 100, 0x0000, 0.0f, 0.0f, 0, 0.4f, 0.2f, -17.0f, -1.5f, 25.0f, 300.0f, 35.0f}, // 35 fsh_ga
    {dAquarium_c::IDX_POND, dFishMusData_c::HIT_CAPSULE, 4.5f, 9.5f, 11.0f, 6.0f, 0.0f, 0.05f, 500, 350, 2000, 1700, 0x0000, 0.0f, 0.0f, 0, 0.4f, 0.25f, -14.5f, -7.8f, 40.0f, 1000.0f, 42.0f}, // 36 fsh_piraruku
    {dAquarium_c::IDX_CYL_RIGHT, dFishMusData_c::HIT_CYLINDER, 2.5f, 6.0f, 1.0f, 74.0f, 0.0f, 0.0f, 5, 2, 5, 2, 0x0000, 84.0f, 49.0f, 0, 0.9f, 0.5f, 0.0f, 2.5f, 20.0f, 170.0f, 4.0f}, // 37 fsh_kurione
    {dAquarium_c::IDX_CYL_RIGHT, dFishMusData_c::HIT_CYLINDER, 7.0f, 10.0f, 7.0f, 71.0f, 0.0f, 0.0f, 5, 2, 4, 2, 0x0000, 81.0f, 31.0f, 0, 1.0f, 0.3f, 0.0f, 1.5f, 20.0f, 170.0f, 7.0f}, // 38 fsh_kurage
    {dAquarium_c::IDX_CYL_RIGHT, dFishMusData_c::HIT_CYLINDER, 1.0f, 11.0f, 2.5f, 11.0f, 0.0f, 0.0f, 6, 4, 7, 4, 0x0000, 41.0f, 1.0f, 0, 0.7f, 0.3f, 0.0f, 5.0f, 20.0f, 170.0f, 5.0f}, // 39 fsh_tatsu
    {dAquarium_c::IDX_CYL_RIGHT, dFishMusData_c::HIT_CYLINDER, 2.0f, 3.3f, 5.5f, 3.0f, 0.0f, 1.0f, 150, 80, 80, 5, 0x71C7, 8.0f, -2.0f, 3, 0.7f, 0.35f, -3.4f, -2.0f, 12.0f, 10.0f, 5.5f}, // 40 fsh_kumanomi
    {dAquarium_c::IDX_CYL_RIGHT, dFishMusData_c::HIT_CYLINDER, 1.5f, 7.0f, 8.6f, 56.0f, 0.0f, 1.2f, 150, 80, 130, 80, 0x5555, 74.0f, 11.0f, 7, 1.3f, 0.3f, -4.0f, -4.0f, 18.0f, 10.0f, 8.6f}, // 41 fsh_nanyou
    {dAquarium_c::IDX_SEA, dFishMusData_c::HIT_CYLINDER, 1.0f, 9.0f, 6.8f, 35.0f, 0.01f, 0.6f, 150, 100, 130, 80, 0x5555, 46.0f, 26.0f, 7, 1.3f, 0.3f, -3.0f, -2.5f, 18.0f, 30.0f, 6.8f}, // 42 fsh_chouchou
    {dAquarium_c::IDX_SEA, dFishMusData_c::HIT_CAPSULE, 8.0f, 30.0f, 25.5f, 28.5f, 0.007f, 0.4f, 120, 80, 260, 180, 0x5555, 30.0f, 8.0f, 3, 0.8f, 0.3f, -8.0f, -7.7f, 40.0f, 170.0f, 25.0f}, // 43 fsh_napoleon
    {dAquarium_c::IDX_SEA, dFishMusData_c::HIT_CYLINDER, 5.5f, 9.6f, 9.0f, 36.0f, 0.007f, 0.4f, 120, 80, 260, 180, 0x4000, 52.0f, 22.0f, 3, 0.7f, 0.3f, -7.0f, -7.0f, 15.0f, 25.0f, 9.0f}, // 44 fsh_minokasago
    {dAquarium_c::IDX_SEA, dFishMusData_c::HIT_CYLINDER, 7.0f, 13.0f, 8.0f, 60.0f, 0.01f, 0.6f, 150, 100, 150, 100, 0x5555, 90.0f, 19.0f, 15, 0.7f, 0.0f, 1.5f, 0.0f, 15.0f, 25.0f, 10.0f}, // 45 fsh_harisen
    {dAquarium_c::IDX_SEA, dFishMusData_c::HIT_CYLINDER, 3.0f, 4.0f, 8.9f, 69.0f, 0.0f, 1.0f, 0, 0, 0, 0, 0x0000, 0.0f, 40.0f, 0, 1.8f, 0.0f, -4.0f, -4.0f, 20.0f, 110.0f, 12.0f}, // 46 fsh_aji
    {dAquarium_c::IDX_SEA, dFishMusData_c::HIT_CYLINDER, 4.5f, 5.6f, 9.0f, 54.0f, 0.01f, 0.6f, 250, 150, 150, 100, 0x471C, 72.0f, 37.0f, 3, 0.7f, 0.3f, -5.0f, -5.0f, 26.0f, 50.0f, 9.0f}, // 47 fsh_ishidai
    {dAquarium_c::IDX_SEA, dFishMusData_c::HIT_CAPSULE, 4.0f, 6.0f, 13.5f, 54.0f, 0.01f, 0.6f, 250, 150, 150, 100, 0x471C, 72.0f, 37.0f, 3, 0.7f, 0.3f, -7.0f, -7.0f, 15.0f, 60.0f, 13.5f}, // 48 fsh_suzuki
    {dAquarium_c::IDX_SEA, dFishMusData_c::HIT_CYLINDER, 5.5f, 7.7f, 9.4f, 54.0f, 0.01f, 0.6f, 250, 150, 150, 100, 0x471C, 72.0f, 37.0f, 3, 0.7f, 0.3f, -7.0f, -7.0f, 25.0f, 50.0f, 9.4f}, // 49 fsh_tai
    {dAquarium_c::IDX_SEA, dFishMusData_c::HIT_CYLINDER, 9.0f, 3.0f, 10.0f, -1.0f, 0.0f, 1.0f, 470, 180, 1335, 450, 0x8000, 0.0f, -1.0f, 3, 1.5f, 0.1f, 0.0f, 0.0f, 10.0f, 60.0f, 12.0f}, // 50 fsh_karei
    {dAquarium_c::IDX_SEA, dFishMusData_c::HIT_CYLINDER, 9.0f, 3.0f, 10.0f, -1.0f, 0.0f, 1.0f, 470, 180, 1335, 450, 0x8000, 0.0f, -1.0f, 3, 1.5f, 0.1f, 0.0f, 0.0f, 10.0f, 60.0f, 12.0f}, // 51 fsh_hirame
    {dAquarium_c::IDX_SEA, dFishMusData_c::HIT_CAPSULE, 5.0f, 6.0f, 13.0f, 59.0f, 0.0f, 1.0f, 0, 0, 0, 0, 0x0000, 89.0f, 25.0f, 0, 1.0f, 0.0f, 0.0f, 0.0f, 20.0f, 25.0f, 13.0f}, // 52 fsh_ika
    {dAquarium_c::IDX_SEA, dFishMusData_c::HIT_NONE, 0.0f, 0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0, 0, 0, 0, 0x0000, 0.0f, 0.0f, 0, 0.0f, 0.7f, 0.0f, 0.0f, 0.0f, 0.0f, 13.0f}, // 53 fsh_tako
    {dAquarium_c::IDX_SEA, dFishMusData_c::HIT_CYLINDER, 5.0f, 5.0f, 16.0f, -0.5f, 0.0f, 0.1f, 1500, 1300, 600, 550, 0x0000, 0.0f, 0.0f, 0, 0.8f, 0.1f, 0.0f, 5.0f, 0.0f, 120.0f, 30.0f}, // 54 fsh_lobster
    {dAquarium_c::IDX_SEA, dFishMusData_c::HIT_CYLINDER, 0.0f, 40.0f, 30.0f, 0.0f, 0.0f, 0.0f, 0, 0, 0, 0, 0x0000, 0.0f, 0.0f, 0, 0.0f, 1.0f, 6.0f, 35.0f, 0.0f, 2000.0f, 35.0f}, // 55 m_fsh_utsubo
    {dAquarium_c::IDX_SEA, dFishMusData_c::HIT_CYLINDER, 5.0f, 12.0f, 11.5f, 5.0f, 0.01f, 0.05f, 500, 350, 2000, 1700, 0x471C, 0.0f, 0.0f, 0, 0.3f, 0.1f, -7.0f, -7.0f, 15.0f, 70.0f, 11.5f}, // 56 fsh_ankou
    {dAquarium_c::IDX_SEA, dFishMusData_c::HIT_CAPSULE, 6.0f, 10.5f, 19.0f, 64.0f, 0.0f, 1.8f, 0, 0, 0, 0, 0x0000, 0.0f, 40.0f, 0, 0.9f, 0.0f, -14.0f, -14.0f, 20.0f, 110.0f, 19.0f}, // 57 fsh_maguro
    {dAquarium_c::IDX_SEA, dFishMusData_c::HIT_CAPSULE, 6.0f, 10.0f, 24.6f, 72.0f, 0.0f, 1.5f, 0, 0, 0, 0, 0x0000, 0.0f, 40.0f, 0, 0.8f, 0.0f, -9.0f, -9.0f, 15.0f, 120.0f, 26.5f}, // 58 fsh_kajiki
    {dAquarium_c::IDX_SEA, dFishMusData_c::HIT_CYLINDER, 15.0f, 7.0f, 20.0f, 54.0f, 0.0f, 0.4f, 0, 0, 0, 0, 0x0000, 87.0f, 30.0f, 0, 0.5f, 0.0f, -2.1f, 12.0f, 30.0f, 600.0f, 30.0f}, // 59 fsh_ei
    {dAquarium_c::IDX_SEA, dFishMusData_c::HIT_NONE, 5.5f, 0.0f, 0.0f, 60.0f, 0.0f, 0.15f, 0, 0, 0, 0, 0x0000, 0.0f, 0.0f, 0, 0.7f, 0.0f, 0.0f, 0.0f, 20.0f, 0.0f, 27.0f}, // 60 fsh_manbou
    {dAquarium_c::IDX_SEA, dFishMusData_c::HIT_CAPSULE, 12.0f, 14.6f, 30.0f, 69.5f, 0.0f, 1.2f, 0, 0, 0, 0, 0x0000, 0.0f, 0.0f, 0, 0.9f, 0.0f, -20.0f, -19.0f, 30.0f, 800.0f, 50.0f}, // 61 fsh_shumoku
    {dAquarium_c::IDX_SEA, dFishMusData_c::HIT_CAPSULE, 12.5f, 21.3f, 32.0f, 77.0f, 0.0f, 1.5f, 0, 0, 0, 0, 0x0000, 0.0f, 0.0f, 0, 0.3f, 0.0f, -20.0f, -23.0f, 32.0f, 1000.0f, 60.0f}, // 62 fsh_same
    {dAquarium_c::IDX_SEA, dFishMusData_c::HIT_CAPSULE, 6.5f, 10.0f, 21.0f, 6.3f, 0.01f, 0.05f, 500, 350, 2000, 1700, 0x38E4, 0.0f, 0.0f, 0, 0.6f, 0.3f, -8.0f, -8.0f, 26.0f, 100.0f, 21.5f}, // 63 fsh_siira
};

// The debug copy of one fish's data (never written: only the bubble interval is used).
struct dFishMusHIO_c {
    // mFishData is initialised first, so it reads mFishIdx before mFishIdx(0) runs (zero in .bss anyway).
    dFishMusHIO_c()
        : mFishData(sFishData[mFishIdx]), _5C(0), mFishIdx(0), _5E(0), _5F(0), _60(0), _61(0), _62(0),
          _63(0), _64(0), mBubbleTimeMax(240), mBubbleTimeMin(60) {}
    ~dFishMusHIO_c() {}

    /* 0x00 */ dFishMusData_c mFishData;
    /* 0x5C */ u8 _5C;
    /* 0x5D */ u8 mFishIdx;
    /* 0x5E */ u8 _5E;
    /* 0x5F */ u8 _5F;
    /* 0x60 */ u8 _60;
    /* 0x61 */ u8 _61;
    /* 0x62 */ u8 _62;
    /* 0x63 */ u8 _63;
    /* 0x64 */ u8 _64;
    /* 0x68 */ int mBubbleTimeMax;
    /* 0x6C */ int mBubbleTimeMin;
}; // size 0x70

struct CorrectObjList_c {
    dAquariumCorrectObj_c **mpObjs;
    u32 mNum;
};

extern dAquarium_c *sAquariums[dAquarium_c::IDX_NUM];
enum { BUBBLE_NUM = 14 };
extern dAquariumEffBubble_c *sBubbles[BUBBLE_NUM];
extern CorrectObjList_c sCorrectObjLists[dAquarium_c::IDX_NUM];
extern dFishMusHIO_c l_HIO;

// 0x85C
void *dFishMus_c_classInit() {
    return new dFishMus_c();
}

fProf::fBaseProfile_c g_profile_FISH_MUSEUM = {&dFishMus_c_classInit, fProf::FISH_MUSEUM,
                                               fProf::DRAW_ORDER::FISH_MUSEUM};

} // namespace dFishMuseum

// 0x88C
BOOL dFishMusBaseMove_c::create(nw4r::g3d::ResFile res) {
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    int nodeNum = 3;
    switch (mFishIdx) {
    case FISH_NAPOLEON:
        nodeNum = 4;
        break;
    }
    mMdl.create(mdl, &mAllocator, nodeNum, 0, 1, 0, 1, NULL);
    mAnm.create(mdl, res.GetResAnmChr("swim"), &mAllocator, NULL);
    mMdl.setAnm(mAnm);
    mpMdl = &mMdl;
    calcMtx();
    getAquarium()->addFish(this);
    getAquarium()->addHitFish(this);
    init();
    return TRUE;
}

#include "d_fish_mus_base_move.inc"
#include "d_fish_mus_big.inc"
#include "d_fish_mus_common_move.inc"
#include "d_fish_mus_cuttlefish.inc"
#include "d_fish_mus_eel.inc"
#include "d_fish_mus_elegance.inc"
#include "d_fish_mus_fast.inc"
#include "d_fish_mus_flatfish.inc"
#include "d_fish_mus_float.inc"
#include "d_fish_mus_frog.inc"
#include "d_fish_mus_globefish.inc"
#include "d_fish_mus_keep_swimming.inc"
#include "d_fish_mus_koi.inc"
#include "d_fish_mus_lobster.inc"
#include "d_fish_mus_moray.inc"
#include "d_fish_mus_octopus.inc"
#include "d_fish_mus_piranha.inc"
#include "d_fish_mus_pufferfish.inc"
#include "d_fish_mus_ray.inc"
#include "d_fish_mus_slow.inc"
#include "d_fish_mus_small.inc"
#include "d_fish_mus_obj.inc"

namespace dFishMuseum {

dFishMusHIO_c l_HIO;

} // namespace dFishMuseum

// 0x9A0
dFishMus_c::dFishMus_c() : _26A4(true), _26A5(true) {
    for (int i = 0; i < FISH_NUM; i++) {
        mFirstSpawn[i] = true;
    }
}

// 0xB1C
dFishMus_c::~dFishMus_c() {}

// 0xBB4
int dFishMus_c::create() {
    EGG::Heap *parent = dHeap::localHeap_p;
    BOOL ok = TRUE;
    if (mResHeap == NULL) {
        mResHeap = mHeap::createFrmHeap(0x140000, parent, "dFishMus_c::m_ResHeap", 0x20, mHeap::OPT_NONE);
    }
    for (int i = 0; i < FISH_NUM; i++) {
        if (!mFishBanks[i].loadFish(mResHeap, i)) {
            ok = FALSE;
        }
    }
    if (!ok) {
        return ok;
    }
    for (int i = 0; i < OBJ_KIND_NUM; i++) {
        if (!mObjBanks[i].loadObj(mResHeap, i)) {
            ok = FALSE;
        }
    }
    if (!ok) {
        return ok;
    }
    ok = dBgUtil::isBgiReady();
    if (!ok) {
        return ok;
    }
    for (int i = 0; i < dAquarium_c::IDX_NUM; i++) {
        dFishMuseum::sAquariums[i]->init();
    }
    for (int i = 0; i < dFishMuseum::BUBBLE_NUM; i++) {
        dFishMuseum::sBubbles[i]->init();
    }
    for (int i = 0; i < dAquarium_c::IDX_NUM; i++) {
        dAquariumCorrectObj_c **objs = dFishMuseum::sCorrectObjLists[i].mpObjs;
        if (objs != NULL) {
            for (u32 j = 0; j < dFishMuseum::sCorrectObjLists[i].mNum; j++) {
                (*objs)->init();
                dFishMuseum::sAquariums[i]->addObj(*objs);
                objs++;
            }
        }
    }
    mFishList = NULL;
    for (int i = 0; i < FISH_SLOT_NUM; i++) {
        mChildHeap[i] = mHeap::createFrmHeap(0x1400, parent, "dFishMus_c::m_childHeap", 0x20, mHeap::OPT_NONE);
        if (mChildHeap[i] == NULL) {
            parent->dump();
        }
        mFishSlotUsed[i] = false;
    }
    mObjList = NULL;
    for (int i = 0; i < OBJ_NUM; i++) {
        mObjChildHeap[i] = mHeap::createFrmHeap(0x1400, parent, "dFishMus_c::m_objChildHeap", 0x20, mHeap::OPT_NONE);
        if (mObjChildHeap[i] == NULL) {
            parent->dump();
        }
        mObjSlotUsed[i] = false;
    }
    createAllFish();
    createAllObj();
    sInstance = this;
    return ok;
}

// 0xE44
int dFishMus_c::execute() {
    holdSe(SE_bubble);
    for (dFishMusBaseObj_c **link = &mObjList; *link != NULL; link = &(*link)->mNext) {
        dFishMusBaseObj_c *&obj = *link;
        obj->execute();
    }
    for (dFishMusBase_c **link = &mFishList; *link != NULL; link = &(*link)->mNext) {
        dFishMusBase_c *&fish = *link;
        fish->execute();
        fish->mPush.set(0.0f, 0.0f, 0.0f);
        fish->mHitFrames++;
    }
    for (int i = 0; i < dFishMuseum::BUBBLE_NUM; i++) {
        dFishMuseum::sBubbles[i]->execute();
    }
    for (int i = 0; i < dAquarium_c::IDX_NUM; i++) {
        dFishMuseum::sAquariums[i]->execute();
    }
    return SUCCEEDED;
}

// 0xF58
int dFishMus_c::draw() {
    for (dFishMusBase_c **link = &mFishList; *link != NULL; link = &(*link)->mNext) {
        dFishMusBase_c *&fish = *link;
        if (!fish->isOutOfView()) {
            fish->draw();
        }
    }
    for (dFishMusBaseObj_c **link = &mObjList; *link != NULL; link = &(*link)->mNext) {
        dFishMusBaseObj_c *&obj = *link;
        if (!obj->isOutOfView()) {
            obj->draw();
        }
    }
    return SUCCEEDED;
}

// 0x1004
int dFishMus_c::doDelete() {
    BOOL ok = TRUE;
    for (int i = 0; i < FISH_NUM; i++) {
        if (!mFishBanks[i].unload(FALSE)) {
            ok = FALSE;
        }
    }
    if (!ok) {
        return ok;
    }
    for (int i = 0; i < OBJ_KIND_NUM; i++) {
        if (!mObjBanks[i].unload(FALSE)) {
            ok = FALSE;
        }
    }
    if (!ok) {
        return ok;
    }
    if (mResHeap != NULL) {
        mHeap::destroyFrmHeap(mResHeap);
        mResHeap = NULL;
    }
    deleteAllFish();
    deleteAllObj();
    for (int i = 0; i < FISH_SLOT_NUM; i++) {
        if (mChildHeap[i] != NULL) {
            mHeap::destroyFrmHeap(mChildHeap[i]);
            mChildHeap[i] = NULL;
        }
    }
    for (int i = 0; i < OBJ_NUM; i++) {
        if (mObjChildHeap[i] != NULL) {
            mHeap::destroyFrmHeap(mObjChildHeap[i]);
            mObjChildHeap[i] = NULL;
        }
    }
    sInstance = NULL;
    return ok;
}

// 0x114C
void dFishMus_c::addFish(dFishMusBase_c *fish) {
    dFishMusBase_c **link = &mFishList;
    while (*link != NULL) {
        link = &(*link)->mNext;
    }
    *link = fish;
}

// 0x116C
void dFishMus_c::removeFish(dFishMusBase_c *fish) {
    dFishMusBase_c **link = &mFishList;
    while (*link != fish) {
        link = &(*link)->mNext;
    }
    if (*link == fish) {
        *link = fish->mNext;
    }
}

// 0x1194
void dFishMus_c::addObj(dFishMusBaseObj_c *obj) {
    dFishMusBaseObj_c **link = &mObjList;
    while (*link != NULL) {
        link = &(*link)->mNext;
    }
    *link = obj;
}

// 0x11B4
void dFishMus_c::removeObj(dFishMusBaseObj_c *obj) {
    dFishMusBaseObj_c **link = &mObjList;
    while (*link != obj) {
        link = &(*link)->mNext;
    }
    if (*link == obj) {
        *link = obj->mNext;
    }
}

namespace dFishMuseum {

inline dFishMusData_c *getFishData(int idx) { return &sFishData[idx]; }

// Needed to match, probably a stand-in for a larger spawn parameter block: the index must live in a 32-bit member of a
// local object built before the loop (only that gives it the optimizer temp behind the target's `mr r28, r5`), with
// the tank lookup reading the member and the other uses going through an int copy.
struct dFishMusSpawn_c {
    dFishMusSpawn_c(s32 aquariumIdx) : mAquariumIdx(aquariumIdx) {}

    /* 0x0 */ s32 mAquariumIdx;
};

} // namespace dFishMuseum

// 0x11DC
BOOL dFishMus_c::createFish(int fishIdx, int aquariumIdx) {
    dFishMuseum::dFishMusSpawn_c spawn(aquariumIdx);
    for (int slot = 0; slot < FISH_SLOT_NUM; slot++) {
        dItem::Item item(dItem::ITEM_IDX_BITTERLING, fishIdx, FALSE);
        if (!mFishSlotUsed[slot]) {
            mFishSlotUsed[slot] = true;
            dFishMusData_c *data = dFishMuseum::getFishData(fishIdx);
            dFishMusBase_c *fish = dFishMuseum::sFishCreate[fishIdx]->mCreate(mChildHeap[slot]);
            dAquarium_c *aquarium = dFishMuseum::sAquariums[spawn.mAquariumIdx];
            mVec3_c pos;
            if (aquarium->getType() == dAquarium_c::TYPE_BOX) {
                mVec3_c min = ((dAquariumBox_c *)aquarium)->getMin();
                mVec3_c max = ((dAquariumBox_c *)aquarium)->getMax();
                if (mFirstSpawn[fishIdx]) {
                    mFirstSpawn[fishIdx] = false;
                    dFishMuseum::sFishData[fishIdx].mSwimY += min.y;
                    data->mSwimYMax += min.y;
                    data->mSwimYMin += min.y;
                }
                pos.x = cM::rndRange(min.x, max.x);
                pos.z = cM::rndRange(min.z, max.z);
            } else if (aquarium->getType() == dAquarium_c::TYPE_CYLINDER) {
                mVec3_c center = ((dAquariumCyl_c *)aquarium)->getCenter();
                if (mFirstSpawn[fishIdx]) {
                    mFirstSpawn[fishIdx] = false;
                    dFishMuseum::sFishData[fishIdx].mSwimY += center.y;
                    data->mSwimYMax += center.y;
                    data->mSwimYMin += center.y;
                }
                pos.x = center.x;
                pos.z = center.z;
            }
            // The target draws a random yaw and discards it, then uses the aquarium index as the yaw.
            dFishMuseum::degToAng(cM::rndRange(mAng::MinDegree, mAng::MaxDegree));
            int idx = spawn.mAquariumIdx;
            fish->mPos = fish->mHomePos = pos;
            fish->mHomeAngle.set(0, idx, 0);
            fish->mAngle.set(0, idx, 0);
            fish->mAllocator.attach(mChildHeap[slot], 0x20);
            fish->mPos.y = data->mSwimY;
            fish->mSlot = slot;
            fish->mFishIdx = fishIdx;
            fish->mAquariumIdx = idx;
            fish->mPush.set(0.0f, 0.0f, 0.0f);
            fish->create(nw4r::g3d::ResFile(mFishBanks[fishIdx].getData()));
            addFish(fish);
            return TRUE;
        }
    }
    return FALSE;
}

// 0x1488
BOOL dFishMus_c::createAllFish() {
    for (int i = 0; i < FISH_NUM; i++) {
        dItem::Item item(dItem::ITEM_IDX_BITTERLING, i, FALSE);
        if (dSaveData_c::getTown()->mMuseum.isDonated(item)) {
            createFish(i, dFishMuseum::sFishData[i].mAquariumIdx);
        }
    }
    return FALSE;
}

char dFishMuseum::sDebugFishName[] = "fsh_koi";

// 0x1520
BOOL dFishMus_c::deleteFish(dFishMusBase_c *fish) {
    if (fish != NULL) {
        int slot = fish->mSlot;
        if (fish->doDelete()) {
            mFishSlotUsed[slot] = false;
            removeFish(fish);
            fish->~dFishMusBase_c();
            mChildHeap[slot]->free(MEM_FRM_HEAP_FREE_ALL);
            return TRUE;
        }
    }
    return FALSE;
}

// 0x15D4
void dFishMus_c::deleteAllFish() {
    dFishMusBase_c *fish;
    do {
        fish = mFishList;
        deleteFish(fish);
    } while (fish != NULL);
}

// 0x161C
mVec3_c dFishMus_c::getLocatorPos(const char *name, int idx) {
    static char nodeName[0x20];
    mVec3_c pos;
    snprintf(nodeName, sizeof(nodeName), "locator_%s_%.2d", name, idx);
    dBgUtil::getNodePos(&pos, nodeName, -1, -1);
    return pos;
}

// 0x1688
BOOL dFishMus_c::createLotus(int kind, int *objIdx) {
    if (!mObjSlotUsed[*objIdx]) {
        mObjSlotUsed[*objIdx] = true;
        dFishMusObjLotus_c *obj = new (mObjChildHeap[*objIdx]) dFishMusObjLotus_c();
        dAquariumBox_c *aquarium = (dAquariumBox_c *)dFishMuseum::sAquariums[dAquarium_c::IDX_RIVER];
        mVec3_c min = aquarium->getMin();
        mVec3_c max = aquarium->getMax();
        mVec3_c pos;
        pos.x = min.x + 13.0f * (max.x - min.x) / 16.0f;
        pos.y = 2.1f + max.y;
        pos.z = 0.5f * (min.z + max.z);
        obj->mAllocator.attach(mObjChildHeap[*objIdx], 0x20);
        obj->mPos = pos;
        obj->mAngle.set(0, 0, 0);
        obj->mHomeAngle.set(0, 0, 0);
        obj->mSlot = *objIdx;
        obj->create(nw4r::g3d::ResFile(mObjBanks[kind].getData()));
        addObj(obj);
        (*objIdx)++;
        return TRUE;
    }
    return FALSE;
}

// 0x1820
BOOL dFishMus_c::createWakame00(int kind, int *objIdx) {
    BOOL ok = TRUE;
    for (int i = 0; i < 4; i++) {
        if (!mObjSlotUsed[*objIdx]) {
            mObjSlotUsed[*objIdx] = true;
            dFishMusObjWakame00_c *obj = new (mObjChildHeap[*objIdx]) dFishMusObjWakame00_c();
            obj->mAllocator.attach(mObjChildHeap[*objIdx], 0x20);
            obj->mAnmRate = 0.1f;
            obj->mAnmFrameMax = 40.0f;
            obj->mPos = getLocatorPos("wakame00", i);
            obj->mAngle.set(0, 0, 0);
            obj->mHomeAngle.set(0, 0, 0);
            obj->mCullRadius = 15.0f;
            obj->mSlot = *objIdx;
            obj->create(nw4r::g3d::ResFile(mObjBanks[kind].getData()));
            addObj(obj);
            ok &= TRUE;
            (*objIdx)++;
        }
    }
    return ok != FALSE;
}

// 0x19C0
BOOL dFishMus_c::createWakame01(int kind, int *objIdx) {
    BOOL ok = TRUE;
    for (int i = 0; i < 4; i++) {
        if (!mObjSlotUsed[*objIdx]) {
            mObjSlotUsed[*objIdx] = true;
            dFishMusObjWakame01_c *obj = new (mObjChildHeap[*objIdx]) dFishMusObjWakame01_c();
            obj->mAllocator.attach(mObjChildHeap[*objIdx], 0x20);
            obj->mAnmRate = 0.1f;
            obj->mAnmFrameMax = 40.0f;
            obj->mPos = getLocatorPos("wakame01", i);
            obj->mAngle.set(0, 0, 0);
            obj->mHomeAngle.set(0, 0, 0);
            obj->mCullRadius = 33.0f;
            obj->mSlot = *objIdx;
            obj->create(nw4r::g3d::ResFile(mObjBanks[kind].getData()));
            addObj(obj);
            ok &= TRUE;
            (*objIdx)++;
        }
    }
    return ok != FALSE;
}

// 0x1B60
BOOL dFishMus_c::createWaterWeed00(int kind, int *objIdx) {
    BOOL ok = TRUE;
    for (int i = 0; i < 16; i++) {
        if (!mObjSlotUsed[*objIdx]) {
            mObjSlotUsed[*objIdx] = true;
            dFishMusObjWaterWeed00_c *obj = new (mObjChildHeap[*objIdx]) dFishMusObjWaterWeed00_c();
            obj->mAllocator.attach(mObjChildHeap[*objIdx], 0x20);
            obj->mAnmRate = 0.1f;
            obj->mAnmFrameMax = 40.0f;
            obj->mPos = getLocatorPos("mizukusa00", i);
            obj->mAngle.set(0, 0, 0);
            obj->mHomeAngle.set(0, 0, 0);
            obj->mCullRadius = 25.0f;
            obj->mSlot = *objIdx;
            obj->create(nw4r::g3d::ResFile(mObjBanks[kind].getData()));
            addObj(obj);
            ok &= TRUE;
            (*objIdx)++;
        }
    }
    return ok != FALSE;
}

// 0x1D00
BOOL dFishMus_c::createWaterWeed01(int kind, int *objIdx) {
    BOOL ok = TRUE;
    for (int i = 0; i < 8; i++) {
        if (!mObjSlotUsed[*objIdx]) {
            mObjSlotUsed[*objIdx] = true;
            dFishMusObjWaterWeed01_c *obj = new (mObjChildHeap[*objIdx]) dFishMusObjWaterWeed01_c();
            obj->mAllocator.attach(mObjChildHeap[*objIdx], 0x20);
            obj->mAnmRate = 0.1f;
            obj->mAnmFrameMax = 40.0f;
            obj->mPos = getLocatorPos("mizukusa01", i);
            obj->mAngle.set(0, 0, 0);
            obj->mHomeAngle.set(0, 0, 0);
            obj->mCullRadius = 20.0f;
            obj->mSlot = *objIdx;
            obj->create(nw4r::g3d::ResFile(mObjBanks[kind].getData()));
            addObj(obj);
            ok &= TRUE;
            (*objIdx)++;
        }
    }
    return ok != FALSE;
}

// 0x1EA0
BOOL dFishMus_c::createSeaAnemone(int kind, int *objIdx) {
    BOOL ok = TRUE;
    for (int i = 0; i < 3; i++) {
        if (!mObjSlotUsed[*objIdx]) {
            mObjSlotUsed[*objIdx] = true;
            dFishMusObjSeaAnemone_c *obj = new (mObjChildHeap[*objIdx]) dFishMusObjSeaAnemone_c();
            obj->mAllocator.attach(mObjChildHeap[*objIdx], 0x20);
            obj->mAnmRate = 1.0f;
            obj->mAnmFrameMax = 400.0f;
            obj->mPos = getLocatorPos("anemone", i);
            obj->mAngle.set(0, 0, 0);
            obj->mHomeAngle.set(0, 0, 0);
            obj->mCullRadius = 20.0f;
            obj->mSlot = *objIdx;
            obj->create(nw4r::g3d::ResFile(mObjBanks[kind].getData()));
            addObj(obj);
            ok &= TRUE;
            (*objIdx)++;
        }
    }
    return ok != FALSE;
}

// 0x2040
BOOL dFishMus_c::createOctopusPod(int kind, int *objIdx) {
    BOOL ok = TRUE;
    for (int i = 0; i < 1; i++) {
        if (!mObjSlotUsed[*objIdx]) {
            mObjSlotUsed[*objIdx] = true;
            dFishMusObjOctopusPod_c *obj = new (mObjChildHeap[*objIdx]) dFishMusObjOctopusPod_c();
            obj->mAllocator.attach(mObjChildHeap[*objIdx], 0x20);
            obj->mPos = getLocatorPos("takotsubo", i);
            obj->mAngle.set(0, 0, 0);
            obj->mHomeAngle.set(0, 0, 0);
            obj->mCullRadius = 18.0f;
            obj->mSlot = *objIdx;
            obj->create(nw4r::g3d::ResFile(mObjBanks[kind].getData()));
            addObj(obj);
            ok &= TRUE;
            (*objIdx)++;
        }
    }
    return ok != FALSE;
}

// 0x219C
BOOL dFishMus_c::createAllObj() {
    int objIdx = 0;
    for (int kind = 0; kind < OBJ_KIND_NUM; kind++) {
        if (kind == OBJ_LOTUS) {
            createLotus(kind, &objIdx);
        } else if (kind == OBJ_WAKAME00) {
            createWakame00(kind, &objIdx);
        } else if (kind == OBJ_WAKAME01) {
            createWakame01(kind, &objIdx);
        } else if (kind == OBJ_WATER_WEED00) {
            createWaterWeed00(kind, &objIdx);
        } else if (kind == OBJ_WATER_WEED01) {
            createWaterWeed01(kind, &objIdx);
        } else if (kind == OBJ_SEA_ANEMONE) {
            createSeaAnemone(kind, &objIdx);
        } else if (kind == OBJ_OCTOPUS_POD) {
            createOctopusPod(kind, &objIdx);
            return TRUE;
        } else {
            return FALSE;
        }
    }
    return FALSE;
}

// 0x22B8
BOOL dFishMus_c::deleteObj(dFishMusBaseObj_c *obj) {
    if (obj != NULL) {
        int slot = obj->mSlot;
        if (obj->doDelete()) {
            mObjSlotUsed[slot] = false;
            removeObj(obj);
            obj->~dFishMusBaseObj_c();
            mChildHeap[slot]->free(MEM_FRM_HEAP_FREE_ALL); // (sic) the fish heap, not mObjChildHeap
            return TRUE;
        }
    }
    return FALSE;
}

// 0x236C
void dFishMus_c::deleteAllObj() {
    dFishMusBaseObj_c *obj;
    do {
        obj = mObjList;
        deleteObj(obj);
    } while (obj != NULL);
}

namespace dFishMuseum {

inline f32 clamp01(f32 x) {
    return x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x);
}

// 0x23B4
BOOL hitCylFish(dFishMusBase_c *a, dFishMusBase_c *b, mVec3_c *pushA, mVec3_c *pushB) {
    dFishMusData_c *dataA = &sFishData[a->mFishIdx];
    dFishMusData_c *dataB = &sFishData[b->mFishIdx];
    mVec3_c posA;
    a->getCenterPos(posA);
    mVec3_c posB;
    b->getCenterPos(posB);
    mVec3_c diff = posB - posA;
    f32 dy = nw4r::math::FAbs(diff.y);
    f32 height = 0.5f * (dataA->mHitHeight + dataB->mHitHeight);
    f32 radius = dataA->mHitLength + dataB->mHitLength;
    if (dy < height) {
        f32 distSq = diff.x * diff.x + diff.z * diff.z;
        if (distSq < radius * radius) {
            f32 ratio = 2.0f * (height - dy) / height;
            ratio = ratio > 1.0f ? 1.0f : ratio;
            f32 depth = radius * ratio - nw4r::math::FSqrt(distSq);
            f32 len = depth < 0.0f ? 0.0f : depth;
            f32 weightSum = dataA->mWeight + dataB->mWeight;
            f32 rateA = dataB->mWeight / weightSum;
            f32 rateB = dataA->mWeight / weightSum;
            diff.y = 0.0f;
            if (!diff.normalizeRS()) {
                diff.set(0.0f, 0.0f, 1.0f);
            }
            *pushA = diff * (-len * rateA);
            *pushB = diff * len * rateB;
            return TRUE;
        }
    }
    return FALSE;
}

// 0x26C8
BOOL hitCylCapsuleFish(dFishMusBase_c *a, dFishMusBase_c *b, mVec3_c *pushA, mVec3_c *pushB) {
    dFishMusData_c *dataA = &sFishData[a->mFishIdx];
    dFishMusData_c *dataB = &sFishData[b->mFishIdx];
    mVec3_c posA;
    a->getCenterPos(posA);
    mVec3_c start;
    b->getCenterPos(start);
    mVec3_c end = start;
    f32 halfLen = dataB->mHitLength - dataB->mHitWidth;
    mVec3_c axis(0.0f, 0.0f, halfLen);
    f32 segLen = 2.0f * halfLen;
    axis.rotY(b->mAngle.y);
    start += axis;
    end -= axis;
    f32 dy = nw4r::math::FAbs(posA.y - start.y);
    f32 height = 0.5f * (dataA->mHitHeight + dataB->mHitHeight);
    f32 radius = dataA->mHitLength + dataB->mHitWidth;
    if (dy < height) {
        mVec3_c toPos = posA - end;
        toPos.y = 0.0f;
        mVec3_c seg = axis * 2.0f;
        f32 dot = toPos.dot(seg);
        mVec3_c nearest;
        if (dot < 0.0f) {
            nearest = end;
        } else if (dot > segLen * segLen) {
            nearest = start;
        } else {
            f32 t = dot / (segLen * segLen);
            nearest = start * t + end * (1.0f - t);
        }
        mVec3_c d = nearest - posA;
        f32 distSq = d.x * d.x + d.z * d.z;
        if (distSq < radius * radius) {
            f32 ratio = 2.0f * (height - dy) / height;
            ratio = ratio > 1.0f ? 1.0f : ratio;
            f32 depth = radius * ratio - nw4r::math::FSqrt(distSq);
            f32 len = depth < 0.0f ? 0.0f : depth;
            f32 weightSum = dataA->mWeight + dataB->mWeight;
            f32 rateA = dataB->mWeight / weightSum;
            f32 rateB = dataA->mWeight / weightSum;
            d.y = 0.0f;
            if (!d.normalizeRS()) {
                d.set(0.0f, 0.0f, 1.0f);
            }
            *pushA = d * (-len * rateA);
            *pushB = d * len * rateB;
            return TRUE;
        }
    }
    return FALSE;
}

// 0x2B8C
BOOL hitCylBoxFish(dFishMusBase_c *a, dFishMusBase_c *b, mVec3_c *pushA, mVec3_c *pushB) {
    dFishMusData_c *dataB = &sFishData[b->mFishIdx];
    dFishMusData_c *dataA = &sFishData[a->mFishIdx];
    dFishMusEel_c *eel = (dFishMusEel_c *)b;
    mVec3_c posA;
    a->getCenterPos(posA);
    mVec3_c posB;
    b->getCenterPos(posB);
    mVec3_c diff = posA - posB;
    f32 dy = nw4r::math::FAbs(diff.y);
    f32 height = 0.5f * (dataB->mHitHeight + dataA->mHitHeight);
    f32 radius = dataB->mHitLength + dataA->mHitLength;
    BOOL hit = FALSE;
    pushA->set(0.0f, 0.0f, 0.0f);
    pushB->set(0.0f, 0.0f, 0.0f);
    if (dy < height) {
        // Node indices go to getHitCorner, which masks them with & 3: only corners 0 and 2 are tested.
        for (int i = 0; i < dFishMusEel_c::NODE_NUM; i += 2) {
            mVec3_c corner = eel->getHitCorner(i);
            mVec3_c d = posA - corner;
            f32 ratio, distSq, len, rateB, rateA, weightSum;
            distSq = d.x * d.x + d.z * d.z;
            if (distSq < radius * radius) {
                ratio = 2.0f * (height - dy) / height;
                if (ratio > 1.0f) {
                    ratio = 1.0f;
                }
                len = radius * ratio - nw4r::math::FSqrt(distSq);
                if (len < 0.0f) {
                    len = 0.0f;
                }
                weightSum = dataB->mWeight + dataA->mWeight;
                rateB = dataA->mWeight / weightSum;
                rateA = dataB->mWeight / weightSum;
                d.y = 0.0f;
                if (!d.normalizeRS()) {
                    d.set(0.0f, 0.0f, 1.0f);
                }
                *pushA += d * len * rateA;
                *pushB += d * (-len * rateB);
                hit = TRUE;
            }
        }
    }
    return hit;
}

// 0x2F74
BOOL hitCapsuleCylFish(dFishMusBase_c *a, dFishMusBase_c *b, mVec3_c *pushA, mVec3_c *pushB) {
    return hitCylCapsuleFish(b, a, pushB, pushA);
}

// 0x2F90
BOOL hitCapsuleFish(dFishMusBase_c *a, dFishMusBase_c *b, mVec3_c *pushA, mVec3_c *pushB) {
    dFishMusData_c *dataA = &sFishData[a->mFishIdx];
    dFishMusData_c *dataB = &sFishData[b->mFishIdx];
    mVec3_c startA, endA, startB, endB;
    a->getCenterPos(startA);
    b->getCenterPos(startB);
    endA = startA;
    endB = startB;
    f32 halfA = dataA->mHitLength - dataA->mHitWidth;
    f32 halfB = dataB->mHitLength - dataB->mHitWidth;
    mVec3_c axisA(0.0f, 0.0f, halfA);
    mVec3_c axisB(0.0f, 0.0f, halfB);
    mVec3_c closestA, closestB;
    axisA.rotY(a->mAngle.y);
    axisB.rotY(b->mAngle.y);
    startA += axisA;
    endA -= axisA;
    startB += axisB;
    endB -= axisB;
    f32 dy = nw4r::math::FAbs(startA.y - startB.y);
    f32 height = 0.5f * (dataA->mHitHeight + dataB->mHitHeight);
    f32 radius = dataA->mHitWidth + dataB->mHitWidth;
    if (dy < height) {
        mVec3_c v = startA - startB;
        mVec3_c segA = endA - startA;
        mVec3_c segB = endB - startB;
        f32 aa = segA.x * segA.x + segA.z * segA.z;
        f32 ab = segA.x * segB.x + segA.z * segB.z;
        f32 bb = segB.x * segB.x + segB.z * segB.z;
        f32 va = v.dot(segA);
        f32 vb = v.dot(segB);
        f32 denom = aa * bb - ab * ab;
        f32 s;
        if (denom <= FLT_EPSILON) {
            s = 0.0f;
        } else {
            s = clamp01((-va * bb + vb * ab) / denom);
        }
        f32 t = (vb + ab * s) / bb;
        if (t < 0.0f) {
            t = 0.0f;
            s = clamp01(-va / aa);
        } else if (t > 1.0f) {
            t = 1.0f;
            s = clamp01((ab - va) / aa);
        }
        if (s < 1.0f && s > 0.0f && t < 1.0f && t > 0.0f) {
            f32 push = 0.5f * (dy - height);
            if (startA.y > startB.y) {
                pushA->set(0.0f, -push, 0.0f);
                pushB->set(0.0f, push, 0.0f);
            } else {
                pushA->set(0.0f, push, 0.0f);
                pushB->set(0.0f, -push, 0.0f);
            }
            return TRUE;
        }
        closestA = startA + segA * s;
        closestB = startB + segB * t;
        mVec3_c d = closestB - closestA;
        f32 distSq = d.x * d.x + d.z * d.z;
        if (distSq < radius * radius) {
            f32 ratio = 2.0f * (height - dy) / height;
            ratio = ratio > 1.0f ? 1.0f : ratio;
            f32 depth = radius * ratio - nw4r::math::FSqrt(distSq);
            f32 len = depth < 0.0f ? 0.0f : depth;
            f32 weightSum = dataA->mWeight + dataB->mWeight;
            f32 rateA = dataB->mWeight / weightSum;
            f32 rateB = dataA->mWeight / weightSum;
            d.y = 0.0f;
            if (!d.normalizeRS()) {
                d.set(0.0f, 0.0f, 1.0f);
            }
            *pushA = d * (-len * rateA);
            *pushB = d * len * rateB;
            return TRUE;
        }
    }
    return FALSE;
}

// 0x3674
BOOL hitBoxCylFish(dFishMusBase_c *a, dFishMusBase_c *b, mVec3_c *pushA, mVec3_c *pushB) {
    return hitCylBoxFish(b, a, pushB, pushA);
}

// 0x3690
HitFishFunc getHitFishFunc(int typeA, int typeB) {
    return sHitFishFuncs[typeA][typeB];
}

HitFishFunc sHitFishFuncs[3][3] = {
    {hitCylFish, hitCylCapsuleFish, hitCylBoxFish},
    {hitCapsuleCylFish, hitCapsuleFish, NULL},
    {hitBoxCylFish, NULL, NULL},
};

dAquariumBox_c sAquariumRiver("left_suisou", mVec3_c(120.0f, 36.5f, 41.0f));
dAquariumBox_c sAquariumPond("right_suisou", mVec3_c(120.0f, 36.5f, 41.0f));
dAquariumBox_c sAquariumSea("sea_suisou", mVec3_c(471.0f, 109.0f, 42.0f), mVec3_c(7.0f, 0.0f, 0.0f));
dAquariumCyl_c sAquariumCylLeft("left_cylinder_suisou", 26.0f, 112.0f);
dAquariumCyl_c sAquariumCylRight("right_cylinder_suisou", 26.0f, 114.0f);

dAquariumEffBubble_c sBubble0("af_msm_abuku_flesh", mVec3_c(70.0f, 0.0f, -20.0f), dAquarium_c::IDX_RIVER);
dAquariumEffBubble_c sBubble1("af_msm_abuku_flesh", mVec3_c(-100.0f, 0.0f, -35.0f), dAquarium_c::IDX_RIVER);
dAquariumEffBubble_c sBubble2("af_msm_abuku_flesh", mVec3_c(-40.0f, 0.0f, 30.0f), dAquarium_c::IDX_RIVER);
dAquariumEffBubble_c sBubble3("af_msm_abuku_flesh", mVec3_c(-30.0f, 0.0f, 0.0f), dAquarium_c::IDX_POND);
dAquariumEffBubble_c sBubble4("af_msm_abuku_flesh", mVec3_c(110.0f, 0.0f, 30.0f), dAquarium_c::IDX_POND);
dAquariumEffBubble_c sBubble5("af_msm_abuku_sea", mVec3_c(-410.0f, 0.0f, -15.0f), dAquarium_c::IDX_SEA);
dAquariumEffBubble_c sBubble6("af_msm_abuku_sea", mVec3_c(5.0f, 0.0f, -15.0f), dAquarium_c::IDX_SEA);
dAquariumEffBubble_c sBubble7("af_msm_abuku_sea", mVec3_c(405.0f, 0.0f, 0.0f), dAquarium_c::IDX_SEA);
dAquariumEffBubble_c sBubble8("af_msm_abuku_sea", mVec3_c(-430.0f, 0.0f, 30.0f), dAquarium_c::IDX_SEA);
dAquariumEffBubble_c sBubble9("af_msm_abuku_sea", mVec3_c(-180.0f, 0.0f, 20.0f), dAquarium_c::IDX_SEA);
dAquariumEffBubble_c sBubble10("af_msm_abuku_sea", mVec3_c(180.0f, 0.0f, 30.0f), dAquarium_c::IDX_SEA);
dAquariumEffBubble_c sBubble11("af_msm_abuku_sea", mVec3_c(310.0f, 0.0f, -20.0f), dAquarium_c::IDX_SEA);
dAquariumEffBubble_c sBubble12("af_msm_abuku_cyl_l", mVec3_c(0.0f, 0.0f, 0.0f), dAquarium_c::IDX_CYL_LEFT);
dAquariumEffBubble_c sBubble13("af_msm_abuku_cyl_r", mVec3_c(0.0f, 0.0f, 0.0f), dAquarium_c::IDX_CYL_RIGHT);

dAquariumCorrectObjCyl_c sRock00("rock00", mVec3_c(5.0f, 0.0f, 0.0f), 33.0f, 25.0f);
dAquariumCorrectObjCyl_c sRock01("rock01", mVec3_c(-5.0f, 0.0f, -1.0f), 31.0f, 27.0f);
dAquariumCorrectObjCyl_c sRock02("rock02", mVec3_c(3.0f, 0.0f, 0.0f), 18.0f, 23.0f);
dAquariumCorrectObjCyl_c sRock03("rock03", mVec3_c(-3.0f, 0.0f, 0.0f), 17.0f, 12.0f);
dAquariumCorrectObjCyl_c sRock04("rock04", mVec3_c(0.0f, 0.0f, -13.0f), 24.0f, 38.0f);
dAquariumCorrectObjCyl_c sRock05("rock05", mVec3_c(-1.0f, 0.0f, -3.0f), 16.0f, 10.0f);
dAquariumCorrectObjCyl_c sRock06a("rock06", mVec3_c(7.0f, 0.0f, -1.0f), 17.0f, 11.0f);
dAquariumCorrectObjCyl_c sRock06b("rock06", mVec3_c(-17.0f, 0.0f, -2.0f), 19.0f, 10.0f);
dAquariumCorrectObjCyl_c sRock07("rock07", mVec3_c(-1.0f, 0.0f, -2.0f), 16.0f, 5.0f);
dAquariumCorrectObjCyl_c sRock08("rock08", mVec3_c(3.0f, 0.0f, -2.0f), 24.0f, 27.0f);
dAquariumCorrectObjCyl_c sRock09("rock09", 10.0f, 6.0f);
dAquariumCorrectObjCyl_c sAnemone("locator_anemone_02", mVec3_c(2.0f, 2.0f, 1.0f), 7.0f, 5.0f);
dAquariumCorrectObjCyl_c sOctopusPod("locator_takotsubo_00", mVec3_c(0.0f, 0.0f, -5.0f), 13.0f, 12.0f);
dAquariumCorrectObjCyl_c sSeaPillar0(mVec3_c(42.0f, 35.0f, 121.4f), 12.0f, 109.0f);
dAquariumCorrectObjCyl_c sSeaPillar1(mVec3_c(983.0f, 35.0f, 121.0f), 12.0f, 109.0f);

dAquarium_c *sAquariums[dAquarium_c::IDX_NUM] = {
    &sAquariumRiver, &sAquariumPond, &sAquariumSea, &sAquariumCylLeft, &sAquariumCylRight,
};

dAquariumEffBubble_c *sBubbles[] = {
    &sBubble0, &sBubble1, &sBubble2, &sBubble3,  &sBubble4,  &sBubble5,  &sBubble6,
    &sBubble7, &sBubble8, &sBubble9, &sBubble10, &sBubble11, &sBubble12, &sBubble13,
};

dAquariumCorrectObj_c *sRiverObjs[] = {
    &sRock05,
    &sRock06a,
    &sRock06b,
    &sRock07,
};

dAquariumCorrectObj_c *sSeaObjs[] = {
    &sRock00, &sRock01, &sRock02, &sRock03, &sRock04, &sRock08, &sOctopusPod, &sSeaPillar0, &sSeaPillar1,
};

dAquariumCorrectObj_c *sCylRightObjs[] = {
    &sRock09,
    &sAnemone,
};

CorrectObjList_c sCorrectObjLists[dAquarium_c::IDX_NUM] = {
    {sRiverObjs, ARRAY_SIZE(sRiverObjs)}, {NULL, 0}, {sSeaObjs, ARRAY_SIZE(sSeaObjs)}, {NULL, 0},
    {sCylRightObjs, ARRAY_SIZE(sCylRightObjs)},
};

} // namespace dFishMuseum

dFishMus_c *dFishMus_c::sInstance;

using namespace dFishMuseum;

// 0x36AC
void dAquarium_c::init() {}

// 0x36B0
dAquariumBox_c::dAquariumBox_c(const char *nodeName, const mVec3_c &halfSize)
    : dAquarium_c(nodeName), mHalfSize(halfSize), mOffset(0.0f, 0.0f, 0.0f) {}

// 0x3700
dAquariumBox_c::dAquariumBox_c(const char *nodeName, const mVec3_c &halfSize, const mVec3_c &offset)
    : dAquarium_c(nodeName), mHalfSize(halfSize), mOffset(offset) {}

// 0x3754
void dAquariumBox_c::init() {
    mHitList = NULL;
    mFishList = NULL;
    mVec3_c pos;
    dBgUtil::getNodePos(&pos, mNodeName, -1, -1);
    pos += mOffset;
    mCenter = mMax = pos;
    mMin.x = mCenter.x - mHalfSize.x;
    mMin.y = mCenter.y;
    mMin.y += 2.0f;
    mMin.z = mCenter.z - mHalfSize.z;
    mMax += mHalfSize;
}

// 0x3834
void dAquariumCyl_c::init() {
    mHitList = NULL;
    mFishList = NULL;
    dBgUtil::getNodePos(&mCenter, mNodeName, -1, -1);
    mCenter.y += 1.0f;
}

// 0x3890
BOOL dAquarium_c::hitFish(dFishMusBase_c *a, dFishMusBase_c *b) {
    if (a->getHitType() < dFishMusData_c::HIT_NONE && b->getHitType() < dFishMusData_c::HIT_NONE) {
        HitFishFunc func = getHitFishFunc(a->getHitType(), b->getHitType());
        if (func != NULL) {
            mVec3_c pushA;
            mVec3_c pushB;
            if (func(a, b, &pushA, &pushB)) {
                a->onHitFish(b, pushA);
                b->onHitFish(a, pushB);
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 0x39A8
void dAquarium_c::hitObjs(dFishMusBase_c *fish) {
    for (dAquariumCorrectObj_c *obj = mObjList; obj != NULL; obj = obj->mNext) {
        mVec3_c push(0.0f, 0.0f, 0.0f);
        if (obj->hitCheck(fish, &push)) {
            fish->onHitObj(obj, push);
        }
    }
}

// 0x3A50
void dAquarium_c::addFish(dFishMusBase_c *fish) {
    dFishMusBase_c **p = &mFishList;
    while (*p != NULL) {
        p = &(*p)->mTankNext;
    }
    *p = fish;
}

// 0x3A70
void dAquarium_c::addHitFish(dFishMusBase_c *fish) {
    dFishMusBase_c **p = &mHitList;
    while (*p != NULL) {
        p = &(*p)->mHitNext;
    }
    *p = fish;
}

// 0x3A90
void dAquarium_c::addObj(dAquariumCorrectObj_c *obj) {
    dAquariumCorrectObj_c **p = &mObjList;
    while (*p != NULL) {
        p = &(*p)->mNext;
    }
    *p = obj;
}

// 0x3AB0
void dAquarium_c::removeFish(dFishMusBase_c *fish) {
    dFishMusBase_c **p = &mFishList;
    while (*p != fish) {
        p = &(*p)->mTankNext;
    }
    if (*p == fish) {
        *p = fish->mTankNext;
    }
}

// 0x3AD8
void dAquarium_c::removeHitFish(dFishMusBase_c *fish) {
    dFishMusBase_c **p = &mHitList;
    while (*p != fish) {
        p = &(*p)->mHitNext;
    }
    if (*p == fish) {
        *p = fish->mHitNext;
    }
}

// 0x3B00
void dAquarium_c::execute() {
    for (dFishMusBase_c **a = &mHitList; *a != NULL; a = &(*a)->mHitNext) {
        for (dFishMusBase_c **b = &(*a)->mHitNext; *b != NULL; b = &(*b)->mHitNext) {
            hitFish(*a, *b);
        }
    }
    for (dFishMusBase_c **fish = &mFishList; *fish != NULL; fish = &(*fish)->mTankNext) {
        hitObjs(*fish);
        correctFish(*fish);
    }
}

// 0x3BB8
BOOL dAquariumBox_c::correctFish(dFishMusBase_c *fish) {
    dFishMusData_c *data = &sFishData[fish->mFishIdx];
    BOOL moved = FALSE;
    f32 cosY = fish->mAngle.y.cos();
    f32 halfX, halfZ;
    data->getHalfSizeXZ(cosY, halfX, halfZ);
    f32 minX = mMin.x + halfX;
    f32 maxX = mMax.x - halfX;
    f32 minZ = mMin.z + halfZ;
    f32 maxZ = mMax.z - halfZ;
    f32 minY = mMin.y + 0.5f * data->mHitHeight;
    f32 maxY = mMax.y - 0.5f * data->mHitHeight;
    mVec3_c push(0.0f, 0.0f, 0.0f);
    mVec3_c center;
    fish->getCenterPos(center);
    if (center.x < minX) {
        push.x = minX - center.x;
        moved = TRUE;
    } else if (center.x > maxX) {
        push.x = maxX - center.x;
        moved = TRUE;
    }
    if (center.z < minZ) {
        push.z = minZ - center.z;
        moved = TRUE;
    } else if (center.z > maxZ) {
        push.z = maxZ - center.z;
        moved = TRUE;
    }
    if (center.y < minY) {
        push.y = minY - center.y;
        moved = TRUE;
    } else if (center.y > maxY) {
        push.y = maxY - center.y;
        moved = TRUE;
    }
    fish->addPos(push);
    return moved;
}

// 0x3E2C
BOOL dAquariumCyl_c::correctFish(dFishMusBase_c *fish) {
    dFishMusData_c *data = &sFishData[fish->mFishIdx];
    BOOL moved = FALSE;
    mVec3_c push(0.0f, 0.0f, 0.0f);
    f32 radius = data->mHitLength;
    mVec3_c center;
    fish->getCenterPos(center);
    mVec3_c dir = center;
    dir.x -= mCenter.x;
    dir.z -= mCenter.z;
    if (center.y - data->mHitHeight < mCenter.y) {
        push.y = data->mHitHeight + (mCenter.y - center.y);
        moved = TRUE;
    } else if (center.y > mCenter.y + mHeight) {
        push.y = mCenter.y + mHeight - center.y;
        moved = TRUE;
    }
    dir.y = 0.0f;
    f32 distSq = dir.xzLenSq();
    radius = mRadius - radius;
    f32 radiusSq = radius * radius;
    if (distSq > radiusSq && dir.normalizeRS()) {
        moved = TRUE;
        dir *= radius;
        push.x = mCenter.x + dir.x - center.x;
        push.z = mCenter.z + dir.z - center.z;
    }
    fish->addPos(push);
    return moved;
}

// 0x4050
void dAquariumCorrectObj_c::init() {}

// 0x4054
dAquariumCorrectObjCyl_c::dAquariumCorrectObjCyl_c(const mVec3_c &pos, f32 radius, f32 height)
    : mPos(pos), mRadius(radius), mHeight(height), mUseNode(false) {
    mOffset.set(0.0f, 0.0f, 0.0f);
}

// 0x40A4
dAquariumCorrectObjCyl_c::dAquariumCorrectObjCyl_c(const char *nodeName, f32 radius, f32 height)
    : mRadius(radius), mHeight(height), mNodeName(nodeName), mUseNode(true) {
    mOffset.set(0.0f, 0.0f, 0.0f);
}

// 0x40E4
dAquariumCorrectObjCyl_c::dAquariumCorrectObjCyl_c(const char *nodeName, const mVec3_c &offset, f32 radius,
                                                   f32 height)
    : mOffset(offset), mRadius(radius), mHeight(height), mNodeName(nodeName), mUseNode(true) {}

// 0x4128
void dAquariumCorrectObjCyl_c::init() {
    if (mUseNode) {
        dBgUtil::getNodePos(&mPos, mNodeName, -1, -1);
    }
    mPos += mOffset;
}

// 0x41A0
BOOL dAquariumCorrectObjCyl_c::hitCheck(dFishMusBase_c *fish, mVec3_c *push) {
    dFishMusData_c *data = &sFishData[fish->mFishIdx];
    mVec3_c center;
    fish->getCenterPos(center);
    HitCyl_c cyl(mPos, mRadius, mHeight);
    switch (fish->getHitType()) {
    case dFishMusData_c::HIT_CYLINDER: {
        HitCyl_c fishCyl(center, data->mHitLength, data->mHitHeight);
        return hitCyl(&fishCyl, &cyl, push);
    }
    case dFishMusData_c::HIT_CAPSULE: {
        mVec3_c start = center;
        mVec3_c end = center;
        mVec3_c half(0.0f, 0.0f, data->mHitLength - data->mHitWidth);
        half.rotY(fish->mAngle.y);
        start += half;
        end -= half;
        HitCapsule_c capsule(start, end, data->mHitLength, data->mHitHeight);
        return hitCapsule(&capsule, &cyl, push);
    }
    case dFishMusData_c::HIT_BOX: {
        dFishMusEel_c *eel = static_cast<dFishMusEel_c *>(fish);
        HitQuad_c quad(eel->getHitCorner(0), eel->getHitCorner(1), eel->getHitCorner(2), eel->getHitCorner(3),
                       data->mHitLength, data->mHitHeight);
        return hitQuad(&quad, &cyl, push);
    }
    }
    return FALSE;
}

// 0x452C
dFishMusMdl_c::dFishMusMdl_c()
    : mCallback(this), mNodeMtx(NULL), mNodeRot(NULL), mTrailHead(0), mTrailTail(0), mTrailCount(0),
      mTrailStep(20.0f), mBendRate(0.25f), mNodeNum(0), mStartNode(0), mMoved(false) {
    for (int i = 0; i < TRAIL_NUM; i++) {
        mTrail[i].set(0.0f, 0.0f, 0.0f);
        mTrailDist[i] = 0.0f;
    }
}

// 0x4700
dFishMusMdl_c::~dFishMusMdl_c() {}

// 0x4780
BOOL dFishMusMdl_c::create(nw4r::g3d::ResMdl mdl, mAllocator_c *allocator, int nodeNum, int startNode, int,
                           ulong bufferOption, int viewCount, size_t *objSize) {
    if (!smdl_c::create(mdl, allocator, bufferOption, viewCount, objSize)) {
        return FALSE;
    }
    mNodeMtx = (nw4r::math::MTX34 *)allocator->alloc(nodeNum * sizeof(nw4r::math::MTX34));
    mNodeRot = (EGG::Quatf *)allocator->alloc(nodeNum * sizeof(EGG::Quatf));
    if (mNodeMtx == NULL || mNodeRot == NULL) {
        return FALSE;
    }
    mNodeNum = nodeNum;
    mStartNode = startNode;
    for (int i = 0; i < mNodeNum; i++) {
        mNodeRot[i].set(1.0f, 0.0f, 0.0f, 0.0f);
        PSMTXIdentity(mNodeMtx[i]);
    }
    nw4r::g3d::ScnMdlSimple *scn = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdlSimple>(mpScn);
    scn->SetScnMdlCallback(&mCallback);
    scn->EnableScnMdlCallbackTiming(nw4r::g3d::ScnObj::CALLBACK_TIMING_B);
    scn->SetScnMdlCallbackNodeID(startNode);
    return TRUE;
}

// 0x4908
void dFishMusMdl_c::shiftTrail(const mVec3_c &move) {
    for (int i = 0; i < TRAIL_NUM; i++) {
        mTrail[i] += move;
    }
}

// 0x4A48
void dFishMusMdl_c::calcNode(nw4r::g3d::WorldMtxManip *manip, ulong nodeId) {
    nw4r::math::MTX34 *mtx = &mNodeMtx[nodeId - mStartNode];
    EGG::Quatf *rot = &mNodeRot[nodeId - mStartNode];
    manip->GetMatrix(mtx);
    if (nodeId == mStartNode) {
        mVec3_c *last = &mTrail[mTrailHead];
        mVec3_c pos(mtx->_03, mtx->_13, mtx->_23);
        mRootPos = pos;
        mRootDir.set(mtx->_00, mtx->_10, mtx->_20);
        f32 dist = EGG::Mathf::sqrt(PSVECSquareDistance(*last, pos));
        if (dist > 3.0f) {
            mMoved = true;
        } else {
            mMoved = false;
            mDist = -dist;
            mTrailIdx = mTrailHead - 1;
            if (mTrailIdx < 0) {
                mTrailIdx = TRAIL_NUM - 1;
            }
            return;
        }
        int head = mTrailHead;
        if (++mTrailHead == TRAIL_NUM) {
            mTrailHead = 0;
        }
        if (mTrailTail == mTrailHead) {
            if (++mTrailTail == TRAIL_NUM) {
                mTrailTail = 0;
            }
        }
        mVec3_c *next = &mTrail[mTrailHead];
        if (mTrailCount != TRAIL_NUM) {
            if (mTrailCount == 0) {
                last->set(mtx->_03, mtx->_13, mtx->_23);
            }
            mTrailCount++;
        }
        *next = pos;
        mDist = 0.0f;
        mTrailIdx = head;
        mTrailDist[head] = dist;
    } else {
        mVec3_c *rootPos = &mRootPos;
        mVec3_c pos(mtx->_03, mtx->_13, mtx->_23);
        mVec3_c prevDir = mRootDir;
        mDist += mTrailStep;
        mVec3_c dir = prevDir;
        while (true) {
            int idx = mTrailIdx;
            if (idx == mTrailTail) {
                dir = mTrail[idx] - *rootPos;
                if (!dir.normalizeRS()) {
                    dir = prevDir;
                }
                return;
            }
            if (mDist < mTrailDist[idx]) {
                f32 t = mDist / mTrailDist[idx];
                f32 rest = 1.0f - t;
                int prev = idx - 1;
                if (prev < 0) {
                    prev = TRAIL_NUM - 1;
                }
                dir = mTrail[idx] * rest + mTrail[prev] * t;
                dir -= *rootPos;
                if (!dir.normalizeRS()) {
                    dir = prevDir;
                }
                break;
            }
            mDist -= mTrailDist[idx];
            if (--mTrailIdx < 0) {
                mTrailIdx = TRAIL_NUM - 1;
            }
        }
        EGG::Quatf q;
        q.makeVectorRotation(prevDir, dir);
        rot->slerpTo(q, mBendRate, *rot);
        rot->normalise();
        if (rot->w < 0.0f) {
            rot->w *= -1.0f;
            rot->x *= -1.0f;
            rot->y *= -1.0f;
            rot->z *= -1.0f;
        }
        mMtx_c rotMtx;
        ((EGG::Matrix34f *)(nw4r::math::MTX34 *)(rotMtx))->makeQ(*rot);
        PSMTXMultVecSR(rotMtx, mRootDir, mRootDir);
        PSMTXConcat(rotMtx, *mtx, *mtx);
        mtx->_03 = pos.x;
        mtx->_13 = pos.y;
        mtx->_23 = pos.z;
        manip->SetMatrix(*mtx);
        *rootPos = pos;
    }
}

// 0x4EC8
dFishMusMdl_c::mdlCallback_c::mdlCallback_c(dFishMusMdl_c *owner) : mOwner(owner) {}

// 0x4EDC
dFishMusMdl_c::mdlCallback_c::~mdlCallback_c() {}

// 0x4F1C
void dFishMusMdl_c::mdlCallback_c::ExecCallbackB(nw4r::g3d::WorldMtxManip *manip, nw4r::g3d::ResMdl mdl,
                                                 nw4r::g3d::FuncObjCalcWorld *funcObj) {
    dFishMusMdl_c *owner = mOwner;
    ulong nodeId = funcObj->GetNodeID();
    ulong next = nodeId + 1;
    if (next >= owner->mNodeNum + owner->mStartNode) {
        next = owner->mStartNode;
    }
    owner->calcNode(manip, nodeId);
    funcObj->SetNodeID(next);
}

// 0x4F78
BOOL dFishMusBank_c::loadFish(EGG::Heap *heap, int fishIdx) {
    static char path[0x20];
    snprintf(path, sizeof(path), "/Fish/%s.brres", sFishCreate[fishIdx]->mName);
    return load(path, heap, 0);
}

// 0x4FF8
BOOL dFishMusBank_c::loadObj(EGG::Heap *heap, int objIdx) {
    if (objIdx == dFishMus_c::OBJ_LOTUS) {
        return load("/Fish/m_lotus.brres", heap, 0);
    } else if (objIdx == dFishMus_c::OBJ_WAKAME00) {
        return load("/Fish/m_wakame00.brres", heap, 0);
    } else if (objIdx == dFishMus_c::OBJ_WAKAME01) {
        return load("/Fish/m_wakame01.brres", heap, 0);
    } else if (objIdx == dFishMus_c::OBJ_WATER_WEED00) {
        return load("/Fish/m_mizukusa00.brres", heap, 0);
    } else if (objIdx == dFishMus_c::OBJ_WATER_WEED01) {
        return load("/Fish/m_mizukusa01.brres", heap, 0);
    } else if (objIdx == dFishMus_c::OBJ_SEA_ANEMONE) {
        return load("/Fish/m_anemone.brres", heap, 0);
    } else {
        return load("/Fish/m_tsubo.brres", heap, 0);
    }
}

// 0x50A0
dFishMusBaseObj_c::dFishMusBaseObj_c()
    : mAnmRate(0.0f), mAnmFrameMax(0.0f), mPos(0.0f, 0.0f, 0.0f), mHomeAngle(0, 0, 0), mAngle(0, 0, 0),
      mCullRadius(100.0f), mSlot(0) {}

// 0x5158
dFishMusBaseObj_c::~dFishMusBaseObj_c() {}

// 0x51D0
BOOL dFishMusBaseObj_c::create(nw4r::g3d::ResFile res) {
    return TRUE;
}

// 0x51D8
BOOL dFishMusBaseObj_c::execute() {
    return TRUE;
}

// 0x51E0
BOOL dFishMusBaseObj_c::draw() {
    mMdl.entry();
    return TRUE;
}

// 0x5210
BOOL dFishMusBaseObj_c::doDelete() {
    mMdl.remove();
    return TRUE;
}

// 0x5240
BOOL dFishMusBaseObj_c::isOutOfView() {
    dCamera_c *camera = (dCamera_c *)dScene::getSceneE848();
    mSphere_c sphere(mPos, mCullRadius);
    return !camera->mCullFrustum.intersectSphere(&sphere);
}

// 0x52A0
void *dFishMusBaseObj_c::operator new(size_t size, EGG::Heap *heap) {
    void *p = EGG::Heap::alloc(size, 4, heap);
    if (p == NULL) {
        return NULL;
    }
    memset(p, 0, size);
    return p;
}

// 0x5300
dAquariumEffBubble_c::dAquariumEffBubble_c(const char *effName, const mVec3_c &offset, int aquariumIdx)
    : mEffName(effName), mOffset(offset), mAquariumIdx(aquariumIdx), mTimer(0) {}

// 0x5338
dAquariumEffBubble_c::~dAquariumEffBubble_c() {}

// 0x5378
void dAquariumEffBubble_c::init() {
    dAquarium_c *aquarium = sAquariums[mAquariumIdx];
    if (aquarium->getType() == dAquarium_c::TYPE_BOX) {
        mPos = static_cast<dAquariumBox_c *>(aquarium)->getCenter() + mOffset;
    } else if (aquarium->getType() == dAquarium_c::TYPE_CYLINDER) {
        mPos = static_cast<dAquariumCyl_c *>(aquarium)->getCenter() + mOffset;
    }
}

// 0x5460
void dAquariumEffBubble_c::execute() {
    if (--mTimer < 0) {
        mTimer = cM::rndRange<int>(l_HIO.mBubbleTimeMin, l_HIO.mBubbleTimeMax);
        dEffect::create(mEffName, &mPos, NULL, NULL);
    }
}

// 0x54C4
dFishMusBase_c::dFishMusBase_c()
    : mpMdl(NULL), mCenterOffset(0.0f, 0.0f, 0.0f), mMdlOffset(0.0f, 0.0f, 0.0f), mVelocity(0.0f, 0.0f, 0.0f),
      mNext(NULL), mHitNext(NULL), mTankNext(NULL) {}

// 0x556C
dFishMusBase_c::~dFishMusBase_c() {}

// 0x55E4
void dFishMusBase_c::calcMtx() {
    if (mpMdl != NULL) {
        mMtx_c mtx;
        mMtx_c rot;
        rot.YrotS(mAngle.y);
        rot.XrotM(mAngle.x);
        rot.ZrotM(mAngle.z);
        mVec3_c pos;
        PSMTXMultVecSR(rot, mMdlOffset, pos);
        pos += mPos;
        mVec3_c curved;
        mAng curveAngle = dWorld::curvePosition(&curved, &pos);
        PSMTXTrans(mtx, curved.x, curved.y, curved.z);
        mtx.XrotM(curveAngle);
        PSMTXConcat(mtx, rot, mtx);
        mpMdl->setLocalMtx(&mtx);
    }
}

// 0x56EC
void dFishMusBase_c::addPos(const mVec3_c &push) {
    mPush += push;
    mPos += push;
}

// 0x5744
void dFishMusBase_c::onHitFish(dFishMusBase_c *other, const mVec3_c &push) {
    mVec3_c move = push * 0.25f;
    mPush += move;
    mPos += move;
}

// 0x57E0
void dFishMusBase_c::onHitObj(dAquariumCorrectObj_c *obj, const mVec3_c &push) {
    mVec3_c move = push * 0.5f;
    mPush += move;
    mPos += move;
}

// 0x587C
int dFishMusBase_c::getHitType() {
    return sFishData[mFishIdx].mHitType;
}

// 0x5898
dAquarium_c *dFishMusBase_c::getAquarium() {
    return sAquariums[mAquariumIdx];
}

// 0x58B0
void dFishMusBase_c::setTankArea() {
    if (getAquarium()->getType() == dAquarium_c::TYPE_BOX) {
        dAquariumBox_c *box = static_cast<dAquariumBox_c *>(getAquarium());
        mTankMin = box->getMin();
        mTankMax = box->getMax();
    }
}

// 0x5950
BOOL dFishMusBase_c::isNearWall() {
    if ((mTankMin.x - 10.0f <= mHeadPos.x && mHeadPos.x <= 10.0f + mTankMin.x) ||
        (mTankMax.x - 10.0f <= mHeadPos.x && mHeadPos.x <= 10.0f + mTankMax.x) ||
        (mTankMin.z - 10.0f <= mHeadPos.z && mHeadPos.z <= 10.0f + mTankMin.z) ||
        (mTankMax.z - 10.0f <= mHeadPos.z && mHeadPos.z <= 10.0f + mTankMax.z)) {
        return TRUE;
    }
    return FALSE;
}

// 0x5A20
BOOL dFishMusBase_c::isOutOfView() {
    dCamera_c *camera = (dCamera_c *)dScene::getSceneE848();
    dFishMusData_c *data = &sFishData[mFishIdx];
    mVec3_c center;
    getCenterPos(center);
    mSphere_c sphere(center, data->mCullRadius);
    if (camera->mCullFrustum.intersectSphere(&sphere)) {
        return FALSE;
    }
    return TRUE;
}

// 0x5B3C
BOOL dFishMusBase_c::create(nw4r::g3d::ResFile res) {
    return TRUE;
}

// 0x5B44
BOOL dFishMusBase_c::execute() {
    return TRUE;
}

// 0x5B4C
BOOL dFishMusBase_c::draw() {
    mMdl.entry();
    return TRUE;
}

// 0x5B7C
BOOL dFishMusBase_c::doDelete() {
    mMdl.remove();
    mAnm.remove();
    getAquarium()->removeFish(this);
    getAquarium()->removeHitFish(this);
    return TRUE;
}

// 0x5BF0
void *dFishMusBase_c::operator new(size_t size, EGG::Heap *heap) {
    void *p = EGG::Heap::alloc(size, 4, heap);
    if (p == NULL) {
        return NULL;
    }
    memset(p, 0, size);
    return p;
}

namespace dFishMuseum {

const char l_tanago[] = "fsh_tanago";
const char l_oikawa[] = "fsh_oikawa";
const char l_funa[] = "fsh_funa";
const char l_ugui[] = "fsh_ugui";
const char l_nigoi[] = "fsh_nigoi";
const char l_koi[] = "fsh_koi";
const char l_nishiki[] = "fsh_nishiki";
const char l_kingyo[] = "fsh_kingyo";
const char l_demekin[] = "fsh_demekin";
const char l_medaka[] = "fsh_medaka";
const char l_zarigani[] = "fsh_zarigani";
const char l_kaeru[] = "m_fsh_kaeru";
const char l_donko[] = "fsh_donko";
const char l_dojou[] = "fsh_dojou";
const char l_namazu[] = "fsh_namazu";
const char l_unagi[] = "fsh_unagi";
const char l_raigyo[] = "fsh_raigyo";
const char l_blue[] = "fsh_blue";
const char l_yellow[] = "fsh_yellow";
const char l_black[] = "fsh_black";
const char l_pike[] = "fsh_pike";
const char l_wakasagi[] = "fsh_wakasagi";
const char l_ayu[] = "fsh_ayu";
const char l_yamame[] = "fsh_yamame";
const char l_ooiwana[] = "fsh_ooiwana";
const char l_nijimasu[] = "fsh_nijimasu";
const char l_itou[] = "fsh_itou";
const char l_sake[] = "fsh_sake";
const char l_king[] = "fsh_king";
const char l_guppi[] = "fsh_guppi";
const char l_angel[] = "fsh_angel";
const char l_neon[] = "fsh_neon";
const char l_pirania[] = "fsh_pirania";
const char l_arowana[] = "fsh_arowana";
const char l_dorado[] = "fsh_dorado";
const char l_ga[] = "fsh_ga";
const char l_piraruku[] = "fsh_piraruku";
const char l_kurione[] = "fsh_kurione";
const char l_kurage[] = "fsh_kurage";
const char l_tatsu[] = "fsh_tatsu";
const char l_kumanomi[] = "fsh_kumanomi";
const char l_nanyou[] = "fsh_nanyou";
const char l_chouchou[] = "fsh_chouchou";
const char l_napoleon[] = "fsh_napoleon";
const char l_minokasago[] = "fsh_minokasago";
const char l_harisen[] = "fsh_harisen";
const char l_aji[] = "fsh_aji";
const char l_ishidai[] = "fsh_ishidai";
const char l_suzuki[] = "fsh_suzuki";
const char l_tai[] = "fsh_tai";
const char l_karei[] = "fsh_karei";
const char l_hirame[] = "fsh_hirame";
const char l_ika[] = "fsh_ika";
const char l_tako[] = "fsh_tako";
const char l_lobster[] = "fsh_lobster";
const char l_utsubo[] = "m_fsh_utsubo";
const char l_ankou[] = "fsh_ankou";
const char l_maguro[] = "fsh_maguro";
const char l_kajiki[] = "fsh_kajiki";
const char l_ei[] = "fsh_ei";
const char l_manbou[] = "fsh_manbou";
const char l_shumoku[] = "fsh_shumoku";
const char l_same[] = "fsh_same";
const char l_siira[] = "fsh_siira";

FishCreate_c sFishCreateData[FISH_KIND_NUM] = {
    {createFish<dFishMusBaseMove_c, FISH_TANAGO>, l_tanago},
    {createFish<dFishMusBaseMove_c, FISH_OIKAWA>, l_oikawa},
    {createFish<dFishMusBaseMove_c, FISH_FUNA>, l_funa},
    {createFish<dFishMusBaseMove_c, FISH_UGUI>, l_ugui},
    {createFish<dFishMusBaseMove_c, FISH_NIGOI>, l_nigoi},
    {createFish<dFishMusKoi_c, FISH_KOI>, l_koi},
    {createFish<dFishMusKoi_c, FISH_NISHIKI>, l_nishiki},
    {createFish<dFishMusElegance_c, FISH_KINGYO>, l_kingyo},
    {createFish<dFishMusElegance_c, FISH_DEMEKIN>, l_demekin},
    {createFish<dFishMusSmall_c, FISH_MEDAKA>, l_medaka},
    {createFish<dFishMusLobster_c, FISH_ZARIGANI>, l_zarigani},
    {createFish<dFishMusFrog_c, FISH_KAERU>, l_kaeru},
    {createFish<dFishMusBaseMove_c, FISH_DONKO>, l_donko},
    {createFish<dFishMusBaseMove_c, FISH_DOJOU>, l_dojou},
    {createFish<dFishMusBaseMove_c, FISH_NAMAZU>, l_namazu},
    {createFish<dFishMusEel_c, FISH_UNAGI>, l_unagi},
    {createFish<dFishMusBaseMove_c, FISH_RAIGYO>, l_raigyo},
    {createFish<dFishMusBaseMove_c, FISH_BLUE>, l_blue},
    {createFish<dFishMusBaseMove_c, FISH_YELLOW>, l_yellow},
    {createFish<dFishMusFast_c, FISH_BLACK>, l_black},
    {createFish<dFishMusFast_c, FISH_PIKE>, l_pike},
    {createFish<dFishMusSmall_c, FISH_WAKASAGI>, l_wakasagi},
    {createFish<dFishMusBaseMove_c, FISH_AYU>, l_ayu},
    {createFish<dFishMusBaseMove_c, FISH_YAMAME>, l_yamame},
    {createFish<dFishMusBaseMove_c, FISH_OOIWANA>, l_ooiwana},
    {createFish<dFishMusBaseMove_c, FISH_NIJIMASU>, l_nijimasu},
    {createFish<dFishMusFast_c, FISH_ITOU>, l_itou},
    {createFish<dFishMusFast_c, FISH_SAKE>, l_sake},
    {createFish<dFishMusFast_c, FISH_KING>, l_king},
    {createFish<dFishMusSmall_c, FISH_GUPPI>, l_guppi},
    {createFish<dFishMusElegance_c, FISH_ANGEL>, l_angel},
    {createFish<dFishMusSmall_c, FISH_NEON>, l_neon},
    {createFish<dFishMusPiranha_c, FISH_PIRANIA>, l_pirania},
    {createFish<dFishMusSlow_c, FISH_AROWANA>, l_arowana},
    {createFish<dFishMusFast_c, FISH_DORADO>, l_dorado},
    {createFish<dFishMusSlow_c, FISH_GA>, l_ga},
    {createFish<dFishMusSlow_c, FISH_PIRARUKU>, l_piraruku},
    {createFish<dFishMusFloat_c, FISH_KURIONE>, l_kurione},
    {createFish<dFishMusFloat_c, FISH_KURAGE>, l_kurage},
    {createFish<dFishMusFloat_c, FISH_TATSU>, l_tatsu},
    {createFish<dFishMusSmall_c, FISH_KUMANOMI>, l_kumanomi},
    {createFish<dFishMusSmall_c, FISH_NANYOU>, l_nanyou},
    {createFish<dFishMusBaseMove_c, FISH_CHOUCHOU>, l_chouchou},
    {createFish<dFishMusBaseMove_c, FISH_NAPOLEON>, l_napoleon},
    {createFish<dFishMusBaseMove_c, FISH_MINOKASAGO>, l_minokasago},
    {createFish<dFishMusPufferfish_c, FISH_HARISEN>, l_harisen},
    {createFish<dFishMusKeepSwimming_c, FISH_AJI>, l_aji},
    {createFish<dFishMusBaseMove_c, FISH_ISHIDAI>, l_ishidai},
    {createFish<dFishMusBaseMove_c, FISH_SUZUKI>, l_suzuki},
    {createFish<dFishMusBaseMove_c, FISH_TAI>, l_tai},
    {createFish<dFishMusFlatfish_c, FISH_KAREI>, l_karei},
    {createFish<dFishMusFlatfish_c, FISH_HIRAME>, l_hirame},
    {createFish<dFishMusCuttlefish_c, FISH_IKA>, l_ika},
    {createFish<dFishMusOctopus_c, FISH_TAKO>, l_tako},
    {createFish<dFishMusLobster_c, FISH_LOBSTER>, l_lobster},
    {createFish<dFishMusMoray_c, FISH_UTSUBO>, l_utsubo},
    {createFish<dFishMusBaseMove_c, FISH_ANKOU>, l_ankou},
    {createFish<dFishMusKeepSwimming_c, FISH_MAGURO>, l_maguro},
    {createFish<dFishMusKeepSwimming_c, FISH_KAJIKI>, l_kajiki},
    {createFish<dFishMusRay_c, FISH_EI>, l_ei},
    {createFish<dFishMusGlobefish_c, FISH_MANBOU>, l_manbou},
    {createFish<dFishMusKeepSwimming_c, FISH_SHUMOKU>, l_shumoku},
    {createFish<dFishMusKeepSwimming_c, FISH_SAME>, l_same},
    {createFish<dFishMusBaseMove_c, FISH_SHIIRA>, l_siira},
};

FishCreate_c *sFishCreate[FISH_KIND_NUM] = {
    &sFishCreateData[0], &sFishCreateData[1], &sFishCreateData[2], &sFishCreateData[3],
    &sFishCreateData[4], &sFishCreateData[5], &sFishCreateData[6], &sFishCreateData[7],
    &sFishCreateData[8], &sFishCreateData[9], &sFishCreateData[10], &sFishCreateData[11],
    &sFishCreateData[12], &sFishCreateData[13], &sFishCreateData[14], &sFishCreateData[15],
    &sFishCreateData[16], &sFishCreateData[17], &sFishCreateData[18], &sFishCreateData[19],
    &sFishCreateData[20], &sFishCreateData[21], &sFishCreateData[22], &sFishCreateData[23],
    &sFishCreateData[24], &sFishCreateData[25], &sFishCreateData[26], &sFishCreateData[27],
    &sFishCreateData[28], &sFishCreateData[29], &sFishCreateData[30], &sFishCreateData[31],
    &sFishCreateData[32], &sFishCreateData[33], &sFishCreateData[34], &sFishCreateData[35],
    &sFishCreateData[36], &sFishCreateData[37], &sFishCreateData[38], &sFishCreateData[39],
    &sFishCreateData[40], &sFishCreateData[41], &sFishCreateData[42], &sFishCreateData[43],
    &sFishCreateData[44], &sFishCreateData[45], &sFishCreateData[46], &sFishCreateData[47],
    &sFishCreateData[48], &sFishCreateData[49], &sFishCreateData[50], &sFishCreateData[51],
    &sFishCreateData[52], &sFishCreateData[53], &sFishCreateData[54], &sFishCreateData[55],
    &sFishCreateData[56], &sFishCreateData[57], &sFishCreateData[58], &sFishCreateData[59],
    &sFishCreateData[60], &sFishCreateData[61], &sFishCreateData[62], &sFishCreateData[63],
};

} // namespace dFishMuseum
