// ALAE v2 VCV RACK PLUGIN - CODE BY LUKAS ÖSTREICH 2025


// Include nescessary files
#include "plugin.hpp"
#include <vector>
#include <samplerate.h>

// define buffer length and max count of unison channels as constants

#define	BufferLength  (1<<19) // buffer length with bit shift operation - on 48khz 16bit this should be around 2,73 seconds
#define uni_chans (8)


// create custom knobs


struct BigLuggezKnob : RoundKnob {
	BigLuggezKnob() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BigLuggezKnob.svg")));
	}
};

struct MediumLuggezKnob : RoundKnob {
	MediumLuggezKnob() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/MediumLuggezKnob.svg")));
	}
};

struct SmallLuggezKnob : RoundKnob {
	SmallLuggezKnob() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SmallLuggezKnob.svg")));
	}
};




// Module Struct: Params, Inputs, Outputs & Lights


struct Alae : Module {
	enum ParamId {
		ATT_TUNE_TRCK,
		ATT_TUNE_FM,
		PRM_TUNE,
		PRM_FINE_TUNE,
		ATT_TUNE_SPRD,
		PRM_TUNE_SPRD,
		PRM_VOX_COUNT,
		ATT_FLTR_FREQ,
		PRM_FLTR_FREQ,
		ATT_FLTR_SPRD,
		PRM_FLTR_SPRD,
		ATT_FLTR_RES,
		PRM_FLTR_RES,
		ATT_DEC,
		PRM_DEC,
		ATT_FB,
		PRM_FB,
		PRM_VCA,
		ATT_DRYWET,
		PRM_DRYWET,
		ATT_FB_IN,
		BTN_TYPE,
		BTN_KEYTRACK,
		PARAMS_LEN
	};
	enum InputId {
		IN_TUNE_1VOCT,
		IN_TUNE_FM,
		IN_TUNE_SPRD,
		IN_FLTR_FREQ,
		IN_FLTR_SPRD,
		IN_FLTR_RES,
		IN_DEC,
		IN_FB,
		IN_VCA,
		IN_DRYWET,
		IN_AUDIO,
		IN_AUDIO_FB,
		INPUTS_LEN
	};
	enum OutputId {
		OUT_AUDIO_LEFT,
		OUT_AUDIO_RIGHT,
		OUTPUTS_LEN
	};
	enum LightId {
		LGHT_LP,
		LGHT_BP,
		LGHT_HP,
		LGHT_NO,
		LGHT_TYPE,
		LGHT_KEYTRACK,
		LIGHTS_LEN
	};

	// init filters and triggers inside module to ensure they arent shared accross multiple instances

	std::vector<dsp::BiquadFilter> filters;
    std::vector<dsp::BiquadFilter> dc_block;
    dsp::BooleanTrigger a_type_trigger;
    dsp::BooleanTrigger a_keytrack_trigger;
    dsp::BooleanTrigger a_tune_sprd_trigger;
    dsp::BooleanTrigger a_fltr_sprd_trigger;

    // module contructor. setting the channel count for filters and dc_block
    
	Alae() :  filters(uni_chans), dc_block(uni_chans)  {

		// config Params Inputs and Outputs

		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(PRM_VOX_COUNT, 1, uni_chans, 8, "Unison Voice Count");

		configParam(PRM_TUNE, -1.f, 1.f, 0.4f, "Tuning");
		configParam(PRM_FINE_TUNE, -1.f, 1.f, 0.f, "Fine Tuning");
		configParam(PRM_DEC, -1.f, 1.f, 0.3f, "Decay");
		configParam(PRM_TUNE_SPRD, -1.f, 1.f, 0.f, "Tuning Spread");
		configParam(PRM_FB, 0.f, 1.f, 0.f, "Feedback");
		configParam(PRM_VCA, 0.f, 5.f, 2.5f, "VCA");
		configParam(PRM_DRYWET, 0.f, 1.f, 1.f, "DRY / WET");

		configParam(PRM_FLTR_FREQ, -0.1f, 1.f, 0.2f, "Filter Freq");
		configParam(PRM_FLTR_SPRD, -1.f, 1.f, 0.f, "Filter Spread");
		configParam(PRM_FLTR_RES, 0.f, 1.f, 0.707f, "Filter Resonance");

		configParam(ATT_TUNE_TRCK, -10.f, 5.f, -0.f, "Tracking Adjustment");
		configParam(ATT_TUNE_FM, -1.f, 1.f, 0.f, "FM");
		configParam(ATT_TUNE_SPRD, -1.f, 1.f, 0.f, "Spread");
		configParam(ATT_FLTR_FREQ, -1.f, 1.f, 0.f, "Filter FM");
		configParam(ATT_FLTR_SPRD, -1.f, 1.f, 0.f, "Filter Spread");
		configParam(ATT_FLTR_RES, -1.f, 1.f, 0.f, "Filter Resonance");
		configParam(ATT_DEC, -1.f, 1.f, 0.f, "Decay");
		configParam(ATT_FB, -1.f, 1.f, 0.f, "Feedback Saturation");
		configParam(ATT_DRYWET, -1.f, 1.f, 0.f, "Dry / Wet");
		configParam(ATT_FB_IN, -1.f, 1.f, 0.f, "External Feedback");


		configButton(BTN_TYPE, "Filter Type");
		configButton(BTN_KEYTRACK, "Filter Keytracking");

		configInput(IN_TUNE_1VOCT, "1VOCT");
		configInput(IN_TUNE_FM, "FM");
		configInput(IN_TUNE_SPRD, "Spread");
		configInput(IN_FLTR_FREQ, "Filter FM");
		configInput(IN_FLTR_SPRD, "Filter Spread");
		configInput(IN_FLTR_RES, "Filter Resonance");
		configInput(IN_DEC, "Decay");
		configInput(IN_FB, "Feedback Saturation");
		configInput(IN_VCA, "VCA");
		configInput(IN_DRYWET, "DRY / WET");
		configInput(IN_AUDIO_FB, "External Feedback Input");
		
		configInput(IN_AUDIO, "");
		
		configOutput(OUT_AUDIO_LEFT, "");
		configOutput(OUT_AUDIO_RIGHT, "");

	}

	// init variables
	


	float buffers[uni_chans][BufferLength] = {};
	float audio_in, audio_out_left, audio_out_right, delay_out[uni_chans], last_delay_out[uni_chans], delay_in[uni_chans];
	float a_tune[uni_chans], a_freq[uni_chans], a_filter_freq[uni_chans];
	float a_filter_res, a_t60[uni_chans], a_decay[uni_chans], a_phase, a_feedback, a_vca_mod, a_mix, a_base_freq;
	float a_base_freq_norm, a_filter_norm[uni_chans], a_base_tune, a_spread_tune, a_spread_filter;
	float a_dc_block_freq[uni_chans];
	float a_filter_freq_min = 30.f;
	float a_filter_freq_max = 20000.f;

	// init spread factors

	float a_harmonic_spread[uni_chans] = {-3.5f, -2.5f, -1.5f, -0.5f, 0.5f, 1.5f, 2.5f, 3.5f};
	float a_inharm_factor[uni_chans] = {0.8375f, 0.1923f, 0.5234f, 0.6152f, 0.4032f, 0.9948f, 0.2345f, 0.7812f}; 

	float a_index[uni_chans], a_read[uni_chans];
	int a_write[uni_chans];
	int activeChannels = 1;
	int a_type = 1;
	int a_keytrack = 1;
	int a_sat_select = 1;

	// init spread modes to random	
	int a_tune_sprd_mode = 1; 
	int a_fltr_sprd_mode = 1; 

	// init interpolation to linear
	int InterpolationSelect = 1;

	// proces function
	void process(const ProcessArgs& args) override {   

		// get the active channel count from the VOX param
		activeChannels = int((params[PRM_VOX_COUNT].getValue()));

		// read audio input and add attenuated external feedback
		audio_in = inputs[IN_AUDIO].getVoltage() + (inputs[IN_AUDIO_FB].getVoltage() * params[ATT_FB_IN].getValue());

		// rescale input voltage to -1 - +1 for calculations
		audio_in = rack::math::rescale(audio_in, -5.f, 5.f, -1.f, 1.f);

		// read FB-SAT param
		a_feedback = params[PRM_FB].getValue();

		// apply attenuated input CV
		a_feedback += rack::math::rescale(inputs[IN_FB].getVoltage(), -5.f, 5.f, -1.f, 1.f) * params[ATT_FB].getValue(); 

		// scale FB-SAT exponential for better control over small feedback falues
		a_feedback = 1.f + (10.f - 1.f ) * pow(a_feedback, 3.f); 

		// clamp feedback to make sure it stays in its range
		a_feedback = clamp(a_feedback, 1.f, 10.f); 

		// apply noise for self oscillation if input is not connected
		if (!inputs[IN_AUDIO].isConnected()) {
			audio_in += (a_feedback - 1) * ((rand() % 100) * 0.000001f); // apply noise to self oscillate
		}

		// reset audio outputs  to fill in with new added samples in the loop
		audio_out_left = 0;
		audio_out_right = 0;  

		// decay polarity phase detetction
		a_phase = params[PRM_DEC].getValue() + rack::math::rescale(inputs[IN_DEC].getVoltage(), -5.f, 5.f, -1.f, 1.f) * params[ATT_DEC].getValue();
		if(a_phase > 0) {a_phase = 1.f;}
		if(a_phase < 0) {a_phase = -1.f;}


		// filter switch logic
		if (a_type_trigger.process(params[BTN_TYPE].getValue())) a_type++;
		if (a_type > 5) a_type = 1;

		// keytracking switch logic
		if (a_keytrack_trigger.process(params[BTN_KEYTRACK].getValue())) a_keytrack++;
		if (a_keytrack > 1) a_keytrack = 0;
		
		// calculate lowest possible pitch 1VOct factor - calculation based on sample rate
		float lowest_pitch = std::log2((args.sampleRate / BufferLength) / dsp::FREQ_C4); 

		// split tuning knob in audio and delay range
		a_base_tune = params[PRM_TUNE].getValue() + (params[PRM_FINE_TUNE].getValue() * 0.01); // read pitch param + fine tune
		if (a_base_tune >= 0.f) {
			a_base_tune = rack::math::rescale(a_base_tune, 0.f, 1.f, -3.3f, 4.5f); // audible tuning range
		} else {
			a_base_tune = rack::math::rescale(a_base_tune, -1.f, 0.f, lowest_pitch, -3.3f); // delay tuning range based on lowest possible pitch
		}

		// add 1V Oct input
		a_base_tune += inputs[IN_TUNE_1VOCT].getVoltage();
		// calculate frequency based on 1vOCT
		a_base_freq = dsp::FREQ_C4 * std::pow(2.f, a_base_tune); 

		// normalize freq between 0-1 for keytracking logic
		a_base_freq_norm = (std::log2(a_base_freq) - std::log2(a_filter_freq_min)) / (std::log2(a_filter_freq_max) - std::log2(a_filter_freq_min)); 

		// read SPRD parameter for PITCH
		a_spread_tune = params[PRM_TUNE_SPRD].getValue(); 
		// apply attenuated CV
		a_spread_tune += rack::math::rescale(inputs[IN_TUNE_SPRD].getVoltage(), -5.f, 5.f, -1.f, 1.f) * params[ATT_TUNE_SPRD].getValue(); 
		
		// read SPRD parameter for fitler FREQ
		a_spread_filter = params[PRM_FLTR_SPRD].getValue();
		// apply attenuated CV
		a_spread_filter += rack::math::rescale(inputs[IN_FLTR_SPRD].getVoltage(), -5.f, 5.f, -1.f, 1.f) * params[ATT_FLTR_SPRD].getValue(); 

		// exponential scaling for precise control over small values, rescaling
		a_spread_tune = std::copysign(pow(a_spread_tune, 2.f), a_spread_tune); ; 
		a_spread_filter = std::copysign(0.25f * pow(a_spread_filter, 2.f),a_spread_filter); 



		// delay line processing for the active channels
		for (int ch = 0; ch < activeChannels; ch++) {

			// transfer the base tuning to the processing loop
			a_tune[ch] = a_base_tune; 

			// apply spread to the base tuning for each voice
			if (activeChannels % 2 == 1 and ch == 0){
				// keep channel 0 at base_freq for odd voice counts	
				a_tune[ch] = a_tune[ch];

			// distrubute the alternating pitches to the other channels		
			} else {
				if (ch % 2 == 0) {
					a_tune[ch] += a_spread_tune * (a_harmonic_spread[ch] * ((a_tune_sprd_mode == 1) ? a_inharm_factor[ch] : 1.0f));
				} else {
					a_tune[ch] += a_spread_tune * (a_harmonic_spread[uni_chans - ch] * ((a_tune_sprd_mode == 1) ? a_inharm_factor[uni_chans - ch] : 1.0f));	
				} 
			}

			// apply attenuated pitch modulation input to each channel
			a_tune[ch] += inputs[IN_TUNE_FM].getVoltage() * params[ATT_TUNE_FM].getValue(); 
			a_tune[ch] = clamp(a_tune[ch], lowest_pitch, 8.f);

			// calculate frequency in hz
			a_freq[ch] = dsp::FREQ_C4 * std::pow(2.f, a_tune[ch]);  

			// calculate index in samples
			a_index[ch] = args.sampleRate / a_freq[ch]; 

			
			// read decay param and apply attenuated cv input
			a_decay[ch] = params[PRM_DEC].getValue() + rack::math::rescale(inputs[IN_DEC].getVoltage(), -5.f, 5.f, -1.f, 1.f) * params[ATT_DEC].getValue(); 
			// scale decay exponentially for precise control over smaller values
			a_decay[ch] = 0.f + (60.f - 0.f) * pow(a_decay[ch], 4.f); 
			// calculate a decay fator for each channel based on the pitch
			a_t60[ch] = args.sampleRate * abs(a_decay[ch]) / a_index[ch];
			a_decay[ch] = pow(10, -3 / a_t60[ch]) * a_phase;
			// clamp decay factor to stay < 1 
			a_decay[ch] = clamp (a_decay[ch], -0.99999f, 0.99999f); 

			
			// add last delay sample to form a feedback loop and apply feedback factor
			delay_in[ch] = audio_in + last_delay_out[ch] * a_decay[ch];

			// write input to buffer
			buffers[ch][a_write[ch]] = delay_in[ch];

			// read position for the buffer
			a_read[ch] = a_write[ch] - a_index[ch] - params[ATT_TUNE_TRCK].getValue(); 

			// correct the buffer index if the buffer is run to its end - circular buffer
			if (a_read[ch]<0) a_read[ch] += BufferLength; 


			// get the interpolated sample value depending on the selected interpolation type
			switch (InterpolationSelect) {
				case 1:
					delay_out[ch] = getInterpolatedSample(a_read[ch], buffers[ch], BufferLength, LINEAR);
				break;
				case 2:
					delay_out[ch] = getInterpolatedSample(a_read[ch], buffers[ch], BufferLength, LAGRANGE);
				break;
				case 3:
					delay_out[ch] = getInterpolatedSample(a_read[ch], buffers[ch], BufferLength, CUBIC_SPLINE);
				break;
				case 4:
					delay_out[ch] = getInterpolatedSample(a_read[ch], buffers[ch], BufferLength, QUADRATIC);
				break;
				case 5:
					delay_out[ch] = getInterpolatedSample(a_read[ch], buffers[ch], BufferLength, NO_INTERPOLATION);
				break;
			}
			
 

			// read filter parameter
			a_filter_freq[ch] = params[PRM_FLTR_FREQ].getValue(); 
			// apply attenuated modulation to filter frequency
			a_filter_freq[ch] += rack::math::rescale(inputs[IN_FLTR_FREQ].getVoltage(), -5.f, 5.f, -1.f, 1.f) * params[ATT_FLTR_FREQ].getValue(); 
			
			// apply keytracking based on the base pitch frequency

			if (a_keytrack == 1) {
				a_filter_norm[ch] = clamp(a_base_freq_norm + a_filter_freq[ch], -1.f, 1.f); // apply keytrack
				lights[LGHT_KEYTRACK].setBrightness(1.f);

			} else {
				a_filter_norm[ch] = clamp(a_filter_freq[ch], -1.f, 1.f);
				lights[LGHT_KEYTRACK].setBrightness(0.f);
			}



			// apply filter spread
			if (activeChannels % 2 == 1 and ch == 0){
				// keep channel 0 at base_freq for odd voice counts		
				a_filter_norm[ch] = a_filter_norm[ch];

			// alternate spread values between the rest of the channels	
			} else {
				if (ch % 2 == 0) {
					a_filter_norm[ch] += a_spread_filter * (a_harmonic_spread[ch] * ((a_fltr_sprd_mode == 1) ? a_inharm_factor[ch] : 1.0f));
				} else {
					a_filter_norm[ch] += a_spread_filter * (a_harmonic_spread[uni_chans - ch] * ((a_fltr_sprd_mode == 1) ? a_inharm_factor[uni_chans - ch] : 1.0f));	
				} 
			}

			// calculate filter frequency in its actual range
			a_filter_freq[ch] = std::pow(2.f, std::log2(a_filter_freq_min) + a_filter_norm[ch] * (std::log2(a_filter_freq_max) - std::log2(a_filter_freq_min)));
			
			// calculate filter coeffizient
			a_filter_freq[ch] = a_filter_freq[ch] / args.sampleRate;  

			// clamp filter coeffizient to avoid errors
			a_filter_freq[ch] = clamp(a_filter_freq[ch], 0.001f, 0.499f);

			// read resonance param
			a_filter_res = params[PRM_FLTR_RES].getValue(); 

			// apply and rescale attenuated cv input
			a_filter_res += rack::math::rescale(inputs[IN_FLTR_RES].getVoltage(), -5.f, 5.f, -1.f, 1.f) * params[ATT_FLTR_RES].getValue(); 

			// clamp resonance to avoid errors
			a_filter_res = clamp(a_filter_res, 0.001f, 2.f); 

			// switch between selected filter types and apply filter to signal. switch the LED state for the GUI
			switch(a_type) {
 				case 1:
    				lights[LGHT_LP].setBrightness(1.f);
					lights[LGHT_BP].setBrightness(0.f);
					lights[LGHT_HP].setBrightness(0.f);
					lights[LGHT_NO].setBrightness(0.f);
					filters[ch].setParameters(filters[ch].LOWPASS, a_filter_freq[ch], a_filter_res, 1.0f);
					delay_out[ch] = filters[ch].process(delay_out[ch]);
    			break;
    			case 2:
    				lights[LGHT_LP].setBrightness(0.f);
					lights[LGHT_BP].setBrightness(1.f);
					lights[LGHT_HP].setBrightness(0.f);
					lights[LGHT_NO].setBrightness(0.f);
					filters[ch].setParameters(filters[ch].BANDPASS, a_filter_freq[ch], a_filter_res, 1.0f);
					delay_out[ch] = filters[ch].process(delay_out[ch]);
    			break; 
    			case 3:
    				lights[LGHT_LP].setBrightness(0.f);
					lights[LGHT_BP].setBrightness(0.f);
					lights[LGHT_HP].setBrightness(1.f);
					lights[LGHT_NO].setBrightness(0.f);
					filters[ch].setParameters(filters[ch].HIGHPASS, a_filter_freq[ch], a_filter_res, 1.0f);
					delay_out[ch] = filters[ch].process(delay_out[ch]);
    			break; 
    			case 4:
    				lights[LGHT_LP].setBrightness(0.f);
					lights[LGHT_BP].setBrightness(0.f);
					lights[LGHT_HP].setBrightness(0.f);
					lights[LGHT_NO].setBrightness(1.f);
					filters[ch].setParameters(filters[ch].NOTCH, a_filter_freq[ch], a_filter_res, 1.0f);
					delay_out[ch] = filters[ch].process(delay_out[ch]);
    			break;
    			case 5:
    				lights[LGHT_LP].setBrightness(0.f);
					lights[LGHT_BP].setBrightness(0.f);
					lights[LGHT_HP].setBrightness(0.f);
					lights[LGHT_NO].setBrightness(0.f);
    			break;    			
    		}



			// filter out low frequencys 3 octaves below the channels pitch frequency to prevent low frequency build ups at higher pitches
    		a_dc_block_freq[ch] = clamp((a_freq[ch] / 3), 10.f, args.sampleRate / 2.f);
    		dc_block[ch].setParameters(dc_block[ch].HIGHPASS, a_dc_block_freq[ch] / args.sampleRate, 0.701, 1.0f);
			delay_out[ch] = dc_block[ch].process(delay_out[ch]);

			// add saturation to limit the signal between -1 and +1
			delay_out[ch] = clamp(saturate(delay_out[ch] * a_feedback),-2.f, 2.f); 

			// write the current sample to a new variable to form a feedback loop
			last_delay_out[ch] = delay_out[ch];

			

			// alternate the voices between the two output channels to form a stereo signal
			if (outputs[OUT_AUDIO_LEFT].isConnected() && outputs[OUT_AUDIO_RIGHT].isConnected()) {
				// if the channel count is odd center channel 0 to get a even stereo loudness
				if (activeChannels % 2 == 1 and ch == 0){
					audio_out_left = delay_out[ch] / activeChannels;
					audio_out_right = delay_out[ch] / activeChannels;				

				// alternate the rest of the channels between the two outputs to form a stereo signal
				} else {
					if (ch % 2 == 0) {
						audio_out_left += delay_out[ch] / (activeChannels / 2);
					} else {
						audio_out_right += delay_out[ch] / (activeChannels / 2);	
					} 
			    }

			// if only one output is connected make a mono signal on both channels
			} else {
				audio_out_left += delay_out[ch] / activeChannels;
				audio_out_right += delay_out[ch] / activeChannels;	
			}

			

			// last but not least increment the buffers for the next loop
			a_write[ch] += 1;

			// reset the write position if it hits the end of the buffer
			if (a_write[ch]>=BufferLength) a_write[ch] -= BufferLength; 
		}


		
		// If VCA Modulation input is connected, modulate the gain with unipolar CV signal
		if (inputs[IN_VCA].isConnected() == 1) {
			a_vca_mod = rack::math::rescale(inputs[IN_VCA].getVoltage(), -5.f, 5.f, -1.f, 1.f);
			// clamp to positive range only to avoid inversions of the signal
			a_vca_mod = clamp(a_vca_mod, 0.f, 1.f); 
		// if VCA mod input isnt connected nothing changes
		} else {
			a_vca_mod = 1.f;
		}


		// apply VCA modulation and parameter to the signal
		audio_out_left = audio_out_left * params[PRM_VCA].getValue() * a_vca_mod;
		audio_out_right = audio_out_right * params[PRM_VCA].getValue() * a_vca_mod;
		
		// read and scale the DRY/WET parameter and apply attenuated CV
		a_mix = params[PRM_DRYWET].getValue();
		a_mix += rack::math::rescale(inputs[IN_DRYWET].getVoltage(), -5.f, 5.f, -1.f, 1.f) * params[ATT_DRYWET].getValue();
		a_mix = clamp(a_mix, 0.f, 1.f);

		// mix the dry and wet signal
		audio_out_left = crossfade(audio_in, audio_out_left, a_mix);
		audio_out_right = crossfade(audio_in, audio_out_right, a_mix);


		// rescale outputs to match -5V to + 5V
		audio_out_left = rack::math::rescale(audio_out_left, -1.f, 1.f, -5.f, 5.f);
		audio_out_right = rack::math::rescale(audio_out_right, -1.f, 1.f, -5.f, 5.f);


		// set audio outputs
		outputs[OUT_AUDIO_LEFT].setVoltage(audio_out_left);
		outputs[OUT_AUDIO_RIGHT].setVoltage(audio_out_right);

	}


	// saturation functions

	float softClip(float x) {
    	if (x > 1.0f) return 1.0f;
    	if (x < -1.0f) return -1.0f;
    	return x - (x * x * x) / 3.0f;
	}

	float hardClip(float x) {
    	return clamp(x, -1.f, 1.f);
	}

	float expSaturation(float x) {
    	return (1.0f - std::exp(-std::abs(x))) * (x > 0 ? 1.0f : -1.0f);
	}

	float sigmoidSaturation(float x, float sharpness = 5.0f) {
    	return x / (1.0f + sharpness * std::abs(x));
	}	

	float atanSaturation(float x) {
    	return std::atan(x);
	}

	float cubicSaturation(float x) {
    	return clamp(x - (x * x * x) / 3.0f, -5.f, 5.f);
	}

	float asymmetricSaturation(float x) {
 	   if (x > 0.0f) {
   	     	return std::tanh(x); // Positive side: tanh
   		} else {
   	     	return x / (1.0f + std::abs(x)); // Negative side: smoother saturation
   	 	}
	}

	float softExpSaturation(float x) {
    	return x / (1.0f + std::exp(-x));
	}

	float waveshaperSaturation(float x) {
    	// Custom polynomial curve
    	return x - 0.5f * x * x + 0.1f * x * x * x;
	}

	float sineSaturation(float x) {
    	return std::sin(x);
	}


	// saturation selection function
	float saturate(float x) {
    	switch (a_sat_select) {
        	case  1: return tanh(x);
       	 	case  2: return softClip(x);
       		case  3: return hardClip(x);
    	    case  4: return expSaturation(x);
    	    case  5: return sigmoidSaturation(x);
    	    case  6: return atanSaturation(x);
    	    case  7: return cubicSaturation(x);
    	    case  8: return asymmetricSaturation(x);
    	    case  9: return softExpSaturation(x);
    	    case 10: return waveshaperSaturation(x);
    	    case 11: return sineSaturation(x);
        	default: return x;
    	}
	}


	// INTERPOLATION FUNCTIONS

	// Linear Interpolation
	float linearInterpolation(float x, float y0, float y1) {
	    return y0 + x * (y1 - y0);
	}

	// Lagrange Interpolation (3rd degree, 4 points)
	float lagrangeInterpolation(float x, float* samples) {
	    float y0 = samples[0];
	    float y1 = samples[1];
	    float y2 = samples[2];
	    float y3 = samples[3];

	    return y0 * ((x - 1) * (x - 2) * (x - 3)) / (-6.0f)
	         + y1 * ((x) * (x - 2) * (x - 3)) / (2.0f)
	         + y2 * ((x) * (x - 1) * (x - 3)) / (-2.0f)
	         + y3 * ((x) * (x - 1) * (x - 2)) / (6.0f);
	}

	// Cubic Spline Interpolation (using 4 points)
	float cubicSplineInterpolation(float x, float* samples) {
	    float y0 = samples[0];
	    float y1 = samples[1];
	    float y2 = samples[2];
	    float y3 = samples[3];

	    float a = -0.5f * y0 + 1.5f * y1 - 1.5f * y2 + 0.5f * y3;
	    float b = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
	    float c = -0.5f * y0 + 0.5f * y2;
	    float d = y1;

	    return a * x * x * x + b * x * x + c * x + d;
	}

	// Quadratic Interpolation
	float quadraticInterpolation(float x, float* samples) {
	    float y0 = samples[0];
	    float y1 = samples[1];
	    float y2 = samples[2];

	    float a = 0.5f * (y0 - 2 * y1 + y2);
	    float b = 0.5f * (y2 - y0);
	    float c = y1;

	    return a * x * x + b * x + c;
	}

	// Function to get interpolated sample based on the chosen method
	enum InterpolationType { LINEAR, LAGRANGE, CUBIC_SPLINE, QUADRATIC, NO_INTERPOLATION };

	float getInterpolatedSample(float fractionalIndex, float* buffer, int bufferSize, InterpolationType type) {
	    int i1 = (int)fractionalIndex;
	    float x = fractionalIndex - i1; // Fractional part

	    // Wrap indices for circular buffer
	    int i0 = (i1 - 1 + bufferSize) % bufferSize;
	    int i2 = (i1 + 1) % bufferSize;
	    int i3 = (i1 + 2) % bufferSize;

	    float samples[4] = { buffer[i0], buffer[i1], buffer[i2], buffer[i3] };

	    switch (type) {
	        case LINEAR:
	            return linearInterpolation(x, samples[1], samples[2]);
	        case LAGRANGE:
	            return lagrangeInterpolation(x, samples);
	        case CUBIC_SPLINE:
	            return cubicSplineInterpolation(x, samples);
	        case QUADRATIC:
	            return quadraticInterpolation(x, samples)
;	        case NO_INTERPOLATION:
	            return samples[1];
	        default:
	            return samples[1]; // Default to nearest neighbor
	    }
	}






   // Override `dataToJson` to save values without a param to JSON
    json_t *dataToJson() override {
        // Create a JSON object
        json_t *rootJ = json_object();
        json_object_set_new(rootJ, "a_type", json_integer(a_type));
        json_object_set_new(rootJ, "a_keytrack", json_integer(a_keytrack));
        json_object_set_new(rootJ, "a_sat_select", json_integer(a_sat_select));
        json_object_set_new(rootJ, "InterpolationSelect", json_integer(InterpolationSelect));
        json_object_set_new(rootJ, "a_tune_sprd_mode", json_integer(a_tune_sprd_mode));
        json_object_set_new(rootJ, "a_fltr_sprd_mode", json_integer(a_fltr_sprd_mode));

        return rootJ;
    }

    // Override `dataFromJson` to load the values from JSON
    void dataFromJson(json_t *rootJ) override {
        json_t *intJ1 = json_object_get(rootJ, "a_type");
        if (intJ1) a_type = json_integer_value(intJ1);

        json_t *intJ2 = json_object_get(rootJ, "a_keytrack");
        if (intJ2) a_keytrack = json_integer_value(intJ2);

        json_t *intJ3 = json_object_get(rootJ, "a_sat_select");
        if (intJ3) a_sat_select = json_integer_value(intJ3);

        json_t *intJ4 = json_object_get(rootJ, "InterpolationSelect");
        if (intJ4) InterpolationSelect = json_integer_value(intJ4);

        json_t *intJ5 = json_object_get(rootJ, "a_tune_sprd_mode");
        if (intJ5) a_tune_sprd_mode = json_integer_value(intJ5);

        json_t *intJ6 = json_object_get(rootJ, "a_fltr_sprd_mode");
        if (intJ6) a_fltr_sprd_mode = json_integer_value(intJ6);
    }
};


// module widget constructor
struct AlaeWidget : ModuleWidget {
	AlaeWidget(Alae* module) {
		setModule(module);
		// load panel svg
		setPanel(createPanel(asset::plugin(pluginInstance, "res/alae_v2_rev3.svg")));

		// place screws
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// place components
		addParam(createParamCentered<BigLuggezKnob>(mm2px(Vec(36.852, 20.109)), module, Alae::PRM_TUNE));
		addParam(createParamCentered<BigLuggezKnob>(mm2px(Vec(64.852, 20.109)), module, Alae::PRM_FLTR_FREQ));

		addParam(createParamCentered<MediumLuggezKnob>(mm2px(Vec(36.852, 58.814)), module, Alae::PRM_TUNE_SPRD));
		addParam(createParamCentered<MediumLuggezKnob>(mm2px(Vec(50.852, 48.069)), module, Alae::PRM_VOX_COUNT));
		addParam(createParamCentered<MediumLuggezKnob>(mm2px(Vec(64.852, 58.814)), module, Alae::PRM_FLTR_SPRD));
		addParam(createParamCentered<SmallLuggezKnob> (mm2px(Vec(64.852, 39.924)), module, Alae::PRM_FLTR_RES));
		addParam(createParamCentered<MediumLuggezKnob>(mm2px(Vec(16.852, 77.028)), module, Alae::PRM_DEC));
		addParam(createParamCentered<MediumLuggezKnob>(mm2px(Vec(84.852, 77.620)), module, Alae::PRM_FB));
		addParam(createParamCentered<MediumLuggezKnob>(mm2px(Vec(64.852, 87.220)), module, Alae::PRM_VCA));
		addParam(createParamCentered<MediumLuggezKnob>(mm2px(Vec(36.852, 87.220)), module, Alae::PRM_DRYWET));

		addParam(createParamCentered<SmallLuggezKnob>(mm2px(Vec(21.852,  12.942)), module, Alae::ATT_TUNE_TRCK));
		addParam(createParamCentered<SmallLuggezKnob>(mm2px(Vec(21.852,  36.424)), module, Alae::ATT_TUNE_FM));
		addParam(createParamCentered<SmallLuggezKnob>(mm2px(Vec(21.852,  58.424)), module, Alae::ATT_TUNE_SPRD)); 
		addParam(createParamCentered<SmallLuggezKnob>(mm2px(Vec(79.852,  12.924)), module, Alae::ATT_FLTR_FREQ));
		addParam(createParamCentered<SmallLuggezKnob>(mm2px(Vec(79.852,  58.424)), module, Alae::ATT_FLTR_SPRD));
		addParam(createParamCentered<SmallLuggezKnob>(mm2px(Vec(79.852,  36.424)), module, Alae::ATT_FLTR_RES));
		addParam(createParamCentered<SmallLuggezKnob>(mm2px(Vec(16.852,  90.636)), module, Alae::ATT_DEC));
		addParam(createParamCentered<SmallLuggezKnob>(mm2px(Vec(84.852,  91.228)), module, Alae::ATT_FB));
		addParam(createParamCentered<SmallLuggezKnob>(mm2px(Vec(50.852,  95.825)), module, Alae::ATT_DRYWET));
		addParam(createParamCentered<SmallLuggezKnob>(mm2px(Vec(50.852, 115.078)), module, Alae::ATT_FB_IN));
		addParam(createParamCentered<SmallLuggezKnob>(mm2px(Vec(36.852,  39.924)), module, Alae::PRM_FINE_TUNE));


		// place jacks
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec( 8.738,  13.217)), module, Alae::IN_TUNE_1VOCT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec( 8.738,  36.717)), module, Alae::IN_TUNE_FM));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec( 8.738,  58.217)), module, Alae::IN_TUNE_SPRD));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(92.738,  13.217)), module, Alae::IN_FLTR_FREQ));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(92.738,  36.717)), module, Alae::IN_FLTR_RES));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(92.738,  58.127)), module, Alae::IN_FLTR_SPRD));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(16.738, 104.670)), module, Alae::IN_DEC));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(84.738, 104.670)), module, Alae::IN_FB));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.926, 116.117)), module, Alae::IN_AUDIO));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(64.738, 104.670)), module, Alae::IN_VCA));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(36.738, 104.670)), module, Alae::IN_DRYWET));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.550, 116.117)), module, Alae::IN_AUDIO_FB));
	
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(78.926, 116.117)), module, Alae::OUT_AUDIO_LEFT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(90.550, 116.117)), module, Alae::OUT_AUDIO_RIGHT));



		// place buttons
		addParam(createLightParamCentered<VCVLightButton<MediumSimpleLight<YellowLight>>>(mm2px(Vec(50.852, 68.999)), module, Alae::BTN_TYPE, Alae::LGHT_TYPE));
		addParam(createLightParamCentered<VCVLightButton<MediumSimpleLight<YellowLight>>>(mm2px(Vec(50.852, 19.986)), module, Alae::BTN_KEYTRACK, Alae::LGHT_KEYTRACK));
		
		// place lights
		addChild(createLightCentered<SmallLight<YellowLight>>(mm2px(Vec(44.316, 74.655)), module, Alae::LGHT_LP));
		addChild(createLightCentered<SmallLight<YellowLight>>(mm2px(Vec(48.673, 74.655)), module, Alae::LGHT_BP));
		addChild(createLightCentered<SmallLight<YellowLight>>(mm2px(Vec(53.031, 74.655)), module, Alae::LGHT_HP));
		addChild(createLightCentered<SmallLight<YellowLight>>(mm2px(Vec(57.389, 74.655)), module, Alae::LGHT_NO));
	}


	// create custom menu items for the right click menu
	MenuItem* createMenuItem(const std::string& label, const std::string& rightText, std::function<void()> action) {
	    struct ActionMenuItem : MenuItem {
	        std::function<void()> action;
	        void onAction(const event::Action& e) override {
	            if (action)
	                action();
	        }
	    };

	    ActionMenuItem* item = new ActionMenuItem();
	    item->text = label;
	    item->rightText = rightText;
	    item->action = action;
	    return item;
	}

	// create the right click menus
	void appendContextMenu(Menu* menu) override {
	    ModuleWidget::appendContextMenu(menu);

	    // Cast the module to access its variable
	    Alae* module = dynamic_cast<Alae*>(this->module);
	    if (!module) return;

	    // Add a separator
	    menu->addChild(new MenuSeparator);

    	menu->addChild(createSubmenuItem("Saturation Type", "", [module, this](Menu* submenu) {
		    // Add menu items with checkmarks
		    submenu->addChild(this->createMenuItem("tanh", module->a_sat_select == 1 ? "✔" : "", 				[module]() { module->a_sat_select = 1; }));
		    submenu->addChild(this->createMenuItem("soft clipping", module->a_sat_select == 2 ? "✔" : "", 		[module]() { module->a_sat_select = 2; }));
		    submenu->addChild(this->createMenuItem("hard clipping", module->a_sat_select == 3 ? "✔" : "", 		[module]() { module->a_sat_select = 3; }));
		    submenu->addChild(this->createMenuItem("exponential", module->a_sat_select == 4 ? "✔" : "", 		[module]() { module->a_sat_select = 4; }));
		    submenu->addChild(this->createMenuItem("sigmoid", module->a_sat_select == 5 ? "✔" : "", 			[module]() { module->a_sat_select = 5; }));
		    submenu->addChild(this->createMenuItem("atan", module->a_sat_select == 6 ? "✔" : "", 				[module]() { module->a_sat_select = 6; }));
		    submenu->addChild(this->createMenuItem("cubic", module->a_sat_select == 7 ? "✔" : "", 				[module]() { module->a_sat_select = 7; }));
		    submenu->addChild(this->createMenuItem("asymetric", module->a_sat_select == 8 ? "✔" : "", 			[module]() { module->a_sat_select = 8; }));
		    submenu->addChild(this->createMenuItem("soft exponential", module->a_sat_select == 9 ? "✔" : "",	[module]() { module->a_sat_select = 9; }));
		    submenu->addChild(this->createMenuItem("waveshaper", module->a_sat_select == 10 ? "✔" : "", 		[module]() { module->a_sat_select = 10; }));
		    submenu->addChild(this->createMenuItem("sine", module->a_sat_select == 11 ? "✔" : "",		 		[module]() { module->a_sat_select = 11; }));
		}));

		// Add a separator
	    menu->addChild(new MenuSeparator);

    	menu->addChild(createSubmenuItem("InterpolationType", "", [module, this](Menu* submenu) {
		    // Add menu items with checkmarks
		    submenu->addChild(this->createMenuItem("Linear", 			module->InterpolationSelect == 1 ? "✔" : "", 			[module]() { module->InterpolationSelect = 1; }));
		    submenu->addChild(this->createMenuItem("Lagrange", 			module->InterpolationSelect == 2 ? "✔" : "", 			[module]() { module->InterpolationSelect = 2; }));
		    submenu->addChild(this->createMenuItem("Cubic Spline", 		module->InterpolationSelect == 3 ? "✔" : "", 			[module]() { module->InterpolationSelect = 3; }));
		    submenu->addChild(this->createMenuItem("Quadratic", 		module->InterpolationSelect == 4 ? "✔" : "", 			[module]() { module->InterpolationSelect = 4; }));
		    submenu->addChild(this->createMenuItem("No Interpolation", 	module->InterpolationSelect == 5 ? "✔" : "", 			[module]() { module->InterpolationSelect = 5; }));
		}));

    	// Add a separator
	    menu->addChild(new MenuSeparator);

    	menu->addChild(createSubmenuItem("Spread type - Tuning", "", [module, this](Menu* submenu) {
		    // Add menu items with checkmarks
		    submenu->addChild(this->createMenuItem("Even", 		module->a_tune_sprd_mode == 0 ? "✔" : "", 	[module]() { module->a_tune_sprd_mode = 0; }));
		    submenu->addChild(this->createMenuItem("Random", 	module->a_tune_sprd_mode == 1 ? "✔" : "", 	[module]() { module->a_tune_sprd_mode = 1; }));
		}));

		menu->addChild(new MenuSeparator);

    	menu->addChild(createSubmenuItem("Spread type - Filter", "", [module, this](Menu* submenu) {
		    // Add menu items with checkmarks
		    submenu->addChild(this->createMenuItem("Even", 		module->a_fltr_sprd_mode == 0 ? "✔" : "", 	[module]() { module->a_fltr_sprd_mode = 0; }));
		    submenu->addChild(this->createMenuItem("Random", 	module->a_fltr_sprd_mode == 1 ? "✔" : "", 	[module]() { module->a_fltr_sprd_mode = 1; }));
		}));
	}
};



Model* modelAlae = createModel<Alae, AlaeWidget>("Alae");