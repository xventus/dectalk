/**
 * @file S1V30120_const.h
 * @author Petr Vanek (petr@fotoventus.cz)
 * @brief Definitions and constants for the circuit S1V30120 from  "S1V30120 Protocol Specification.pdf"
 * @version 0.1
 * @date 2022-08-29
 * 
 * @copyright Copyright (c) 2022 Petr Vanek (petr@fotoventus.cz)
 * 
 *  
 */

#pragma once

/** Boot Mode Messages && System Messages **/

// Test & register host hw interface
#define ISC_TEST_REQ 			0x0003
#define ISC_TEST_RESP 			0x0004

// Version Request Message
#define ISC_VERSION_REQ			0x0005
#define ISC_VERSION_RESP		0x0006

// Loads code image into SRAM
#define ISC_BOOT_LOAD_REQ		0x1000
#define ISC_BOOT_LOAD_RESP		0x1001

// Start executing image
#define ISC_BOOT_RUN_REQ 		0x1002
#define ISC_BOOT_RUN_RESP 		0x1003


/** System Messages **/ 

// Configure Audio Output
#define ISC_AUDIO_CONFIG_REQ 	0x0008
#define ISC_AUDIO_CONFIG_RESP   0x0009

// Set volume (analogue gain)
#define ISC_AUDIO_VOLUME_REQ	0x000A
#define ISC_AUDIO_VOLUME_RESP	0x000B

// Mute audio output
#define ISC_AUDIO_MUTE_REQ		0x000C
#define ISC_AUDIO_MUTE_RESP		0x000D

// Fatal error indication
#define ISC_ERROR_IND			0x0000

// Request blocked
#define ISC_MSG_BLOCKED_RESP	0x0007


/** TTS Messages **/  

// Configure the TTS
#define ISC_TTS_CONFIG_REQ		0x0012
#define ISC_TTS_CONFIG_RESP		0x0013

// Start the TTS (optionally attach data)
#define ISC_TTS_SPEAK_REQ 		0x0014
#define ISC_TTS_SPEAK_RESP 		0x0015

// TTS ready for more data
#define ISC_TTS_READY_IND       0x0020

// TTS output is finished
#define ISC_TTS_FINISHED_IND    0x0021

// Pause the TTS output
#define ISC_TTS_PAUSE_REQ       0x0016
#define ISC_TTS_PAUSE_RESP      0x0017

// Stop TTS immediately
#define ISC_TTS_STOP_REQ        0x0018
#define ISC_TTS_STOP_RESP       0x0019

// Send user dictionary data to the device
#define ISC_TTS_UDICT_DATA_REQ  0x00CE
#define ISC_TTS_UDICT_DATA_RESP 0x00D0


/** System Messages **/ 

// Register / De-register for control of GPIO interface
#define ISC_GPIO_REGISTER_REQ   0x0045
#define ISC_GPIO_REGISTER_RESP  0x0046

// Define the GPIO output types
#define ISC_GPIO_OUTPUT_CONFIG_REQ      0x004E
#define ISC_GPIO_OUTPUT_CONFIG_RESP     0x004F

// Configure GPIO output value
#define ISC_GPIO_OUTPUT_SET_REQ         0x0050
#define ISC_GPIO_OUTPUT_SET_RESP        0x0051



/** Presets - ISC_TTS_CONFIG_REQ **/ 

// tts_sample_rate - 0x00 : reserved
#define TTS_CONFIG_SAMPLE_RATE		0x01

// tts_voice - 0x00 : Voice 0 ... 0x08 : Voice 9 (custom voice)
#define TTS_CONFIG_VOICE			0x00

// tts_epson_parse - 0x00 : Disable / 0x01 : Enable
#define TTS_CONFIG_EPSON_PARSE  	0x01
#define TTS_CONFIG_DEC_PARSE  	    0x00

// tts_languagec - 0x00 : US English, 0x01 : Castilian Spanish, 0x04 : Latin Spanish
#define TTS_CONFIG_LANGUAGE  		0x00

// tts_speaking_rate (lsb) - Speaking rate in words per minute
// The valid range is 0x004B to 0x0258.
// A suitable default value is 200 words/min (0x00C8)
#define TTS_CONFIG_SPEAK_RATE_LSB	0xC8
#define TTS_CONFIG_SPEAK_RATE_MSB	0x00

// tts_datasource Set to 0x00
#define TTS_CONFIG_DATASOURCE		0x00



/** Presets - SC_AUDIO_CONFIG_REQ **/ 

//  audio_stereo - 0x00 : Mono
#define AUDIO_CONFIG_STEREO 0x00

// audio_gain - 0x00 : Mute -> 0x43 : +18dB
#define AUDIO_CONFIG_GAIN 	0x43

// audio_amp - 0x00 : Not Selected
#define AUDIO_CONFIG_AMP	0x00

// audio_sample_rate - 0x00 : 8KHz, 0x01 : 11.025kHz .. 
#define AUDIO_CONFIG_ASR 	0x01

// audio_routing - 0x00 : Application to DAC
#define AUDIO_CONFIG_AR 	0x00

// audio_tone_control - DEPRECIATED, set to 0
#define AUDIO_CONFIG_ATC 	0x00

// audio_clock_source - 0x00: Internally generated audio clock
#define AUDIO_CONFIG_ACS 	0x00

// DAC_permanently_on - 0x00: DAC is on only while speech decoder or TTS synthesis is outputting audio.
#define AUDIO_CONFIG_DCA 	0x00

