
#include "SDT.h"

// AGC
#define MAX_SAMPLE_RATE (24000.0)
#define MAX_N_TAU (8)
#define MAX_TAU_ATTACK (0.01)
#define RB_SIZE (int)(MAX_SAMPLE_RATE * MAX_N_TAU * MAX_TAU_ATTACK + 1)

#define debug_alternate_NR
const int boundary_blank = 14;  // 14. For first trials very large!!!!
int8_t NB_taps = 10;
int8_t NB_impulse_samples = 7;
const int impulse_length = NB_impulse_samples;  // 7.  Has to be odd!!!! 7 / 3 should be enough
const int PL = (impulse_length - 1) / 2;        // 6. 3 has to be (impulse_length-1)/2 !!!!
uint8_t decay_type = 0;
bool hang_enable = false;
uint8_t NB_test = 0;
auto attack_buffsize = 0;
int hang_counter = 0;
auto n_tau = 0;
int out_index = -1;
int pmode = 1;
uint32_t in_index = 0;
auto notchFreq = 1000;
auto notchCenterBin = 0;
auto state = 0;
uint32_t ring_buffsize = RB_SIZE;
float32_t ring[RB_SIZE * 2];
float32_t knee_dBFS = -15.0;
float32_t abs_ring[RB_SIZE]{ 0.0 };
float32_t attack_mult{ 0.0 };
float32_t abs_out_sample{ 0.0 };
float32_t decay_mult{ 0.0 };
float32_t fast_backaverage{ .0 };
float32_t fast_backmult{ 0.0 };
float32_t fast_decay_mult{ 0.0 };
float32_t fixed_gain = 1.0;
float32_t hang_backaverage{ 0.0 };
float32_t hang_backmult{ 0.0 };
float32_t hang_decay_mult{ 0.0 };
float32_t hang_thresh{ 0.0 };
float32_t hang_level{ 0.0 };
float32_t hangtime{ 0.0 };
float32_t inv_max_input{ 0.0 };
float32_t inv_out_target{ 0.0 };
float32_t max_gain{ 0.0 };
float32_t max_input = -0.1;
float32_t min_volts{ 0.0 };
float32_t NB_thresh = 2.5;
float32_t onemfast_backmult{ 0.0 };
float32_t onemhang_backmult{ 0.0 };
float32_t out_sample[2]{ 0.0 };
float32_t out_targ{ 0.0 };
float32_t out_target{ 0.0 };
float32_t pop_ratio{ 0.0 };
float32_t ring_max = 0.0;
float32_t save_volts = 0.0;
float32_t slope_constant{ 0.0 };
float32_t tau_attack{ 0.0 };
float32_t tau_decay{ 0.0 };
float32_t tau_fast_backaverage = 0.0;
float32_t tau_fast_decay{ 0.0 };
float32_t tau_hang_backmult{ 0.0 };
float32_t tau_hang_decay{ 0.0 };
float32_t var_gain{ 0.0 };
float32_t volts{ 0.0 };


void CalcNotchBins() {
  bin_BW = SR[SampleRate].rate / 16;  // sample rate/2/8
  // calculate notch centre bin for FFT512
  notchCenterBin = roundf(notchFreq / bin_BW);
  // calculate bins  for deletion of bins in the iFFT_buffer
  // set iFFT_buffer[notch_L] to iFFT_buffer[notch_R] to zero
}


// ================= AGC
// G0ORX broke this code out so can be called from other places

void AGCLoadValues() {
  float32_t tmp;
  float32_t sample_rate = (float32_t)SR[SampleRate].rate / DF;

  //calculate internal parameters
  switch (EEPROMData.AGCMode) {
    case 0:  //agcOFF
      break;

    case 1:  //agcLONG
      // G0ORX
      hangtime = 2.000;
      tau_decay = 2.000;
      break;

    case 2:  //agcSLOW
      // G0ORX
      hangtime = 1.000;
      tau_decay = 0.5;
      break;

    case 3:  //agcMED        hang_thresh = 1.0;
      // G0ORX
      hangtime = 0.000;
      tau_decay = 0.250;
      break;

    case 4:  //agcFAST
      hang_thresh = 1.0;
      // G0ORX
      hangtime = 0.0;
      tau_decay = 0.050;
      break;

    default:
      break;
  }

  max_gain = powf(10.0, (float32_t)bands[EEPROMData.currentBand].AGC_thresh / 20.0);
  attack_buffsize = (int)ceil(sample_rate * n_tau * tau_attack);
  in_index = attack_buffsize + out_index;
  attack_mult = 1.0 - expf(-1.0 / (sample_rate * tau_attack));
  decay_mult = 1.0 - expf(-1.0 / (sample_rate * tau_decay));
  fast_decay_mult = 1.0 - expf(-1.0 / (sample_rate * tau_fast_decay));
  fast_backmult = 1.0 - expf(-1.0 / (sample_rate * tau_fast_backaverage));

  onemfast_backmult = 1.0 - fast_backmult;

  out_target = out_targ * (1.0 - expf(-(float32_t)n_tau)) * 0.9999;
  min_volts = out_target / (var_gain * max_gain);
  inv_out_target = 1.0 / out_target;

  tmp = log10f(out_target / (max_input * var_gain * max_gain));
  if (tmp == 0.0)
    tmp = 1e-16;
  slope_constant = (out_target * (1.0 - 1.0 / var_gain)) / tmp;

  inv_max_input = 1.0 / max_input;

  tmp = powf(10.0, (hang_thresh - 1.0) / 0.125);
  hang_level = (max_input * tmp + (out_target / (var_gain * max_gain)) * (1.0 - tmp)) * 0.637;

  hang_backmult = 1.0 - expf(-1.0 / (sample_rate * tau_hang_backmult));
  onemhang_backmult = 1.0 - hang_backmult;

  hang_decay_mult = 1.0 - expf(-1.0 / (sample_rate * tau_hang_decay));
}


/*****
  Purpose: Setup AGC()
  Parameter list:
    void
  Return value;
    void
*****/
void AGCPrep() {
  // Start variables taken from wdsp

  tau_attack = 0.001;  // tau_attack
  tau_decay = 0.250;   // G0ORX
  n_tau = 4;           // G0ORX

  // max_gain = 1000.0 to be applied??? or is this AGC threshold = knee level?
  max_gain = 10000.0;  // G0ORX
  fixed_gain = 20.0;   // G0ORX
  max_input = 1.0;     // G0ORX
  out_targ = 1.0;      // G0ORX       // target value of audio after AGC
  var_gain = 1.5;      // G0ORX

  tau_fast_backaverage = 0.250;  // tau_fast_backaverage
  tau_fast_decay = 0.005;        // tau_fast_decay
  pop_ratio = 5.0;               // pop_ratio
  hang_enable = 1;               // G0ORX
  tau_hang_backmult = 0.500;     // tau_hang_backmult
  hangtime = 0.250;              // hangtime
  hang_thresh = 0.250;           // hang_thresh
  tau_hang_decay = 0.100;        // tau_hang_decay

  AGCLoadValues();  // G0ORX
}


void AGCThresholdChanged() {
  max_gain = powf(10.0, (float32_t)bands[EEPROMData.currentBand].AGC_thresh / 20.0);
}


/*****
  Purpose: Audio AGC()
  Parameter list:
    void
  Return value;
    void
*****/
void AGC() {
  int k;
  float32_t mult;
  if (EEPROMData.AGCMode == 0)  // AGC OFF
  {
    for (unsigned i = 0; i < FFT_length / 2; i++) {
      iFFT_buffer[FFT_length + 2 * i + 0] = fixed_gain * iFFT_buffer[FFT_length + 2 * i + 0];
      iFFT_buffer[FFT_length + 2 * i + 1] = fixed_gain * iFFT_buffer[FFT_length + 2 * i + 1];
    }
    return;
  }

  for (unsigned i = 0; i < FFT_length / 2; i++) {
    if (++out_index >= (int)ring_buffsize) out_index -= ring_buffsize;
    if (++in_index >= ring_buffsize) in_index -= ring_buffsize;

    out_sample[0] = ring[2 * out_index + 0];
    out_sample[1] = ring[2 * out_index + 1];
    abs_out_sample = abs_ring[out_index];
    ring[2 * in_index + 0] = iFFT_buffer[FFT_length + 2 * i + 0];
    ring[2 * in_index + 1] = iFFT_buffer[FFT_length + 2 * i + 1];
    if (pmode == 0)  // MAGNITUDE CALCULATION
      abs_ring[in_index] = max(fabs(ring[2 * in_index + 0]), fabs(ring[2 * in_index + 1]));
    else
      abs_ring[in_index] = sqrtf(ring[2 * in_index + 0] * ring[2 * in_index + 0] + ring[2 * in_index + 1] * ring[2 * in_index + 1]);

    fast_backaverage = fast_backmult * abs_out_sample + onemfast_backmult * fast_backaverage;
    hang_backaverage = hang_backmult * abs_out_sample + onemhang_backmult * hang_backaverage;

    if ((abs_out_sample >= ring_max) && (abs_out_sample > 0.0)) {
      ring_max = 0.0;
      k = out_index;
      for (int j = 0; j < attack_buffsize; j++) {
        if (++k == (int)ring_buffsize)
          k = 0;
        if (abs_ring[k] > ring_max)
          ring_max = abs_ring[k];
      }
    }
    if (abs_ring[in_index] > ring_max)
      ring_max = abs_ring[in_index];

    if (hang_counter > 0) --hang_counter;

    switch (state) {
      case 0:
        if (ring_max >= volts) {
          volts += (ring_max - volts) * attack_mult;
        } else {
          if (volts > pop_ratio * fast_backaverage) {
            state = 1;
            volts += (ring_max - volts) * fast_decay_mult;
          } else {
            if (hang_enable && (hang_backaverage > hang_level)) {
              state = 2;
              hang_counter = (int)(hangtime * SR[SampleRate].rate / DF);
              decay_type = 1;
            } else {
              state = 3;
              volts += (ring_max - volts) * decay_mult;
              decay_type = 0;
            }
          }
        }
        break;

      case 1:
        if (ring_max >= volts) {
          state = 0;
          volts += (ring_max - volts) * attack_mult;
        } else {
          if (volts > save_volts) {
            volts += (ring_max - volts) * fast_decay_mult;
          } else {
            if (hang_counter > 0) {
              state = 2;
            } else {
              if (decay_type == 0) {
                state = 3;
                volts += (ring_max - volts) * decay_mult;
              } else {
                state = 4;
                volts += (ring_max - volts) * hang_decay_mult;
              }
            }
          }
        }
        break;

      case 2:
        if (ring_max >= volts) {
          state = 0;
          save_volts = volts;
          volts += (ring_max - volts) * attack_mult;
        } else {
          if (hang_counter == 0) {
            state = 4;
            volts += (ring_max - volts) * hang_decay_mult;
          }
        }
        break;

      case 3:
        if (ring_max >= volts) {
          state = 0;
          save_volts = volts;
          volts += (ring_max - volts) * attack_mult;
        } else {
          volts += (ring_max - volts) * decay_mult * .05;
        }
        break;

      case 4:
        if (ring_max >= volts) {
          state = 0;
          save_volts = volts;
          volts += (ring_max - volts) * attack_mult;
        } else {
          volts += (ring_max - volts) * hang_decay_mult;
        }
        break;
    }
    if (volts < min_volts) {
      volts = min_volts;  // no AGC action is taking place
      agc_action = false;
    } else {
      agc_action = true;  // LED indicator for AGC action
    }

    //#ifdef USE_LOG10FAST
    mult = (out_target - slope_constant * min(0.0, log10f_fast(inv_max_input * volts))) / volts;
    //#else
    //  mult = (out_target - slope_constant * min (0.0, log10f(inv_max_input * volts))) / volts;
    //#endif
    iFFT_buffer[FFT_length + 2 * i + 0] = out_sample[0] * mult;
    iFFT_buffer[FFT_length + 2 * i + 1] = out_sample[1] * mult;
  }
}
