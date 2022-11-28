/**
 * @file S1V30120.h
 * @author Petr Vanek (petr@fotoventus.cz)
 * @brief Class for voice synthesizer control
 * @version 0.1
 * @date 2022-08-29
 *
 * @copyright Copyright (c) 2022 Petr Vanek (petr@fotoventus.cz)
 *
 *
 */
#pragma once

#include <inttypes.h>
#include <string.h>
#include "S1V30120_const.h"
#include "S1V30120_init_data.h"
#include <SPI.h>
#include <mutex>

class S1V30120
{

public:

    static const uint16_t maximumMsgSize = 248;//121;
    static const uint16_t maximumBufferSize = maximumMsgSize + 7;

    /**
     * @brief Construct a new S1V30120 object
     *
     * @param spi   - SPI object
     * @param resetPin  - reset pin - NRESET
     * @param rdyPin    - ready pin - GPIOA3
     * @param mutePin   - mute aplifier, e.g. LM386 - gain
     */
    explicit S1V30120(SPIClass* spi, uint8_t resetPin, uint8_t rdyPin, uint8_t mutePin) : _spi(spi),
                                                                                         _resetPin(resetPin),
                                                                                         _rdyPin(rdyPin),
                                                                                         _mutePin(mutePin)
    {
        pinMode(_spi->pinSS(), OUTPUT); // CS
        pinMode(_resetPin, OUTPUT);     // RESET
        pinMode(_rdyPin, INPUT);        // RDY
        pinMode(_mutePin, OUTPUT);      // MUTE
    }

    
    /**
     * @brief S1V30120 initialization and firmware upload. Repeated calls are possible.
     * 
     * @param epson  - true - Epson parse, false DECtalk
     * @return true if success 
     * @return false 
     */
    bool init(bool epson = false)
    {
        auto rc = false;
        std::lock_guard<std::mutex> lck(_mtx);
        do
        {
            reset(); 

            // verify IC availability and find out the HW version
            if (!version()) 
                break;

            // upload firmware
            if (!uploadFW())
                break;

            // run firmware
            if (!run()) 
                break;   

            // registration 
            if (!test()) 
                break;   

            // check FW version
            if (!version()) 
                break;  

            // audio configuration
            if (!audioCfg()) 
                break; 

            // maximum volume
            if (! maxVolume()) 
                break; 
 
            // TTS
            if (! setupTTS(epson)) 
                break; 

            rc = true;
        } while (false);
        return rc;
    }

// 0x00 – flush disabled – this string is spoken after any previous speak requests have been finished. 
// 0x01 – flush enabled – this TTS output is flushed and the current string is spoken immediately
    
    /**
     * @brief 
     * 
     * @param text  - the text of the message
     * @param flush -  true =  this TTS output is flushed and the current string is spoken immediately
     *                 false= this string is spoken after any previous speak requests have been finished
     * @return true 
     * @return false 
     */

    /// @brief plays a text message with a maximum length of maximumMsgSize characters
    /// @param text the text of the message
    /// @param mute  true - muted
    /// @param flush true =  this TTS output is flushed and the current string is spoken immediately
    ///              false= this string is spoken after any previous speak requests have been finished
    /// @return true / false
    bool speak(const String &text, bool mute = false, bool flush = true)
    {
        std::lock_guard<std::mutex> lck(_mtx);
        
        if (text.isEmpty()) return true;

        _inaction = true;
        
        digitalWrite(_mutePin, mute);
        
        memset(_buffer,0,sizeof(_buffer));
        auto sz = text.length();
        if (sz > maximumMsgSize) sz = maximumMsgSize;
        auto len = sz + 6;
        _buffer[0] = len & 0xFF;          
        _buffer[1] = (len & 0xFF00) >> 8; 
        _buffer[2] = ISC_TTS_SPEAK_REQ & 0xFF;
        _buffer[3] = (ISC_TTS_SPEAK_REQ & 0xFF00) >> 8;
        _buffer[4] = flush ? 0x01 : 0x00; 

        for (auto i = 0; i < sz; i++)
        {
            _buffer[i+5] = text[i];
        }

        sendMsg((uint8_t*) _buffer, len);
        return checkResponse(ISC_TTS_SPEAK_RESP, 0x0000, 16);
    }

    /// @brief is already speak finished
    /// @return 
    bool isFinished() {
        std::lock_guard<std::mutex> lck(_mtx);
        _inaction = !checkResponse(ISC_TTS_FINISHED_IND, 0x0000, 16); 
        return !_inaction; 
    }

    bool isRunning() {
        std::lock_guard<std::mutex> lck(_mtx);
        return _inaction;
    }

    /// @brief hardware version
    /// @return version or zero if failed
    uint16_t getHWVersion() const
    {
        return _versionHW;
    }

    /// @brief firmware version
    /// @return version or zero if failed
    uint16_t getFWVersion() const
    {
        return _versionFW;
    }

    uint32_t getFWFeatures() const
    {
        return _versionFWFeatures;
    }

private:
    /// @brief reset S1V30120 and init SPI CLK
    void reset()
    {
        digitalWrite(_mutePin, false);
        digitalWrite(_spi->pinSS(), HIGH);
        digitalWrite(_resetPin, LOW);
        _spi->beginTransaction(_spiSetting);
        _spi->transfer(0x00);
        _spi->endTransaction();
        delay(10);
        digitalWrite(_resetPin, HIGH);
        delay(150);
    }

    /// @brief S1V30120 detection of firmware presence and version
    /// @return
    bool version()
    {
        sendMsg(_verReq, 0x04);
        while (digitalRead(_rdyPin) == 0);
        digitalWrite(_spi->pinSS(), LOW);
        _spi->beginTransaction(_spiSetting);
        while (_spi->transfer(0x00) != 0xAA);

        for (auto i = 0; i < 20; i++)
        {
            _buffer[i] = _spi->transfer(0x00);
        }

        sendPadding(16);
        _spi->endTransaction();
        digitalWrite(_spi->pinSS(), HIGH);
        _versionHW = _buffer[4] << 8 | _buffer[5];
        _versionFW = _buffer[6] << 8 | _buffer[7];
        _versionFWFeatures =  (_buffer[11] << 24) | (_buffer[10] << 16) | (_buffer[9] << 8) | _buffer[8];

        if (_versionHW != 0x0402)
        {
            _versionHW = 0;
            _versionFW = 0;
            _versionFWFeatures = 0;
            return false;
        }
        return true;
    }

    /// @brief wait for ready and send
    /// @param data
    /// @param len
    void sendMsg(const uint8_t data[], uint8_t len)
    {
        while (digitalRead(_rdyPin) == 1);
        digitalWrite(_spi->pinSS(), LOW);
        delay(200);
        _spi->beginTransaction(_spiSetting);
        _spi->transfer(0xAA);
        for (auto i = 0; i < len; i++)
        {
            _spi->transfer(data[i]);
        }
        _spi->endTransaction();
    }

    /// @brief send padding zeros
    /// @param len  - number of zeros
    void sendPadding(uint8_t len)
    {
        for (auto i = 0; i < len; i++)
        {
            _spi->transfer(0x00);
        }
    }


    /// @brief check response
    /// @param msg 
    /// @param result checked response code ISC_XXXXXX_RESP
    /// @param padding additional padding
    /// @return true - success
    bool checkResponse(uint16_t msg, uint16_t result, uint16_t padding)
    {
        auto rc = false;

        while (digitalRead(_rdyPin) == 0)
            ;
        digitalWrite(_spi->pinSS(), LOW);
        delay(20);
        _spi->beginTransaction(_spiSetting);
        while (_spi->transfer(0x00) != 0xAA)
            ;
        for (auto i = 0; i < 6; i++)
        {
            _buffer[i] = _spi->transfer(0x00);
        }
        sendPadding(padding);
        _spi->endTransaction();
        digitalWrite(_spi->pinSS(), HIGH);

        uint16_t val = _buffer[3] << 8 | _buffer[2];
        if (val == msg)
        {
            val = _buffer[5] << 8 | _buffer[4];
            if (val == result)
                rc = true;
        }
        return rc;
    }

    /// @brief run firmware
    /// @return true - success
    bool run()
    {
        sendMsg(_runReq, 0x04);
        return checkResponse(ISC_BOOT_RUN_RESP, 0x0001, 8);
    }

    /// @brief Test & register host hw interface
    /// @return true - sucess
    ///          false - failed. 
    ///  Note: cen be clocked if init data is invalid !!!
    bool test()
    {
        sendMsg(_testReq, 0x0C);
        return checkResponse(ISC_TEST_RESP, 0x0000, 16);
    }


    /// @brief upload firmware (init data) S1V30120_INIT_DATA_ver2_1_6
    /// @return true - success
    bool uploadFW()
    {
        auto rc = true;
        uint16_t initDataSize = sizeof(S1V30120_INIT_DATA_ver2_1_6);
        uint16_t numberOfBlocks = initDataSize / _msgsize;
        uint16_t lastBlockSize = initDataSize - (numberOfBlocks * _msgsize);
        uint16_t pos = 0;
        for (auto part = 0; part < numberOfBlocks; part++)
        {
            if (!uploadPart(pos, _msgsize))
            {
                rc = false;
                break;
            }
            pos += _msgsize;
        }

        if (!uploadPart(pos, lastBlockSize))
        {
            rc = false;
        }
        return rc;
    }


    /// @brief Loads the block into the IC. The block size is limited.
    /// The recommended size for these blocks is _msgsize (2048 - 4 head) bytes.
    /// @param fromPos position of init data (S1V30120_INIT_DATA_ver2_1_6)
    /// @param len size of sneding block, usem max. _msgsize
    /// @return true - success
    bool uploadPart(uint16_t fromPos, uint16_t len)
    {
        digitalWrite(_spi->pinSS(), LOW);
        delay(20);
        _spi->beginTransaction(_spiSetting);
        _spi->transfer(0xAA);
        _spi->transfer((len + 4) & 0xFF);
        _spi->transfer(((len + 4) & 0xFF00) >> 8);
        _spi->transfer(0x00);
        _spi->transfer(0x10);

        for (auto i = 0; i < len; i++)
        {
            _spi->transfer(S1V30120_INIT_DATA_ver2_1_6[fromPos + i]);
        }

        _spi->endTransaction();
        delay(1);
        digitalWrite(_spi->pinSS(), HIGH);
        return checkResponse(ISC_BOOT_LOAD_RESP, 0x0001, 16);
    }

    /// @brief 
    /// @return 
    bool audioCfg()
    {
        sendMsg(_audioReq, 0x0C);
        return checkResponse(ISC_AUDIO_CONFIG_RESP, 0x0000, 16);
    }

    /// @brief 
    /// @return 
    bool maxVolume()
    {
        sendMsg(_volumeMaxReq, 0x06);
        return checkResponse(ISC_AUDIO_VOLUME_RESP, 0x0000, 16);
    }


    /// @brief 
    /// @param epson - use Epson or Dectalk fotmat
    /// @return 
    bool setupTTS(bool epson)
    {
        sendMsg(epson?_ttsReqEpson:_ttsReqDec, 0x0C);
        return checkResponse(ISC_TTS_CONFIG_RESP, 0x0000, 16);
    }

private:
    bool       _inaction {false};   // if true in processor progress
    std::mutex _mtx;         // exclusive access   
    char _buffer[maximumBufferSize];       // temporary buffer
    uint16_t _versionHW{0};  // version HW
    uint16_t _versionFW{0};  // version FW
    uint32_t _versionFWFeatures{0}; // vertion FW features
    SPIClass *_spi{nullptr}; // SPI communication
    uint8_t _resetPin{0};
    uint8_t _rdyPin{0};
    uint8_t _mutePin{0};
    const uint16_t _msgsize{2044}; // The size of the message should not exceed 2048 bytes (minus header)
    const SPISettings _spiSetting{750000, MSBFIRST, SPI_MODE3};

    // messages defs.
    const uint8_t _testReq[12] = {0x0C, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t _verReq[4] = {0x04, 0x00, 0x05, 0x00};
    const uint8_t _runReq[4] = {0x04, 0x00, 0x02, 0x10};
    const uint8_t _ttsReqEpson[12] = {0x0C, 0x00, ISC_TTS_CONFIG_REQ & 0xFF, (ISC_TTS_CONFIG_REQ & 0xFF00) >> 8,
                                 TTS_CONFIG_SAMPLE_RATE, TTS_CONFIG_VOICE, TTS_CONFIG_EPSON_PARSE, TTS_CONFIG_LANGUAGE,
                                 TTS_CONFIG_SPEAK_RATE_LSB, TTS_CONFIG_SPEAK_RATE_MSB, TTS_CONFIG_DATASOURCE, 0x00};
     const uint8_t _ttsReqDec[12] = {0x0C, 0x00, ISC_TTS_CONFIG_REQ & 0xFF, (ISC_TTS_CONFIG_REQ & 0xFF00) >> 8,
                                 TTS_CONFIG_SAMPLE_RATE, TTS_CONFIG_VOICE, TTS_CONFIG_DEC_PARSE, TTS_CONFIG_LANGUAGE,
                                 TTS_CONFIG_SPEAK_RATE_LSB, TTS_CONFIG_SPEAK_RATE_MSB, TTS_CONFIG_DATASOURCE, 0x00};
    const uint8_t _volumeMaxReq[6] = {0x06, 0x00, 0x0A, 0x00, 0x00, 0x00};
    const uint8_t _audioReq[13] = {0x0C, 0x00, ISC_AUDIO_CONFIG_REQ & 0xFF, (ISC_AUDIO_CONFIG_REQ & 0xFF00) >> 8,
                                   AUDIO_CONFIG_STEREO, AUDIO_CONFIG_GAIN, AUDIO_CONFIG_AMP, AUDIO_CONFIG_ASR, AUDIO_CONFIG_AR,
                                   AUDIO_CONFIG_ATC, AUDIO_CONFIG_ACS, AUDIO_CONFIG_DCA, 0x00};
};
