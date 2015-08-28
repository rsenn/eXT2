//----------------------------------------------------------------------------
/*
    ladspa2vst - ladspa 2 vst plugins bridge class

    Copyright (C) 2007 kRAkEn/gORe

    This is experimental software. Is based around ladspa plugins
    specification and steinberg vst specifications.

    To compile your ladspa plugins under a vst environment you will
    need to download a ladspa plugin, the complete vst sdk from steinberg.
    then define you LADSPA_INIT function (typically _init) and
    LADSPA_FINI function (typically _fini) and what is the global
    descriptor for the plugin (usually g_psDescriptor). then include
    you ladspa .h/.c file and compile. This would produce a shared
    library loadable from any linux native vst hosts.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
//----------------------------------------------------------------------------

#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <string.h>

#include <AudioEffect.cpp>
#include <audioeffectx.h>
#include <audioeffectx.cpp>

#include "ladspa/ladspa.h"

#define LADSPA2VST_VERSION      0.3


#if defined (AMB_PLUGINS) // AMBisonic plugins port

    void null () {}
    #define LADSPA_INIT         null
    #define LADSPA_FINI         null
    #include "amb/filter1.cc"
    #include "amb/ambis2.cc"
    #include "amb/ambis2_if.cc"

    #if defined MONOPANNER
        #define LADSPA_DESCRIPTOR &moddescr [0]
    #elif defined STEREOPANNER
        #define LADSPA_DESCRIPTOR &moddescr [1]
    #elif defined ROTATOR
        #define LADSPA_DESCRIPTOR &moddescr [2]
    #endif

#elif defined (MCP_PLUGINS) // MCP plugins port

    void null () {}
    #define LADSPA_INIT         null
    #define LADSPA_FINI         null

    #if defined CHORUS1
        #include "mcp/cs_chorus.cc"
        #include "mcp/cs_chorus_if.cc"
        #define LADSPA_DESCRIPTOR &moddescr [0]
    #elif defined CHORUS2
        #include "mcp/cs_chorus.cc"
        #include "mcp/cs_chorus_if.cc"
        #define LADSPA_DESCRIPTOR &moddescr [1]
    #elif defined PHASER1
        #include "mcp/exp2ap.cc"
        #include "mcp/cs_phaser.cc"
        #include "mcp/cs_phaser_if.cc"
        #define LADSPA_DESCRIPTOR &moddescr [0]
    #elif defined PHASER2
        #include "mcp/exp2ap.cc"
        #include "mcp/cs_phaser.cc"
        #include "mcp/cs_phaser_if.cc"
        #define LADSPA_DESCRIPTOR &moddescr [1]
    #elif defined LPF24
        #include "mcp/exp2ap.cc"
        #include "mcp/mvclpf24.cc"
        #include "mcp/mvclpf24_if.cc"
        #define LADSPA_DESCRIPTOR &moddescr [0]
    #elif defined HPF24
        #include "mcp/exp2ap.cc"
        #include "mcp/mvchpf24.cc"
        #include "mcp/mvchpf24_if.cc"
        #define LADSPA_DESCRIPTOR &moddescr [0]
    #endif

#elif defined (FIL_PLUGIN) // FIL parametric filter plugin port

    void null () {}
    #define LADSPA_INIT         null
    #define LADSPA_FINI         null
    #define LADSPA_DESCRIPTOR &moddescr [0]
    #include "mcp/exp2ap.cc"
    #include "fil/filters.cc"
    #include "fil/filters_if.cc"

#elif defined (TAP_PLUGINS) // Tap plugins port

    #define LADSPA_INIT         _init
    #define LADSPA_FINI         _fini

    #if defined AUTOPAN
        #include "tap/tap_autopan.c"
        #define LADSPA_DESCRIPTOR mono_descriptor
    #elif defined CHORUSFLANGER
        #include "tap/tap_chorusflanger.c"
        #define LADSPA_DESCRIPTOR stereo_descriptor
    #elif defined DEESSER
        #include "tap/tap_deesser.c"
        #define LADSPA_DESCRIPTOR mono_descriptor
    #elif defined DOUBLER
        #include "tap/tap_doubler.c"
        #define LADSPA_DESCRIPTOR stereo_descriptor
    #elif defined DYNAMICS_M
        #include "tap/tap_dynamics_m.c"
        #define LADSPA_DESCRIPTOR mono_descriptor
    #elif defined DYNAMICS_ST
        #include "tap/tap_dynamics_st.c"
        #define LADSPA_DESCRIPTOR stereo_descriptor
    #elif defined ECHO
        #include "tap/tap_echo.c"
        #define LADSPA_DESCRIPTOR stereo_descriptor
    #elif defined EQ
        #include "tap/tap_eq.c"
        #define LADSPA_DESCRIPTOR eqDescriptor
    #elif defined EQBW
        #include "tap/tap_eqbw.c"
        #define LADSPA_DESCRIPTOR eqDescriptor
    #elif defined LIMITER
        #include "tap/tap_limiter.c"
        #define LADSPA_DESCRIPTOR mono_descriptor
    #elif defined PINKNOISE
        #include "tap/tap_pinknoise.c"
        #define LADSPA_DESCRIPTOR mono_descriptor
    #elif defined PITCH
        #include "tap/tap_pitch.c"
        #define LADSPA_DESCRIPTOR mono_descriptor
    #elif defined REFLECTOR
        #include "tap/tap_reflector.c"
        #define LADSPA_DESCRIPTOR mono_descriptor
    #elif defined REVERB
        #include "tap/tap_reverb.c"
        #define LADSPA_DESCRIPTOR stereo_descriptor
    #elif defined ROTSPEAK
        #include "tap/tap_rotspeak.c"
        #define LADSPA_DESCRIPTOR stereo_descriptor
    #elif defined SIGMOID
        #include "tap/tap_sigmoid.c"
        #define LADSPA_DESCRIPTOR mono_descriptor
    #elif defined TREMOLO
        #include "tap/tap_tremolo.c"
        #define LADSPA_DESCRIPTOR mono_descriptor
    #elif defined TUBEWARMTH
        #include "tap/tap_tubewarmth.c"
        #define LADSPA_DESCRIPTOR mono_descriptor
    #elif defined VIBRATO
        #include "tap/tap_vibrato.c"
        #define LADSPA_DESCRIPTOR mono_descriptor
    #endif

#elif defined (VCF_PLUGINS) // Vcf plugins port (broken)

    #define LADSPA_INIT         _init
    #define LADSPA_FINI         _fini
    #include "vcf/vcf.c"

    #if defined RESLP
        #define LADSPA_DESCRIPTOR vcf_reslp_Descriptor
    #elif defined LP
        #define LADSPA_DESCRIPTOR vcf_lp_Descriptor
    #elif defined HP
        #define LADSPA_DESCRIPTOR vcf_hp_Descriptor
    #elif defined BP1
        #define LADSPA_DESCRIPTOR vcf_bp1_Descriptor
    #elif defined BP2
        #define LADSPA_DESCRIPTOR vcf_bp2_Descriptor
    #elif defined NOTCH
        #define LADSPA_DESCRIPTOR vcf_notch_Descriptor
    #elif defined PEAKEQ
        #define LADSPA_DESCRIPTOR vcf_peakeq_Descriptor
    #elif defined LSHELF
        #define LADSPA_DESCRIPTOR vcf_lshelf_Descriptor
    #elif defined HSHELF
        #define LADSPA_DESCRIPTOR vcf_hshelf_Descriptor
    #endif

#elif defined (VCO_PLUGINS) // VCO plugins port

    void null () {}
    #define LADSPA_INIT         null
    #define LADSPA_FINI         null
    #include "vco/exp2ap.cc"
    #include "vco/blvco.cc"
    #include "vco/blvco_if.cc"

    #if defined PULSE
        #define LADSPA_DESCRIPTOR &moddescr [0]
    #elif defined SAW
        #define LADSPA_DESCRIPTOR &moddescr [1]
    #elif defined REV
        #define LADSPA_DESCRIPTOR &moddescr [2]
    #endif

#elif defined (REV_PLUGIN) // G2 Reverb plugin port

    void null () {}
    #define LADSPA_INIT         null
    #define LADSPA_FINI         null

    #define LADSPA_DESCRIPTOR   &moddescr [0]
    #include "rev/exp2ap.cc"
    #include "rev/greverb.cc"
    #include "rev/g2reverb.cc"
    #include "rev/g2reverb_if.cc"

#elif defined (VOCODER_PLUGIN) // Vocoder plugin port

    #include "vocoder/vocoder.c"
    #define LADSPA_INIT         _init
    #define LADSPA_FINI         _fini
    #define LADSPA_DESCRIPTOR   g_psDescriptor

#elif defined (CMT_PLUGINS) // CMT plugins port

    #define LADSPA_INIT         _init
    #define LADSPA_FINI         _fini

#elif defined (PVOC_PLUGINS) // PVOC plugins port

    #include "pvoc/pvoc.cc"
    #include "pvoc/Plugins.cc"
    #include "pvoc/interface.cc"

    #define LADSPA_INIT         pvoc_init
    #define LADSPA_FINI         pvoc_fini
    
    #if defined EXAGGERATE
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[0]);
    #elif defined TRANSPOSE
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[1]);
    #elif defined ACCUMULATE
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[2]);
    #endif

#elif defined (CAPS_PLUGINS) // CAPS plugins port

    #include "caps/Amp.cc"
    #include "caps/Cabinet.cc"
    #include "caps/Chorus.cc"
    #include "caps/Click.cc"
    #include "caps/Clip.cc"
    #include "caps/Compress.cc"
    #include "caps/Eq.cc"
    #include "caps/HRTF.cc"
    #include "caps/Lorenz.cc"
    #include "caps/Pan.cc"
    #include "caps/Phaser.cc"
    #include "caps/Preamp.cc"
    #include "caps/Reverb.cc"
    #include "caps/Roessler.cc"
    #include "caps/Scape.cc"
    #include "caps/Sin.cc"
    #include "caps/SweepVF.cc"
    #include "caps/ToneControls.cc"
    #include "caps/VCO.cc"
    #include "caps/White.cc"
    #include "caps/interface.cc"

    #define LADSPA_INIT         caps_init
    #define LADSPA_FINI         caps_fini
    
    #if defined EQ
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[0]);
    #elif defined COMPRESS
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[1]);
    #elif defined PAN
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[2]);
    #elif defined PREAMPIII
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[3]);
    #elif defined PREAMPIV
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[4]);
    #elif defined AMPIII
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[5]);
    #elif defined AMPIV
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[6]);
    #elif defined AMPV
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[7]);
    #elif defined CABINETI
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[8]);
    #elif defined CABINETII
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[9]);
    #elif defined CLIP
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[10]);
    #elif defined CHORUSI
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[11]);
    #elif defined STEREOCHORUSI
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[12]);
    #elif defined CHORUSII
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[13]);
    #elif defined STEREOCHORUSII
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[14]);
    #elif defined PHASERI
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[15]);
    #elif defined PHASERII
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[16]);
    #elif defined SWEEPVFI
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[17]);
    #elif defined SWEEPVFII
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[18]);
    #elif defined SCAPE
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[19]);
    #elif defined VCOS
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[20]);
    #elif defined VOCD
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[21]);
    #elif defined CEO
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[22]);
    #elif defined SIN
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[23]);
    #elif defined WHITE
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[24]);
    #elif defined LORENZ
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[25]);
    #elif defined ROESSLER
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[26]);
    #elif defined JVREV
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[27]);
    #elif defined PLATE
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[28]);
    #elif defined PLATE2x2
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[29]);
    #elif defined CLICK
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[30]);
    #elif defined DIRAC
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[31]);
    #elif defined HRTF
        #define LADSPA_DESCRIPTOR dynamic_cast<LADSPA_Descriptor*> (descriptors[32]);
    #endif
    
#elif defined (SWH_PLUGINS) // SWH plugins port

    #define LADSPA_INIT         _init
    #define LADSPA_FINI         _fini

    #if defined ALIAS
        #include "swh/alias_1407.c"
        #define LADSPA_DESCRIPTOR aliasDescriptor
    #elif defined ALLPASS
        #include "swh/allpass_1895.c"
        #define LADSPA_DESCRIPTOR allpass_nDescriptor
    #elif defined AMPITCHSHIFT
        #include "swh/am_pitchshift_1433.c"
        #define LADSPA_DESCRIPTOR amPitchshiftDescriptor
    #elif defined AMP
        #include "swh/amp_1181.c"
        #define LADSPA_DESCRIPTOR ampDescriptor
    #elif defined ANALOGUEOSC
        #include "swh/analogue_osc_1416.c"
        #define LADSPA_DESCRIPTOR analogueOscDescriptor
    #elif defined BANDPASS_A_IIR
        #include "swh/bandpass_a_iir_1893.c"
        #define LADSPA_DESCRIPTOR bandpass_a_iirDescriptor
    #elif defined BANDPASS_IIR
        #include "swh/bandpass_iir_1892.c"
        #define LADSPA_DESCRIPTOR bandpass_iirDescriptor
    #elif defined BODESHIFTER
        #include "swh/bode_shifter_1431.c"
        #define LADSPA_DESCRIPTOR bodeShifterDescriptor
    #elif defined BODESHIFTER_CV
        #include "swh/bode_shifter_cv_1432.c"
        #define LADSPA_DESCRIPTOR bodeShifterCVDescriptor
    #elif defined BUTTERWORTH
        #include "swh/butterworth_1902.c"
        #define LADSPA_DESCRIPTOR bwxover_iirDescriptor
    #elif defined CHEBSTORTION
        #include "swh/chebstortion_1430.c"
        #define LADSPA_DESCRIPTOR chebstortionDescriptor
    #elif defined COMB1190
        #include "swh/comb_1190.c"
        #define LADSPA_DESCRIPTOR combDescriptor
    #elif defined COMB1887
        #include "swh/comb_1887.c"
        #define LADSPA_DESCRIPTOR comb_nDescriptor
    #elif defined COMBSPLITTER
        #include "swh/comb_splitter_1411.c"
        #define LADSPA_DESCRIPTOR combSplitterDescriptor
    #elif defined SWHCONST
        #include "swh/const_1909.c"
        #define LADSPA_DESCRIPTOR constDescriptor
    #elif defined CROSSOVERDIST
        #include "swh/crossover_dist_1404.c"
        #define LADSPA_DESCRIPTOR crossoverDistDescriptor
    #elif defined DCREMOVE
        #include "swh/dc_remove_1207.c"
        #define LADSPA_DESCRIPTOR dcRemoveDescriptor
    #elif defined DECAY
        #include "swh/decay_1886.c"
        #define LADSPA_DESCRIPTOR decayDescriptor
    #elif defined DECIMATOR
        #include "swh/decimator_1202.c"
        #define LADSPA_DESCRIPTOR decimatorDescriptor
    #elif defined DECLIP
        #include "swh/declip_1195.c"
        #define LADSPA_DESCRIPTOR declipDescriptor
    #elif defined DELAY1898
        #include "swh/delay_1898.c"
        #define LADSPA_DESCRIPTOR delay_nDescriptor
    #elif defined DELAYORAMA
        #include "swh/delayorama_1402.c"
        #define LADSPA_DESCRIPTOR delayoramaDescriptor
    #elif defined DIODE
        #include "swh/diode_1185.c"
        #define LADSPA_DESCRIPTOR diodeDescriptor
    #elif defined DIVIDER
        #include "swh/divider_1186.c"
        #define LADSPA_DESCRIPTOR dividerDescriptor
    #elif defined DJEQ
        #include "swh/dj_eq_1901.c"
        #define LADSPA_DESCRIPTOR dj_eqDescriptor
    #elif defined DJFLANGER
        #include "swh/dj_flanger_1438.c"
        #define LADSPA_DESCRIPTOR djFlangerDescriptor
    #elif defined DYSONCOMPRESS
        #include "swh/dyson_compress_1403.c"
        #define LADSPA_DESCRIPTOR dysonCompressDescriptor
    #elif defined FADDELAY
        #include "swh/fad_delay_1192.c"
        #define LADSPA_DESCRIPTOR fadDelayDescriptor
    #elif defined FASTLOOKAHEADLIMITER
        #include "swh/fast_lookahead_limiter_1913.c"
        #define LADSPA_DESCRIPTOR fastLookaheadLimiterDescriptor
    #elif defined FLANGER
        #include "swh/flanger_1191.c"
        #define LADSPA_DESCRIPTOR flangerDescriptor
    #elif defined FMOSC
        #include "swh/fm_osc_1415.c"
        #define LADSPA_DESCRIPTOR fmOscDescriptor
    #elif defined FOLDOVER
        #include "swh/foldover_1213.c"
        #define LADSPA_DESCRIPTOR foldoverDescriptor
    #elif defined FOVERDRIVE
        #include "swh/foverdrive_1196.c"
        #define LADSPA_DESCRIPTOR foverdriveDescriptor
    #elif defined FREQTRACKER
        #include "swh/freq_tracker_1418.c"
        #define LADSPA_DESCRIPTOR freqTrackerDescriptor
    #elif defined GATE
        #include "swh/gate_1410.c"
        #define LADSPA_DESCRIPTOR gateDescriptor
    #elif defined GIANTFLANGE
        #include "swh/giant_flange_1437.c"
        #define LADSPA_DESCRIPTOR giantFlangeDescriptor
    #elif defined GONG
        #include "swh/gong_1424.c"
        #define LADSPA_DESCRIPTOR gongDescriptor
    #elif defined GONGBEATER
        #include "swh/gong_beater_1439.c"
        #define LADSPA_DESCRIPTOR gongBeaterDescriptor
    #elif defined GSM
        #include "swh/gsm_1215.c"
        #define LADSPA_DESCRIPTOR gsmDescriptor
    #elif defined GVERB
        #include "swh/gverb_1216.c"
        #define LADSPA_DESCRIPTOR gverbDescriptor
    #elif defined HARDLIMITER
        #include "swh/hard_limiter_1413.c"
        #define LADSPA_DESCRIPTOR hardLimiterDescriptor
    #elif defined HARMONICGEN
        #include "swh/harmonic_gen_1220.c"
        #define LADSPA_DESCRIPTOR harmonicGenDescriptor
    #elif defined HERMESFILTER
        #include "swh/hermes_filter_1200.c"
        #define LADSPA_DESCRIPTOR hermesFilterDescriptor
    #elif defined HIGHPASSIIR
        #include "swh/highpass_iir_1890.c"
        #define LADSPA_DESCRIPTOR highpass_iirDescriptor
    #elif defined HILBERT
        #include "swh/hilbert_1440.c"
        #define LADSPA_DESCRIPTOR hilbertDescriptor
    #elif defined IMP
        #include "swh/imp_1199.c"
        #define LADSPA_DESCRIPTOR impDescriptor
    #elif defined IMPULSE
        #include "swh/impulse_1885.c"
        #define LADSPA_DESCRIPTOR impulse_fcDescriptor
    #elif defined INV
        #include "swh/inv_1429.c"
        #define LADSPA_DESCRIPTOR invDescriptor
    #elif defined KARAOKE
        #include "swh/karaoke_1409.c"
        #define LADSPA_DESCRIPTOR karaokeDescriptor
    #elif defined LATENCY
        #include "swh/latency_1914.c"
        #define LADSPA_DESCRIPTOR artificialLatencyDescriptor
    #elif defined LCRDELAY
        #include "swh/lcr_delay_1436.c"
        #define LADSPA_DESCRIPTOR lcrDelayDescriptor
    #elif defined LOOKAHEADLIMITER
        #include "swh/lookahead_limiter_1435.c"
        #define LADSPA_DESCRIPTOR lookaheadLimiterDescriptor
    #elif defined LOOKAHEADLIMITERCONST
        #include "swh/lookahead_limiter_const_1906.c"
        #define LADSPA_DESCRIPTOR lookaheadLimiterConstDescriptor
    #elif defined LOWPASSIIR
        #include "swh/lowpass_iir_1891.c"
        #define LADSPA_DESCRIPTOR lowpass_iirDescriptor
    #elif defined LSFILTER
        #include "swh/ls_filter_1908.c"
        #define LADSPA_DESCRIPTOR lsFilterDescriptor
    #elif defined MATRIXMSST
        #include "swh/matrix_ms_st_1421.c"
        #define LADSPA_DESCRIPTOR matrixMSStDescriptor
    #elif defined MATRIXSPATIALISER
        #include "swh/matrix_spatialiser_1422.c"
        #define LADSPA_DESCRIPTOR matrixSpatialiserDescriptor
    #elif defined MATRIXSTMS
        #include "swh/matrix_st_ms_1420.c"
        #define LADSPA_DESCRIPTOR matrixStMSDescriptor
    #elif defined MBEQ
        #include "swh/mbeq_1197.c"
        #define LADSPA_DESCRIPTOR mbeqDescriptor
    #elif defined MODDELAY
        #include "swh/mod_delay_1419.c"
        #define LADSPA_DESCRIPTOR modDelayDescriptor
    #elif defined MULTIVOICECHORUS
        #include "swh/multivoice_chorus_1201.c"
        #define LADSPA_DESCRIPTOR multivoiceChorusDescriptor
    #elif defined NOTCHIIR
        #include "swh/notch_iir_1894.c"
        #define LADSPA_DESCRIPTOR notch_iirDescriptor
    #elif defined PHASERS
        #include "swh/phasers_1217.c"
        #define LADSPA_DESCRIPTOR fourByFourPoleDescriptor
    #elif defined PITCHSCALELOW
        #include "swh/pitch_scale_1193.c"
        #define LADSPA_DESCRIPTOR pitchScaleDescriptor
    #elif defined PITCHSCALEHIGH
        #include "swh/pitch_scale_1194.c"
        #define LADSPA_DESCRIPTOR pitchScaleHQDescriptor
    #elif defined PLATE
        #include "swh/plate_1423.c"
        #define LADSPA_DESCRIPTOR plateDescriptor
    #elif defined POINTERCAST
        #include "swh/pointer_cast_1910.c"
        #define LADSPA_DESCRIPTOR pointerCastDistortionDescriptor
    #elif defined RATESHIFTER
        #include "swh/rate_shifter_1417.c"
        #define LADSPA_DESCRIPTOR rateShifterDescriptor
    #elif defined RETROFLANGE
        #include "swh/retro_flange_1208.c"
        #define LADSPA_DESCRIPTOR retroFlangeDescriptor
    #elif defined REVDELAY
        #include "swh/revdelay_1605.c"
        #define LADSPA_DESCRIPTOR revdelayDescriptor
    #elif defined RINGMOD
        #include "swh/ringmod_1188.c"
        #define LADSPA_DESCRIPTOR ringmod_1i1o1lDescriptor
    #elif defined SATANMAXIMISER
        #include "swh/satan_maximiser_1408.c"
        #define LADSPA_DESCRIPTOR satanMaximiserDescriptor
    #elif defined SC11425
        #include "swh/sc1_1425.c"
        #define LADSPA_DESCRIPTOR sc1Descriptor
    #elif defined SC21426
        #include "swh/sc2_1426.c"
        #define LADSPA_DESCRIPTOR sc2Descriptor
    #elif defined SC31427
        #include "swh/sc3_1427.c"
        #define LADSPA_DESCRIPTOR sc3Descriptor
    #elif defined SC41434
        #include "swh/sc4_1434.c"
        #define LADSPA_DESCRIPTOR sc4Descriptor
    #elif defined SC41882
        #include "swh/sc4_1882.c"
        #define LADSPA_DESCRIPTOR sc4Descriptor
    #elif defined SC4M1916
        #include "swh/sc4m_1916.c"
        #define LADSPA_DESCRIPTOR sc4mDescriptor
    #elif defined SE41883
        #include "swh/se4_1883.c"
        #define LADSPA_DESCRIPTOR se4Descriptor
    #elif defined SHAPER
        #include "swh/shaper_1187.c"
        #define LADSPA_DESCRIPTOR shaperDescriptor
    #elif defined SIFTER
        #include "swh/sifter_1210.c"
        #define LADSPA_DESCRIPTOR sifterDescriptor
    #elif defined SINCOS
        #include "swh/sin_cos_1881.c"
        #define LADSPA_DESCRIPTOR sinCosDescriptor
    #elif defined SINGLEPARA
        #include "swh/single_para_1203.c"
        #define LADSPA_DESCRIPTOR singleParaDescriptor
    #elif defined SINUSWAVEWRAPPER
        #include "swh/sinus_wavewrapper_1198.c"
        #define LADSPA_DESCRIPTOR sinusWavewrapperDescriptor
    #elif defined SMOOTHDECIMATE
        #include "swh/smooth_decimate_1414.c"
        #define LADSPA_DESCRIPTOR smoothDecimateDescriptor
    #elif defined SPLIT
        #include "swh/split_1406.c"
        #define LADSPA_DESCRIPTOR splitDescriptor
    #elif defined STEPMUXER
        #include "swh/step_muxer_1212.c"
        #define LADSPA_DESCRIPTOR stepMuxerDescriptor
    #elif defined SURROUNDENCODER
        #include "swh/surround_encoder_1401.c"
        #define LADSPA_DESCRIPTOR surroundEncoderDescriptor
    #elif defined SVF
        #include "swh/svf_1214.c"
        #define LADSPA_DESCRIPTOR svfDescriptor
    #elif defined TAPEDELAY
        #include "swh/tape_delay_1211.c"
        #define LADSPA_DESCRIPTOR tapeDelayDescriptor
    #elif defined TRANSIENT
        #include "swh/transient_1206.c"
        #define LADSPA_DESCRIPTOR transientDescriptor
    #elif defined TRIPLEPARA
        #include "swh/triple_para_1204.c"
        #define LADSPA_DESCRIPTOR tripleParaDescriptor
    #elif defined VALVE
        #include "swh/valve_1209.c"
        #define LADSPA_DESCRIPTOR valveDescriptor
    #elif defined VALVERECT
        #include "swh/valve_rect_1405.c"
        #define LADSPA_DESCRIPTOR valveRectDescriptor
    #elif defined VYNIL
        #include "swh/vynil_1905.c"
        #define LADSPA_DESCRIPTOR vynilDescriptor
    #elif defined WAVETERRAIN
        #include "swh/wave_terrain_1412.c"
        #define LADSPA_DESCRIPTOR waveTerrainDescriptor
    #elif defined XFADE
        #include "swh/xfade_1915.c"
        #define LADSPA_DESCRIPTOR xfadeDescriptor
    #elif defined ZM1
        #include "swh/zm1_1428.c"
        #define LADSPA_DESCRIPTOR zm1Descriptor
    #endif

#else

    #error You don't have defined any ladspa plugin type !

#endif
