//----------------------------------------------------------------------------
// MyEQ.cpp  - effects processor for MyEQ graphical equalizer
// written by G.Sternagl, September 2003
// based on the mbeq LADSPA plugin, written by Steve Harris
//----------------------------------------------------------------------------

#include "iostream"
#include "cstdlib"
#include "cmath"

#include "AudioEffect.cpp"
#include "audioeffectx.h"
#include "audioeffectx.cpp"

#include "fftw3.h"
typedef fftwf_plan fft_plan;
typedef float fftw_real;

fft_plan plan_rc1 = NULL, plan_rc2 = NULL;
fft_plan plan_cr1 = NULL, plan_cr2 = NULL;

#define FFT_LENGTH  1024
#define OVER_SAMP   4

// define here either 15 bands or 31 bands (1/3 octave)
// other values than 15 or 31 are not supported at the moment !!!
#ifndef NR_EQ_BANDS
#   define NR_EQ_BANDS  31
#elif ((NR_EQ_BANDS != 15) && (NR_EQ_BANDS != 31))
#   error
#endif

#define FIRST_BAND  0
#define LAST_BAND   (NR_EQ_BANDS - 1)
#define K_OUTPUT    (LAST_BAND + 1)
#define K_RESET     (K_OUTPUT + 1)

#define FIRST_PARAM FIRST_BAND
#define LAST_PARAM  K_OUTPUT

#define LIMIT(v,l,u) ((v)<(l)?(l):((v)>(u)?(u):(v)))

#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif

//-----------------------------------------------------------------------------------------
// define EQ Bands for 15 Bands (2/3 octave) and 31 Bands (1/3 octave)
float bands [NR_EQ_BANDS] =
{
#if (NR_EQ_BANDS == 15)
    25.0f, 40.0f, 63.0f, 100.0f, 160.0f, 250.0f, 400.0f, 630.0f, 1000.0f,
  1600.0f, 2500.0f, 4000.0f, 6300.0f, 10000.0f, 16000.0f
#else
    20.0f, 25.0f, 31.5f, 40.0f, 50.0f, 63.0f, 80.0f, 100.0f, 125.0f,
   160.0f, 200.0f, 250.0f, 315.0f, 400.0f, 500.0f, 630.0f, 800.0f,
  1000.0f, 1250.0f, 1600.0f, 2000.0f, 2500.0f, 3150.0f, 4000.0f,
  5000.0f, 6300.0f, 8000.0f, 10000.0f, 12500.0f, 16000.0f, 20000.0f
#endif
};

//-----------------------------------------------------------------------------------------
enum
{
    // Global
    kNumPrograms = 10,

    // Parameters Tags
    kNumParams = NR_EQ_BANDS + 1
};

//-----------------------------------------------------------------------------------------
class MyEQ : public AudioEffectX
{
public:
    MyEQ(audioMasterCallback audioMaster);
    ~MyEQ();

    virtual void    process (float **inputs, float **outputs, long sampleFrames);
    virtual void    processReplacing (float **inputs, float **outputs, long sampleFrames);
    void            processAudio (float **inputs, float **outputs, long sampleFrames, bool replace);

    virtual void    resume ();

    virtual void    setParameter (long index, float value);
    virtual float   getParameter (long index);
    virtual void    getParameterName (long index, char *text);
    virtual void    getParameterDisplay (long index, char *text);

    virtual void    setSampleRate (float sampleRate);

    void    allocateBuffers();
    void    releaseBuffers();
    void    calcBins();
    void    calcCoeffs();
    void    finishEQ();
    void    initEQ();
    void    resetEQ();
    void    setValuesEQ();

    long dispatcher (long opCode, long index, long value, void *ptr, float opt);

protected:

    // gain parameters for EQ-bands and output gain
    float   fGains[NR_EQ_BANDS+1];
    float   fOutput;

    // EQ-frequencies
    float   *fBands;
    int     *bin_base;
    float   *bin_delta;

    fftw_real   *comp1, *comp2;

    float   *db_table;
    float   *coeffs;
    long    fifo_pos;
    float   *in_fifo1, *in_fifo2;
    float   *out_accum1, *out_accum2;
    float   *out_fifo1, *out_fifo2;

    fftw_real   *real1, *real2;

    float   *window;
    float   run_adding_gain;

    float   fix;
    int     step_size;
    int     latency;

    unsigned long s_rate;
};

//-----------------------------------------------------------------------------
MyEQ::MyEQ (audioMasterCallback audioMaster)
    : AudioEffectX (audioMaster, kNumPrograms, (LAST_PARAM - FIRST_PARAM + 1))    // 1 program
{
    s_rate = 44100;

    initEQ();

    setUniqueID ((int)"x0x1");
    setNumInputs(2);        // stereo in
    setNumOutputs(2);        // stereo out
    canMono();                // makes sense to feed both inputs with the same signal
    canProcessReplacing();    // supports both accumulating and replacing output
    hasVu();
}

//-----------------------------------------------------------------------------------------
MyEQ::~MyEQ()
{
    finishEQ();
}

//-----------------------------------------------------------------------------------------
void MyEQ::setParameter (long index, float value)
{
    if ((index >= FIRST_BAND) && (index <= LAST_BAND))
    {
        fGains [index - FIRST_BAND] = value * 2.0f;
    }
    else if (index == K_OUTPUT)
    {
        fOutput = value;
    }
    setValuesEQ();
}

//-----------------------------------------------------------------------------------------
float MyEQ::getParameter (long index)
{
    float value;

    if ((index >= FIRST_BAND) && (index <= LAST_BAND))
    {
        value = fGains [index - FIRST_BAND] * 0.5f;
    }
    else if (index == K_OUTPUT)
    {
        value = fOutput;
    }

    return value;
}

//-----------------------------------------------------------------------------------------
void MyEQ::getParameterName (long index, char *text)
{
    if ((index >= FIRST_BAND) && (index <= LAST_BAND))
    {
        sprintf (text, "%.1f Hz", bands [index - FIRST_BAND]);
    }
    else if (index == K_OUTPUT)
    {
        strcpy (text, "Gain");
    }
}

//-----------------------------------------------------------------------------------------
void MyEQ::getParameterDisplay (long index, char *text)
{
    if ((index >= FIRST_BAND) && (index <= LAST_BAND))
    {
        sprintf (text, "%d", (int)((fGains [index - FIRST_BAND] - 1.0f) * 100));
    }
    else if (index == K_OUTPUT)
    {
        sprintf (text, "%d", (int)(fOutput * 100));
    }
}

//-----------------------------------------------------------------------------------------
void MyEQ::allocateBuffers()
{
    in_fifo1   = (float *) calloc (FFT_LENGTH, sizeof(float));
    in_fifo2   = (float *) calloc (FFT_LENGTH, sizeof(float));
    out_fifo1  = (float *) calloc (FFT_LENGTH, sizeof(float));
    out_fifo2  = (float *) calloc (FFT_LENGTH, sizeof(float));
    out_accum1 = (float *) calloc (FFT_LENGTH * 2, sizeof(float));
    out_accum2 = (float *) calloc (FFT_LENGTH * 2, sizeof(float));
    real1      = (float *) calloc (FFT_LENGTH, sizeof(fftw_real));
    real2      = (float *) calloc (FFT_LENGTH, sizeof(fftw_real));
    comp1      = (float *) calloc (FFT_LENGTH, sizeof(fftw_real));
    comp2      = (float *) calloc (FFT_LENGTH, sizeof(fftw_real));
    window     = (float *) calloc (FFT_LENGTH, sizeof(float));
    bin_base   = (int *)   calloc (FFT_LENGTH/2, sizeof(int));
    bin_delta  = (float *) calloc (FFT_LENGTH/2, sizeof(float));
    coeffs     = (float *) calloc (FFT_LENGTH/2+1, sizeof(float));
    db_table   = (float *) malloc (1000 * sizeof(float));
}

//-----------------------------------------------------------------------------------------
void MyEQ::calcBins()
{
    int        i;
    int        bin = 0;
    float    last_bin;
    float    next_bin;
    float    hz_per_bin = (float)s_rate / (float)FFT_LENGTH;

    while (bin <= bands[0]/hz_per_bin) {
        bin_base[bin] = 0;
        bin_delta[bin++] = 0.0f;
    }

    for (i = 1; 1 < NR_EQ_BANDS - 1 && bin < (FFT_LENGTH/2)-1 && bands[i+1] < s_rate/2; i++) {
        last_bin = (float)bin;
        next_bin = (bands[i+1])/hz_per_bin;
        while (bin <= next_bin) {
            bin_base[bin] = i;
            bin_delta[bin] = (float)(bin - last_bin) / (float)(next_bin - last_bin);
            bin++;
        }
    }
    for (; bin < (FFT_LENGTH/2); bin++) {
        bin_base[bin] = NR_EQ_BANDS - 1;
        bin_delta[bin] = 0.0f;
    }
}

//-----------------------------------------------------------------------------------------
void MyEQ::calcCoeffs()
{
    // Calculate coefficients for each bin of FFT

    coeffs[0] = 0.0f;

    for (int bin = 1; bin < (FFT_LENGTH / 2 - 1); bin++)
    {
        coeffs[bin] = ((1.0f - bin_delta[bin]) * fGains [bin_base[bin]])
                        + (bin_delta[bin] * fGains [bin_base[bin] + 1]);
    }
}

//-----------------------------------------------------------------------------------------
void MyEQ::setSampleRate (float sampleRate)
{
    s_rate = (unsigned long) sampleRate;

    calcBins();
    calcCoeffs();
}

//-----------------------------------------------------------------------------------------
void MyEQ::setValuesEQ()
{
    calcCoeffs();
}

//-----------------------------------------------------------------------------------------
void MyEQ::resetEQ()
{
    fGains [NR_EQ_BANDS] = 0.0f;     // this one is needed for the FFT
    for (int i = 0; i<NR_EQ_BANDS; i++)
    {
        fGains[i] = 1.0f; // 0 dB
    }
    fOutput = 1.0f; // 0 dB
}

//-----------------------------------------------------------------------------------------
void MyEQ::initEQ()
{
    int i;
    float db;

/*
    VstTimeInfo *timeInfo = getTimeInfo (kVstPpqPosValid|kVstTempoValid|kVstBarsValid | kVstTimeSigValid|kVstCyclePosValid|kVstSmpteValid);

    if (timeInfo)
        s_rate = (unsigned long)timeInfo->sampleRate;
    else
        s_rate = 44100;

    std::cout << s_rate << std::endl;
*/

    allocateBuffers();

    // Setup FFT transformations
    plan_rc1 = fftwf_plan_r2r_1d (FFT_LENGTH, real1, comp1, FFTW_R2HC, FFTW_MEASURE);
    plan_rc2 = fftwf_plan_r2r_1d (FFT_LENGTH, real2, comp2, FFTW_R2HC, FFTW_MEASURE);

    plan_cr1 = fftwf_plan_r2r_1d (FFT_LENGTH, comp1, real1, FFTW_HC2R, FFTW_MEASURE);
    plan_cr2 = fftwf_plan_r2r_1d (FFT_LENGTH, comp2, real2, FFTW_HC2R, FFTW_MEASURE);

    // Create raised cosine window table
    for (i=0; i < FFT_LENGTH; i++) {
        window[i] = (float)(-0.5f*cos(2.0f*M_PI*(double)i/(double)FFT_LENGTH)+0.5f);
    }

    // Create db->coeffiecnt lookup table
    for (i=0; i < 1000; i++) {
        db = ((float) i / 41.6666f) - 12.0f;            // -12 .. +12
        db_table[i] = (float) pow (10.0f, db / 20.0f);    // ~ 0.2511 .. 3.98
    }

    // Setup fixed values
    fix = 2.0f / ((float) FFT_LENGTH * (float) OVER_SAMP);
    step_size = FFT_LENGTH / OVER_SAMP;
    latency = FFT_LENGTH - step_size;

    // Create FFT bin -> band + delta tables
    calcBins();

    // initialize EQ bands gains
    resetEQ();

    // initialize coeffecient table
    calcCoeffs();

    // Frequency Bands;
    fBands = bands;

    fifo_pos = 0;
}

//-----------------------------------------------------------------------------------------
void MyEQ::releaseBuffers()
{
    free(in_fifo1);
    free(in_fifo2);
    free(out_fifo1);
    free(out_fifo2);
    free(out_accum1);
    free(out_accum2);
    free(real1);
    free(real2);
    free(comp1);
    free(comp2);
    free(window);
    free(bin_base);
    free(bin_delta);
    free(db_table);
    free(coeffs);
}

//-----------------------------------------------------------------------------------------
void MyEQ::finishEQ()
{
    releaseBuffers();
}

//-----------------------------------------------------------------------------------------
void MyEQ::resume ()
{
    wantEvents (false);
}

//-----------------------------------------------------------------------------------------
void MyEQ::process (float **inputs, float **outputs, long sampleFrames)
{
    processAudio (inputs, outputs, sampleFrames, false);
}

//-----------------------------------------------------------------------------------------
void MyEQ::processReplacing (float **inputs, float **outputs, long sampleFrames)
{
    processAudio (inputs, outputs, sampleFrames, true);
}

#undef buffer_wr_repl
#undef buffer_rw_add

#define buffer_wr_repl(b, v) (b  = (v * tmp_gain))
#define buffer_wr_add(b, v) (b += (v) * tmp_gain)

//-----------------------------------------------------------------------------------------
void MyEQ::processAudio (float **inputs, float **outputs, long sampleFrames, bool replace)
{
    float *in1  =  inputs[0];        // Left  channel input
    float *in2  =  inputs[1];        // Right channel input
    float *out1 = outputs[0];        // Left  channel output
    float *out2 = outputs[1];        // Right channel output

    fftw_real *tmp_comp1    = MyEQ::comp1;
    fftw_real *tmp_comp2    = MyEQ::comp2;
    long    tmp_fifo_pos    = MyEQ::fifo_pos;
    float  *tmp_in_fifo1    = MyEQ::in_fifo1;
    float  *tmp_in_fifo2    = MyEQ::in_fifo2;
    float  *tmp_out_accum1  = MyEQ::out_accum1;
    float  *tmp_out_accum2  = MyEQ::out_accum2;
    float  *tmp_out_fifo1   = MyEQ::out_fifo1;
    float  *tmp_out_fifo2   = MyEQ::out_fifo2;
    fftw_real *tmp_real1    = MyEQ::real1;
    fftw_real *tmp_real2    = MyEQ::real2;
    float  *tmp_window      = MyEQ::window;
    float  tmp_gain         = MyEQ::fOutput;
    float  *tmp_coeffs      = MyEQ::coeffs;

    int i, pos;

    if (tmp_fifo_pos == 0)
        tmp_fifo_pos = latency;

    for (pos = 0; pos < sampleFrames; pos++)
    {
        tmp_in_fifo1[tmp_fifo_pos] = in1[pos];
        tmp_in_fifo2[tmp_fifo_pos] = in2[pos];

        if (replace) {
            buffer_wr_repl(out1[pos], tmp_out_fifo1[tmp_fifo_pos - latency]);
            buffer_wr_repl(out2[pos], tmp_out_fifo2[tmp_fifo_pos - latency]);
        }
        else {
            buffer_wr_add(out1[pos], tmp_out_fifo1[tmp_fifo_pos - latency]);
            buffer_wr_add(out2[pos], tmp_out_fifo2[tmp_fifo_pos - latency]);
        }
        tmp_fifo_pos++;

        // If the FIFO is full
        if (tmp_fifo_pos >= FFT_LENGTH)
        {
            tmp_fifo_pos = latency;

            // Window input FIFO
            for (i=0; i < FFT_LENGTH; i++) {
                tmp_real1[i] = tmp_in_fifo1[i] * tmp_window[i];
                tmp_real2[i] = tmp_in_fifo2[i] * tmp_window[i];
            }

            // Run the real->complex transform
            fftwf_execute (plan_rc1);
            fftwf_execute (plan_rc2);

            // Multiply the bins magnitudes by the coeficients
            for (i = 0; i < FFT_LENGTH/2; i++) {
                float coeff = tmp_coeffs[i];
                tmp_comp1[i] *= coeff;
                tmp_comp2[i] *= coeff;
                tmp_comp1[FFT_LENGTH-i] *= coeff;
                tmp_comp2[FFT_LENGTH-i] *= coeff;
            }

            // Run the complex->real transform
            fftwf_execute (plan_cr1);
            fftwf_execute (plan_cr2);

            // Window into the output accumulator
            for (i = 0; i < FFT_LENGTH; i++) {
                tmp_out_accum1[i] += fix * tmp_window[i] * tmp_real1[i];
                tmp_out_accum2[i] += fix * tmp_window[i] * tmp_real2[i];

            }
            for (i = 0; i < step_size; i++) {
                tmp_out_fifo1[i] = tmp_out_accum1[i];
                tmp_out_fifo2[i] = tmp_out_accum2[i];
            }

            // Shift output accumulator
            memmove (tmp_out_accum1, tmp_out_accum1 + step_size, FFT_LENGTH*sizeof(float));
            memmove (tmp_out_accum2, tmp_out_accum2 + step_size, FFT_LENGTH*sizeof(float));

            // Shift input fifo
            for (i = 0; i < latency; i++) {
                tmp_in_fifo1[i] = tmp_in_fifo1[i+step_size];
                tmp_in_fifo2[i] = tmp_in_fifo2[i+step_size];
            }
        }
    }

    // Store the fifo_position
    MyEQ::fifo_pos = tmp_fifo_pos;
}

//-----------------------------------------------------------------------------------------
long MyEQ::dispatcher (long opCode, long index, long value, void *ptr, float opt)
{
    int result = 0;

    switch (opCode)
    {
        case effSetSampleRate:
        {
            setSampleRate ((float)((int) opt));
            finishEQ ();
            initEQ ();
            break;
        }

        default:
            result = AudioEffect::dispatcher (opCode, index, value, ptr, opt);
    }

    return result;
}


//-----------------------------------------------------------------------------------------
#ifdef __GNUC__
  AEffect* main_plugin (audioMasterCallback audioMaster) asm ("main");
  #define main main_plugin
#endif

AEffect *main (audioMasterCallback audioMaster)
{
    std::cout << "main called !" << std::endl;

    MyEQ* effect = new MyEQ (audioMaster);
    if (effect == 0)
        return 0;

    std::cout << "effect exists!" << std::endl;

    return effect->getAeffect ();
}
