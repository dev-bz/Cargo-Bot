/*
 *  sfxr.cpp
 *  sfxr
 *
 *  Original code by Tomas Pettersson 2007.
 *
 *  Modifications are copyright Christopher Gassib 2009.
 *  This file is released under the MIT license as described in readme.txt
 *
 */
#include "sfxr.h"
#include <algorithm>
#include <math.h>
typedef short sint16;
//#define SFXR_LOG(x) printf( "get %-50s %10f\n" , #x, (float)(x) )
#define SFXR_LOG(x) \
	while (false) {   \
	}
#define PARAM_SCALE 2.0f
#define PARAM_OFFSET -1.0f
#define PARAM_SCALE_GET 1.0f / (PARAM_SCALE)
#define PARAM_OFFSET_GET -1.0f * (PARAM_OFFSET)
static int clampToChar(int a) { return std::max(-127, std::min(a, 127)); }
const unsigned int sfxr::fileVersion;
const unsigned int sfxr::fileVersionFull;
const float PI = 3.14159265f;
// Galois Linear feedback shift register function.
// Straight from wikipedia.
inline unsigned int lfsr() {
	static unsigned int seed = 1;
	return seed = (seed >> 1u) ^ (0u - (seed & 1u) & 0xd0000001u);
}
// Floating point noise function using the LFSR function above.
//  Return values are in the range of -1.0 to 1.0.
inline float flfsr() {
	const unsigned int max = std::numeric_limits<unsigned int>::max();
	return (static_cast<double>(lfsr()) / max * 2.0) - 1.0f;
}
inline int rnd(int n) { return rand() % (n + 1); }
inline float frnd(const float range) { return static_cast<double>(rand()) / static_cast<double>(RAND_MAX) * range; }
sfxr::sfxr() : playing_sample(false), master_vol(0.05f), sound_vol(0.5f), filesample(0.0f), fileacc(0), wav_freq(44100), wav_bits(16), mute_stream(false), phase(0), fperiod(0.0), fmaxperiod(0.0), fslide(0.0), fdslide(0.0), period(0), square_duty(0.0f), square_slide(0.0f), env_stage(0), env_time(0), env_vol(0.0f), fphase(0.0f), fdphase(0.0f), iphase(0), ipp(0), fltp(0.0f), fltdp(0.0f), fltw(0.0f), fltw_d(0.0f), fltdmp(0.0f), fltphp(0.0f), flthp(0.0f), flthp_d(0.0f), vib_phase(0.0f), vib_speed(0.0f), vib_amp(0.0f), rep_time(0), rep_limit(0), arp_time(0), arp_limit(0), arp_mod(0.0), file_sampleswritten(0) {
	srand(time(NULL));
	ResetParams();
	memset(env_length, 0, sizeof(env_length));
	memset(phaser_buffer, 0, sizeof(phaser_buffer));
	memset(noise_buffer, 0, sizeof(noise_buffer));
}
sfxr::sfxr(const sfxr &original) : playing_sample(original.playing_sample), master_vol(original.master_vol), sound_vol(original.sound_vol), filesample(original.filesample), fileacc(original.fileacc), wav_freq(original.wav_freq), wav_bits(original.wav_bits), mute_stream(original.mute_stream), phase(original.phase), fperiod(original.fperiod), fmaxperiod(original.fmaxperiod), fslide(original.fslide), fdslide(original.fdslide), period(original.period), square_duty(original.square_duty), square_slide(original.square_slide), env_stage(original.env_stage), env_time(original.env_time), env_vol(original.env_vol), fphase(original.fphase), fdphase(original.fdphase), iphase(original.iphase), ipp(original.ipp), fltp(original.fltp), fltdp(original.fltdp), fltw(original.fltw), fltw_d(original.fltw_d), fltdmp(original.fltdmp), fltphp(original.fltphp), flthp(original.flthp), flthp_d(original.flthp_d), vib_phase(original.vib_phase), vib_speed(original.vib_speed), vib_amp(original.vib_amp), rep_time(original.rep_time), rep_limit(original.rep_limit), arp_time(original.arp_time), arp_limit(original.arp_limit), arp_mod(original.arp_mod), file_sampleswritten(original.file_sampleswritten), wave_type(original.wave_type), p_base_freq(original.p_base_freq), p_freq_limit(original.p_freq_limit), p_freq_ramp(original.p_freq_ramp), p_freq_dramp(original.p_freq_dramp), p_duty(original.p_duty), p_duty_ramp(original.p_duty_ramp), p_vib_strength(original.p_vib_strength), p_vib_speed(original.p_vib_speed), p_vib_delay(original.p_vib_delay), p_env_attack(original.p_env_attack), p_env_sustain(original.p_env_sustain), p_env_decay(original.p_env_decay), p_env_punch(original.p_env_punch), filter_on(original.filter_on), p_lpf_resonance(original.p_lpf_resonance), p_lpf_freq(original.p_lpf_freq), p_lpf_ramp(original.p_lpf_ramp), p_hpf_freq(original.p_hpf_freq), p_hpf_ramp(original.p_hpf_ramp), p_pha_offset(original.p_pha_offset), p_pha_ramp(original.p_pha_ramp), p_repeat_speed(original.p_repeat_speed), p_arp_speed(original.p_arp_speed), p_arp_mod(original.p_arp_mod) {
	memcpy(env_length, original.env_length, sizeof(env_length));
	memcpy(phaser_buffer, original.phaser_buffer, sizeof(phaser_buffer));
	memcpy(noise_buffer, original.phaser_buffer, sizeof(noise_buffer));
}
sfxr::~sfxr() {}
sfxr &sfxr::operator=(const sfxr &rhs) {
	if (&rhs == this) {
		return *this;
	}
	playing_sample = rhs.playing_sample;
	master_vol = rhs.master_vol;
	sound_vol = rhs.sound_vol;
	filesample = rhs.filesample;
	fileacc = rhs.fileacc;
	wav_freq = rhs.wav_freq;
	wav_bits = rhs.wav_bits;
	mute_stream = rhs.mute_stream;
	phase = rhs.phase;
	fperiod = rhs.fperiod;
	fmaxperiod = rhs.fmaxperiod;
	fslide = rhs.fslide;
	fdslide = rhs.fdslide;
	period = rhs.period;
	square_duty = rhs.square_duty;
	square_slide = rhs.square_slide;
	env_stage = rhs.env_stage;
	env_time = rhs.env_time;
	env_vol = rhs.env_vol;
	fphase = rhs.fphase;
	fdphase = rhs.fdphase;
	iphase = rhs.iphase;
	ipp = rhs.ipp;
	fltp = rhs.fltp;
	fltdp = rhs.fltdp;
	fltw = rhs.fltw;
	fltw_d = rhs.fltw_d;
	fltdmp = rhs.fltdmp;
	fltphp = rhs.fltphp;
	flthp = rhs.flthp;
	flthp_d = rhs.flthp_d;
	vib_phase = rhs.vib_phase;
	vib_speed = rhs.vib_speed;
	vib_amp = rhs.vib_amp;
	rep_time = rhs.rep_time;
	rep_limit = rhs.rep_limit;
	arp_time = rhs.arp_time;
	arp_limit = rhs.arp_limit;
	arp_mod = rhs.arp_mod;
	file_sampleswritten = rhs.file_sampleswritten;
	wave_type = rhs.wave_type;
	p_base_freq = rhs.p_base_freq;
	p_freq_limit = rhs.p_freq_limit;
	p_freq_ramp = rhs.p_freq_ramp;
	p_freq_dramp = rhs.p_freq_dramp;
	p_duty = rhs.p_duty;
	p_duty_ramp = rhs.p_duty_ramp;
	p_vib_strength = rhs.p_vib_strength;
	p_vib_speed = rhs.p_vib_speed;
	p_vib_delay = rhs.p_vib_delay;
	p_env_attack = rhs.p_env_attack;
	p_env_sustain = rhs.p_env_sustain;
	p_env_decay = rhs.p_env_decay;
	p_env_punch = rhs.p_env_punch;
	filter_on = rhs.filter_on;
	p_lpf_resonance = rhs.p_lpf_resonance;
	p_lpf_freq = rhs.p_lpf_freq;
	p_lpf_ramp = rhs.p_lpf_ramp;
	p_hpf_freq = rhs.p_hpf_freq;
	p_hpf_ramp = rhs.p_hpf_ramp;
	p_pha_offset = rhs.p_pha_offset;
	p_pha_ramp = rhs.p_pha_ramp;
	p_repeat_speed = rhs.p_repeat_speed;
	p_arp_speed = rhs.p_arp_speed;
	p_arp_mod = rhs.p_arp_mod;
	memcpy(env_length, rhs.env_length, sizeof(env_length));
	memcpy(phaser_buffer, rhs.phaser_buffer, sizeof(phaser_buffer));
	memcpy(noise_buffer, rhs.phaser_buffer, sizeof(noise_buffer));
	return *this;
}
bool sfxr::IsPlaying() { return playing_sample && !mute_stream; }
// This is a replacement for the original sfxr callback function.
int sfxr::get(unsigned char *sampleBuffer, int byteCount) {
	if (playing_sample && !mute_stream) {
		unsigned int l = byteCount / sizeof(sint16);
		float fbuf[l];
		memset(fbuf, 0, sizeof(fbuf));
		int samplesWritten = SynthSample(l, fbuf, NULL);
		int samplesLeft = samplesWritten;
		while (samplesLeft--) {
			float f = fbuf[samplesLeft];
			if (f < -1.0)
				f = -1.0;
			if (f > 1.0)
				f = 1.0;
			((sint16 *)sampleBuffer)[samplesLeft] = (sint16)(f * 32767);
		}
		return samplesWritten * sizeof(sint16);
	}
	memset(sampleBuffer, 0, byteCount);
	return 0;
}
bool sfxr::LoadSettings(std::istream &stream) {
	int start = stream.tellg();
	unsigned char version = 0;
	stream.read(reinterpret_cast<char *>(&version), sizeof(version));
	if (version != fileVersionFull) {
		if (version == fileVersion) {
			stream.seekg(start);
			return LoadSettingsShort(stream);
		}
		return false;
	}
	stream.read(reinterpret_cast<char *>(&wave_type), sizeof(wave_type));
	stream.read(reinterpret_cast<char *>(&sound_vol), sizeof(sound_vol));
	stream.read(reinterpret_cast<char *>(&p_base_freq), sizeof(p_base_freq));
	stream.read(reinterpret_cast<char *>(&p_freq_limit), sizeof(p_freq_limit));
	stream.read(reinterpret_cast<char *>(&p_freq_ramp), sizeof(p_freq_ramp));
	stream.read(reinterpret_cast<char *>(&p_freq_dramp), sizeof(p_freq_dramp));
	stream.read(reinterpret_cast<char *>(&p_duty), sizeof(p_duty));
	stream.read(reinterpret_cast<char *>(&p_duty_ramp), sizeof(p_duty_ramp));
	stream.read(reinterpret_cast<char *>(&p_vib_strength), sizeof(p_vib_strength));
	stream.read(reinterpret_cast<char *>(&p_vib_speed), sizeof(p_vib_speed));
	stream.read(reinterpret_cast<char *>(&p_vib_delay), sizeof(p_vib_delay));
	stream.read(reinterpret_cast<char *>(&p_env_attack), sizeof(p_env_attack));
	stream.read(reinterpret_cast<char *>(&p_env_sustain), sizeof(p_env_sustain));
	stream.read(reinterpret_cast<char *>(&p_env_decay), sizeof(p_env_decay));
	stream.read(reinterpret_cast<char *>(&p_env_punch), sizeof(p_env_punch));
	stream.read(reinterpret_cast<char *>(&filter_on), sizeof(filter_on));
	stream.read(reinterpret_cast<char *>(&p_lpf_resonance), sizeof(p_lpf_resonance));
	stream.read(reinterpret_cast<char *>(&p_lpf_freq), sizeof(p_lpf_freq));
	stream.read(reinterpret_cast<char *>(&p_lpf_ramp), sizeof(p_lpf_ramp));
	stream.read(reinterpret_cast<char *>(&p_hpf_freq), sizeof(p_hpf_freq));
	stream.read(reinterpret_cast<char *>(&p_hpf_ramp), sizeof(p_hpf_ramp));
	stream.read(reinterpret_cast<char *>(&p_pha_offset), sizeof(p_pha_offset));
	stream.read(reinterpret_cast<char *>(&p_pha_ramp), sizeof(p_pha_ramp));
	stream.read(reinterpret_cast<char *>(&p_repeat_speed), sizeof(p_repeat_speed));
	stream.read(reinterpret_cast<char *>(&p_arp_speed), sizeof(p_arp_speed));
	stream.read(reinterpret_cast<char *>(&p_arp_mod), sizeof(p_arp_mod));
	return stream.good();
}
bool sfxr::SaveSettings(std::ostream &stream) const {
	unsigned char version = fileVersionFull;
	stream.write(reinterpret_cast<const char *>(&version), sizeof(version));
	stream.write(reinterpret_cast<const char *>(&wave_type), sizeof(wave_type));
	stream.write(reinterpret_cast<const char *>(&sound_vol), sizeof(sound_vol));
	stream.write(reinterpret_cast<const char *>(&p_base_freq), sizeof(p_base_freq));
	stream.write(reinterpret_cast<const char *>(&p_freq_limit), sizeof(p_freq_limit));
	stream.write(reinterpret_cast<const char *>(&p_freq_ramp), sizeof(p_freq_ramp));
	stream.write(reinterpret_cast<const char *>(&p_freq_dramp), sizeof(p_freq_dramp));
	stream.write(reinterpret_cast<const char *>(&p_duty), sizeof(p_duty));
	stream.write(reinterpret_cast<const char *>(&p_duty_ramp), sizeof(p_duty_ramp));
	stream.write(reinterpret_cast<const char *>(&p_vib_strength), sizeof(p_vib_strength));
	stream.write(reinterpret_cast<const char *>(&p_vib_speed), sizeof(p_vib_speed));
	stream.write(reinterpret_cast<const char *>(&p_vib_delay), sizeof(p_vib_delay));
	stream.write(reinterpret_cast<const char *>(&p_env_attack), sizeof(p_env_attack));
	stream.write(reinterpret_cast<const char *>(&p_env_sustain), sizeof(p_env_sustain));
	stream.write(reinterpret_cast<const char *>(&p_env_decay), sizeof(p_env_decay));
	stream.write(reinterpret_cast<const char *>(&p_env_punch), sizeof(p_env_punch));
	stream.write(reinterpret_cast<const char *>(&filter_on), sizeof(filter_on));
	stream.write(reinterpret_cast<const char *>(&p_lpf_resonance), sizeof(p_lpf_resonance));
	stream.write(reinterpret_cast<const char *>(&p_lpf_freq), sizeof(p_lpf_freq));
	stream.write(reinterpret_cast<const char *>(&p_lpf_ramp), sizeof(p_lpf_ramp));
	stream.write(reinterpret_cast<const char *>(&p_hpf_freq), sizeof(p_hpf_freq));
	stream.write(reinterpret_cast<const char *>(&p_hpf_ramp), sizeof(p_hpf_ramp));
	stream.write(reinterpret_cast<const char *>(&p_pha_offset), sizeof(p_pha_offset));
	stream.write(reinterpret_cast<const char *>(&p_pha_ramp), sizeof(p_pha_ramp));
	stream.write(reinterpret_cast<const char *>(&p_repeat_speed), sizeof(p_repeat_speed));
	stream.write(reinterpret_cast<const char *>(&p_arp_speed), sizeof(p_arp_speed));
	stream.write(reinterpret_cast<const char *>(&p_arp_mod), sizeof(p_arp_mod));
	return stream.good();
}
bool sfxr::LoadSettingsShort(std::istream &stream) {
	unsigned char version = 0;
	stream.read(reinterpret_cast<char *>(&version), sizeof(version));
	if (version != fileVersion) {
		return false;
	}
#define PROCESS_VAR(var)                                  \
	{                                                       \
		char c;                                               \
		stream.read(reinterpret_cast<char *>(&c), sizeof(c)); \
		var = ((typeof(var))(c)) / 128.f;                     \
	}
#define PROCESS_SET(func)                                 \
	{                                                       \
		char c;                                               \
		stream.read(reinterpret_cast<char *>(&c), sizeof(c)); \
		(func)(((float)(c)) / 128.f);                         \
	}
#define PROCESS_SET_NORMWITH(func, norm)                  \
	{                                                       \
		char c;                                               \
		stream.read(reinterpret_cast<char *>(&c), sizeof(c)); \
		(func)(((float)(c)) / (norm));                        \
	}
#define PROCESS_VAR_UNNORM(var)                           \
	{                                                       \
		char c;                                               \
		stream.read(reinterpret_cast<char *>(&c), sizeof(c)); \
		var = ((typeof(var))(c));                             \
	}
	PROCESS_VAR_UNNORM(wave_type)
	PROCESS_VAR(sound_vol)
	PROCESS_SET(SetStartFrequency)	 // PROCESS_VAR(p_base_freq)
	PROCESS_SET(SetMinimumFrequency) // PROCESS_VAR(p_freq_limit)
	PROCESS_SET(SetSlide)						 // PROCESS_VAR(p_freq_ramp)
	PROCESS_SET(SetDeltaSlide)			 // PROCESS_VAR(p_freq_dramp)
	PROCESS_SET(SetSquareDuty)			 // PROCESS_VAR(p_duty)
	PROCESS_SET(SetDutySweep)				 // PROCESS_VAR(p_duty_ramp)
	PROCESS_SET(SetVibratoDepth)		 // PROCESS_VAR(p_vib_strength)
	PROCESS_SET(SetVibratoSpeed)		 // PROCESS_VAR(p_vib_speed)
	PROCESS_SET(SetVibratoDelay)		 // PROCESS_VAR(p_vib_delay) //TODO: NEEDS FIX
	// PROCESS_SET_NORMWITH(SetAttackTime,25) //PROCESS_VAR(p_env_attack)
	// PROCESS_SET_NORMWITH(SetSustainTime,25) //PROCESS_VAR(p_env_sustain)
	// PROCESS_SET_NORMWITH(SetDecayTime,25) //PROCESS_VAR(p_env_decay)
	stream.read(reinterpret_cast<char *>(&p_env_attack), sizeof(p_env_attack));
	stream.read(reinterpret_cast<char *>(&p_env_sustain), sizeof(p_env_sustain));
	stream.read(reinterpret_cast<char *>(&p_env_decay), sizeof(p_env_decay));
	PROCESS_SET(SetSustainPunch) // PROCESS_VAR(p_env_punch)
	PROCESS_VAR_UNNORM(filter_on)
	PROCESS_SET(SetLowPassFilterResonance)		// PROCESS_VAR(p_lpf_resonance)
	PROCESS_SET(SetLowPassFilterCutoff)				// PROCESS_VAR(p_lpf_freq)
	PROCESS_SET(SetLowPassFilterCutoffSweep)	// PROCESS_VAR(p_lpf_ramp)
	PROCESS_SET(SetHighPassFilterCutoff)			// PROCESS_VAR(p_hpf_freq)
	PROCESS_SET(SetHighPassFilterCutoffSweep) // PROCESS_VAR(p_hpf_ramp)
	PROCESS_SET(SetPhaserOffset)							// PROCESS_VAR(p_pha_offset)
	PROCESS_SET(SetPhaserSweep)								// PROCESS_VAR(p_pha_ramp)
	PROCESS_SET(SetRepeatSpeed)								// PROCESS_VAR(p_repeat_speed)
	PROCESS_SET(SetChangeSpeed)								// PROCESS_VAR(p_arp_speed)
	PROCESS_SET(SetChangeAmount)							// PROCESS_VAR(p_arp_mod)
#undef PROCESS_VAR
#undef PROCESS_SET
#undef PROCESS_SET_NORMWITH
#undef PROCESS_VAR_UNNORM
	return stream.good();
}
bool sfxr::SaveSettingsShort(std::ostream &stream) const {
	unsigned char version = fileVersion;
	stream.write(reinterpret_cast<const char *>(&version), sizeof(version));
#define PROCESS_VAR(var)                                            \
	{                                                                 \
		char c = clampToChar(var * 128);                                \
		stream.write(reinterpret_cast<const char *>(&c), sizeof(char)); \
	}
#define PROCESS_VAR_UNNORM(var)                                     \
	{                                                                 \
		char c = clampToChar(var);                                      \
		stream.write(reinterpret_cast<const char *>(&c), sizeof(char)); \
	}
#define PROCESS_VAR_NORMWITH(var, norm)                             \
	{                                                                 \
		char c = clampToChar((var) * (norm));                           \
		stream.write(reinterpret_cast<const char *>(&c), sizeof(char)); \
	}
	// stream.write(reinterpret_cast<const char*>(&wave_type), sizeof(wave_type));
	PROCESS_VAR_UNNORM(wave_type)
	// stream.write(reinterpret_cast<const char*>(&sound_vol), sizeof(sound_vol));
	PROCESS_VAR(sound_vol)
	PROCESS_VAR(GetStartFrequency());		// PROCESS_VAR(p_base_freq)
	PROCESS_VAR(GetMinimumFrequency()); // PROCESS_VAR(p_freq_limit)
	PROCESS_VAR(GetSlide());						// PROCESS_VAR(p_freq_ramp)
	PROCESS_VAR(GetDeltaSlide());				// PROCESS_VAR(p_freq_dramp)
	PROCESS_VAR(GetSquareDuty());				// PROCESS_VAR(p_duty)
	PROCESS_VAR(GetDutySweep());				// PROCESS_VAR(p_duty_ramp)
	PROCESS_VAR(GetVibratoDepth());			// PROCESS_VAR(p_vib_strength)
	PROCESS_VAR(GetVibratoSpeed());			// PROCESS_VAR(p_vib_speed)
	PROCESS_VAR(GetVibratoDelay());
	// PROCESS_VAR_NORMWITH(GetAttackTime(),25); //PROCESS_VAR(p_env_attack)
	// PROCESS_VAR_NORMWITH(GetSustainTime(),25); //PROCESS_VAR(p_env_sustain)
	// PROCESS_VAR_NORMWITH(GetDecayTime(),25); //PROCESS_VAR(p_env_decay)
	stream.write(reinterpret_cast<const char *>(&p_env_attack), sizeof(p_env_attack));
	stream.write(reinterpret_cast<const char *>(&p_env_sustain), sizeof(p_env_sustain));
	stream.write(reinterpret_cast<const char *>(&p_env_decay), sizeof(p_env_decay));
	PROCESS_VAR(GetSustainPunch()); // PROCESS_VAR(p_env_punch)
	PROCESS_VAR_UNNORM(filter_on)
	PROCESS_VAR(GetLowPassFilterResonance());		 // PROCESS_VAR(p_lpf_resonance)
	PROCESS_VAR(GetLowPassFilterCutoff());			 // PROCESS_VAR(p_lpf_freq)
	PROCESS_VAR(GetLowPassFilterCutoffSweep());	// PROCESS_VAR(p_lpf_ramp)
	PROCESS_VAR(GetHighPassFilterCutoff());			 // PROCESS_VAR(p_hpf_freq)
	PROCESS_VAR(GetHighPassFilterCutoffSweep()); // PROCESS_VAR(p_hpf_ramp)
	PROCESS_VAR(GetPhaserOffset());							 // PROCESS_VAR(p_pha_offset)
	PROCESS_VAR(GetPhaserSweep());							 // PROCESS_VAR(p_pha_ramp)
	PROCESS_VAR(GetRepeatSpeed());							 // PROCESS_VAR(p_repeat_speed)
	PROCESS_VAR(GetChangeSpeed());							 // PROCESS_VAR(p_arp_speed)
	PROCESS_VAR(GetChangeAmount())
#undef PROCESS_VAR_UNNORM
#undef PROCESS_VAR
#undef PROCESS_VAR_NORMWITH
	return stream.good();
}
bool sfxr::LoadSettings(const char *filename) {
	FILE *file = fopen(filename, "rb");
	if (!file)
		return false;
	int version = 0;
	fread(&version, 1, sizeof(int), file);
	if (version != 100 && version != 101 && version != 102)
		return false;
	fread(&wave_type, 1, sizeof(int), file);
	sound_vol = 0.5f;
	if (version == 102)
		fread(&sound_vol, 1, sizeof(float), file);
	fread(&p_base_freq, 1, sizeof(float), file);
	fread(&p_freq_limit, 1, sizeof(float), file);
	fread(&p_freq_ramp, 1, sizeof(float), file);
	if (version >= 101)
		fread(&p_freq_dramp, 1, sizeof(float), file);
	fread(&p_duty, 1, sizeof(float), file);
	fread(&p_duty_ramp, 1, sizeof(float), file);
	fread(&p_vib_strength, 1, sizeof(float), file);
	fread(&p_vib_speed, 1, sizeof(float), file);
	fread(&p_vib_delay, 1, sizeof(float), file);
	fread(&p_env_attack, 1, sizeof(float), file);
	fread(&p_env_sustain, 1, sizeof(float), file);
	fread(&p_env_decay, 1, sizeof(float), file);
	fread(&p_env_punch, 1, sizeof(float), file);
	fread(&filter_on, 1, sizeof(bool), file);
	fread(&p_lpf_resonance, 1, sizeof(float), file);
	fread(&p_lpf_freq, 1, sizeof(float), file);
	fread(&p_lpf_ramp, 1, sizeof(float), file);
	fread(&p_hpf_freq, 1, sizeof(float), file);
	fread(&p_hpf_ramp, 1, sizeof(float), file);
	fread(&p_pha_offset, 1, sizeof(float), file);
	fread(&p_pha_ramp, 1, sizeof(float), file);
	fread(&p_repeat_speed, 1, sizeof(float), file);
	if (version >= 101) {
		fread(&p_arp_speed, 1, sizeof(float), file);
		fread(&p_arp_mod, 1, sizeof(float), file);
	}
	fclose(file);
	return true;
}
bool sfxr::SaveSettings(const char *filename) {
	FILE *file = fopen(filename, "wb");
	if (!file)
		return false;
	int version = 102;
	fwrite(&version, 1, sizeof(int), file);
	fwrite(&wave_type, 1, sizeof(int), file);
	fwrite(&sound_vol, 1, sizeof(float), file);
	fwrite(&p_base_freq, 1, sizeof(float), file);
	fwrite(&p_freq_limit, 1, sizeof(float), file);
	fwrite(&p_freq_ramp, 1, sizeof(float), file);
	fwrite(&p_freq_dramp, 1, sizeof(float), file);
	fwrite(&p_duty, 1, sizeof(float), file);
	fwrite(&p_duty_ramp, 1, sizeof(float), file);
	fwrite(&p_vib_strength, 1, sizeof(float), file);
	fwrite(&p_vib_speed, 1, sizeof(float), file);
	fwrite(&p_vib_delay, 1, sizeof(float), file);
	fwrite(&p_env_attack, 1, sizeof(float), file);
	fwrite(&p_env_sustain, 1, sizeof(float), file);
	fwrite(&p_env_decay, 1, sizeof(float), file);
	fwrite(&p_env_punch, 1, sizeof(float), file);
	fwrite(&filter_on, 1, sizeof(bool), file);
	fwrite(&p_lpf_resonance, 1, sizeof(float), file);
	fwrite(&p_lpf_freq, 1, sizeof(float), file);
	fwrite(&p_lpf_ramp, 1, sizeof(float), file);
	fwrite(&p_hpf_freq, 1, sizeof(float), file);
	fwrite(&p_hpf_ramp, 1, sizeof(float), file);
	fwrite(&p_pha_offset, 1, sizeof(float), file);
	fwrite(&p_pha_ramp, 1, sizeof(float), file);
	fwrite(&p_repeat_speed, 1, sizeof(float), file);
	fwrite(&p_arp_speed, 1, sizeof(float), file);
	fwrite(&p_arp_mod, 1, sizeof(float), file);
	fclose(file);
	return true;
}
bool sfxr::ExportWAV(const char *filename) {
	FILE *foutput = fopen(filename, "wb");
	if (!foutput)
		return false;
	// write wav header
	unsigned int dword = 0;
	unsigned short word = 0;
	fwrite("RIFF", 4, 1, foutput); // "RIFF"
	dword = 0;
	fwrite(&dword, 1, 4, foutput); // remaining file size
	fwrite("WAVE", 4, 1, foutput); // "WAVE"
	fwrite("fmt ", 4, 1, foutput); // "fmt "
	dword = 16;
	fwrite(&dword, 1, 4, foutput); // chunk size
	word = 1;
	fwrite(&word, 1, 2, foutput); // compression code
	word = 1;
	fwrite(&word, 1, 2, foutput); // channels
	dword = wav_freq;
	fwrite(&dword, 1, 4, foutput); // sample rate
	dword = wav_freq * wav_bits / 8;
	fwrite(&dword, 1, 4, foutput); // bytes/sec
	word = wav_bits / 8;
	fwrite(&word, 1, 2, foutput); // block align
	word = wav_bits;
	fwrite(&word, 1, 2, foutput);	// bits per sample
	fwrite("data", 4, 1, foutput); // "data"
	dword = 0;
	int foutstream_datasize = ftell(foutput);
	fwrite(&dword, 1, 4, foutput); // chunk size
	// write sample data
	mute_stream = true;
	file_sampleswritten = 0;
	filesample = 0.0f;
	fileacc = 0;
	PlaySample();
	while (playing_sample)
		SynthSample(256, NULL, foutput);
	mute_stream = false;
	// seek back to header and write size info
	fseek(foutput, 4, SEEK_SET);
	dword = 0;
	dword = foutstream_datasize - 4 + file_sampleswritten * wav_bits / 8;
	fwrite(&dword, 1, 4, foutput); // remaining file size
	fseek(foutput, foutstream_datasize, SEEK_SET);
	dword = file_sampleswritten * wav_bits / 8;
	fwrite(&dword, 1, 4, foutput); // chunk size (data)
	fclose(foutput);
	return true;
}
void sfxr::ResetParams() {
	wave_type = 0;
	p_base_freq = 0.3f;
	p_freq_limit = 0.0f;
	p_freq_ramp = 0.0f;
	p_freq_dramp = 0.0f;
	p_duty = 0.0f;
	p_duty_ramp = 0.0f;
	p_vib_strength = 0.0f;
	p_vib_speed = 0.0f;
	p_vib_delay = 0.0f;
	p_env_attack = 0.0f;
	p_env_sustain = 0.3f;
	p_env_decay = 0.4f;
	p_env_punch = 0.0f;
	filter_on = false;
	p_lpf_resonance = 0.0f;
	p_lpf_freq = 1.0f;
	p_lpf_ramp = 0.0f;
	p_hpf_freq = 0.0f;
	p_hpf_ramp = 0.0f;
	p_pha_offset = 0.0f;
	p_pha_ramp = 0.0f;
	p_repeat_speed = 0.0f;
	p_arp_speed = 0.0f;
	p_arp_mod = 0.0f;
	sound_vol = 0.5f;
}
void sfxr::ResetSample(bool restart) {
	if (!restart)
		phase = 0;
	fperiod = 100.0 / (p_base_freq * p_base_freq + 0.001);
	period = (int)fperiod;
	fmaxperiod = 100.0 / (p_freq_limit * p_freq_limit + 0.001);
	fslide = 1.0 - pow((double)p_freq_ramp, 3.0) * 0.01;
	fdslide = -pow((double)p_freq_dramp, 3.0) * 0.000001;
	square_duty = 0.5f - p_duty * 0.5f;
	square_slide = -p_duty_ramp * 0.00005f;
	if (p_arp_mod >= 0.0f)
		arp_mod = 1.0 - pow((double)p_arp_mod, 2.0) * 0.9;
	else
		arp_mod = 1.0 + pow((double)p_arp_mod, 2.0) * 10.0;
	arp_time = 0;
	arp_limit = (int)(pow(1.0f - p_arp_speed, 2.0f) * 20000 + 32);
	if (p_arp_speed == 1.0f)
		arp_limit = 0;
	if (!restart) {
		// reset filter
		fltp = 0.0f;
		fltdp = 0.0f;
		fltw = pow(p_lpf_freq, 3.0f) * 0.1f;
		fltw_d = 1.0f + p_lpf_ramp * 0.0001f;
		fltdmp = 5.0f / (1.0f + pow(p_lpf_resonance, 2.0f) * 20.0f) * (0.01f + fltw);
		if (fltdmp > 0.8f)
			fltdmp = 0.8f;
		fltphp = 0.0f;
		flthp = pow(p_hpf_freq, 2.0f) * 0.1f;
		flthp_d = 1.0 + p_hpf_ramp * 0.0003f;
		// reset vibrato
		vib_phase = 0.0f;
		vib_speed = pow(p_vib_speed, 2.0f) * 0.01f;
		vib_amp = p_vib_strength * 0.5f;
		// reset envelope
		env_vol = 0.0f;
		env_stage = 0;
		env_time = 0;
		env_length[0] = (int)(p_env_attack * p_env_attack * 100000.0f);
		env_length[1] = (int)(p_env_sustain * p_env_sustain * 100000.0f);
		env_length[2] = (int)(p_env_decay * p_env_decay * 100000.0f);
		fphase = pow(p_pha_offset, 2.0f) * 1020.0f;
		if (p_pha_offset < 0.0f)
			fphase = -fphase;
		fdphase = pow(p_pha_ramp, 2.0f) * 1.0f;
		if (p_pha_ramp < 0.0f)
			fdphase = -fdphase;
		iphase = abs((int)fphase);
		ipp = 0;
		for (int i = 0; i < 1024; i++)
			phaser_buffer[i] = 0.0f;
		for (int i = 0; i < 32; i++)
			noise_buffer[i] = frnd(2.0f) - 1.0f;
		rep_time = 0;
		rep_limit = (int)(pow(1.0f - p_repeat_speed, 2.0f) * 20000 + 32);
		if (p_repeat_speed == 0.0f)
			rep_limit = 0;
	}
}
void sfxr::PlaySample() {
	ResetSample(false);
	playing_sample = true;
}
void sfxr::SetSeed(unsigned seed) { srand(seed); }
int sfxr::SynthSample(int length, float *buffer, FILE *file) {
	int numFloatsWritten = 0;
	for (int i = 0; i < length; i++) {
		if (!playing_sample)
			break;
		rep_time++;
		if (rep_limit != 0 && rep_time >= rep_limit) {
			rep_time = 0;
			ResetSample(true);
		}
		// frequency envelopes/arpeggios
		arp_time++;
		if (arp_limit != 0 && arp_time >= arp_limit) {
			arp_limit = 0;
			fperiod *= arp_mod;
		}
		fslide += fdslide;
		fperiod *= fslide;
		if (fperiod > fmaxperiod) {
			fperiod = fmaxperiod;
			if (p_freq_limit > 0.0f)
				playing_sample = false;
		}
		float rfperiod = fperiod;
		if (vib_amp > 0.0f) {
			vib_phase += vib_speed;
			rfperiod = fperiod * (1.0 + sin(vib_phase) * vib_amp);
		}
		period = (int)rfperiod;
		if (period < 8)
			period = 8;
		square_duty += square_slide;
		if (square_duty < 0.0f)
			square_duty = 0.0f;
		if (square_duty > 0.5f)
			square_duty = 0.5f;
		// volume envelope
		env_time++;
		if (env_time > env_length[env_stage]) {
			env_time = 0;
			env_stage++;
			if (env_stage == 3)
				playing_sample = false;
		}
		if (env_stage == 0)
			env_vol = (float)env_time / env_length[0];
		if (env_stage == 1)
			env_vol = 1.0f + pow(1.0f - (float)env_time / env_length[1], 1.0f) * 2.0f * p_env_punch;
		if (env_stage == 2)
			env_vol = 1.0f - (float)env_time / env_length[2];
		// phaser step
		fphase += fdphase;
		iphase = abs((int)fphase);
		if (iphase > 1023)
			iphase = 1023;
		if (flthp_d != 0.0f) {
			flthp *= flthp_d;
			if (flthp < 0.00001f)
				flthp = 0.00001f;
			if (flthp > 0.1f)
				flthp = 0.1f;
		}
		float ssample = 0.0f;
		for (int si = 0; si < 8; si++) // 8x supersampling
		{
			float sample = 0.0f;
			phase++;
			if (phase >= period) {
				//				phase=0;
				phase %= period;
				if (wave_type == 3)
					for (int i = 0; i < 32; i++)
						noise_buffer[i] = flfsr(); // NOTE: The original sfxr used: frnd(2.0f)-1.0f
																			 //  as the noise function.  The iPhone CPU wasn't
																			 //  fast enough to keep up with the audio output.
			}
			// base waveform
			float fp = (float)phase / period;
			switch (wave_type) {
			case 0: // square
				if (fp < square_duty)
					sample = 0.5f;
				else
					sample = -0.5f;
				break;
			case 1: // sawtooth
				sample = 1.0f - fp * 2;
				break;
			case 2: // sine
				sample = (float)sin(fp * 2 * PI);
				break;
			case 3: // noise
				sample = noise_buffer[phase * 32 / period];
				break;
			}
			// lp filter
			float pp = fltp;
			fltw *= fltw_d;
			if (fltw < 0.0f)
				fltw = 0.0f;
			if (fltw > 0.1f)
				fltw = 0.1f;
			if (p_lpf_freq != 1.0f) {
				fltdp += (sample - fltp) * fltw;
				fltdp -= fltdp * fltdmp;
			} else {
				fltp = sample;
				fltdp = 0.0f;
			}
			fltp += fltdp;
			// hp filter
			fltphp += fltp - pp;
			fltphp -= fltphp * flthp;
			sample = fltphp;
			// phaser
			phaser_buffer[ipp & 1023] = sample;
			sample += phaser_buffer[(ipp - iphase + 1024) & 1023];
			ipp = (ipp + 1) & 1023;
			// final accumulation and envelope application
			ssample += sample * env_vol;
		}
		ssample = ssample / 8 * master_vol;
		ssample *= 2.0f * sound_vol;
		numFloatsWritten++;
		if (buffer != NULL) {
			if (ssample > 1.0f)
				ssample = 1.0f;
			if (ssample < -1.0f)
				ssample = -1.0f;
			*buffer++ = ssample;
		}
		if (file != NULL) {
			// quantize depending on format
			// accumulate/count to accomodate variable sample rate?
			ssample *= 4.0f; // arbitrary gain to get reasonable output volume...
			if (ssample > 1.0f)
				ssample = 1.0f;
			if (ssample < -1.0f)
				ssample = -1.0f;
			filesample += ssample;
			fileacc++;
			if (wav_freq == 44100 || fileacc == 2) {
				filesample /= fileacc;
				fileacc = 0;
				if (wav_bits == 16) {
					short isample = (short)(filesample * 32000);
					fwrite(&isample, 1, 2, file);
				} else {
					unsigned char isample = (unsigned char)(filesample * 127 + 128);
					fwrite(&isample, 1, 1, file);
				}
				filesample = 0.0f;
			}
			file_sampleswritten++;
		}
	}
	return numFloatsWritten;
}
float sfxr::GetSoundVolume() const {
	SFXR_LOG(sound_vol);
	return sound_vol;
}
void sfxr::SetSoundVolume(const float value) { sound_vol = value; }
WaveformGenerator sfxr::GetWaveform() const {
	SFXR_LOG(wave_type);
	switch (wave_type) {
	case 0:
		return SquareWave;
	case 1:
		return Sawtooth;
	case 2:
		return SineWave;
	case 3:
		return Noise;
	default:
		abort();
	}
}
void sfxr::SetWaveform(const int waveform) {
	SFXR_LOG(waveform);
	wave_type = waveform;
	/*switch (waveform) {
			case SquareWave:
					wave_type = 0;
					break;
			case Sawtooth:
					wave_type = 1;
					break;
			case SineWave:
					wave_type = 2;
					break;
			case Noise:
					wave_type = 3;
					break;
			default:
					abort();
	}*/
}
float sfxr::GetAttackTime() const {
	SFXR_LOG(p_env_attack);
	return p_env_attack; //(p_env_attack+ PARAM_OFFSET_GET)*PARAM_SCALE_GET;
}
void sfxr::SetAttackTime(const float value) {
	p_env_attack = value; // value * 2.0f - 1.0f;
}
float sfxr::GetSustainTime() const {
	SFXR_LOG(p_env_sustain);
	return p_env_sustain; //(p_env_sustain+ PARAM_OFFSET_GET)*PARAM_SCALE_GET;
}
void sfxr::SetSustainTime(const float value) {
	p_env_sustain = value; // * 2.0f - 1.0f;
}
float sfxr::GetSustainPunch() const {
	SFXR_LOG((p_env_punch + PARAM_OFFSET_GET) * PARAM_SCALE_GET);
	return (p_env_punch + PARAM_OFFSET_GET) * PARAM_SCALE_GET;
}
void sfxr::SetSustainPunch(const float value) { p_env_punch = value * PARAM_SCALE + PARAM_OFFSET; }
float sfxr::GetDecayTime() const {
	SFXR_LOG(p_env_decay);
	return p_env_decay; //(p_env_decay+ PARAM_OFFSET_GET)*PARAM_SCALE_GET;
}
void sfxr::SetDecayTime(const float value) {
	p_env_decay = value; // * 2.0f - 1.0f;
}
float sfxr::GetStartFrequency() const {
	SFXR_LOG(p_base_freq);
	return p_base_freq;
}
void sfxr::SetStartFrequency(const float value) { p_base_freq = value; }
float sfxr::GetMinimumFrequency() const {
	SFXR_LOG(p_freq_limit);
	return p_freq_limit;
}
void sfxr::SetMinimumFrequency(const float value) { p_freq_limit = value; }
float sfxr::GetSlide() const {
	SFXR_LOG((p_freq_ramp + PARAM_OFFSET_GET) * PARAM_SCALE_GET);
	return (p_freq_ramp + PARAM_OFFSET_GET) * PARAM_SCALE_GET;
}
void sfxr::SetSlide(const float value) { p_freq_ramp = value * PARAM_SCALE + PARAM_OFFSET; }
float sfxr::GetDeltaSlide() const {
	SFXR_LOG((p_freq_dramp + PARAM_OFFSET_GET) * PARAM_SCALE_GET);
	return (p_freq_dramp + PARAM_OFFSET_GET) * PARAM_SCALE_GET;
}
void sfxr::SetDeltaSlide(const float value) { p_freq_dramp = value * PARAM_SCALE + PARAM_OFFSET; }
float sfxr::GetVibratoDepth() const {
	SFXR_LOG((p_vib_strength + PARAM_OFFSET_GET) * PARAM_SCALE_GET);
	return (p_vib_strength + PARAM_OFFSET_GET) * PARAM_SCALE_GET;
}
void sfxr::SetVibratoDepth(const float value) { p_vib_strength = value * PARAM_SCALE + PARAM_OFFSET; }
float sfxr::GetVibratoSpeed() const {
	SFXR_LOG((p_vib_speed + PARAM_OFFSET_GET) * PARAM_SCALE_GET);
	return (p_vib_speed + PARAM_OFFSET_GET) * PARAM_SCALE_GET;
}
void sfxr::SetVibratoSpeed(const float value) { p_vib_speed = value * PARAM_SCALE + PARAM_OFFSET; }
float sfxr::GetVibratoDelay() const {
	SFXR_LOG((p_vib_delay + PARAM_OFFSET_GET) * PARAM_SCALE_GET);
	return (p_vib_delay + PARAM_OFFSET_GET) * PARAM_SCALE_GET;
}
void sfxr::SetVibratoDelay(const float value) { p_vib_delay = value * PARAM_SCALE + PARAM_OFFSET; }
float sfxr::GetChangeAmount() const {
	SFXR_LOG((p_arp_mod + PARAM_OFFSET_GET) * PARAM_SCALE_GET);
	return (p_arp_mod + PARAM_OFFSET_GET) * PARAM_SCALE_GET;
}
void sfxr::SetChangeAmount(const float value) { p_arp_mod = value * PARAM_SCALE + PARAM_OFFSET; }
float sfxr::GetChangeSpeed() const {
	SFXR_LOG((p_arp_speed + PARAM_OFFSET_GET) * PARAM_SCALE_GET);
	return (p_arp_speed + PARAM_OFFSET_GET) * PARAM_SCALE_GET;
}
void sfxr::SetChangeSpeed(const float value) { p_arp_speed = value * PARAM_SCALE + PARAM_OFFSET; }
float sfxr::GetSquareDuty() const {
	SFXR_LOG((p_duty + PARAM_OFFSET_GET) * PARAM_SCALE_GET);
	return (p_duty + PARAM_OFFSET_GET) * PARAM_SCALE_GET;
}
void sfxr::SetSquareDuty(const float value) { p_duty = value * PARAM_SCALE + PARAM_OFFSET; }
float sfxr::GetDutySweep() const {
	SFXR_LOG((p_duty_ramp + PARAM_OFFSET_GET) * PARAM_SCALE_GET);
	return (p_duty_ramp + PARAM_OFFSET_GET) * PARAM_SCALE_GET;
}
void sfxr::SetDutySweep(const float value) { p_duty_ramp = value * PARAM_SCALE + PARAM_OFFSET; }
float sfxr::GetRepeatSpeed() const {
	SFXR_LOG((p_repeat_speed + PARAM_OFFSET_GET) * PARAM_SCALE_GET);
	return (p_repeat_speed + PARAM_OFFSET_GET) * PARAM_SCALE_GET;
}
void sfxr::SetRepeatSpeed(const float value) { p_repeat_speed = value * PARAM_SCALE + PARAM_OFFSET; }
float sfxr::GetPhaserOffset() const {
	SFXR_LOG((p_pha_offset + PARAM_OFFSET_GET) * PARAM_SCALE_GET);
	return (p_pha_offset + PARAM_OFFSET_GET) * PARAM_SCALE_GET;
}
void sfxr::SetPhaserOffset(const float value) { p_pha_offset = value * PARAM_SCALE + PARAM_OFFSET; }
float sfxr::GetPhaserSweep() const {
	SFXR_LOG((p_pha_ramp + PARAM_OFFSET_GET) * PARAM_SCALE_GET);
	return (p_pha_ramp + PARAM_OFFSET_GET) * PARAM_SCALE_GET;
}
void sfxr::SetPhaserSweep(const float value) { p_pha_ramp = value * PARAM_SCALE + PARAM_OFFSET; }
float sfxr::GetLowPassFilterCutoff() const {
	SFXR_LOG((p_lpf_freq + PARAM_OFFSET_GET) * PARAM_SCALE_GET);
	return (p_lpf_freq + PARAM_OFFSET_GET) * PARAM_SCALE_GET;
}
void sfxr::SetLowPassFilterCutoff(const float value) { p_lpf_freq = value * PARAM_SCALE + PARAM_OFFSET; }
float sfxr::GetLowPassFilterCutoffSweep() const {
	SFXR_LOG((p_lpf_ramp + PARAM_OFFSET_GET) * PARAM_SCALE_GET);
	return (p_lpf_ramp + PARAM_OFFSET_GET) * PARAM_SCALE_GET;
}
void sfxr::SetLowPassFilterCutoffSweep(const float value) { p_lpf_ramp = value * PARAM_SCALE + PARAM_OFFSET; }
float sfxr::GetLowPassFilterResonance() const {
	SFXR_LOG((p_lpf_resonance + PARAM_OFFSET_GET) * PARAM_SCALE_GET);
	return (p_lpf_resonance + PARAM_OFFSET_GET) * PARAM_SCALE_GET;
}
void sfxr::SetLowPassFilterResonance(const float value) { p_lpf_resonance = value * PARAM_SCALE + PARAM_OFFSET; }
float sfxr::GetHighPassFilterCutoff() const {
	SFXR_LOG((p_hpf_freq + PARAM_OFFSET_GET) * PARAM_SCALE_GET);
	return (p_hpf_freq + PARAM_OFFSET_GET) * PARAM_SCALE_GET;
}
void sfxr::SetHighPassFilterCutoff(const float value) { p_hpf_freq = value * PARAM_SCALE + PARAM_OFFSET; }
float sfxr::GetHighPassFilterCutoffSweep() const {
	SFXR_LOG((p_hpf_ramp + PARAM_OFFSET_GET) * PARAM_SCALE_GET);
	return (p_hpf_ramp + PARAM_OFFSET_GET) * PARAM_SCALE_GET;
}
void sfxr::SetHighPassFilterCutoffSweep(const float value) { p_hpf_ramp = value * PARAM_SCALE + PARAM_OFFSET; }
void sfxr::PickupCoinButtonPressed() {
	ResetParams();
	p_base_freq = 0.4f + frnd(0.5f);
	p_env_attack = 0.0f;
	p_env_sustain = frnd(0.1f);
	p_env_decay = 0.1f + frnd(0.4f);
	p_env_punch = 0.3f + frnd(0.3f);
	if (rnd(1)) {
		p_arp_speed = 0.5f + frnd(0.2f);
		p_arp_mod = 0.2f + frnd(0.4f);
	}
	PlaySample();
}
void sfxr::LaserShootButtonPressed() {
	ResetParams();
	wave_type = rnd(2);
	if (wave_type == 2 && rnd(1))
		wave_type = rnd(1);
	p_base_freq = 0.5f + frnd(0.5f);
	p_freq_limit = p_base_freq - 0.2f - frnd(0.6f);
	if (p_freq_limit < 0.2f)
		p_freq_limit = 0.2f;
	p_freq_ramp = -0.15f - frnd(0.2f);
	if (rnd(2) == 0) {
		p_base_freq = 0.3f + frnd(0.6f);
		p_freq_limit = frnd(0.1f);
		p_freq_ramp = -0.35f - frnd(0.3f);
	}
	if (rnd(1)) {
		p_duty = frnd(0.5f);
		p_duty_ramp = frnd(0.2f);
	} else {
		p_duty = 0.4f + frnd(0.5f);
		p_duty_ramp = -frnd(0.7f);
	}
	p_env_attack = 0.0f;
	p_env_sustain = 0.1f + frnd(0.2f);
	p_env_decay = frnd(0.4f);
	if (rnd(1))
		p_env_punch = frnd(0.3f);
	if (rnd(2) == 0) {
		p_pha_offset = frnd(0.2f);
		p_pha_ramp = -frnd(0.2f);
	}
	if (rnd(1))
		p_hpf_freq = frnd(0.3f);
	PlaySample();
}
void sfxr::ExplosionButtonPressed() {
	ResetParams();
	wave_type = 3;
	if (rnd(1)) {
		p_base_freq = 0.1f + frnd(0.4f);
		p_freq_ramp = -0.1f + frnd(0.4f);
	} else {
		p_base_freq = 0.2f + frnd(0.7f);
		p_freq_ramp = -0.2f - frnd(0.2f);
	}
	p_base_freq *= p_base_freq;
	if (rnd(4) == 0)
		p_freq_ramp = 0.0f;
	if (rnd(2) == 0)
		p_repeat_speed = 0.3f + frnd(0.5f);
	p_env_attack = 0.0f;
	p_env_sustain = 0.1f + frnd(0.3f);
	p_env_decay = frnd(0.5f);
	if (rnd(1) == 0) {
		p_pha_offset = -0.3f + frnd(0.9f);
		p_pha_ramp = -frnd(0.3f);
	}
	p_env_punch = 0.2f + frnd(0.6f);
	if (rnd(1)) {
		p_vib_strength = frnd(0.7f);
		p_vib_speed = frnd(0.6f);
	}
	if (rnd(2) == 0) {
		p_arp_speed = 0.6f + frnd(0.3f);
		p_arp_mod = 0.8f - frnd(1.6f);
	}
	PlaySample();
}
void sfxr::PowerupButtonPressed() {
	ResetParams();
	if (rnd(1))
		wave_type = 1;
	else
		p_duty = frnd(0.6f);
	if (rnd(1)) {
		p_base_freq = 0.2f + frnd(0.3f);
		p_freq_ramp = 0.1f + frnd(0.4f);
		p_repeat_speed = 0.4f + frnd(0.4f);
	} else {
		p_base_freq = 0.2f + frnd(0.3f);
		p_freq_ramp = 0.05f + frnd(0.2f);
		if (rnd(1)) {
			p_vib_strength = frnd(0.7f);
			p_vib_speed = frnd(0.6f);
		}
	}
	p_env_attack = 0.0f;
	p_env_sustain = frnd(0.4f);
	p_env_decay = 0.1f + frnd(0.4f);
	PlaySample();
}
void sfxr::HitHurtButtonPressed() {
	ResetParams();
	wave_type = rnd(2);
	if (wave_type == 2)
		wave_type = 3;
	if (wave_type == 0)
		p_duty = frnd(0.6f);
	p_base_freq = 0.2f + frnd(0.6f);
	p_freq_ramp = -0.3f - frnd(0.4f);
	p_env_attack = 0.0f;
	p_env_sustain = frnd(0.1f);
	p_env_decay = 0.1f + frnd(0.2f);
	if (rnd(1))
		p_hpf_freq = frnd(0.3f);
	PlaySample();
}
void sfxr::JumpButtonPressed() {
	ResetParams();
	wave_type = 0;
	p_duty = frnd(0.6f);
	p_base_freq = 0.3f + frnd(0.3f);
	p_freq_ramp = 0.1f + frnd(0.2f);
	p_env_attack = 0.0f;
	p_env_sustain = 0.1f + frnd(0.3f);
	p_env_decay = 0.1f + frnd(0.2f);
	if (rnd(1))
		p_hpf_freq = frnd(0.3f);
	if (rnd(1))
		p_lpf_freq = 1.0f - frnd(0.6f);
	PlaySample();
}
void sfxr::BlitSelectButtonPressed() {
	ResetParams();
	wave_type = rnd(1);
	if (wave_type == 0)
		p_duty = frnd(0.6f);
	p_base_freq = 0.2f + frnd(0.4f);
	p_env_attack = 0.0f;
	p_env_sustain = 0.1f + frnd(0.1f);
	p_env_decay = frnd(0.2f);
	p_hpf_freq = 0.1f;
	PlaySample();
}
void sfxr::MutateButtonPressed() {
	if (rnd(1))
		p_base_freq += frnd(0.1f) - 0.05f;
	// if(rnd(1)) p_freq_limit+=frnd(0.1f)-0.05f;
	if (rnd(1))
		p_freq_ramp += frnd(0.1f) - 0.05f;
	if (rnd(1))
		p_freq_dramp += frnd(0.1f) - 0.05f;
	if (rnd(1))
		p_duty += frnd(0.1f) - 0.05f;
	if (rnd(1))
		p_duty_ramp += frnd(0.1f) - 0.05f;
	if (rnd(1))
		p_vib_strength += frnd(0.1f) - 0.05f;
	if (rnd(1))
		p_vib_speed += frnd(0.1f) - 0.05f;
	if (rnd(1))
		p_vib_delay += frnd(0.1f) - 0.05f;
	if (rnd(1))
		p_env_attack += frnd(0.1f) - 0.05f;
	if (rnd(1))
		p_env_sustain += frnd(0.1f) - 0.05f;
	if (rnd(1))
		p_env_decay += frnd(0.1f) - 0.05f;
	if (rnd(1))
		p_env_punch += frnd(0.1f) - 0.05f;
	if (rnd(1))
		p_lpf_resonance += frnd(0.1f) - 0.05f;
	if (rnd(1))
		p_lpf_freq += frnd(0.1f) - 0.05f;
	if (rnd(1))
		p_lpf_ramp += frnd(0.1f) - 0.05f;
	if (rnd(1))
		p_hpf_freq += frnd(0.1f) - 0.05f;
	if (rnd(1))
		p_hpf_ramp += frnd(0.1f) - 0.05f;
	if (rnd(1))
		p_pha_offset += frnd(0.1f) - 0.05f;
	if (rnd(1))
		p_pha_ramp += frnd(0.1f) - 0.05f;
	if (rnd(1))
		p_repeat_speed += frnd(0.1f) - 0.05f;
	if (rnd(1))
		p_arp_speed += frnd(0.1f) - 0.05f;
	if (rnd(1))
		p_arp_mod += frnd(0.1f) - 0.05f;
	PlaySample();
}
void sfxr::RandomizeButtonPressed() {
	p_base_freq = pow(frnd(2.0f) - 1.0f, 2.0f);
	if (rnd(1))
		p_base_freq = pow(frnd(2.0f) - 1.0f, 3.0f) + 0.5f;
	p_freq_limit = 0.0f;
	p_freq_ramp = pow(frnd(2.0f) - 1.0f, 5.0f);
	if (p_base_freq > 0.7f && p_freq_ramp > 0.2f)
		p_freq_ramp = -p_freq_ramp;
	if (p_base_freq < 0.2f && p_freq_ramp < -0.05f)
		p_freq_ramp = -p_freq_ramp;
	p_freq_dramp = pow(frnd(2.0f) - 1.0f, 3.0f);
	p_duty = frnd(2.0f) - 1.0f;
	p_duty_ramp = pow(frnd(2.0f) - 1.0f, 3.0f);
	p_vib_strength = pow(frnd(2.0f) - 1.0f, 3.0f);
	p_vib_speed = frnd(2.0f) - 1.0f;
	p_vib_delay = frnd(2.0f) - 1.0f;
	p_env_attack = pow(frnd(2.0f) - 1.0f, 3.0f);
	p_env_sustain = pow(frnd(2.0f) - 1.0f, 2.0f);
	p_env_decay = frnd(2.0f) - 1.0f;
	p_env_punch = pow(frnd(0.8f), 2.0f);
	if (p_env_attack + p_env_sustain + p_env_decay < 0.2f) {
		p_env_sustain += 0.2f + frnd(0.3f);
		p_env_decay += 0.2f + frnd(0.3f);
	}
	p_lpf_resonance = frnd(2.0f) - 1.0f;
	p_lpf_freq = 1.0f - pow(frnd(1.0f), 3.0f);
	p_lpf_ramp = pow(frnd(2.0f) - 1.0f, 3.0f);
	if (p_lpf_freq < 0.1f && p_lpf_ramp < -0.05f)
		p_lpf_ramp = -p_lpf_ramp;
	p_hpf_freq = pow(frnd(1.0f), 5.0f);
	p_hpf_ramp = pow(frnd(2.0f) - 1.0f, 5.0f);
	p_pha_offset = pow(frnd(2.0f) - 1.0f, 3.0f);
	p_pha_ramp = pow(frnd(2.0f) - 1.0f, 3.0f);
	p_repeat_speed = frnd(2.0f) - 1.0f;
	p_arp_speed = frnd(2.0f) - 1.0f;
	p_arp_mod = frnd(2.0f) - 1.0f;
	PlaySample();
}
int sfxr::Read(const char *bytes, int length) {
	int offset = 0;
#define PROCESS_VAR_UNNORM(var) \
	{ var = bytes[offset++]; }
#define PROCESS_VAR(var) \
	{ var = bytes[offset++] / 128.0f; }
#define PROCESS_VAR_READ(var)                 \
	{                                           \
		var = *((typeof(var) *)(bytes + offset)); \
		offset += sizeof(var);                    \
	}
#define PROCESS_SET(func) \
	{ func(bytes[offset++] / 128.0f); }
	char version;
	PROCESS_VAR_UNNORM(version);
	PROCESS_VAR_UNNORM(wave_type)
	PROCESS_VAR(sound_vol)
	PROCESS_SET(SetStartFrequency)	 // PROCESS_VAR(p_base_freq)
	PROCESS_SET(SetMinimumFrequency) // PROCESS_VAR(p_freq_limit)
	PROCESS_SET(SetSlide)						 // PROCESS_VAR(p_freq_ramp)
	PROCESS_SET(SetDeltaSlide)			 // PROCESS_VAR(p_freq_dramp)
	PROCESS_SET(SetSquareDuty)			 // PROCESS_VAR(p_duty)
	PROCESS_SET(SetDutySweep)				 // PROCESS_VAR(p_duty_ramp)
	PROCESS_SET(SetVibratoDepth)		 // PROCESS_VAR(p_vib_strength)
	PROCESS_SET(SetVibratoSpeed)		 // PROCESS_VAR(p_vib_speed)
	PROCESS_SET(SetVibratoDelay)		 // PROCESS_VAR(p_vib_delay) //TODO: NEEDS FIX
	// PROCESS_SET_NORMWITH(SetAttackTime,25) //PROCESS_VAR(p_env_attack)
	// PROCESS_SET_NORMWITH(SetSustainTime,25) //PROCESS_VAR(p_env_sustain)
	// PROCESS_SET_NORMWITH(SetDecayTime,25) //PROCESS_VAR(p_env_decay)
	PROCESS_VAR_READ(p_env_attack);
	PROCESS_VAR_READ(p_env_sustain);
	PROCESS_VAR_READ(p_env_decay);
	PROCESS_SET(SetSustainPunch) // PROCESS_VAR(p_env_punch)
	PROCESS_VAR_UNNORM(filter_on)
	PROCESS_SET(SetLowPassFilterResonance)		// PROCESS_VAR(p_lpf_resonance)
	PROCESS_SET(SetLowPassFilterCutoff)				// PROCESS_VAR(p_lpf_freq)
	PROCESS_SET(SetLowPassFilterCutoffSweep)	// PROCESS_VAR(p_lpf_ramp)
	PROCESS_SET(SetHighPassFilterCutoff)			// PROCESS_VAR(p_hpf_freq)
	PROCESS_SET(SetHighPassFilterCutoffSweep) // PROCESS_VAR(p_hpf_ramp)
	PROCESS_SET(SetPhaserOffset)							// PROCESS_VAR(p_pha_offset)
	PROCESS_SET(SetPhaserSweep)								// PROCESS_VAR(p_pha_ramp)
	PROCESS_SET(SetRepeatSpeed)								// PROCESS_VAR(p_repeat_speed)
	PROCESS_SET(SetChangeSpeed)								// PROCESS_VAR(p_arp_speed)
	PROCESS_SET(SetChangeAmount)							// PROCESS_VAR(p_arp_mod)
	return offset;
}
#include <AL/al.h>
#include <AL/alc.h>
#include <map>
#include <sstream>
#include <vector>
class Sound : public sfxr {
public:
	//初始化OpenAL
	bool InitOpenAL() {
		// 注意：这里没有进行失败检测，仅用于示例
		ALCdevice *pDevice = alcOpenDevice(NULL); // 打开默认音频设备
		ALCcontext *pContext = alcCreateContext(pDevice, NULL);
		alcMakeContextCurrent(pContext);
		return true;
	}
	//关闭OpenAL
	void ShutdownOpenAL() {
		ALCcontext *pContext;
		ALCdevice *pDevice;
		pContext = alcGetCurrentContext();
		pDevice = alcGetContextsDevice(pContext);
		alcMakeContextCurrent(NULL);
		alcDestroyContext(pContext);
		alcCloseDevice(pDevice);
	}
	std::vector<ALuint> Sources;		// 用于播放声音
	std::map<long, ALuint> Buffers; // 声音数据
	//std::vector<ALuint> Playing;
	//载入数据
	bool LoadData() {
		return true;
	}
	//卸载数据
	void UnloadData() {
		for (auto &Source : Sources) {
			alSourceStop(Source);
			alSourcei(Source, AL_BUFFER, 0);
		}
		
		for (auto &Buffer : Buffers)
			alDeleteBuffers(1, &Buffer.second);
		alDeleteSources(Sources.size(), Sources.data());
		Buffers.clear();
		Sources.clear();
	}
	void Clear() { /*Playing.clear();*/ }
	void Play(long hash) {
		ALint state = AL_PLAYING;
		ALuint used = 0;
		for (auto &Source : Sources) {
			alGetSourcei(Source, AL_SOURCE_STATE, &state);
			if (state != AL_PLAYING) {
				used = Source;
				break;
			}
		}
		if (used == 0) {
			alGenSources(1, &used);
			if (0 == used)
				return;
			ALboolean loop = 0; // 循环播放
			// alGenSources(SOURCE, Sources);
			// 源声音的位置
			ALfloat SourcePos[] = {0.0, 0.0, 0.0};
			// 源声音的速度
			ALfloat SourceVel[] = {0.0, 0.0, 0.0};
			alSourcef(used, AL_PITCH, 1.0f);
			// alSourcef(Source, AL_GAIN, 1.0f);
			alSourcefv(used, AL_POSITION, SourcePos);
			alSourcefv(used, AL_VELOCITY, SourceVel);
			alSourcei(used, AL_LOOPING, loop);
			Sources.push_back(used);
			pass = Sources.size();
		}
		auto buff = Buffers.find(hash);
		ALuint temp;
		bool init = false;
		if (buff == Buffers.end()) {
			alGenBuffers(1, &temp);
			Buffers[hash] = temp;
			init = true;
		} else {
			temp = buff->second;
			// if (std::find(Playing.begin(), Playing.end(), temp) != Playing.end())return;
		}
		if (init) {
			alSourcei(used, AL_BUFFER, 0);
			static const ALsizei size = 147456 * sizeof(short); // 载入变量.
			static unsigned char data[size];
			/*static ALsizei length = 0;
			static unsigned char *data = nullptr;
			float buffer[size];
			float max = SHRT_MAX;
			int index = 0, total = 0;
			memset(buffer, 0, size * sizeof(float));*/
			PlaySample();
			int total = get(data, size);
			/*while (IsPlaying()) {
				int play = get( buffer,size);
				total += play;
				if (total > length) {
					length = total;
					data = (short *)realloc(data, length * sizeof(short));
				}
				int idx = 0;
				while (play--) {
					short &e = data[index++];
					e = (short)(max * buffer[idx++]);
				}
			}*/
			// 载入WAV数据
			alBufferData(temp, AL_FORMAT_MONO16, data, total, wav_freq);
			while (IsPlaying()) {
				total = get(data, size);
			}
			//++pass;
		}
		alSourcef(used, AL_GAIN, play_vol);
		// alSourceQueueBuffers(used, SOURCE, Buffers);
		alSourcei(used, AL_BUFFER, temp);
		alSourcePlay(used);
		play_vol = 1.0;
		//Playing.push_back(temp);
	}
	void Select(char *bytes) {
		ResetParams();
		Read(bytes, strlen(bytes));
	}
	void SetVol(float vol) { play_vol = vol; }
	int pass;
	float play_vol;
};
static Sound sound;
extern "C" {
#include "b64/cdecode.h"
static long calculateHash(const char *str, size_t len) {
	static std::locale loc;
	static const std::collate<char> &collate = std::use_facet<std::collate<char>>(loc);
	return collate.hash(str, str + len);
}
void Sound_init() {
	sound.InitOpenAL();
	sound.LoadData();
}
void Sound_clear() { sound.Clear(); }
int Sound_play(const char *data, long length, float vol) {
	if (vol < 0.1)
		return sound.pass;
	if (data) {
		if (strcmp("none", data) == 0)
			return ++sound.pass;
		base64_decodestate state;
		base64_init_decodestate(&state);
		char bytes[128];
		int len = base64_decode_block(data, length, bytes, &state);
		sound.Select(bytes);
	}
	if (vol > 1)
		vol = 1;
	sound.SetVol(vol);
	sound.Play(data ? calculateHash(data, length) : length);
	return sound.pass;
}
void Sound_exit() {
	sound.UnloadData();
	sound.ShutdownOpenAL();
}
#include "sound.h"
#include <lauxlib.h>
#include <lua.h>
#ifndef MAX
#define MAX(a, b) (a > b ? a : b)
#endif
#ifndef MIN
#define MIN(a, b) (a < b ? a : b)
#endif
long readTableIntoSFXRInstance(struct lua_State *L, int index) {
	sound.ResetParams();
	char code[4096] = {0};
#define SETUP_VALUE_NAMED(_n, _f, _min, _max)           \
	{                                                     \
		strcat(code, #_n);                                  \
		lua_pushliteral(L, #_n);                            \
		lua_gettable(L, index);                             \
		if (lua_isnumber(L, -1)) {                          \
			lua_Number num = lua_tonumber(L, -1);             \
			num = MAX((_min), MIN((_max), num));              \
			sound.Set##_f(num);                               \
			sprintf(code + strlen(code), "%s=%f;", #_n, num); \
		}                                                   \
		lua_pop(L, 1);                                      \
	}
#define SETUP_VALUE(_f) SETUP_VALUE_NAMED(_f, _f, -1, 1)
	{
		lua_pushliteral(L, "Waveform");
		lua_gettable(L, index);
		if (lua_isnumber(L, -1)) {
			lua_Integer waveform = lua_tointeger(L, -1);
			sound.SetWaveform(waveform);
		}
		lua_pop(L, 1);
	}
	SETUP_VALUE_NAMED(AttackTime, AttackTime, 0, 10);
	SETUP_VALUE_NAMED(SustainTime, SustainTime, 0, 10);
	SETUP_VALUE(SustainPunch);
	SETUP_VALUE_NAMED(DecayTime, DecayTime, 0, 10);
	SETUP_VALUE(StartFrequency);
	SETUP_VALUE(MinimumFrequency);
	SETUP_VALUE(Slide);
	SETUP_VALUE(DeltaSlide);
	SETUP_VALUE(VibratoDepth);
	SETUP_VALUE(VibratoSpeed);
	SETUP_VALUE(ChangeAmount);
	SETUP_VALUE(ChangeSpeed);
	SETUP_VALUE(SquareDuty);
	SETUP_VALUE(DutySweep);
	SETUP_VALUE(RepeatSpeed);
	SETUP_VALUE(PhaserSweep);
	SETUP_VALUE(LowPassFilterCutoff);
	SETUP_VALUE(LowPassFilterCutoffSweep);
	SETUP_VALUE(LowPassFilterResonance);
	SETUP_VALUE(HighPassFilterCutoff);
	SETUP_VALUE(HighPassFilterCutoffSweep);
	SETUP_VALUE_NAMED(Volume, SoundVolume, 0, 1);
#undef SETUP_VALUE
#undef SETUP_VALUE_NAMED
	return calculateHash(code, strlen(code));
}
}
