
/**
 * @file dblreset.h
 * @author Petr Vanek (petr@fotoventus.cz)
 * @brief  SPIFFS Filesystem
 * @version 0.1
 * @date 2022-04-05
 * 
 * @copyright Copyright (c) 2022 Petr Vanek
 * 
 */

#pragma once
#include <Arduino.h>
#include "file_sys.h"
#include <esp_system.h>

class DblReset {
public:
   

private:
    const int32_t _on  {0x006A0F55};  //6950741
    const int32_t _off {0x0FA65501};  //262558977

    const uint32_t _startupTime {5000};
    bool           _waitForDbl  {false};
    ItemFS*        _fs          {nullptr};
       
    
public:

    DblReset(ItemFS* fs): _fs(fs) {
    }

    void stop() {
	    reset();
	    _waitForDbl = false;
    }


    void update() {
        if (_waitForDbl && millis() > _startupTime) {
           _waitForDbl = false;
           reset();
        }
    } 

    bool isDblRestActivated() {

        auto detected = isSet();
        if (detected) {
            reset();
        } else {
            set();
            _waitForDbl = true;
        }
	    return detected;
    } 

private:

    bool isSet() {
        uint32_t rc = 0;
        if (_fs)  rc = _fs->readInt(ItemFS::Data::dblrst);
        return rc == _on;
    }

    void set() {
        if (_fs)  _fs->writeInt(ItemFS::Data::dblrst,_on);
    }

    void reset() {
         if (_fs)  _fs->writeInt(ItemFS::Data::dblrst,_off);
    }

};