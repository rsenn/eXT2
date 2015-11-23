#include <cstdlib>
#include <cmath>
#include <string.h>

#include "../ladspa/ladspa.h"


#define VCF_INPUT              0
#define VCF_OUTPUT             1
#define VCF_GAIN               2
#define VCF_FREQ_OFS           3
#define VCF_FREQ_PITCH         4

#ifdef WITH_CV_IN

#define VCF_FREQ_IN            5
#define VCF_RESO_OFS           6
#define VCF_RESO_IN            7
#define VCF_DBGAIN_OFS         8
#define VCF_DBGAIN_IN          9
#define PORT_COUNT_1           8
#define PORT_COUNT_2          10

#else

#define VCF_RESO_OFS           5
#define VCF_DBGAIN_OFS         6
#define PORT_COUNT_1           6
#define PORT_COUNT_2           7

#endif

#define MIN_FREQ              20
#define MAX_FREQ           20000
#define Q_MIN              0.001
#define Q_MAX                1.0
#define DBGAIN_MIN           6.0
#define DBGAIN_MAX          24.0 
#define DBGAIN_SCALE         5.0
#define Q_SCALE             32.0

LADSPA_Descriptor *vcf_reslp_Descriptor = NULL;
LADSPA_Descriptor *vcf_lp_Descriptor = NULL;
LADSPA_Descriptor *vcf_hp_Descriptor = NULL;
LADSPA_Descriptor *vcf_bp1_Descriptor = NULL;
LADSPA_Descriptor *vcf_bp2_Descriptor = NULL;
LADSPA_Descriptor *vcf_notch_Descriptor = NULL;
LADSPA_Descriptor *vcf_peakeq_Descriptor = NULL;
LADSPA_Descriptor *vcf_lshelf_Descriptor = NULL;
LADSPA_Descriptor *vcf_hshelf_Descriptor = NULL;

typedef struct {
  LADSPA_Data *input;
  LADSPA_Data *output;
  LADSPA_Data *gain;
  LADSPA_Data *freq_ofs;
  LADSPA_Data *freq_pitch;
  LADSPA_Data *reso_ofs;  

#ifdef WITH_CV_IN  
  
  LADSPA_Data *freq_in;
  LADSPA_Data *reso_in;

#endif
 
  double rate, buf[2];
} filtType1;

typedef struct {
  LADSPA_Data *input;
  LADSPA_Data *output;
  LADSPA_Data *gain;
  LADSPA_Data *freq_ofs;
  LADSPA_Data *freq_pitch;
  LADSPA_Data *reso_ofs;

#ifdef WITH_CV_IN
  
  LADSPA_Data *freq_in;
  LADSPA_Data *reso_in; 
         
#endif

  double rate, buf[4];
} filtType2;

typedef struct {
  LADSPA_Data *input;
  LADSPA_Data *output; 
  LADSPA_Data *gain;
  LADSPA_Data *freq_ofs;  
  LADSPA_Data *freq_pitch;
  LADSPA_Data *reso_ofs;
  LADSPA_Data *dBgain_ofs;
  
#ifdef WITH_CV_IN  
  
  LADSPA_Data *freq_in;
  LADSPA_Data *reso_in; 
  LADSPA_Data *dBgain_in;
  
#endif  
  
  double rate, buf[4];
} filtType3;

const LADSPA_Descriptor *ladspa_descriptor(unsigned long index) {

  switch (index) {
  case 0: return vcf_reslp_Descriptor;
  case 1: return vcf_lp_Descriptor;
  case 2: return vcf_hp_Descriptor;
  case 3: return vcf_bp1_Descriptor;
  case 4: return vcf_bp2_Descriptor;
  case 5: return vcf_notch_Descriptor;
  case 6: return vcf_peakeq_Descriptor;
  case 7: return vcf_lshelf_Descriptor;
  case 8: return vcf_hshelf_Descriptor;
  default: return 0;
  }
}

/****************** Resonant Lowpass, Formula by Paul Kellett **********************/

LADSPA_Handle instantiate_vcf_reslp(const LADSPA_Descriptor *descriptor, 
                                    unsigned long rate) {
 
  filtType1 *pluginData = (filtType1 *)malloc(sizeof(filtType1));
  pluginData->rate = rate;
  return (LADSPA_Handle)pluginData;
}

void activate_vcf_reslp(LADSPA_Handle instance) {
 
  filtType1 *pluginData = (filtType1 *)instance;
  pluginData->buf[0] = 0;
  pluginData->buf[1] = 0;
}

void connect_port_vcf_reslp(LADSPA_Handle instance, unsigned long port, 
                            LADSPA_Data *data) {
                            
  filtType1 *pluginData = (filtType1 *)instance;
  switch (port) {
  case VCF_INPUT: 
    pluginData->input = data;    
    break;
  case VCF_OUTPUT: 
    pluginData->output = data;
    break;
  case VCF_GAIN: 
    pluginData->gain = data;
    break;
  case VCF_FREQ_OFS: 
    pluginData->freq_ofs = data; 
    break;
  case VCF_FREQ_PITCH: 
    pluginData->freq_pitch = data;
    break;
  case VCF_RESO_OFS:
    pluginData->reso_ofs = data;
    break;
    
#ifdef WITH_CV_IN    

  case VCF_FREQ_IN: 
    pluginData->freq_in = data;
    break;
  case VCF_RESO_IN:
    pluginData->reso_in = data;
    break;
 
#endif  

  }  
}

void run_vcf_reslp(LADSPA_Handle instance, unsigned long samples) {
  
  int l1;
  double f0, q0, f, q, fa, fb, rate, rate_f, k;
  double *buf;
  filtType1 *pluginData = (filtType1 *)instance;

  LADSPA_Data *input = pluginData->input;
  LADSPA_Data *output = pluginData->output;
  LADSPA_Data gain = *(pluginData->gain);
  LADSPA_Data freq_ofs = *(pluginData->freq_ofs);
  LADSPA_Data freq_pitch = (*(pluginData->freq_pitch) > 0) ? 1.0 + *(pluginData->freq_pitch) / 2.0 : 1.0 / (1.0 - *(pluginData->freq_pitch) / 2.0);
  LADSPA_Data reso_ofs = *(pluginData->reso_ofs);
  
#ifdef WITH_CV_IN

  LADSPA_Data *freq_in = pluginData->freq_in;
  LADSPA_Data *reso_in = pluginData->reso_in; 
  
#endif  
  
  rate = pluginData->rate;
  rate_f = 44100.0 / rate;
  buf = pluginData->buf;
  f0 = freq_ofs / (double)MAX_FREQ * rate_f * 2.85;
  q0 = reso_ofs;

#ifdef WITH_CV_IN
  
  if (!(freq_in || reso_in)) {
  
#endif
  
    f = f0 * freq_pitch;
    if (f > 0.99) f = 0.99;
    q = q0;
    fa = 1.0 - f;
    fb = q * (1.0 + (1.0 / fa));
    for (l1 = 0; l1 < samples; l1++) {
      buf[0] = fa * buf[0] + f * (input[l1] + fb * (buf[0] - buf[1])); 
      buf[1] = fa * buf[1] + f * buf[0];
      output[l1] = gain * buf[1];
    }
 
#ifdef WITH_CV_IN    
    
  } else {
    if (!reso_in) {
      q = q0;
      k =  MAX_FREQ * 2.85;
      for (l1 = 0; l1 < samples; l1++) {
        f = (freq_in[l1] > 0)
          ? (freq_in[l1] * k + (freq_ofs - MIN_FREQ)) / (double)MAX_FREQ * freq_pitch * rate_f
          : f0 * freq_pitch;
        if (f < 0) f = 0;
        if (f > 0.99) f = 0.99;
        fa = 1.0 - f;
        fb = q * (1.0 + (1.0 / fa));
        buf[0] = fa * buf[0] + f * (input[l1] + fb * (buf[0] - buf[1])); 
        buf[1] = fa * buf[1] + f * buf[0];
        output[l1] = gain * buf[1];
      }
    } else {
      for (l1 = 0; l1 < samples; l1++) {
        f = (freq_in && (freq_in[l1] > 0))
          ? (freq_in[l1] * MAX_FREQ * 2.85 + (freq_ofs - MIN_FREQ)) / (double)MAX_FREQ * freq_pitch * rate_f
          : f0 * freq_pitch;
        if (f < 0) f = 0;
        if (f > 0.99) f = 0.99;
        q = q0 + reso_in[l1];
        if (q < 0) q = 0;
        if (q > 1) q = 1;
        fa = 1.0 - f;
        fb = q * (1.0 + (1.0 / fa));
        buf[0] = fa * buf[0] + f * (input[l1] + fb * (buf[0] - buf[1])); 
        buf[1] = fa * buf[1] + f * buf[0];
        output[l1] = gain * buf[1];
      }
    }
  }
  
#endif  
  
}

void cleanup_vcf_reslp(LADSPA_Handle instance) {

  free(instance);
}
 
/******************* Lowpass, Formula by Robert Bristow-Johnson ********************/

LADSPA_Handle instantiate_vcf_lp(const LADSPA_Descriptor *descriptor, 
                                    unsigned long rate) {
 
  filtType2 *pluginData = (filtType2 *)malloc(sizeof(filtType2));
  pluginData->rate = rate;
  return (LADSPA_Handle)pluginData;
}

void activate_vcf_lp(LADSPA_Handle instance) {

  int l1;
 
  filtType2 *pluginData = (filtType2 *)instance;
  for (l1 = 0; l1 < 4; l1++) {
    pluginData->buf[l1] = 0;
  }  
}

void connect_port_vcf_lp(LADSPA_Handle instance, unsigned long port, 
                            LADSPA_Data *data) {
                            
  filtType2 *pluginData = (filtType2 *)instance;
  switch (port) {
  case VCF_INPUT: 
    pluginData->input = data;    
    break;
  case VCF_OUTPUT: 
    pluginData->output = data;
    break;
  case VCF_GAIN: 
    pluginData->gain = data;
    break;
  case VCF_FREQ_OFS: 
    pluginData->freq_ofs = data; 
    break;
  case VCF_FREQ_PITCH: 
    pluginData->freq_pitch = data;
    break;
  case VCF_RESO_OFS:
    pluginData->reso_ofs = data;
    break;
  
#ifdef WITH_CV_IN    
    
  case VCF_FREQ_IN: 
    pluginData->freq_in = data;
    break;
  case VCF_RESO_IN:
    pluginData->reso_in = data;
    break;
    
#endif    
    
  }
}

void run_vcf_lp(LADSPA_Handle instance, unsigned long samples) {
  
  int l1;
  double f0, q0, f, q, fa, fb, pi2_rate;
  double *buf;
  double iv_sin, iv_cos, iv_alpha, inv_a0, a0, a1, a2, b0, b1, b2;
  filtType2 *pluginData = (filtType2 *)instance;

  LADSPA_Data *input = pluginData->input;
  LADSPA_Data *output = pluginData->output;
  LADSPA_Data gain = *(pluginData->gain);
  LADSPA_Data freq_ofs = *(pluginData->freq_ofs);
  LADSPA_Data freq_pitch = (*(pluginData->freq_pitch) > 0) ? 1.0 + *(pluginData->freq_pitch) / 2.0 : 1.0 / (1.0 - *(pluginData->freq_pitch) / 2.0);
  LADSPA_Data reso_ofs = *(pluginData->reso_ofs);

#ifdef WITH_CV_IN

  LADSPA_Data *freq_in = pluginData->freq_in;
  LADSPA_Data *reso_in = pluginData->reso_in; 
  
#endif  
  
  pi2_rate = 2.0 * M_PI / pluginData->rate;
  buf = pluginData->buf;
  f0 = freq_ofs;
  q0 = reso_ofs;
  
#ifdef WITH_CV_IN  
  
  if (!(freq_in || reso_in)) {
  
#endif  
  
    f = f0 * freq_pitch;
    if (f > MAX_FREQ) f = MAX_FREQ;
    q = q0;
    iv_sin = sin(pi2_rate * f);
    iv_cos = cos(pi2_rate * f);
    iv_alpha = iv_sin/(Q_SCALE * q);
    b0 = (1.0 - iv_cos) / 2.0;
    b1 = 1.0 - iv_cos;
    b2 = b0;
    a0 = 1.0 + iv_alpha;
    a1 = -2.0 * iv_cos;
    a2 = 1.0 - iv_alpha;
    inv_a0 = 1.0 / a0;
    for (l1 = 0; l1 < samples; l1++) {
      output[l1] = inv_a0 * (gain * (b0 * input[l1] + b1 * buf[0] + b2 * buf[1])
                             - a1 * buf[2] - a2 * buf[3]);
      buf[1] = buf[0];                          
      buf[0] = input[l1];
      buf[3] = buf[2];
      buf[2] = output[l1];
    }
    
#ifdef WITH_CV_IN    
    
  } else {
    if (!reso_in) {
      q = q0; 
      for (l1 = 0; l1 < samples; l1++) {
        f = (freq_in[l1] > 0) 
        ? (freq_in[l1] * MAX_FREQ + f0 - MIN_FREQ) * freq_pitch 
        : f0 * freq_pitch;
        if (f < MIN_FREQ) f = MIN_FREQ;
        if (f > MAX_FREQ) f = MAX_FREQ;
        iv_sin = sin(pi2_rate * f);
        iv_cos = cos(pi2_rate * f);
        iv_alpha = iv_sin/(Q_SCALE * q);
        b0 = (1.0 - iv_cos) / 2.0;
        b1 = 1.0 - iv_cos;
        b2 = b0;
        a0 = 1.0 + iv_alpha;
        a1 = -2.0 * iv_cos;
        a2 = 1.0 - iv_alpha;
        output[l1] = 1.0 / a0 * (gain * (b0 * input[l1] + b1 * buf[0] + b2 * buf[1])
                                 - a1 * buf[2] - a2 * buf[3]);
        buf[1] = buf[0];                          
        buf[0] = input[l1];
        buf[3] = buf[2];
        buf[2] = output[l1];
      }
    } else {
      for (l1 = 0; l1 < samples; l1++) {
        f = (freq_in && (freq_in[l1] > 0)) 
        ? (freq_in[l1] * MAX_FREQ + f0 - MIN_FREQ) * freq_pitch 
        : f0 * freq_pitch;
        if (f < MIN_FREQ) f = MIN_FREQ;
        if (f > MAX_FREQ) f = MAX_FREQ;
        q = q0  + reso_in[l1];
        if (q < Q_MIN) q = Q_MIN;
        if (q > Q_MAX) q = Q_MAX;
        iv_sin = sin(pi2_rate * f);
        iv_cos = cos(pi2_rate * f);
        iv_alpha = iv_sin/(Q_SCALE * q);
        b0 = (1.0 - iv_cos) / 2.0;
        b1 = 1.0 - iv_cos;
        b2 = b0;
        a0 = 1.0 + iv_alpha;
        a1 = -2.0 * iv_cos;
        a2 = 1.0 - iv_alpha;
        inv_a0 = 1.0 / a0;
        output[l1] = inv_a0 * (gain * (b0 * input[l1] + b1 * buf[0] + b2 * buf[1])
                               - a1 * buf[2] - a2 * buf[3]);
        buf[1] = buf[0];                          
        buf[0] = input[l1];
        buf[3] = buf[2];
        buf[2] = output[l1];
      }
    }
  }
  
#endif
  
}

void cleanup_vcf_lp(LADSPA_Handle instance) {

  free(instance);
}
 
/****************** Highpass, Formula by Robert Bristow-Johnson ********************/

LADSPA_Handle instantiate_vcf_hp(const LADSPA_Descriptor *descriptor, 
                                    unsigned long rate) {
 
  filtType2 *pluginData = (filtType2 *)malloc(sizeof(filtType2));
  pluginData->rate = rate;
  return (LADSPA_Handle)pluginData;
}

void activate_vcf_hp(LADSPA_Handle instance) {

  int l1;
 
  filtType2 *pluginData = (filtType2 *)instance;
  for (l1 = 0; l1 < 4; l1++) {
    pluginData->buf[l1] = 0;
  }  
}

void connect_port_vcf_hp(LADSPA_Handle instance, unsigned long port, 
                            LADSPA_Data *data) {
                            
  filtType2 *pluginData = (filtType2 *)instance;
  switch (port) {
  case VCF_INPUT: 
    pluginData->input = data;    
    break;
  case VCF_OUTPUT: 
    pluginData->output = data;
    break;
  case VCF_GAIN: 
    pluginData->gain = data;
    break;
  case VCF_FREQ_OFS: 
    pluginData->freq_ofs = data; 
    break;
  case VCF_FREQ_PITCH: 
    pluginData->freq_pitch = data;
    break;
  case VCF_RESO_OFS:
    pluginData->reso_ofs = data;
    break;
    
#ifdef WITH_CV_IN    

  case VCF_FREQ_IN: 
    pluginData->freq_in = data;
    break;
  case VCF_RESO_IN:
    pluginData->reso_in = data;
    break;

#endif    
    
  }
}

void run_vcf_hp(LADSPA_Handle instance, unsigned long samples) {
  
  int l1;
  double f0, q0, f, q, fa, fb, pi2_rate;
  double *buf;
  double iv_sin, iv_cos, iv_alpha, inv_a0, a0, a1, a2, b0, b1, b2;
  filtType2 *pluginData = (filtType2 *)instance;

  LADSPA_Data *input = pluginData->input;
  LADSPA_Data *output = pluginData->output;
  LADSPA_Data gain = *(pluginData->gain);
  LADSPA_Data freq_ofs = *(pluginData->freq_ofs);
  LADSPA_Data freq_pitch = (*(pluginData->freq_pitch) > 0) ? 1.0 + *(pluginData->freq_pitch) / 2.0 : 1.0 / (1.0 - *(pluginData->freq_pitch) / 2.0);
  LADSPA_Data reso_ofs = *(pluginData->reso_ofs);

#ifdef WITH_CV_IN
  
  LADSPA_Data *freq_in = pluginData->freq_in;
  LADSPA_Data *reso_in = pluginData->reso_in; 
  
#endif  
  
  pi2_rate = 2.0 * M_PI / pluginData->rate;
  buf = pluginData->buf;
  f0 = freq_ofs;
  q0 = reso_ofs;
  
#ifdef WITH_CV_IN  
  
  if (!(freq_in || reso_in)) {
  
#endif  
  
    f = f0 * freq_pitch;
    if (f > MAX_FREQ) f = MAX_FREQ;
    q = q0;
    iv_sin = sin(pi2_rate * f);
    iv_cos = cos(pi2_rate * f);
    iv_alpha = iv_sin/(Q_SCALE * q);
    b0 = (1.0 + iv_cos) / 2.0;
    b1 = - 1.0 - iv_cos;
    b2 = b0;
    a0 = 1.0 + iv_alpha;
    a1 = -2.0 * iv_cos;
    a2 = 1.0 - iv_alpha;
    inv_a0 = 1.0 / a0;
    for (l1 = 0; l1 < samples; l1++) {
      output[l1] = inv_a0 * (gain * (b0 * input[l1] + b1 * buf[0] + b2 * buf[1])
                             - a1 * buf[2] - a2 * buf[3]);
      buf[1] = buf[0];                          
      buf[0] = input[l1];
      buf[3] = buf[2];
      buf[2] = output[l1];
    }
    
#ifdef WITH_CV_IN    
    
  } else {
    if (!reso_in) {
      q = q0; 
      for (l1 = 0; l1 < samples; l1++) {
        f = (freq_in[l1] > 0) 
        ? (freq_in[l1] * MAX_FREQ + f0 - MIN_FREQ) * freq_pitch 
        : f0 * freq_pitch;
        if (f < MIN_FREQ) f = MIN_FREQ;
        if (f > MAX_FREQ) f = MAX_FREQ;
        iv_sin = sin(pi2_rate * f);
        iv_cos = cos(pi2_rate * f);
        iv_alpha = iv_sin/(Q_SCALE * q);
        b0 = (1.0 + iv_cos) / 2.0;
        b1 = - 1.0 - iv_cos;
        b2 = b0;
        a0 = 1.0 + iv_alpha;
        a1 = -2.0 * iv_cos;
        a2 = 1.0 - iv_alpha;
        output[l1] = 1.0 / a0 * (gain * (b0 * input[l1] + b1 * buf[0] + b2 * buf[1])
                                 - a1 * buf[2] - a2 * buf[3]);
        buf[1] = buf[0];                          
        buf[0] = input[l1];
        buf[3] = buf[2];
        buf[2] = output[l1];
      }
    } else {
      for (l1 = 0; l1 < samples; l1++) {
        f = (freq_in && (freq_in[l1] > 0)) 
        ? (freq_in[l1] * MAX_FREQ + f0 - MIN_FREQ) * freq_pitch 
        : f0 * freq_pitch;
        if (f < MIN_FREQ) f = MIN_FREQ;
        if (f > MAX_FREQ) f = MAX_FREQ;
        q = q0  + reso_in[l1];
        if (q < Q_MIN) q = Q_MIN;
        if (q > Q_MAX) q = Q_MAX;
        iv_sin = sin(pi2_rate * f);
        iv_cos = cos(pi2_rate * f);
        iv_alpha = iv_sin/(Q_SCALE * q);
        b0 = (1.0 + iv_cos) / 2.0;
        b1 = - 1.0 - iv_cos;
        b2 = b0;
        a0 = 1.0 + iv_alpha;
        a1 = -2.0 * iv_cos;
        a2 = 1.0 - iv_alpha;
        inv_a0 = 1.0 / a0;
        output[l1] = inv_a0 * (gain * (b0 * input[l1] + b1 * buf[0] + b2 * buf[1])
                               - a1 * buf[2] - a2 * buf[3]);
        buf[1] = buf[0];                          
        buf[0] = input[l1];
        buf[3] = buf[2];
        buf[2] = output[l1];
      }
    }
  }
  
#endif  
  
}

void cleanup_vcf_hp(LADSPA_Handle instance) {

  free(instance);
}

/***************** Bandpass I, Formula by Robert Bristow-Johnson *******************/

LADSPA_Handle instantiate_vcf_bp1(const LADSPA_Descriptor *descriptor, 
                                    unsigned long rate) {
 
  filtType2 *pluginData = (filtType2 *)malloc(sizeof(filtType2));
  pluginData->rate = rate;
  return (LADSPA_Handle)pluginData;
}

void activate_vcf_bp1(LADSPA_Handle instance) {

  int l1;
 
  filtType2 *pluginData = (filtType2 *)instance;
  for (l1 = 0; l1 < 4; l1++) {
    pluginData->buf[l1] = 0;
  }  
}

void connect_port_vcf_bp1(LADSPA_Handle instance, unsigned long port, 
                            LADSPA_Data *data) {
                            
  filtType2 *pluginData = (filtType2 *)instance;
  switch (port) {
  case VCF_INPUT: 
    pluginData->input = data;    
    break;
  case VCF_OUTPUT: 
    pluginData->output = data;
    break;
  case VCF_GAIN: 
    pluginData->gain = data;
    break;
  case VCF_FREQ_OFS: 
    pluginData->freq_ofs = data; 
    break;
  case VCF_FREQ_PITCH: 
    pluginData->freq_pitch = data;
    break;
  case VCF_RESO_OFS: 
    pluginData->reso_ofs = data;
    break;
    
#ifdef WITH_CV_IN     
    
  case VCF_FREQ_IN: 
    pluginData->freq_in = data;
    break;
  case VCF_RESO_IN:
    pluginData->reso_in = data;
    break;
    
#endif    
    
  }
}

void run_vcf_bp1(LADSPA_Handle instance, unsigned long samples) {
  
  int l1;
  double f0, q0, f, q, fa, fb, pi2_rate;
  double *buf;
  double iv_sin, iv_cos, iv_alpha, inv_a0, a0, a1, a2, b0, b1, b2;
  filtType2 *pluginData = (filtType2 *)instance;

  LADSPA_Data *input = pluginData->input;
  LADSPA_Data *output = pluginData->output;
  LADSPA_Data gain = *(pluginData->gain);
  LADSPA_Data freq_ofs = *(pluginData->freq_ofs);
  LADSPA_Data freq_pitch = (*(pluginData->freq_pitch) > 0) ? 1.0 + *(pluginData->freq_pitch) / 2.0 : 1.0 / (1.0 - *(pluginData->freq_pitch) / 2.0);
  LADSPA_Data reso_ofs = *(pluginData->reso_ofs);

#ifdef WITH_CV_IN

  LADSPA_Data *freq_in = pluginData->freq_in;
  LADSPA_Data *reso_in = pluginData->reso_in; 
  
#endif  
  
  pi2_rate = 2.0 * M_PI / pluginData->rate;
  buf = pluginData->buf;
  f0 = freq_ofs;
  q0 = reso_ofs;
  
#ifdef WITH_CV_IN  
  
  if (!(freq_in || reso_in)) {
  
#endif  
  
    f = f0 * freq_pitch;
    if (f > MAX_FREQ) f = MAX_FREQ;
    q = q0;
    iv_sin = sin(pi2_rate * f);
    iv_cos = cos(pi2_rate * f);
    iv_alpha = iv_sin/(Q_SCALE * q);
    b0 = q * iv_alpha;
    b1 = 0;
    b2 = -q * iv_alpha;
    a0 = 1.0 + iv_alpha;
    a1 = -2.0 * iv_cos;
    a2 = 1.0 - iv_alpha;
    inv_a0 = 1.0 / a0;
    for (l1 = 0; l1 < samples; l1++) {
      output[l1] = inv_a0 * (gain * (b0 * input[l1] + b1 * buf[0] + b2 * buf[1])
                             - a1 * buf[2] - a2 * buf[3]);
      buf[1] = buf[0];                          
      buf[0] = input[l1];
      buf[3] = buf[2];
      buf[2] = output[l1];
    }
    
#ifdef WITH_CV_IN    
    
  } else {
    if (!reso_in) {
      q = q0; 
      for (l1 = 0; l1 < samples; l1++) {
        f = (freq_in[l1] > 0) 
        ? (freq_in[l1] * MAX_FREQ + f0 - MIN_FREQ) * freq_pitch 
        : f0 * freq_pitch;
        if (f < MIN_FREQ) f = MIN_FREQ;
        if (f > MAX_FREQ) f = MAX_FREQ;
        iv_sin = sin(pi2_rate * f);
        iv_cos = cos(pi2_rate * f);
        iv_alpha = iv_sin/(Q_SCALE * q);
        b0 = q * iv_alpha;
        b1 = 0;
        b2 = -q * iv_alpha;
        a0 = 1.0 + iv_alpha;
        a1 = -2.0 * iv_cos;
        a2 = 1.0 - iv_alpha;
        output[l1] = 1.0 / a0 * (gain * (b0 * input[l1] + b1 * buf[0] + b2 * buf[1])
                                 - a1 * buf[2] - a2 * buf[3]);
        buf[1] = buf[0];                          
        buf[0] = input[l1];
        buf[3] = buf[2];
        buf[2] = output[l1];
      }
    } else {
      for (l1 = 0; l1 < samples; l1++) {
        f = (freq_in && (freq_in[l1] > 0)) 
        ? (freq_in[l1] * MAX_FREQ + f0 - MIN_FREQ) * freq_pitch 
        : f0 * freq_pitch;
        if (f < MIN_FREQ) f = MIN_FREQ;
        if (f > MAX_FREQ) f = MAX_FREQ;
        q = q0  + reso_in[l1];
        if (q < Q_MIN) q = Q_MIN;
        if (q > Q_MAX) q = Q_MAX;
        iv_sin = sin(pi2_rate * f);
        iv_cos = cos(pi2_rate * f);
        iv_alpha = iv_sin/(Q_SCALE * q);
        b0 = q * iv_alpha;
        b1 = 0;
        b2 = -q * iv_alpha;
        a0 = 1.0 + iv_alpha;
        a1 = -2.0 * iv_cos;
        a2 = 1.0 - iv_alpha;
        inv_a0 = 1.0 / a0;
        output[l1] = inv_a0 * (gain * (b0 * input[l1] + b1 * buf[0] + b2 * buf[1])
                               - a1 * buf[2] - a2 * buf[3]);
        buf[1] = buf[0];                          
        buf[0] = input[l1];
        buf[3] = buf[2];
        buf[2] = output[l1];
      }
    }
  }

#endif
  
}

void cleanup_vcf_bp1(LADSPA_Handle instance) {

  free(instance);
}
 
/**************** Bandpass II, Formula by Robert Bristow-Johnson *******************/

LADSPA_Handle instantiate_vcf_bp2(const LADSPA_Descriptor *descriptor, 
                                    unsigned long rate) {
 
  filtType2 *pluginData = (filtType2 *)malloc(sizeof(filtType2));
  pluginData->rate = rate;
  return (LADSPA_Handle)pluginData;
}

void activate_vcf_bp2(LADSPA_Handle instance) {

  int l1;
 
  filtType2 *pluginData = (filtType2 *)instance;
  for (l1 = 0; l1 < 4; l1++) {
    pluginData->buf[l1] = 0;
  }  
}

void connect_port_vcf_bp2(LADSPA_Handle instance, unsigned long port, 
                            LADSPA_Data *data) {
                            
  filtType2 *pluginData = (filtType2 *)instance;
  switch (port) {
  case VCF_INPUT: 
    pluginData->input = data;    
    break;
  case VCF_OUTPUT: 
    pluginData->output = data;
    break;
  case VCF_GAIN: 
    pluginData->gain = data;
    break;
  case VCF_FREQ_OFS: 
    pluginData->freq_ofs = data; 
    break;
  case VCF_FREQ_PITCH: 
    pluginData->freq_pitch = data;
    break;
  case VCF_RESO_OFS: 
    pluginData->reso_ofs = data;
    break;

#ifdef WITH_CV_IN

  case VCF_FREQ_IN: 
    pluginData->freq_in = data;
    break;
  case VCF_RESO_IN:
    pluginData->reso_in = data;
    break;
    
#endif    
    
  }
}

void run_vcf_bp2(LADSPA_Handle instance, unsigned long samples) {
  
  int l1;
  double f0, q0, f, q, fa, fb, pi2_rate;
  double *buf;
  double iv_sin, iv_cos, iv_alpha, inv_a0, a0, a1, a2, b0, b1, b2;
  filtType2 *pluginData = (filtType2 *)instance;

  LADSPA_Data *input = pluginData->input;
  LADSPA_Data *output = pluginData->output;
  LADSPA_Data gain = *(pluginData->gain);
  LADSPA_Data freq_ofs = *(pluginData->freq_ofs);
  LADSPA_Data freq_pitch = (*(pluginData->freq_pitch) > 0) ? 1.0 + *(pluginData->freq_pitch) / 2.0 : 1.0 / (1.0 - *(pluginData->freq_pitch) / 2.0);
  LADSPA_Data reso_ofs = *(pluginData->reso_ofs);

#ifdef WITH_CV_IN

  LADSPA_Data *freq_in = pluginData->freq_in;
  LADSPA_Data *reso_in = pluginData->reso_in; 
  
#endif 
  
  pi2_rate = 2.0 * M_PI / pluginData->rate;
  buf = pluginData->buf;
  f0 = freq_ofs;
  q0 = reso_ofs;
  
#ifdef WITH_CV_IN  
  
  if (!(freq_in || reso_in)) {
  
#endif  
  
    f = f0 * freq_pitch;
    if (f > MAX_FREQ) f = MAX_FREQ;
    q = q0;
    iv_sin = sin(pi2_rate * f);
    iv_cos = cos(pi2_rate * f);
    iv_alpha = iv_sin/(Q_SCALE * q);
    b0 = iv_alpha;
    b1 = 0;
    b2 = -iv_alpha;
    a0 = 1.0 + iv_alpha;
    a1 = -2.0 * iv_cos;
    a2 = 1.0 - iv_alpha;
    inv_a0 = 1.0 / a0;
    for (l1 = 0; l1 < samples; l1++) {
      output[l1] = inv_a0 * (gain * (b0 * input[l1] + b1 * buf[0] + b2 * buf[1])
                             - a1 * buf[2] - a2 * buf[3]);
      buf[1] = buf[0];                          
      buf[0] = input[l1];
      buf[3] = buf[2];
      buf[2] = output[l1];
    }

#ifdef WITH_CV_IN
    
  } else {
    if (!reso_in) {
      q = q0; 
      for (l1 = 0; l1 < samples; l1++) {
        f = (freq_in[l1] > 0) 
        ? (freq_in[l1] * MAX_FREQ + f0 - MIN_FREQ) * freq_pitch 
        : f0 * freq_pitch;
        if (f < MIN_FREQ) f = MIN_FREQ;
        if (f > MAX_FREQ) f = MAX_FREQ;
        iv_sin = sin(pi2_rate * f);
        iv_cos = cos(pi2_rate * f);
        iv_alpha = iv_sin/(Q_SCALE * q);
        b0 = iv_alpha;
        b1 = 0;
        b2 = -iv_alpha;
        a0 = 1.0 + iv_alpha;
        a1 = -2.0 * iv_cos;
        a2 = 1.0 - iv_alpha;
        output[l1] = 1.0 / a0 * (gain * (b0 * input[l1] + b1 * buf[0] + b2 * buf[1])
                                 - a1 * buf[2] - a2 * buf[3]);
        buf[1] = buf[0];                          
        buf[0] = input[l1];
        buf[3] = buf[2];
        buf[2] = output[l1];
      }
    } else {
      for (l1 = 0; l1 < samples; l1++) {
        f = (freq_in && (freq_in[l1] > 0)) 
        ? (freq_in[l1] * MAX_FREQ + f0 - MIN_FREQ) * freq_pitch 
        : f0 * freq_pitch;
        if (f < MIN_FREQ) f = MIN_FREQ;
        if (f > MAX_FREQ) f = MAX_FREQ;
        q = q0  + reso_in[l1];
        if (q < Q_MIN) q = Q_MIN;
        if (q > Q_MAX) q = Q_MAX;
        iv_sin = sin(pi2_rate * f);
        iv_cos = cos(pi2_rate * f);
        iv_alpha = iv_sin/(Q_SCALE * q);
        b0 = iv_alpha;
        b1 = 0;
        b2 = -iv_alpha;
        a0 = 1.0 + iv_alpha;
        a1 = -2.0 * iv_cos;
        a2 = 1.0 - iv_alpha;
        inv_a0 = 1.0 / a0;
        output[l1] = inv_a0 * (gain * (b0 * input[l1] + b1 * buf[0] + b2 * buf[1])
                               - a1 * buf[2] - a2 * buf[3]);
        buf[1] = buf[0];                          
        buf[0] = input[l1];
        buf[3] = buf[2];
        buf[2] = output[l1];
      }
    }
  }

#endif
  
}

void cleanup_vcf_bp2(LADSPA_Handle instance) {

  free(instance);
}
 
/******************** Notch, Formula by Robert Bristow-Johnson *********************/

LADSPA_Handle instantiate_vcf_notch(const LADSPA_Descriptor *descriptor, 
                                    unsigned long rate) {
 
  filtType2 *pluginData = (filtType2 *)malloc(sizeof(filtType2));
  pluginData->rate = rate;
  return (LADSPA_Handle)pluginData;
}

void activate_vcf_notch(LADSPA_Handle instance) {

  int l1;
 
  filtType2 *pluginData = (filtType2 *)instance;
  for (l1 = 0; l1 < 4; l1++) {
    pluginData->buf[l1] = 0;
  }  
}

void connect_port_vcf_notch(LADSPA_Handle instance, unsigned long port, 
                            LADSPA_Data *data) {
                            
  filtType2 *pluginData = (filtType2 *)instance;
  switch (port) {
  case VCF_INPUT: 
    pluginData->input = data;    
    break;
  case VCF_OUTPUT: 
    pluginData->output = data;
    break;
  case VCF_GAIN: 
    pluginData->gain = data;
    break;
  case VCF_FREQ_OFS: 
    pluginData->freq_ofs = data; 
    break;
  case VCF_FREQ_PITCH: 
    pluginData->freq_pitch = data;
    break;
  case VCF_RESO_OFS: 
    pluginData->reso_ofs = data;
    break;
    
#ifdef WITH_CV_IN    
    
  case VCF_FREQ_IN: 
    pluginData->freq_in = data;
    break;
  case VCF_RESO_IN:
    pluginData->reso_in = data;
    break;
    
#endif    
    
  }
}

void run_vcf_notch(LADSPA_Handle instance, unsigned long samples) {
  
  int l1;
  double f0, q0, f, q, fa, fb, pi2_rate;
  double *buf;
  double iv_sin, iv_cos, iv_alpha, inv_a0, a0, a1, a2, b0, b1, b2;
  filtType2 *pluginData = (filtType2 *)instance;

  LADSPA_Data *input = pluginData->input;
  LADSPA_Data *output = pluginData->output;
  LADSPA_Data gain = *(pluginData->gain);
  LADSPA_Data freq_ofs = *(pluginData->freq_ofs);
  LADSPA_Data freq_pitch = (*(pluginData->freq_pitch) > 0) ? 1.0 + *(pluginData->freq_pitch) / 2.0 : 1.0 / (1.0 - *(pluginData->freq_pitch) / 2.0);
  LADSPA_Data reso_ofs = *(pluginData->reso_ofs);

#ifdef WITH_CV_IN

  LADSPA_Data *freq_in = pluginData->freq_in;
  LADSPA_Data *reso_in = pluginData->reso_in; 
  
#endif  
  
  pi2_rate = 2.0 * M_PI / pluginData->rate;
  buf = pluginData->buf;
  f0 = freq_ofs;
  q0 = reso_ofs;
  
#ifdef WITH_CV_IN  
  
  if (!(freq_in || reso_in)) {
  
#endif  
  
    f = f0 * freq_pitch;
    if (f > MAX_FREQ) f = MAX_FREQ;
    q = q0;
    iv_sin = sin(pi2_rate * f);
    iv_cos = cos(pi2_rate * f);
    iv_alpha = iv_sin/(Q_SCALE * q);
    b0 = 1;
    b1 = -2.0 * iv_cos;
    b2 = 1;
    a0 = 1.0 + iv_alpha;
    a1 = -2.0 * iv_cos;
    a2 = 1.0 - iv_alpha;
    inv_a0 = 1.0 / a0;
    for (l1 = 0; l1 < samples; l1++) {
      output[l1] = inv_a0 * (gain * (b0 * input[l1] + b1 * buf[0] + b2 * buf[1])
                             - a1 * buf[2] - a2 * buf[3]);
      buf[1] = buf[0];                          
      buf[0] = input[l1];
      buf[3] = buf[2];
      buf[2] = output[l1];
    }
    
#ifdef WITH_CV_IN    
    
  } else {
    if (!reso_in) {
      q = q0; 
      for (l1 = 0; l1 < samples; l1++) {
        f = (freq_in[l1] > 0) 
        ? (freq_in[l1] * MAX_FREQ + f0 - MIN_FREQ) * freq_pitch 
        : f0 * freq_pitch;
        if (f < MIN_FREQ) f = MIN_FREQ;
        if (f > MAX_FREQ) f = MAX_FREQ;
        iv_sin = sin(pi2_rate * f);
        iv_cos = cos(pi2_rate * f);
        iv_alpha = iv_sin/(Q_SCALE * q);
        b0 = 1;
        b1 = -2.0 * iv_cos;
        b2 = 1;
        a0 = 1.0 + iv_alpha;
        a1 = -2.0 * iv_cos;
        a2 = 1.0 - iv_alpha;
        output[l1] = 1.0 / a0 * (gain * (b0 * input[l1] + b1 * buf[0] + b2 * buf[1])
                                 - a1 * buf[2] - a2 * buf[3]);
        buf[1] = buf[0];                          
        buf[0] = input[l1];
        buf[3] = buf[2];
        buf[2] = output[l1];
      }
    } else {
      for (l1 = 0; l1 < samples; l1++) {
        f = (freq_in && (freq_in[l1] > 0)) 
        ? (freq_in[l1] * MAX_FREQ + f0 - MIN_FREQ) * freq_pitch 
        : f0 * freq_pitch;
        if (f < MIN_FREQ) f = MIN_FREQ;
        if (f > MAX_FREQ) f = MAX_FREQ;
        q = q0  + reso_in[l1];
        if (q < Q_MIN) q = Q_MIN;
        if (q > Q_MAX) q = Q_MAX;
        iv_sin = sin(pi2_rate * f);
        iv_cos = cos(pi2_rate * f);
        iv_alpha = iv_sin/(Q_SCALE * q);
        b0 = 1;
        b1 = -2.0 * iv_cos;
        b2 = 1;
        a0 = 1.0 + iv_alpha;
        a1 = -2.0 * iv_cos;
        a2 = 1.0 - iv_alpha;
        inv_a0 = 1.0 / a0;
        output[l1] = inv_a0 * (gain * (b0 * input[l1] + b1 * buf[0] + b2 * buf[1])
                               - a1 * buf[2] - a2 * buf[3]);
        buf[1] = buf[0];                          
        buf[0] = input[l1];
        buf[3] = buf[2];
        buf[2] = output[l1];
      }
    }
  }

#endif
  
}

void cleanup_vcf_notch(LADSPA_Handle instance) {

  free(instance);
}

/***************** Peaking EQ, Formula by Robert Bristow-Johnson *******************/

LADSPA_Handle instantiate_vcf_peakeq(const LADSPA_Descriptor *descriptor, 
                                    unsigned long rate) {
 
  filtType3 *pluginData = (filtType3 *)malloc(sizeof(filtType3));
  pluginData->rate = rate;
  return (LADSPA_Handle)pluginData;
}

void activate_vcf_peakeq(LADSPA_Handle instance) {

  int l1;
 
  filtType3 *pluginData = (filtType3 *)instance;
  for (l1 = 0; l1 < 4; l1++) {
    pluginData->buf[l1] = 0;
  }  
}

void connect_port_vcf_peakeq(LADSPA_Handle instance, unsigned long port, 
                            LADSPA_Data *data) {
                            
  filtType3 *pluginData = (filtType3 *)instance;
  switch (port) {
  case VCF_INPUT: 
    pluginData->input = data;    
    break;
  case VCF_OUTPUT: 
    pluginData->output = data;
    break;
  case VCF_GAIN: 
    pluginData->gain = data;
    break;
  case VCF_FREQ_OFS: 
    pluginData->freq_ofs = data; 
    break;
  case VCF_FREQ_PITCH: 
    pluginData->freq_pitch = data;
    break;
  case VCF_RESO_OFS: 
    pluginData->reso_ofs = data;
    break;
  case VCF_DBGAIN_OFS: 
    pluginData->dBgain_ofs = data;
    break;
    
#ifdef WITH_CV_IN    
    
  case VCF_FREQ_IN: 
    pluginData->freq_in = data;
    break;
  case VCF_RESO_IN:
    pluginData->reso_in = data;
    break;
  case VCF_DBGAIN_IN:
    pluginData->dBgain_in = data;
    break;

#endif

  }
}

void run_vcf_peakeq(LADSPA_Handle instance, unsigned long samples) {
  
  int l1;
  double f0, q0, f, q, fa, fb, pi2_rate, A, dBgain;
  double *buf;
  double iv_sin, iv_cos, iv_alpha, inv_a0, a0, a1, a2, b0, b1, b2;
  filtType3 *pluginData = (filtType3 *)instance;

  LADSPA_Data *input = pluginData->input;
  LADSPA_Data *output = pluginData->output;
  LADSPA_Data gain = *(pluginData->gain);
  LADSPA_Data freq_ofs = *(pluginData->freq_ofs);
  LADSPA_Data freq_pitch = (*(pluginData->freq_pitch) > 0) ? 1.0 + *(pluginData->freq_pitch) / 2.0 : 1.0 / (1.0 - *(pluginData->freq_pitch) / 2.0);
  LADSPA_Data reso_ofs = *(pluginData->reso_ofs);
  LADSPA_Data dBgain_ofs = *(pluginData->dBgain_ofs);
  
#ifdef WITH_CV_IN  
  
  LADSPA_Data *freq_in = pluginData->freq_in;
  LADSPA_Data *reso_in = pluginData->reso_in; 
  LADSPA_Data *dBgain_in = pluginData->dBgain_in; 
  
#endif  
  
  pi2_rate = 2.0 * M_PI / pluginData->rate;
  buf = pluginData->buf;
  f0 = freq_ofs;
  q0 = reso_ofs;
  
#ifdef WITH_CV_IN  
  
  if (!(freq_in || reso_in || dBgain_in)) {
  
#endif  
  
    f = f0 * freq_pitch;
    if (f > MAX_FREQ) f = MAX_FREQ;
    q = q0;
    dBgain = dBgain_ofs;
    iv_sin = sin(pi2_rate * f);
    iv_cos = cos(pi2_rate * f);
    iv_alpha = iv_sin/(Q_SCALE * q);
    A = exp(dBgain/40.0 * log(10.0));
    b0 = 1.0 + iv_alpha * A;
    b1 = -2.0 * iv_cos;
    b2 = 1.0 - iv_alpha * A;
    a0 = 1.0 + iv_alpha / A;
    a1 = -2.0 * iv_cos;
    a2 = 1.0 - iv_alpha / A;
    inv_a0 = 1.0 / a0;
    for (l1 = 0; l1 < samples; l1++) {
      output[l1] = inv_a0 * (gain * (b0 * input[l1] + b1 * buf[0] + b2 * buf[1])
                             - a1 * buf[2] - a2 * buf[3]);
      buf[1] = buf[0];                          
      buf[0] = input[l1];
      buf[3] = buf[2];
      buf[2] = output[l1];
    }
    
#ifdef WITH_CV_IN    
    
  } else {
    if (!(reso_in || dBgain_in)) {
      q = q0; 
      dBgain = dBgain_ofs;
      for (l1 = 0; l1 < samples; l1++) {
        f = (freq_in[l1] > 0) 
        ? (freq_in[l1] * MAX_FREQ + f0 - MIN_FREQ) * freq_pitch 
        : f0 * freq_pitch;
        if (f < MIN_FREQ) f = MIN_FREQ;
        if (f > MAX_FREQ) f = MAX_FREQ;
        iv_sin = sin(pi2_rate * f);
        iv_cos = cos(pi2_rate * f);
        iv_alpha = iv_sin/(Q_SCALE * q);
        A = exp(dBgain/40.0 * log(10.0));
        b0 = 1.0 + iv_alpha * A;
        b1 = -2.0 * iv_cos;
        b2 = 1.0 - iv_alpha * A;
        a0 = 1.0 + iv_alpha / A;
        a1 = -2.0 * iv_cos;
        a2 = 1.0 - iv_alpha / A;
        output[l1] = 1.0 / a0 * (gain * (b0 * input[l1] + b1 * buf[0] + b2 * buf[1])
                                 - a1 * buf[2] - a2 * buf[3]);
        buf[1] = buf[0];                          
        buf[0] = input[l1];
        buf[3] = buf[2];
        buf[2] = output[l1];
      }
    } else {
      for (l1 = 0; l1 < samples; l1++) {
        f = (freq_in && (freq_in[l1] > 0)) 
        ? (freq_in[l1] * MAX_FREQ + f0 - MIN_FREQ) * freq_pitch 
        : f0 * freq_pitch;
        if (f < MIN_FREQ) f = MIN_FREQ;
        if (f > MAX_FREQ) f = MAX_FREQ;
        q = q0  + reso_in[l1];
        if (q < Q_MIN) q = Q_MIN;
        if (q > Q_MAX) q = Q_MAX;
        dBgain = (dBgain_in) ? dBgain_ofs + DBGAIN_SCALE * dBgain_in[l1] : dBgain_ofs;
        iv_sin = sin(pi2_rate * f);
        iv_cos = cos(pi2_rate * f);
        iv_alpha = iv_sin/(Q_SCALE * q);
        A = exp(dBgain/40.0 * log(10.0));
        b0 = 1.0 + iv_alpha * A;
        b1 = -2.0 * iv_cos;
        b2 = 1.0 - iv_alpha * A;
        a0 = 1.0 + iv_alpha / A;
        a1 = -2.0 * iv_cos;
        a2 = 1.0 - iv_alpha / A;
        inv_a0 = 1.0 / a0;
        output[l1] = inv_a0 * (gain * (b0 * input[l1] + b1 * buf[0] + b2 * buf[1])
                               - a1 * buf[2] - a2 * buf[3]);
        buf[1] = buf[0];                          
        buf[0] = input[l1];
        buf[3] = buf[2];
        buf[2] = output[l1];
      }
    }
  }
  
#endif  
  
}

void cleanup_vcf_peakeq(LADSPA_Handle instance) {

  free(instance);
}

/****************** Low Shelf, Formula by Robert Bristow-Johnson *******************/

LADSPA_Handle instantiate_vcf_lshelf(const LADSPA_Descriptor *descriptor, 
                                    unsigned long rate) {
 
  filtType3 *pluginData = (filtType3 *)malloc(sizeof(filtType3));
  pluginData->rate = rate;
  return (LADSPA_Handle)pluginData;
}

void activate_vcf_lshelf(LADSPA_Handle instance) {

  int l1;
 
  filtType3 *pluginData = (filtType3 *)instance;
  for (l1 = 0; l1 < 4; l1++) {
    pluginData->buf[l1] = 0;
  }  
}

void connect_port_vcf_lshelf(LADSPA_Handle instance, unsigned long port, 
                            LADSPA_Data *data) {
                            
  filtType3 *pluginData = (filtType3 *)instance;
  switch (port) {
  case VCF_INPUT: 
    pluginData->input = data;    
    break;
  case VCF_OUTPUT: 
    pluginData->output = data;
    break;
  case VCF_GAIN: 
    pluginData->gain = data;
    break;
  case VCF_FREQ_OFS: 
    pluginData->freq_ofs = data; 
    break;
  case VCF_FREQ_PITCH: 
    pluginData->freq_pitch = data;
    break;
  case VCF_RESO_OFS: 
    pluginData->reso_ofs = data;
    break;
  case VCF_DBGAIN_OFS: 
    pluginData->dBgain_ofs = data;
    break;
    
#ifdef WITH_CV_IN    
    
  case VCF_FREQ_IN: 
    pluginData->freq_in = data;
    break;
  case VCF_RESO_IN:
    pluginData->reso_in = data;
    break;
  case VCF_DBGAIN_IN:
    pluginData->dBgain_in = data;
    break;
    
#endif
    
  }
}

void run_vcf_lshelf(LADSPA_Handle instance, unsigned long samples) {
  
  int l1;
  double f0, q0, f, q, fa, fb, pi2_rate, A, dBgain, iv_beta;
  double *buf;
  double iv_sin, iv_cos, iv_alpha, inv_a0, a0, a1, a2, b0, b1, b2;
  filtType3 *pluginData = (filtType3 *)instance;

  LADSPA_Data *input = pluginData->input;
  LADSPA_Data *output = pluginData->output;
  LADSPA_Data gain = *(pluginData->gain);
  LADSPA_Data freq_ofs = *(pluginData->freq_ofs);
  LADSPA_Data freq_pitch = (*(pluginData->freq_pitch) > 0) ? 1.0 + *(pluginData->freq_pitch) / 2.0 : 1.0 / (1.0 - *(pluginData->freq_pitch) / 2.0);
  LADSPA_Data reso_ofs = *(pluginData->reso_ofs);
  LADSPA_Data dBgain_ofs = *(pluginData->dBgain_ofs);

#ifdef WITH_CV_IN

  LADSPA_Data *freq_in = pluginData->freq_in;
  LADSPA_Data *reso_in = pluginData->reso_in; 
  LADSPA_Data *dBgain_in = pluginData->dBgain_in; 
  
#endif  
  
  pi2_rate = 2.0 * M_PI / pluginData->rate;
  buf = pluginData->buf;
  f0 = freq_ofs;
  q0 = reso_ofs;
  
#ifdef WITH_CV_IN  
  
  if (!(freq_in || reso_in || dBgain_in)) {
  
#endif  
  
    f = f0 * freq_pitch;
    if (f > MAX_FREQ) f = MAX_FREQ;
    q = q0;
    dBgain = dBgain_ofs;
    iv_sin = sin(pi2_rate * f);
    iv_cos = cos(pi2_rate * f);
    iv_alpha = iv_sin/(Q_SCALE * q);
    A = exp(dBgain/40.0 * log(10.0));
    iv_beta = sqrt(A) / q;
    b0 = A * (A + 1.0 - (A - 1.0) * iv_cos + iv_beta * iv_sin);
    b1 = 2.0 * A * (A - 1.0 - (A + 1.0) * iv_cos);
    b2 = A * (A + 1.0 - (A - 1.0) * iv_cos - iv_beta * iv_sin);
    a0 = A + 1.0 + (A - 1.0) * iv_cos + iv_beta * iv_sin;
    a1 = -2.0 * (A - 1.0 + (A + 1.0) * iv_cos);
    a2 = A + 1.0 + (A - 1.0) * iv_cos - iv_beta * iv_sin;
    inv_a0 = 1.0 / a0;
    for (l1 = 0; l1 < samples; l1++) {
      output[l1] = inv_a0 * (gain * (b0 * input[l1] + b1 * buf[0] + b2 * buf[1])
                             - a1 * buf[2] - a2 * buf[3]);
      buf[1] = buf[0];                          
      buf[0] = input[l1];
      buf[3] = buf[2];
      buf[2] = output[l1];
    }
    
#ifdef WITH_CV_IN    
    
  } else {
    if (!(reso_in || dBgain_in)) {
      q = q0; 
      dBgain = dBgain_ofs;
      for (l1 = 0; l1 < samples; l1++) {
        f = (freq_in[l1] > 0) 
        ? (freq_in[l1] * MAX_FREQ + f0 - MIN_FREQ) * freq_pitch 
        : f0 * freq_pitch;
        if (f < MIN_FREQ) f = MIN_FREQ;
        if (f > MAX_FREQ) f = MAX_FREQ;
        iv_sin = sin(pi2_rate * f);
        iv_cos = cos(pi2_rate * f);
        iv_alpha = iv_sin/(Q_SCALE * q);
        A = exp(dBgain/40.0 * log(10.0));
        iv_beta = sqrt(A) / q;
        b0 = A * (A + 1.0 - (A - 1.0) * iv_cos + iv_beta * iv_sin);
        b1 = 2.0 * A * (A - 1.0 - (A + 1.0) * iv_cos);
        b2 = A * (A + 1.0 - (A - 1.0) * iv_cos - iv_beta * iv_sin);
        a0 = A + 1.0 + (A - 1.0) * iv_cos + iv_beta * iv_sin;
        a1 = -2.0 * (A - 1.0 + (A + 1.0) * iv_cos);
        a2 = A + 1.0 + (A - 1.0) * iv_cos - iv_beta * iv_sin;
        output[l1] = 1.0 / a0 * (gain * (b0 * input[l1] + b1 * buf[0] + b2 * buf[1])
                                 - a1 * buf[2] - a2 * buf[3]);
        buf[1] = buf[0];                          
        buf[0] = input[l1];
        buf[3] = buf[2];
        buf[2] = output[l1];
      }
    } else {
      for (l1 = 0; l1 < samples; l1++) {
        f = (freq_in && (freq_in[l1] > 0)) 
        ? (freq_in[l1] * MAX_FREQ + f0 - MIN_FREQ) * freq_pitch 
        : f0 * freq_pitch;
        if (f < MIN_FREQ) f = MIN_FREQ;
        if (f > MAX_FREQ) f = MAX_FREQ;
        q = q0  + reso_in[l1];
        if (q < Q_MIN) q = Q_MIN;
        if (q > Q_MAX) q = Q_MAX;
        dBgain = (dBgain_in) ? dBgain_ofs + DBGAIN_SCALE * dBgain_in[l1] : dBgain_ofs;
        iv_sin = sin(pi2_rate * f);
        iv_cos = cos(pi2_rate * f);
        iv_alpha = iv_sin/(Q_SCALE * q);
        A = exp(dBgain/40.0 * log(10.0));
        iv_beta = sqrt(A) / q;
        b0 = A * (A + 1.0 - (A - 1.0) * iv_cos + iv_beta * iv_sin);
        b1 = 2.0 * A * (A - 1.0 - (A + 1.0) * iv_cos);
        b2 = A * (A + 1.0 - (A - 1.0) * iv_cos - iv_beta * iv_sin);
        a0 = A + 1.0 + (A - 1.0) * iv_cos + iv_beta * iv_sin;
        a1 = -2.0 * (A - 1.0 + (A + 1.0) * iv_cos);
        a2 = A + 1.0 + (A - 1.0) * iv_cos - iv_beta * iv_sin;
        inv_a0 = 1.0 / a0;
        output[l1] = inv_a0 * (gain * (b0 * input[l1] + b1 * buf[0] + b2 * buf[1])
                               - a1 * buf[2] - a2 * buf[3]);
        buf[1] = buf[0];                          
        buf[0] = input[l1];
        buf[3] = buf[2];
        buf[2] = output[l1];
      }
    }
  }
  
#endif  
  
}

void cleanup_vcf_lshelf(LADSPA_Handle instance) {

  free(instance);
}

/***************** High Shelf, Formula by Robert Bristow-Johnson *******************/

LADSPA_Handle instantiate_vcf_hshelf(const LADSPA_Descriptor *descriptor, 
                                    unsigned long rate) {
 
  filtType3 *pluginData = (filtType3 *)malloc(sizeof(filtType3));
  pluginData->rate = rate;
  return (LADSPA_Handle)pluginData;
}

void activate_vcf_hshelf(LADSPA_Handle instance) {

  int l1;
 
  filtType3 *pluginData = (filtType3 *)instance;
  for (l1 = 0; l1 < 4; l1++) {
    pluginData->buf[l1] = 0;
  }  
}

void connect_port_vcf_hshelf(LADSPA_Handle instance, unsigned long port, 
                            LADSPA_Data *data) {
                            
  filtType3 *pluginData = (filtType3 *)instance;
  switch (port) {
  case VCF_INPUT: 
    pluginData->input = data;    
    break;
  case VCF_OUTPUT: 
    pluginData->output = data;
    break;
  case VCF_GAIN: 
    pluginData->gain = data;
    break;
  case VCF_FREQ_OFS: 
    pluginData->freq_ofs = data; 
    break;
  case VCF_FREQ_PITCH: 
    pluginData->freq_pitch = data;
    break;
  case VCF_RESO_OFS: 
    pluginData->reso_ofs = data;
    break;
  case VCF_DBGAIN_OFS: 
    pluginData->dBgain_ofs = data;
    break;
    
#ifdef WITH_CV_IN    
    
  case VCF_FREQ_IN: 
    pluginData->freq_in = data;
    break;
  case VCF_RESO_IN:
    pluginData->reso_in = data;
    break;
  case VCF_DBGAIN_IN:
    pluginData->dBgain_in = data;
    break;
    
#endif    
    
  }
}

void run_vcf_hshelf(LADSPA_Handle instance, unsigned long samples) {
  
  int l1;
  double f0, q0, f, q, fa, fb, pi2_rate, A, dBgain, iv_beta;
  double *buf;
  double iv_sin, iv_cos, iv_alpha, inv_a0, a0, a1, a2, b0, b1, b2;
  filtType3 *pluginData = (filtType3 *)instance;

  LADSPA_Data *input = pluginData->input;
  LADSPA_Data *output = pluginData->output;
  LADSPA_Data gain = *(pluginData->gain);
  LADSPA_Data freq_ofs = *(pluginData->freq_ofs);
  LADSPA_Data freq_pitch = (*(pluginData->freq_pitch) > 0) ? 1.0 + *(pluginData->freq_pitch) / 2.0 : 1.0 / (1.0 - *(pluginData->freq_pitch) / 2.0);
  LADSPA_Data reso_ofs = *(pluginData->reso_ofs);
  LADSPA_Data dBgain_ofs = *(pluginData->dBgain_ofs);
  
#ifdef WITH_CV_IN  
  
  LADSPA_Data *freq_in = pluginData->freq_in;
  LADSPA_Data *reso_in = pluginData->reso_in; 
  LADSPA_Data *dBgain_in = pluginData->dBgain_in; 
  
#endif  
  
  pi2_rate = 2.0 * M_PI / pluginData->rate;
  buf = pluginData->buf;
  f0 = freq_ofs;
  q0 = reso_ofs;
  
#ifdef WITH_CV_IN  
  
  if (!(freq_in || reso_in || dBgain_in)) {
  
#endif  
  
    f = f0 * freq_pitch;
    if (f > MAX_FREQ) f = MAX_FREQ;
    q = q0;
    dBgain = dBgain_ofs;
    iv_sin = sin(pi2_rate * f);
    iv_cos = cos(pi2_rate * f);
    iv_alpha = iv_sin/(Q_SCALE * q);
    A = exp(dBgain/40.0 * log(10.0));
    iv_beta = sqrt(A) / q;
    b0 = A * (A + 1.0 + (A - 1.0) * iv_cos + iv_beta * iv_sin);
    b1 = -2.0 * A * (A - 1.0 + (A + 1.0) * iv_cos);
    b2 = A * (A + 1.0 + (A - 1.0) * iv_cos - iv_beta * iv_sin);
    a0 = A + 1.0 - (A - 1.0) * iv_cos + iv_beta * iv_sin;
    a1 = 2.0 * (A - 1.0 - (A + 1.0) * iv_cos);
    a2 = A + 1.0 - (A - 1.0) * iv_cos - iv_beta * iv_sin;
    inv_a0 = 1.0 / a0;
    for (l1 = 0; l1 < samples; l1++) {
      output[l1] = inv_a0 * (gain * (b0 * input[l1] + b1 * buf[0] + b2 * buf[1])
                             - a1 * buf[2] - a2 * buf[3]);
      buf[1] = buf[0];                          
      buf[0] = input[l1];
      buf[3] = buf[2];
      buf[2] = output[l1];
    }
    
#ifdef WITH_CV_IN    
    
  } else {
    if (!(reso_in || dBgain_in)) {
      q = q0; 
      dBgain = dBgain_ofs;
      for (l1 = 0; l1 < samples; l1++) {
        f = (freq_in[l1] > 0) 
        ? (freq_in[l1] * MAX_FREQ + f0 - MIN_FREQ) * freq_pitch 
        : f0 * freq_pitch;
        if (f < MIN_FREQ) f = MIN_FREQ;
        if (f > MAX_FREQ) f = MAX_FREQ;
        iv_sin = sin(pi2_rate * f);
        iv_cos = cos(pi2_rate * f);
        iv_alpha = iv_sin/(Q_SCALE * q);
        A = exp(dBgain/40.0 * log(10.0));
        iv_beta = sqrt(A) / q;
        b0 = A * (A + 1.0 + (A - 1.0) * iv_cos + iv_beta * iv_sin);
        b1 = -2.0 * A * (A - 1.0 + (A + 1.0) * iv_cos);
        b2 = A * (A + 1.0 + (A - 1.0) * iv_cos - iv_beta * iv_sin);
        a0 = A + 1.0 - (A - 1.0) * iv_cos + iv_beta * iv_sin;
        a1 = 2.0 * (A - 1.0 - (A + 1.0) * iv_cos);
        a2 = A + 1.0 - (A - 1.0) * iv_cos - iv_beta * iv_sin;
        output[l1] = 1.0 / a0 * (gain * (b0 * input[l1] + b1 * buf[0] + b2 * buf[1])
                                 - a1 * buf[2] - a2 * buf[3]);
        buf[1] = buf[0];                          
        buf[0] = input[l1];
        buf[3] = buf[2];
        buf[2] = output[l1];
      }
    } else {
      for (l1 = 0; l1 < samples; l1++) {
        f = (freq_in && (freq_in[l1] > 0)) 
        ? (freq_in[l1] * MAX_FREQ + f0 - MIN_FREQ) * freq_pitch 
        : f0 * freq_pitch;
        if (f < MIN_FREQ) f = MIN_FREQ;
        if (f > MAX_FREQ) f = MAX_FREQ;
        q = q0  + reso_in[l1];
        if (q < Q_MIN) q = Q_MIN;
        if (q > Q_MAX) q = Q_MAX;
        dBgain = (dBgain_in) ? dBgain_ofs + DBGAIN_SCALE * dBgain_in[l1] : dBgain_ofs;
        iv_sin = sin(pi2_rate * f);
        iv_cos = cos(pi2_rate * f);
        iv_alpha = iv_sin/(Q_SCALE * q);
        A = exp(dBgain/40.0 * log(10.0));
        iv_beta = sqrt(A) / q;
        b0 = A * (A + 1.0 + (A - 1.0) * iv_cos + iv_beta * iv_sin);
        b1 = -2.0 * A * (A - 1.0 + (A + 1.0) * iv_cos);
        b2 = A * (A + 1.0 + (A - 1.0) * iv_cos - iv_beta * iv_sin);
        a0 = A + 1.0 - (A - 1.0) * iv_cos + iv_beta * iv_sin;
        a1 = 2.0 * (A - 1.0 - (A + 1.0) * iv_cos);
        a2 = A + 1.0 - (A - 1.0) * iv_cos - iv_beta * iv_sin;
        inv_a0 = 1.0 / a0;
        output[l1] = inv_a0 * (gain * (b0 * input[l1] + b1 * buf[0] + b2 * buf[1])
                               - a1 * buf[2] - a2 * buf[3]);
        buf[1] = buf[0];                          
        buf[0] = input[l1];
        buf[3] = buf[2];
        buf[2] = output[l1];
      }
    }
  }
  
#endif  
  
}

void cleanup_vcf_hshelf(LADSPA_Handle instance) {

  free(instance);
}


void _init()
{
  LADSPA_PortDescriptor *portDescriptors;
  LADSPA_PortRangeHint *portRangeHints;
  char **portNames;
  
  vcf_reslp_Descriptor = (LADSPA_Descriptor *)malloc(sizeof(LADSPA_Descriptor));
  vcf_lp_Descriptor = (LADSPA_Descriptor *)malloc(sizeof(LADSPA_Descriptor));
  vcf_hp_Descriptor = (LADSPA_Descriptor *)malloc(sizeof(LADSPA_Descriptor));
  vcf_bp1_Descriptor = (LADSPA_Descriptor *)malloc(sizeof(LADSPA_Descriptor));
  vcf_bp2_Descriptor = (LADSPA_Descriptor *)malloc(sizeof(LADSPA_Descriptor));
  vcf_notch_Descriptor = (LADSPA_Descriptor *)malloc(sizeof(LADSPA_Descriptor));
  vcf_peakeq_Descriptor = (LADSPA_Descriptor *)malloc(sizeof(LADSPA_Descriptor));
  vcf_lshelf_Descriptor = (LADSPA_Descriptor *)malloc(sizeof(LADSPA_Descriptor));
  vcf_hshelf_Descriptor = (LADSPA_Descriptor *)malloc(sizeof(LADSPA_Descriptor));
  if (vcf_reslp_Descriptor) {
    vcf_reslp_Descriptor->UniqueID = 1721;
    vcf_reslp_Descriptor->Label = strdup("vcf_reslp");
    vcf_reslp_Descriptor->Properties = LADSPA_PROPERTY_HARD_RT_CAPABLE;
    vcf_reslp_Descriptor->Name = strdup("Resonant Lowpass Filter");
    vcf_reslp_Descriptor->Maker = strdup("LADSPA code by Matthias Nagorni, Filter formula by Paul Kellett");
    vcf_reslp_Descriptor->Copyright = strdup("GPL");
    vcf_reslp_Descriptor->PortCount = PORT_COUNT_1;
    portDescriptors = (LADSPA_PortDescriptor *)calloc(PORT_COUNT_1, sizeof(LADSPA_PortDescriptor));
    vcf_reslp_Descriptor->PortDescriptors = (const LADSPA_PortDescriptor *)portDescriptors;    
    portRangeHints = (LADSPA_PortRangeHint *)calloc(PORT_COUNT_1, sizeof(LADSPA_PortRangeHint));
    vcf_reslp_Descriptor->PortRangeHints = (const LADSPA_PortRangeHint *)portRangeHints;
    portNames = (char **)calloc(PORT_COUNT_1, sizeof(char*));
    vcf_reslp_Descriptor->PortNames = (const char **)portNames;
    portDescriptors[VCF_INPUT] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    portDescriptors[VCF_OUTPUT] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
    portDescriptors[VCF_GAIN] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    portDescriptors[VCF_FREQ_OFS] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    portDescriptors[VCF_FREQ_PITCH] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    portDescriptors[VCF_RESO_OFS] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    
#ifdef WITH_CV_IN    
    
    portDescriptors[VCF_FREQ_IN] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    portDescriptors[VCF_RESO_IN] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    
#endif    
    
    portNames[VCF_INPUT] = strdup ("Input");
    portNames[VCF_OUTPUT] = strdup ("Output");
    portNames[VCF_GAIN] = strdup ("Gain");
    portNames[VCF_FREQ_OFS] = strdup ("Frequency Offset");
    portNames[VCF_FREQ_PITCH] = strdup ("Frequency Pitch");
    portNames[VCF_RESO_OFS] = strdup ("Resonance Offset");
    
#ifdef WITH_CV_IN    
    
    portNames[VCF_FREQ_IN] = strdup ("Frequency Input");
    portNames[VCF_RESO_IN] = strdup ("Resonance");
    
#endif    
    
    portRangeHints[VCF_INPUT].HintDescriptor = 0;
    portRangeHints[VCF_OUTPUT].HintDescriptor = 0;
    portRangeHints[VCF_GAIN].HintDescriptor = 
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_GAIN].LowerBound = 0;
    portRangeHints[VCF_GAIN].UpperBound = 1;  
    portRangeHints[VCF_FREQ_OFS].HintDescriptor = 
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_FREQ_OFS].LowerBound = MIN_FREQ;
    portRangeHints[VCF_FREQ_OFS].UpperBound = MAX_FREQ;
    portRangeHints[VCF_FREQ_PITCH].HintDescriptor = 
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_FREQ_PITCH].LowerBound = -2;
    portRangeHints[VCF_FREQ_PITCH].UpperBound = 2;
    portRangeHints[VCF_RESO_OFS].HintDescriptor =  
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_RESO_OFS].LowerBound = Q_MIN;
    portRangeHints[VCF_RESO_OFS].UpperBound = 1;
    
#ifdef WITH_CV_IN    
    
    portRangeHints[VCF_FREQ_IN].HintDescriptor = 0;
    portRangeHints[VCF_RESO_IN].HintDescriptor = 0;
    
#endif    
    
    vcf_reslp_Descriptor->instantiate = instantiate_vcf_reslp;
    vcf_reslp_Descriptor->activate = activate_vcf_reslp;
    vcf_reslp_Descriptor->connect_port = connect_port_vcf_reslp;
    vcf_reslp_Descriptor->run = run_vcf_reslp;
    vcf_reslp_Descriptor->cleanup = cleanup_vcf_reslp;
    vcf_reslp_Descriptor->run_adding = NULL;
    vcf_reslp_Descriptor->deactivate = NULL;
  }
  if (vcf_lp_Descriptor) {
    vcf_lp_Descriptor->UniqueID = 1722;
    vcf_lp_Descriptor->Label = strdup("vcf_lp");
    vcf_lp_Descriptor->Properties = LADSPA_PROPERTY_HARD_RT_CAPABLE;
    vcf_lp_Descriptor->Name = strdup("Lowpass Filter");
    vcf_lp_Descriptor->Maker = strdup("LADSPA code by Matthias Nagorni, Filter formula by Robert Bristow-Johnson");
    vcf_lp_Descriptor->Copyright = strdup("GPL");
    vcf_lp_Descriptor->PortCount = PORT_COUNT_1;
    portDescriptors = (LADSPA_PortDescriptor *)calloc(PORT_COUNT_1, sizeof(LADSPA_PortDescriptor));
    vcf_lp_Descriptor->PortDescriptors = (const LADSPA_PortDescriptor *)portDescriptors;    
    portRangeHints = (LADSPA_PortRangeHint *)calloc(PORT_COUNT_1, sizeof(LADSPA_PortRangeHint));
    vcf_lp_Descriptor->PortRangeHints = (const LADSPA_PortRangeHint *)portRangeHints;
    portNames = (char **)calloc(PORT_COUNT_1, sizeof(char*));
    vcf_lp_Descriptor->PortNames = (const char **)portNames;
    portDescriptors[VCF_INPUT] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    portDescriptors[VCF_OUTPUT] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
    portDescriptors[VCF_GAIN] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    portDescriptors[VCF_FREQ_OFS] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    portDescriptors[VCF_FREQ_PITCH] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    portDescriptors[VCF_RESO_OFS] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    
#ifdef WITH_CV_IN    
    
    portDescriptors[VCF_FREQ_IN] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    portDescriptors[VCF_RESO_IN] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    
#endif    
    
    portNames[VCF_INPUT] = strdup ("Input");
    portNames[VCF_OUTPUT] = strdup ("Output");
    portNames[VCF_GAIN] = strdup ("Gain");
    portNames[VCF_FREQ_OFS] = strdup ("Frequency Offset");
    portNames[VCF_FREQ_PITCH] = strdup ("Frequency Pitch");
    portNames[VCF_RESO_OFS] = strdup ("Resonance Offset");

#ifdef WITH_CV_IN

    portNames[VCF_FREQ_IN] = strdup ("Frequency Input");
    portNames[VCF_RESO_IN] = strdup ("Resonance");
    
#endif    
    
    portRangeHints[VCF_INPUT].HintDescriptor = 0;
    portRangeHints[VCF_OUTPUT].HintDescriptor = 0;
    portRangeHints[VCF_GAIN].HintDescriptor = 
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_GAIN].LowerBound = 0;
    portRangeHints[VCF_GAIN].UpperBound = 1;  
    portRangeHints[VCF_FREQ_OFS].HintDescriptor = 
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_FREQ_OFS].LowerBound = MIN_FREQ;
    portRangeHints[VCF_FREQ_OFS].UpperBound = MAX_FREQ;
    portRangeHints[VCF_FREQ_PITCH].HintDescriptor = 
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_FREQ_PITCH].LowerBound = -2;
    portRangeHints[VCF_FREQ_PITCH].UpperBound = 2;
    portRangeHints[VCF_RESO_OFS].HintDescriptor =  
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_RESO_OFS].LowerBound = Q_MIN;
    portRangeHints[VCF_RESO_OFS].UpperBound = Q_MAX;
    
#ifdef WITH_CV_IN    
    
    portRangeHints[VCF_FREQ_IN].HintDescriptor = 0;
    portRangeHints[VCF_RESO_IN].HintDescriptor = 0;
    
#endif    
    
    vcf_lp_Descriptor->instantiate = instantiate_vcf_lp;
    vcf_lp_Descriptor->activate = activate_vcf_lp;
    vcf_lp_Descriptor->connect_port = connect_port_vcf_lp;
    vcf_lp_Descriptor->run = run_vcf_lp;
    vcf_lp_Descriptor->cleanup = cleanup_vcf_lp;
    vcf_lp_Descriptor->run_adding = NULL;
    vcf_lp_Descriptor->deactivate = NULL;
  }
  if (vcf_hp_Descriptor) {
    vcf_hp_Descriptor->UniqueID = 1723;
    vcf_hp_Descriptor->Label = strdup("vcf_hp");
    vcf_hp_Descriptor->Properties = LADSPA_PROPERTY_HARD_RT_CAPABLE;
    vcf_hp_Descriptor->Name = strdup("Highpass Filter");
    vcf_hp_Descriptor->Maker = strdup("LADSPA code by Matthias Nagorni, Filter formula by Robert Bristow-Johnson");
    vcf_hp_Descriptor->Copyright = strdup("GPL");
    vcf_hp_Descriptor->PortCount = PORT_COUNT_1;
    portDescriptors = (LADSPA_PortDescriptor *)calloc(PORT_COUNT_1, sizeof(LADSPA_PortDescriptor));
    vcf_hp_Descriptor->PortDescriptors = (const LADSPA_PortDescriptor *)portDescriptors;    
    portRangeHints = (LADSPA_PortRangeHint *)calloc(PORT_COUNT_1, sizeof(LADSPA_PortRangeHint));
    vcf_hp_Descriptor->PortRangeHints = (const LADSPA_PortRangeHint *)portRangeHints;
    portNames = (char **)calloc(PORT_COUNT_1, sizeof(char*));
    vcf_hp_Descriptor->PortNames = (const char **)portNames;
    portDescriptors[VCF_INPUT] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    portDescriptors[VCF_OUTPUT] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
    portDescriptors[VCF_GAIN] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    portDescriptors[VCF_FREQ_OFS] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    portDescriptors[VCF_FREQ_PITCH] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    portDescriptors[VCF_RESO_OFS] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    
#ifdef WITH_CV_IN    
    
    portDescriptors[VCF_FREQ_IN] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    portDescriptors[VCF_RESO_IN] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    
#endif    
    
    portNames[VCF_INPUT] = strdup ("Input");
    portNames[VCF_OUTPUT] = strdup ("Output");
    portNames[VCF_GAIN] = strdup ("Gain");
    portNames[VCF_FREQ_OFS] = strdup ("Frequency Offset");
    portNames[VCF_FREQ_PITCH] = strdup ("Frequency Pitch");
    portNames[VCF_RESO_OFS] = strdup ("Resonance Offset");

#ifdef WITH_CV_IN

    portNames[VCF_FREQ_IN] = strdup ("Frequency Input");
    portNames[VCF_RESO_IN] = strdup ("Resonance");
   
#endif    
    
    portRangeHints[VCF_INPUT].HintDescriptor = 0;
    portRangeHints[VCF_OUTPUT].HintDescriptor = 0;
    portRangeHints[VCF_GAIN].HintDescriptor = 
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_GAIN].LowerBound = 0;
    portRangeHints[VCF_GAIN].UpperBound = 1;  
    portRangeHints[VCF_FREQ_OFS].HintDescriptor = 
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_FREQ_OFS].LowerBound = MIN_FREQ;
    portRangeHints[VCF_FREQ_OFS].UpperBound = MAX_FREQ;
    portRangeHints[VCF_FREQ_PITCH].HintDescriptor = 
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_FREQ_PITCH].LowerBound = -2;
    portRangeHints[VCF_FREQ_PITCH].UpperBound = 2;
    portRangeHints[VCF_RESO_OFS].HintDescriptor =  
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_RESO_OFS].LowerBound = Q_MIN;
    portRangeHints[VCF_RESO_OFS].UpperBound = Q_MAX;
    
#ifdef WITH_CV_IN    
    
    portRangeHints[VCF_FREQ_IN].HintDescriptor = 0;
    portRangeHints[VCF_RESO_IN].HintDescriptor = 0;
    
#endif    
    
    vcf_hp_Descriptor->instantiate = instantiate_vcf_hp;
    vcf_hp_Descriptor->activate = activate_vcf_hp;
    vcf_hp_Descriptor->connect_port = connect_port_vcf_hp;
    vcf_hp_Descriptor->run = run_vcf_hp;
    vcf_hp_Descriptor->cleanup = cleanup_vcf_hp;
    vcf_hp_Descriptor->run_adding = NULL;
    vcf_hp_Descriptor->deactivate = NULL;
  }
  if (vcf_bp1_Descriptor) {
    vcf_bp1_Descriptor->UniqueID = 1724;
    vcf_bp1_Descriptor->Label = strdup("vcf_bp1");
    vcf_bp1_Descriptor->Properties = LADSPA_PROPERTY_HARD_RT_CAPABLE;
    vcf_bp1_Descriptor->Name = strdup("Bandpass Filter I");
    vcf_bp1_Descriptor->Maker = strdup("LADSPA code by Matthias Nagorni, Filter formula by Robert Bristow-Johnson");
    vcf_bp1_Descriptor->Copyright = strdup("GPL");
    vcf_bp1_Descriptor->PortCount = PORT_COUNT_1;
    portDescriptors = (LADSPA_PortDescriptor *)calloc(PORT_COUNT_1, sizeof(LADSPA_PortDescriptor));
    vcf_bp1_Descriptor->PortDescriptors = (const LADSPA_PortDescriptor *)portDescriptors;    
    portRangeHints = (LADSPA_PortRangeHint *)calloc(PORT_COUNT_1, sizeof(LADSPA_PortRangeHint));
    vcf_bp1_Descriptor->PortRangeHints = (const LADSPA_PortRangeHint *)portRangeHints;
    portNames = (char **)calloc(PORT_COUNT_1, sizeof(char*));
    vcf_bp1_Descriptor->PortNames = (const char **)portNames;
    portDescriptors[VCF_INPUT] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    portDescriptors[VCF_OUTPUT] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
    portDescriptors[VCF_GAIN] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    portDescriptors[VCF_FREQ_OFS] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    portDescriptors[VCF_FREQ_PITCH] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    portDescriptors[VCF_RESO_OFS] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;

#ifdef WITH_CV_IN

    portDescriptors[VCF_FREQ_IN] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    portDescriptors[VCF_RESO_IN] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    
#endif    
    
    portNames[VCF_INPUT] = strdup ("Input");
    portNames[VCF_OUTPUT] = strdup ("Output");
    portNames[VCF_GAIN] = strdup ("Gain");
    portNames[VCF_FREQ_OFS] = strdup ("Frequency Offset");
    portNames[VCF_FREQ_PITCH] = strdup ("Frequency Pitch");
    portNames[VCF_RESO_OFS] = strdup ("Resonance Offset");

#ifdef WITH_CV_IN

    portNames[VCF_FREQ_IN] = strdup ("Frequency Input");
    portNames[VCF_RESO_IN] = strdup ("Resonance");

#endif
    
    portRangeHints[VCF_INPUT].HintDescriptor = 0;
    portRangeHints[VCF_OUTPUT].HintDescriptor = 0;
    portRangeHints[VCF_GAIN].HintDescriptor = 
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_GAIN].LowerBound = 0;
    portRangeHints[VCF_GAIN].UpperBound = 1;  
    portRangeHints[VCF_FREQ_OFS].HintDescriptor = 
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_FREQ_OFS].LowerBound = MIN_FREQ;
    portRangeHints[VCF_FREQ_OFS].UpperBound = MAX_FREQ;
    portRangeHints[VCF_FREQ_PITCH].HintDescriptor = 
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_FREQ_PITCH].LowerBound = -2;
    portRangeHints[VCF_FREQ_PITCH].UpperBound = 2;
    portRangeHints[VCF_RESO_OFS].HintDescriptor =  
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_RESO_OFS].LowerBound = Q_MIN;
    portRangeHints[VCF_RESO_OFS].UpperBound = Q_MAX;

#ifdef WITH_CV_IN

    portRangeHints[VCF_FREQ_IN].HintDescriptor = 0;
    portRangeHints[VCF_RESO_IN].HintDescriptor = 0;
    
#endif    
    
    vcf_bp1_Descriptor->instantiate = instantiate_vcf_bp1;
    vcf_bp1_Descriptor->activate = activate_vcf_bp1;
    vcf_bp1_Descriptor->connect_port = connect_port_vcf_bp1;
    vcf_bp1_Descriptor->run = run_vcf_bp1;
    vcf_bp1_Descriptor->cleanup = cleanup_vcf_bp1;
    vcf_bp1_Descriptor->run_adding = NULL;
    vcf_bp1_Descriptor->deactivate = NULL;
  }
  if (vcf_bp2_Descriptor) {
    vcf_bp2_Descriptor->UniqueID = 1725;
    vcf_bp2_Descriptor->Label = strdup("vcf_bp2");
    vcf_bp2_Descriptor->Properties = LADSPA_PROPERTY_HARD_RT_CAPABLE;
    vcf_bp2_Descriptor->Name = strdup("Bandpass Filter II");
    vcf_bp2_Descriptor->Maker = strdup("LADSPA code by Matthias Nagorni, Filter formula by Robert Bristow-Johnson");
    vcf_bp2_Descriptor->Copyright = strdup("GPL");
    vcf_bp2_Descriptor->PortCount = PORT_COUNT_1;
    portDescriptors = (LADSPA_PortDescriptor *)calloc(PORT_COUNT_1, sizeof(LADSPA_PortDescriptor));
    vcf_bp2_Descriptor->PortDescriptors = (const LADSPA_PortDescriptor *)portDescriptors;    
    portRangeHints = (LADSPA_PortRangeHint *)calloc(PORT_COUNT_1, sizeof(LADSPA_PortRangeHint));
    vcf_bp2_Descriptor->PortRangeHints = (const LADSPA_PortRangeHint *)portRangeHints;
    portNames = (char **)calloc(PORT_COUNT_1, sizeof(char*));
    vcf_bp2_Descriptor->PortNames = (const char **)portNames;
    portDescriptors[VCF_INPUT] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    portDescriptors[VCF_OUTPUT] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
    portDescriptors[VCF_GAIN] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    portDescriptors[VCF_FREQ_OFS] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    portDescriptors[VCF_FREQ_PITCH] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;

    portDescriptors[VCF_RESO_OFS] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;

#ifdef WITH_CV_IN

    portDescriptors[VCF_FREQ_IN] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    portDescriptors[VCF_RESO_IN] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;

#endif

    portNames[VCF_INPUT] = strdup ("Input");
    portNames[VCF_OUTPUT] = strdup ("Output");
    portNames[VCF_GAIN] = strdup ("Gain");
    portNames[VCF_FREQ_OFS] = strdup ("Frequency Offset");
    portNames[VCF_FREQ_PITCH] = strdup ("Frequency Pitch");
    portNames[VCF_RESO_OFS] = strdup ("Resonance Offset");

#ifdef WITH_CV_IN

    portNames[VCF_FREQ_IN] = strdup ("Frequency Input");
    portNames[VCF_RESO_IN] = strdup ("Resonance");
    
#endif    
    
    portRangeHints[VCF_INPUT].HintDescriptor = 0;
    portRangeHints[VCF_OUTPUT].HintDescriptor = 0;
    portRangeHints[VCF_GAIN].HintDescriptor = 
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_GAIN].LowerBound = 0;
    portRangeHints[VCF_GAIN].UpperBound = 1;  
    portRangeHints[VCF_FREQ_OFS].HintDescriptor = 
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_FREQ_OFS].LowerBound = MIN_FREQ;
    portRangeHints[VCF_FREQ_OFS].UpperBound = MAX_FREQ;
    portRangeHints[VCF_FREQ_PITCH].HintDescriptor = 
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_FREQ_PITCH].LowerBound = -2;
    portRangeHints[VCF_FREQ_PITCH].UpperBound = 2;
    portRangeHints[VCF_RESO_OFS].HintDescriptor =  
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_RESO_OFS].LowerBound = Q_MIN;
    portRangeHints[VCF_RESO_OFS].UpperBound = Q_MAX;
    
#ifdef WITH_CV_IN    
    
    portRangeHints[VCF_FREQ_IN].HintDescriptor = 0;
    portRangeHints[VCF_RESO_IN].HintDescriptor = 0;
    
#endif
    
    vcf_bp2_Descriptor->instantiate = instantiate_vcf_bp2;
    vcf_bp2_Descriptor->activate = activate_vcf_bp2;
    vcf_bp2_Descriptor->connect_port = connect_port_vcf_bp2;
    vcf_bp2_Descriptor->run = run_vcf_bp2;
    vcf_bp2_Descriptor->cleanup = cleanup_vcf_bp2;
    vcf_bp2_Descriptor->run_adding = NULL;
    vcf_bp2_Descriptor->deactivate = NULL;
  }
  if (vcf_notch_Descriptor) {
    vcf_notch_Descriptor->UniqueID = 1726;
    vcf_notch_Descriptor->Label = strdup("vcf_notch");
    vcf_notch_Descriptor->Properties = LADSPA_PROPERTY_HARD_RT_CAPABLE;
    vcf_notch_Descriptor->Name = strdup("Notch Filter");
    vcf_notch_Descriptor->Maker = strdup("LADSPA code by Matthias Nagorni, Filter formula by Robert Bristow-Johnson");
    vcf_notch_Descriptor->Copyright = strdup("GPL");
    vcf_notch_Descriptor->PortCount = PORT_COUNT_1;
    portDescriptors = (LADSPA_PortDescriptor *)calloc(PORT_COUNT_1, sizeof(LADSPA_PortDescriptor));
    vcf_notch_Descriptor->PortDescriptors = (const LADSPA_PortDescriptor *)portDescriptors;    
    portRangeHints = (LADSPA_PortRangeHint *)calloc(PORT_COUNT_1, sizeof(LADSPA_PortRangeHint));
    vcf_notch_Descriptor->PortRangeHints = (const LADSPA_PortRangeHint *)portRangeHints;
    portNames = (char **)calloc(PORT_COUNT_1, sizeof(char*));
    vcf_notch_Descriptor->PortNames = (const char **)portNames;
    portDescriptors[VCF_INPUT] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    portDescriptors[VCF_OUTPUT] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
    portDescriptors[VCF_GAIN] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    portDescriptors[VCF_FREQ_OFS] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    portDescriptors[VCF_FREQ_PITCH] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    portDescriptors[VCF_RESO_OFS] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    
#ifdef WITH_CV_IN    
    
    portDescriptors[VCF_FREQ_IN] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    portDescriptors[VCF_RESO_IN] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    
#endif    
    
    portNames[VCF_INPUT] = strdup ("Input");
    portNames[VCF_OUTPUT] = strdup ("Output");
    portNames[VCF_GAIN] = strdup ("Gain");
    portNames[VCF_FREQ_OFS] = strdup ("Frequency Offset");
    portNames[VCF_FREQ_PITCH] = strdup ("Frequency Pitch");
    portNames[VCF_RESO_OFS] = strdup ("Resonance Offset");

#ifdef WITH_CV_IN

    portNames[VCF_FREQ_IN] = strdup ("Frequency Input");
    portNames[VCF_RESO_IN] = strdup ("Resonance");
    
#endif    
    
    portRangeHints[VCF_INPUT].HintDescriptor = 0;
    portRangeHints[VCF_OUTPUT].HintDescriptor = 0;
    portRangeHints[VCF_GAIN].HintDescriptor = 
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_GAIN].LowerBound = 0;
    portRangeHints[VCF_GAIN].UpperBound = 1;  
    portRangeHints[VCF_FREQ_OFS].HintDescriptor = 
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_FREQ_OFS].LowerBound = MIN_FREQ;
    portRangeHints[VCF_FREQ_OFS].UpperBound = MAX_FREQ;
    portRangeHints[VCF_FREQ_PITCH].HintDescriptor = 
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_FREQ_PITCH].LowerBound = -2;
    portRangeHints[VCF_FREQ_PITCH].UpperBound = 2;
    portRangeHints[VCF_RESO_OFS].HintDescriptor =  
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_RESO_OFS].LowerBound = Q_MIN;
    portRangeHints[VCF_RESO_OFS].UpperBound = Q_MAX;

#ifdef WITH_CV_IN

    portRangeHints[VCF_FREQ_IN].HintDescriptor = 0;
    portRangeHints[VCF_RESO_IN].HintDescriptor = 0;
    
#endif    
    
    vcf_notch_Descriptor->instantiate = instantiate_vcf_notch;
    vcf_notch_Descriptor->activate = activate_vcf_notch;
    vcf_notch_Descriptor->connect_port = connect_port_vcf_notch;
    vcf_notch_Descriptor->run = run_vcf_notch;
    vcf_notch_Descriptor->cleanup = cleanup_vcf_notch;
    vcf_notch_Descriptor->run_adding = NULL;
    vcf_notch_Descriptor->deactivate = NULL;
  }
  if (vcf_peakeq_Descriptor) {
    vcf_peakeq_Descriptor->UniqueID = 1727;
    vcf_peakeq_Descriptor->Label = strdup("vcf_peakeq");
    vcf_peakeq_Descriptor->Properties = LADSPA_PROPERTY_HARD_RT_CAPABLE;
    vcf_peakeq_Descriptor->Name = strdup("Peaking EQ Filter");
    vcf_peakeq_Descriptor->Maker = strdup("LADSPA code by Matthias Nagorni, Filter formula by Robert Bristow-Johnson");
    vcf_peakeq_Descriptor->Copyright = strdup("GPL");
    vcf_peakeq_Descriptor->PortCount = PORT_COUNT_2;
    portDescriptors = (LADSPA_PortDescriptor *)calloc(PORT_COUNT_2, sizeof(LADSPA_PortDescriptor));
    vcf_peakeq_Descriptor->PortDescriptors = (const LADSPA_PortDescriptor *)portDescriptors;    
    portRangeHints = (LADSPA_PortRangeHint *)calloc(PORT_COUNT_2, sizeof(LADSPA_PortRangeHint));
    vcf_peakeq_Descriptor->PortRangeHints = (const LADSPA_PortRangeHint *)portRangeHints;
    portNames = (char **)calloc(PORT_COUNT_2, sizeof(char*));
    vcf_peakeq_Descriptor->PortNames = (const char **)portNames;
    portDescriptors[VCF_INPUT] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    portDescriptors[VCF_OUTPUT] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
    portDescriptors[VCF_GAIN] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    portDescriptors[VCF_FREQ_OFS] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    portDescriptors[VCF_FREQ_PITCH] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    portDescriptors[VCF_RESO_OFS] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    portDescriptors[VCF_DBGAIN_OFS] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;

#ifdef WITH_CV_IN

    portDescriptors[VCF_FREQ_IN] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    portDescriptors[VCF_RESO_IN] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    portDescriptors[VCF_DBGAIN_IN] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    
#endif    
    
    portNames[VCF_INPUT] = strdup ("Input");
    portNames[VCF_OUTPUT] = strdup ("Output");
    portNames[VCF_GAIN] = strdup ("Gain");
    portNames[VCF_FREQ_OFS] = strdup ("Frequency Offset");
    portNames[VCF_FREQ_PITCH] = strdup ("Frequency Pitch");
    portNames[VCF_RESO_OFS] = strdup ("Resonance Offset");
    portNames[VCF_DBGAIN_OFS] = strdup ("dBgain Offset");

#ifdef WITH_CV_IN

    portNames[VCF_FREQ_IN] = strdup ("Frequency Input");
    portNames[VCF_RESO_IN] = strdup ("Resonance");
    portNames[VCF_DBGAIN_IN] = strdup ("dBgain Input");
    
#endif    
    
    portRangeHints[VCF_INPUT].HintDescriptor = 0;
    portRangeHints[VCF_OUTPUT].HintDescriptor = 0;
    portRangeHints[VCF_GAIN].HintDescriptor = 
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_GAIN].LowerBound = 0;
    portRangeHints[VCF_GAIN].UpperBound = 1;  
    portRangeHints[VCF_FREQ_OFS].HintDescriptor = 
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_FREQ_OFS].LowerBound = MIN_FREQ;
    portRangeHints[VCF_FREQ_OFS].UpperBound = MAX_FREQ;
    portRangeHints[VCF_FREQ_PITCH].HintDescriptor = 
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_FREQ_PITCH].LowerBound = -2;
    portRangeHints[VCF_FREQ_PITCH].UpperBound = 2;
    portRangeHints[VCF_RESO_OFS].HintDescriptor =  
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_RESO_OFS].LowerBound = Q_MIN;
    portRangeHints[VCF_RESO_OFS].UpperBound = Q_MAX;
    portRangeHints[VCF_DBGAIN_OFS].HintDescriptor =
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_DBGAIN_OFS].LowerBound = DBGAIN_MIN;
    portRangeHints[VCF_DBGAIN_OFS].UpperBound = DBGAIN_MAX;

#ifdef WITH_CV_IN

    portRangeHints[VCF_FREQ_IN].HintDescriptor = 0;
    portRangeHints[VCF_RESO_IN].HintDescriptor = 0;
    portRangeHints[VCF_DBGAIN_IN].HintDescriptor = 0;
    
#endif    
    
    vcf_peakeq_Descriptor->instantiate = instantiate_vcf_peakeq;
    vcf_peakeq_Descriptor->activate = activate_vcf_peakeq;
    vcf_peakeq_Descriptor->connect_port = connect_port_vcf_peakeq;
    vcf_peakeq_Descriptor->run = run_vcf_peakeq;
    vcf_peakeq_Descriptor->cleanup = cleanup_vcf_peakeq;
    vcf_peakeq_Descriptor->run_adding = NULL;
    vcf_peakeq_Descriptor->deactivate = NULL;
  }
  if (vcf_lshelf_Descriptor) {
    vcf_lshelf_Descriptor->UniqueID = 1728;
    vcf_lshelf_Descriptor->Label = strdup("vcf_lshelf");
    vcf_lshelf_Descriptor->Properties = LADSPA_PROPERTY_HARD_RT_CAPABLE;
    vcf_lshelf_Descriptor->Name = strdup("Low Shelf Filter");
    vcf_lshelf_Descriptor->Maker = strdup("LADSPA code by Matthias Nagorni, Filter formula by Robert Bristow-Johnson");
    vcf_lshelf_Descriptor->Copyright = strdup("GPL");
    vcf_lshelf_Descriptor->PortCount = PORT_COUNT_2;
    portDescriptors = (LADSPA_PortDescriptor *)calloc(PORT_COUNT_2, sizeof(LADSPA_PortDescriptor));
    vcf_lshelf_Descriptor->PortDescriptors = (const LADSPA_PortDescriptor *)portDescriptors;    
    portRangeHints = (LADSPA_PortRangeHint *)calloc(PORT_COUNT_2, sizeof(LADSPA_PortRangeHint));
    vcf_lshelf_Descriptor->PortRangeHints = (const LADSPA_PortRangeHint *)portRangeHints;
    portNames = (char **)calloc(PORT_COUNT_2, sizeof(char*));
    vcf_lshelf_Descriptor->PortNames = (const char **)portNames;
    portDescriptors[VCF_INPUT] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    portDescriptors[VCF_OUTPUT] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
    portDescriptors[VCF_GAIN] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    portDescriptors[VCF_FREQ_OFS] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    portDescriptors[VCF_FREQ_PITCH] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    portDescriptors[VCF_RESO_OFS] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    portDescriptors[VCF_DBGAIN_OFS] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;

#ifdef WITH_CV_IN

    portDescriptors[VCF_FREQ_IN] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    portDescriptors[VCF_RESO_IN] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    portDescriptors[VCF_DBGAIN_IN] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    
#endif    
    
    portNames[VCF_INPUT] = strdup ("Input");
    portNames[VCF_OUTPUT] = strdup ("Output");
    portNames[VCF_GAIN] = strdup ("Gain");
    portNames[VCF_FREQ_OFS] = strdup ("Frequency Offset");
    portNames[VCF_FREQ_PITCH] = strdup ("Frequency Pitch");
    portNames[VCF_RESO_OFS] = strdup ("Resonance Offset");
    portNames[VCF_DBGAIN_OFS] = strdup ("dBgain Offset");

#ifdef WITH_CV_IN

    portNames[VCF_FREQ_IN] = strdup ("Frequency Input");
    portNames[VCF_RESO_IN] = strdup ("Resonance");
    portNames[VCF_DBGAIN_IN] = strdup ("dBgain Input");
    
#endif    
    
    portRangeHints[VCF_INPUT].HintDescriptor = 0;
    portRangeHints[VCF_OUTPUT].HintDescriptor = 0;
    portRangeHints[VCF_GAIN].HintDescriptor = 
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_GAIN].LowerBound = 0;
    portRangeHints[VCF_GAIN].UpperBound = 1;  
    portRangeHints[VCF_FREQ_OFS].HintDescriptor = 
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_FREQ_OFS].LowerBound = MIN_FREQ;
    portRangeHints[VCF_FREQ_OFS].UpperBound = MAX_FREQ;
    portRangeHints[VCF_FREQ_PITCH].HintDescriptor = 
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_FREQ_PITCH].LowerBound = -2;
    portRangeHints[VCF_FREQ_PITCH].UpperBound = 2;
    portRangeHints[VCF_RESO_OFS].HintDescriptor =  
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_RESO_OFS].LowerBound = Q_MIN;
    portRangeHints[VCF_RESO_OFS].UpperBound = Q_MAX;
    portRangeHints[VCF_DBGAIN_OFS].HintDescriptor =
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_DBGAIN_OFS].LowerBound = DBGAIN_MIN;
    portRangeHints[VCF_DBGAIN_OFS].UpperBound = DBGAIN_MAX;

#ifdef WITH_CV_IN

    portRangeHints[VCF_FREQ_IN].HintDescriptor = 0;
    portRangeHints[VCF_RESO_IN].HintDescriptor = 0;
    portRangeHints[VCF_DBGAIN_IN].HintDescriptor = 0;
    
 #endif   
    
    vcf_lshelf_Descriptor->instantiate = instantiate_vcf_lshelf;
    vcf_lshelf_Descriptor->activate = activate_vcf_lshelf;
    vcf_lshelf_Descriptor->connect_port = connect_port_vcf_lshelf;
    vcf_lshelf_Descriptor->run = run_vcf_lshelf;
    vcf_lshelf_Descriptor->cleanup = cleanup_vcf_lshelf;
    vcf_lshelf_Descriptor->run_adding = NULL;
    vcf_lshelf_Descriptor->deactivate = NULL;
  }
  if (vcf_hshelf_Descriptor) {
    vcf_hshelf_Descriptor->UniqueID = 1729;
    vcf_hshelf_Descriptor->Label = strdup("vcf_hshelf");
    vcf_hshelf_Descriptor->Properties = LADSPA_PROPERTY_HARD_RT_CAPABLE;
    vcf_hshelf_Descriptor->Name = strdup("High Shelf Filter");
    vcf_hshelf_Descriptor->Maker = strdup("LADSPA code by Matthias Nagorni, Filter formula by Robert Bristow-Johnson");
    vcf_hshelf_Descriptor->Copyright = strdup("GPL");
    vcf_hshelf_Descriptor->PortCount = PORT_COUNT_2;
    portDescriptors = (LADSPA_PortDescriptor *)calloc(PORT_COUNT_2, sizeof(LADSPA_PortDescriptor));
    vcf_hshelf_Descriptor->PortDescriptors = (const LADSPA_PortDescriptor *)portDescriptors;    
    portRangeHints = (LADSPA_PortRangeHint *)calloc(PORT_COUNT_2, sizeof(LADSPA_PortRangeHint));
    vcf_hshelf_Descriptor->PortRangeHints = (const LADSPA_PortRangeHint *)portRangeHints;
    portNames = (char **)calloc(PORT_COUNT_2, sizeof(char*));
    vcf_hshelf_Descriptor->PortNames = (const char **)portNames;
    portDescriptors[VCF_INPUT] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    portDescriptors[VCF_OUTPUT] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
    portDescriptors[VCF_GAIN] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    portDescriptors[VCF_FREQ_OFS] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    portDescriptors[VCF_FREQ_PITCH] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    portDescriptors[VCF_RESO_OFS] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    portDescriptors[VCF_DBGAIN_OFS] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;

#ifdef WITH_CV_IN

    portDescriptors[VCF_FREQ_IN] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    portDescriptors[VCF_RESO_IN] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    portDescriptors[VCF_DBGAIN_IN] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    
#endif    
    
    portNames[VCF_INPUT] = strdup ("Input");
    portNames[VCF_OUTPUT] = strdup ("Output");
    portNames[VCF_GAIN] = strdup ("Gain");
    portNames[VCF_FREQ_OFS] = strdup ("Frequency Offset");
    portNames[VCF_FREQ_PITCH] = strdup ("Frequency Pitch");
    portNames[VCF_RESO_OFS] = strdup ("Resonance Offset");
    portNames[VCF_DBGAIN_OFS] = strdup ("dBgain Offset");

#ifdef WITH_CV_IN

    portNames[VCF_FREQ_IN] = strdup ("Frequency Input");
    portNames[VCF_RESO_IN] = strdup ("Resonance");
    portNames[VCF_DBGAIN_IN] = strdup ("dBgain Input");
    
#endif    
    
    portRangeHints[VCF_INPUT].HintDescriptor = 0;
    portRangeHints[VCF_OUTPUT].HintDescriptor = 0;
    portRangeHints[VCF_GAIN].HintDescriptor = 
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_GAIN].LowerBound = 0;
    portRangeHints[VCF_GAIN].UpperBound = 1;  
    portRangeHints[VCF_FREQ_OFS].HintDescriptor = 
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_FREQ_OFS].LowerBound = MIN_FREQ;
    portRangeHints[VCF_FREQ_OFS].UpperBound = MAX_FREQ;
    portRangeHints[VCF_FREQ_PITCH].HintDescriptor = 
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_FREQ_PITCH].LowerBound = -2;
    portRangeHints[VCF_FREQ_PITCH].UpperBound = 2;
    portRangeHints[VCF_RESO_OFS].HintDescriptor =  
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_RESO_OFS].LowerBound = Q_MIN;
    portRangeHints[VCF_RESO_OFS].UpperBound = Q_MAX;
    portRangeHints[VCF_DBGAIN_OFS].HintDescriptor =
      LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
    portRangeHints[VCF_DBGAIN_OFS].LowerBound = DBGAIN_MIN;
    portRangeHints[VCF_DBGAIN_OFS].UpperBound = DBGAIN_MAX;

#ifdef WITH_CV_IN

    portRangeHints[VCF_FREQ_IN].HintDescriptor = 0;
    portRangeHints[VCF_RESO_IN].HintDescriptor = 0;
    portRangeHints[VCF_DBGAIN_IN].HintDescriptor = 0;
    
 #endif   
    
    vcf_hshelf_Descriptor->instantiate = instantiate_vcf_hshelf;
    vcf_hshelf_Descriptor->activate = activate_vcf_hshelf;
    vcf_hshelf_Descriptor->connect_port = connect_port_vcf_hshelf;
    vcf_hshelf_Descriptor->run = run_vcf_hshelf;
    vcf_hshelf_Descriptor->cleanup = cleanup_vcf_hshelf;
    vcf_hshelf_Descriptor->run_adding = NULL;
    vcf_hshelf_Descriptor->deactivate = NULL;
  }
}

void _fini()
{
  int l1;

  if (vcf_reslp_Descriptor) {
    free((char *)vcf_reslp_Descriptor->Label);  
    free((char *)vcf_reslp_Descriptor->Name);
    free((char *)vcf_reslp_Descriptor->Copyright);
    free((LADSPA_PortDescriptor *)vcf_reslp_Descriptor->PortDescriptors);
    for (l1 = 0; l1 < vcf_reslp_Descriptor->PortCount; l1++) {
      free((char *)vcf_reslp_Descriptor->PortNames[l1]);
    }
    free((char **)vcf_reslp_Descriptor->PortNames);
    free((LADSPA_PortRangeHint *)vcf_reslp_Descriptor->PortRangeHints);
    free(vcf_reslp_Descriptor);
  }  
  if (vcf_lp_Descriptor) {
    free((char *)vcf_lp_Descriptor->Label);  
    free((char *)vcf_lp_Descriptor->Name);
    free((char *)vcf_lp_Descriptor->Copyright);
    free((LADSPA_PortDescriptor *)vcf_lp_Descriptor->PortDescriptors);
    for (l1 = 0; l1 < vcf_lp_Descriptor->PortCount; l1++) {
      free((char *)vcf_lp_Descriptor->PortNames[l1]);
    }
    free((char **)vcf_lp_Descriptor->PortNames);
    free((LADSPA_PortRangeHint *)vcf_lp_Descriptor->PortRangeHints);
    free(vcf_lp_Descriptor);
  }  
  if (vcf_hp_Descriptor) {
    free((char *)vcf_hp_Descriptor->Label);  
    free((char *)vcf_hp_Descriptor->Name);
    free((char *)vcf_hp_Descriptor->Copyright);
    free((LADSPA_PortDescriptor *)vcf_hp_Descriptor->PortDescriptors);
    for (l1 = 0; l1 < vcf_hp_Descriptor->PortCount; l1++) {
      free((char *)vcf_hp_Descriptor->PortNames[l1]);
    }
    free((char **)vcf_hp_Descriptor->PortNames);
    free((LADSPA_PortRangeHint *)vcf_hp_Descriptor->PortRangeHints);
    free(vcf_hp_Descriptor);
  }  
  if (vcf_bp1_Descriptor) {
    free((char *)vcf_bp1_Descriptor->Label);  
    free((char *)vcf_bp1_Descriptor->Name);
    free((char *)vcf_bp1_Descriptor->Copyright);
    free((LADSPA_PortDescriptor *)vcf_bp1_Descriptor->PortDescriptors);
    for (l1 = 0; l1 < vcf_bp1_Descriptor->PortCount; l1++) {
      free((char *)vcf_bp1_Descriptor->PortNames[l1]);
    }
    free((char **)vcf_bp1_Descriptor->PortNames);
    free((LADSPA_PortRangeHint *)vcf_bp1_Descriptor->PortRangeHints);
    free(vcf_bp1_Descriptor);
  }  
  if (vcf_bp2_Descriptor) {
    free((char *)vcf_bp2_Descriptor->Label);  
    free((char *)vcf_bp2_Descriptor->Name);
    free((char *)vcf_bp2_Descriptor->Copyright);
    free((LADSPA_PortDescriptor *)vcf_bp2_Descriptor->PortDescriptors);
    for (l1 = 0; l1 < vcf_bp2_Descriptor->PortCount; l1++) {
      free((char *)vcf_bp2_Descriptor->PortNames[l1]);
    }
    free((char **)vcf_bp2_Descriptor->PortNames);
    free((LADSPA_PortRangeHint *)vcf_bp2_Descriptor->PortRangeHints);
    free(vcf_bp2_Descriptor);
  }  
  if (vcf_notch_Descriptor) {
    free((char *)vcf_notch_Descriptor->Label);  
    free((char *)vcf_notch_Descriptor->Name);
    free((char *)vcf_notch_Descriptor->Copyright);
    free((LADSPA_PortDescriptor *)vcf_notch_Descriptor->PortDescriptors);
    for (l1 = 0; l1 < vcf_notch_Descriptor->PortCount; l1++) {
      free((char *)vcf_notch_Descriptor->PortNames[l1]);
    }
    free((char **)vcf_notch_Descriptor->PortNames);
    free((LADSPA_PortRangeHint *)vcf_notch_Descriptor->PortRangeHints);
    free(vcf_notch_Descriptor);
  }  
  if (vcf_peakeq_Descriptor) {
    free((char *)vcf_peakeq_Descriptor->Label);  
    free((char *)vcf_peakeq_Descriptor->Name);
    free((char *)vcf_peakeq_Descriptor->Copyright);
    free((LADSPA_PortDescriptor *)vcf_peakeq_Descriptor->PortDescriptors);
    for (l1 = 0; l1 < vcf_peakeq_Descriptor->PortCount; l1++) {
      free((char *)vcf_peakeq_Descriptor->PortNames[l1]);
    }
    free((char **)vcf_peakeq_Descriptor->PortNames);
    free((LADSPA_PortRangeHint *)vcf_peakeq_Descriptor->PortRangeHints);
    free(vcf_peakeq_Descriptor);
  }  
  if (vcf_lshelf_Descriptor) {
    free((char *)vcf_lshelf_Descriptor->Label);  
    free((char *)vcf_lshelf_Descriptor->Name);
    free((char *)vcf_lshelf_Descriptor->Copyright);
    free((LADSPA_PortDescriptor *)vcf_lshelf_Descriptor->PortDescriptors);
    for (l1 = 0; l1 < vcf_lshelf_Descriptor->PortCount; l1++) {
      free((char *)vcf_lshelf_Descriptor->PortNames[l1]);
    }
    free((char **)vcf_lshelf_Descriptor->PortNames);
    free((LADSPA_PortRangeHint *)vcf_lshelf_Descriptor->PortRangeHints);
    free(vcf_lshelf_Descriptor);
  }  
  if (vcf_hshelf_Descriptor) {
    free((char *)vcf_hshelf_Descriptor->Label);  
    free((char *)vcf_hshelf_Descriptor->Name);
    free((char *)vcf_hshelf_Descriptor->Copyright);
    free((LADSPA_PortDescriptor *)vcf_hshelf_Descriptor->PortDescriptors);
    for (l1 = 0; l1 < vcf_hshelf_Descriptor->PortCount; l1++) {
      free((char *)vcf_hshelf_Descriptor->PortNames[l1]);
    }
    free((char **)vcf_hshelf_Descriptor->PortNames);
    free((LADSPA_PortRangeHint *)vcf_hshelf_Descriptor->PortRangeHints);
    free(vcf_hshelf_Descriptor);
  }  
}
