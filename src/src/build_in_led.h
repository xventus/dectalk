
/**
 * @file build_in_led.h
 * @author Petr Vanek (petr@fotoventus.cz)
 * @brief configuration carrier
 * @version 0.1
 * @date 2022-11-26
 * 
 * @copyright Copyright (c) 2022 Petr Vanek
 * 
 */


#pragma once

#include <Arduino.h>

class BuildInLed {
public:
    enum class State { on, off, blink, error, connecting};

private:

   int              _led {2};
   State            _state {State::off};
   unsigned long    _previous {0};
   unsigned long    _interval {0};
   int              _ledState {LOW};

public:

    BuildInLed(int led) : _led(led) {
        pinMode(_led, OUTPUT);
    } 

    void setState(State st) {
        _state = st;
        switch (_state) 
        {
            case State::connecting:
              _interval = 400;
            break;

            case State::blink:
                _interval = 1000;
            break;

            case State::error:
                _interval = 50;
            break;
        }

    }

    void update() {

        if (_state == State::error || _state == State::blink || _state == State::connecting) {
            auto  current = millis();
            if (current - _previous >= _interval) {

                if (_state == State::connecting) {
                    if (_ledState == LOW) _previous = current + (_interval/2);
                    else _previous = current;
                } else {
                    _previous = current;
                }

                if (_ledState == LOW) {
                    _ledState = HIGH;
                } else {
                    _ledState = LOW;
                }

                digitalWrite(_led, _ledState);
            }
        } else {
            if (_state == State::off) digitalWrite(_led, LOW);
            else if (_state == State::on) digitalWrite(_led, HIGH);
        }
    }


};