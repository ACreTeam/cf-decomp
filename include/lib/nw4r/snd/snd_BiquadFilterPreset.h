#ifndef NW4R_SND_BIQUAD_FILTER_PRESET_H
#define NW4R_SND_BIQUAD_FILTER_PRESET_H

#include <types.h>

#include <nw4r/snd/snd_BiquadFilterCallback.h>

namespace nw4r {
namespace snd {
namespace detail {
class BiquadFilterLpf : public BiquadFilterCallback {
  public:
    virtual void GetCoef(int type, f32 value, BiquadCoef *coef) const;

  private:
    static int const COEF_TABLE_SIZE = 112;

    static BiquadCoef const coefTable[COEF_TABLE_SIZE];

  private:
    /* vtable */ // size 0x04, offset 0x00
}; // size 0x04

class BiquadFilterHpf : public BiquadFilterCallback {
  public:
    virtual void GetCoef(int type, f32 value, BiquadCoef *coef) const;

  private:
    static int const COEF_TABLE_SIZE = 97;

    static BiquadCoef const coefTable[COEF_TABLE_SIZE];

  private:
    /* vtable */ // size 0x04, offset 0x00
}; // size 0x04

class BiquadFilterBpf512 : public BiquadFilterCallback {
  public:
    virtual void GetCoef(int type, f32 value, BiquadCoef *coef) const;

  private:
    static int const COEF_TABLE_SIZE = 122;

    static BiquadCoef const coefTable[COEF_TABLE_SIZE];

  private:
    /* vtable */ // size 0x04, offset 0x00
}; // size 0x04

class BiquadFilterBpf1024 : public BiquadFilterCallback {
  public:
    virtual void GetCoef(int type, f32 value, BiquadCoef *coef) const;

  private:
    static int const COEF_TABLE_SIZE = 93;

    static BiquadCoef const coefTable[COEF_TABLE_SIZE];

  private:
    /* vtable */ // size 0x04, offset 0x00
}; // size 0x04

class BiquadFilterBpf2048 : public BiquadFilterCallback {
  public:
    virtual void GetCoef(int type, f32 value, BiquadCoef *coef) const;

  private:
    static int const COEF_TABLE_SIZE = 93;

    static BiquadCoef const coefTable[COEF_TABLE_SIZE];

  private:
    /* vtable */ // size 0x04, offset 0x00
}; // size 0x04
} // namespace detail
} // namespace snd
} // namespace nw4r

#endif
