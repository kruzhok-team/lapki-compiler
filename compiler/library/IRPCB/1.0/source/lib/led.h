#ifndef LED_H_
#define LED_H_

#include "BaseSequencer.h"
#include "color.h"
#include "gd_lib.h"

#if 1  // ==================== LED on/off, no sequences
       // =========================
class LedOnOff {
   protected:
    Pin_t ipin;
    Gpio::OutMode output_mode;

   public:
    LedOnOff(GPIO_TypeDef *apgpio, uint16_t apin, Gpio::OutMode aoutput_mode)
        : ipin(apgpio, apin), output_mode(aoutput_mode) {}
    void Init() {
        ipin.SetupOut(output_mode);
        Off();
    }
    void On() { ipin.SetHi(); }
    void Off() { ipin.SetLo(); }
};
#endif

#if 1  // ========================= Simple LED blinker
       // ==========================
class LedBlinker : public BaseSequencer<BaseChunk>, public LedOnOff {
   protected:
    void ISwitchOff() { Off(); }
    SequencerLoopTask ISetup() {
        ipin.Set(pcurrent_chunk->value);
        pcurrent_chunk++;   // Always increase
        return sltProceed;  // Always proceed
    }

   public:
    LedBlinker(GPIO_TypeDef *apgpio, uint16_t apin, Gpio::OutMode aoutput_mode)
        : BaseSequencer(), LedOnOff(apgpio, apin, aoutput_mode) {}
};
#endif

#if 1  // ======================== Single Led Smooth
       // ============================
class LedSmooth : public BaseSequencer<LedSmoothChunk> {
   protected:
    const PinOutputPWM_t ichnl;
    uint32_t curr_value;
    const uint32_t kPWMFreq;
    void ISwitchOff() { Set(0); }
    SequencerLoopTask ISetup() {
        if (curr_value != pcurrent_chunk->brightness) {
            if (pcurrent_chunk->value == 0) {  // If smooth time is zero,
                curr_value = pcurrent_chunk->brightness;
                SetCurrent();
                pcurrent_chunk++;  // and goto next chunk
            } else {
                if (curr_value < pcurrent_chunk->brightness)
                    curr_value++;
                else if (curr_value > pcurrent_chunk->brightness)
                    curr_value--;
                SetCurrent();
                // Check if completed now
                if (curr_value == pcurrent_chunk->brightness)
                    pcurrent_chunk++;
                else {  // Not completed
                    // Calculate time to next adjustment

                    uint32_t delay =
                        ClrCalcDelay(curr_value, pcurrent_chunk->value);

                    SetupDelay(delay);
                    return sltBreak;
                }  // Not completed
            }  // if time > 256
        }  // if color is different
        else
            pcurrent_chunk++;  // Color is the same, goto next chunk
        return sltProceed;
    }
    void SetCurrent() { ichnl.Set(curr_value); }

   public:
    LedSmooth(const PwmSetup apin_setup, const uint32_t afreq = 0xFFFFFFFF)
        : BaseSequencer(), ichnl(apin_setup), curr_value(0), kPWMFreq(afreq) {}
    void Init() {
        ichnl.Init();
        ichnl.SetFrequencyHz(kPWMFreq);
        Set(0);
    }
    void Set(uint32_t avalue) {
        curr_value = avalue;
        ichnl.Set(curr_value);
    }
};
#endif

#if 0  // ============= Single Led Smooth with brightness
       // control================
// Set LED top value to (255*255)
class LedSmoothWBrt_t : public LedSmooth {
private:
    uint32_t CurrBrt = 0;
    void SetCurrent() {
        // CurrBrt=[0;LED_SMOOTH_MAX_BRT]; curr_value=[0;255]
        uint32_t FValue = curr_value * CurrBrt;
        ipin.Set(FValue);
//        Printf("v=%u\r", FValue);
    }
public:
    LedSmoothWBrt_t(const PwmSetup apin_setup, const uint32_t afreq = 0xFFFFFFFF) : LedSmooth(apin_setup, afreq) {}
    void SetBrightness(uint32_t NewBrt) {
        CurrBrt = NewBrt;
        SetCurrent();
    }
    void Set(uint32_t avalue) {
        curr_value = avalue;
        SetCurrent();
    }
};
#endif

#if 0  // ==================== RGB blinker (no smooth switch)
       // ===================
#define LED_RGB_BLINKER
class LedRgbBlinker_t : public BaseSequencer<LedRGBChunk_t> {
protected:
    PinOutputPushPull_t R, G, B;
    void ISwitchOff() { SetColor(clBlack); }
    SequencerLoopTask ISetup() {
        SetColor(IPCurrentChunk->Color);
        IPCurrentChunk++;   // Always increase
        return sltProceed;  // Always proceed
    }
public:
    LedRgbBlinker_t(const PinOutputPushPull_t ARed, const PinOutputPushPull_t AGreen, const PinOutputPushPull_t ABlue) :
        BaseSequencer(), R(ARed), G(AGreen), B(ABlue) {}
    void Init() {
        R.Init();
        G.Init();
        B.Init();
        SetColor(clBlack);
    }
    void SetColor(Color_t AColor) {
        R.Set(AColor.R);
        G.Set(AColor.G);
        B.Set(AColor.B);
    }
};
#endif

#if 0  // =========================== LedRGB Parent
       // =============================
class LedRGBParent_t : public BaseSequencer<LedRGBChunk_t> {
protected:
    const PinOutputPWM_t  R, G, B;
    const uint32_t kPWMFreq;
    Color_t ICurrColor;
    void ISwitchOff() {
        SetColor(clBlack);
        ICurrColor = clBlack;
    }
    SequencerLoopTask ISetup() {
        if(ICurrColor != IPCurrentChunk->Color) {
            if(IPCurrentChunk->value == 0) {     // If smooth time is zero,
                SetColor(IPCurrentChunk->Color); // set color now,
                ICurrColor = IPCurrentChunk->Color;
                IPCurrentChunk++;                // and goto next chunk
            }
            else {
                ICurrColor.Adjust(IPCurrentChunk->Color);
                SetColor(ICurrColor);
                // Check if completed now
                if(ICurrColor == IPCurrentChunk->Color) IPCurrentChunk++;
                else { // Not completed
                    // Calculate time to next adjustment
                    uint32_t delay = ICurrColor.DelayToNextAdj(IPCurrentChunk->Color, IPCurrentChunk->value);
                    SetupDelay(delay);
                    return sltBreak;
                } // Not completed
            } // if time > 256
        } // if color is different
        else IPCurrentChunk++; // Color is the same, goto next chunk
        return sltProceed;
    }
public:
    LedRGBParent_t(
            const PwmSetup ARed,
            const PwmSetup AGreen,
            const PwmSetup ABlue,
            const uint32_t APWMFreq) :
        BaseSequencer(), R(ARed), G(AGreen), B(ABlue), kPWMFreq(APWMFreq) {}
    void Init() {
        R.Init();
        R.SetFrequencyHz(kPWMFreq);
        G.Init();
        G.SetFrequencyHz(kPWMFreq);
        B.Init();
        B.SetFrequencyHz(kPWMFreq);
        SetColor(clBlack);
    }
    bool IsOff() { return (ICurrColor == clBlack) and IsIdle(); }
    virtual void SetColor(Color_t AColor) {}
};
#endif

#if 0  // ============================== LedRGB
       // =================================
class LedRGB_t : public LedRGBParent_t {
public:
    LedRGB_t(
            const PwmSetup ARed,
            const PwmSetup AGreen,
            const PwmSetup ABlue,
            const uint32_t afreq = 0xFFFFFFFF) :
                LedRGBParent_t(ARed, AGreen, ABlue, afreq) {}

    void SetColor(Color_t AColor) {
        R.Set(AColor.R);
        G.Set(AColor.G);
        B.Set(AColor.B);
    }
};
#endif

#if 0  // =========================== RGB LED with power
       // ========================
class LedRGBwPower_t : public LedRGBParent_t {
private:
    const PinOutput_t PwrPin;
public:
    LedRGBwPower_t(
            const PwmSetup ARed,
            const PwmSetup AGreen,
            const PwmSetup ABlue,
            const PinOutput_t APwrPin,
            const uint32_t afreq = 0xFFFFFFFF) :
                LedRGBParent_t(ARed, AGreen, ABlue, afreq), PwrPin(APwrPin) {}
    void Init() {
        PwrPin.Init();
        LedRGBParent_t::Init();
    }
    void SetColor(Color_t AColor) {
        if(AColor == clBlack) PwrPin.SetLo();
        else PwrPin.SetHi();
        R.Set(AColor.R);
        G.Set(AColor.G);
        B.Set(AColor.B);
    }
};
#endif

#if 0  // ====================== LedRGB with Luminocity
       // =========================
class LedRGBLum_t : public LedRGBParent_t {
public:
    LedRGBLum_t(
            const PwmSetup ARed,
            const PwmSetup AGreen,
            const PwmSetup ABlue,
            const uint32_t afreq = 0xFFFFFFFF) :
                LedRGBParent_t(ARed, AGreen, ABlue, afreq) {}

    void SetColor(Color_t AColor) {
        R.Set(AColor.R * AColor.Brt);
        G.Set(AColor.G * AColor.Brt);
        B.Set(AColor.B * AColor.Brt);
    }
};
#endif

#if 0  // ============================ LedHSV
       // ===================================
class LedHSV_t : public BaseSequencer<LedHSVChunk_t> {
protected:
    const PinOutputPWM_t  R, G, B;
    const uint32_t kPWMFreq;
    ColorHSV_t ICurrColor;
    void ISwitchOff() {
        SetColor(clBlack);
        ICurrColor.V = 0;
    }
    SequencerLoopTask ISetup() {
        if(ICurrColor != IPCurrentChunk->Color) {
            if(IPCurrentChunk->value == 0) {     // If smooth time is zero,
                SetColor(IPCurrentChunk->Color); // set color now,
                ICurrColor = IPCurrentChunk->Color;
                IPCurrentChunk++;                // and goto next chunk
            }
            else {
                ICurrColor.Adjust(IPCurrentChunk->Color);
                SetColor(ICurrColor);
                // Check if completed now
                if(ICurrColor == IPCurrentChunk->Color) IPCurrentChunk++;
                else { // Not completed
                    // Calculate time to next adjustment
                    uint32_t delay = ICurrColor.DelayToNextAdj(IPCurrentChunk->Color, IPCurrentChunk->value);
                    SetupDelay(delay);
                    return sltBreak;
                } // Not completed
            } // if time > 256
        } // if color is different
        else IPCurrentChunk++; // Color is the same, goto next chunk
        return sltProceed;
    }
public:
    LedHSV_t(
            const PwmSetup ARed,
            const PwmSetup AGreen,
            const PwmSetup ABlue,
            const uint32_t APWMFreq = 0xFFFFFFFF) :
        BaseSequencer(), R(ARed), G(AGreen), B(ABlue), kPWMFreq(APWMFreq) {}
    void Init() {
        R.Init();
        R.SetFrequencyHz(kPWMFreq);
        G.Init();
        G.SetFrequencyHz(kPWMFreq);
        B.Init();
        B.SetFrequencyHz(kPWMFreq);
        SetColor(clBlack);
    }
    bool IsOff() { return (ICurrColor == hsvBlack) and IsIdle(); }
    void SetColor(Color_t ColorRgb) {
        R.Set(ColorRgb.R);
        G.Set(ColorRgb.G);
        B.Set(ColorRgb.B);
    }
    void SetColor(ColorHSV_t ColorHsv) {
        SetColor(ColorHsv.ToRGB());
    }
    void SetColorAndMakeCurrent(ColorHSV_t ColorHsv) {
        SetColor(ColorHsv.ToRGB());
        ICurrColor = ColorHsv;
    }
    void SetCurrentH(uint16_t NewH) {
        ICurrColor.H = NewH;
        SetColor(ICurrColor);
    }
};
#endif

#endif  // LED_H_
