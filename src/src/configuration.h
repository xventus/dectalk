
/**
 * @file configuration.h
 * @author Petr Vanek (petr@fotoventus.cz)
 * @brief configuration carrier
 * @version 0.1
 * @date 2022-04-05
 * 
 * @copyright Copyright (c) 2022 Petr Vanek
 * 
 */


#pragma once

#include <Arduino.h>

/**
 * @brief basic configuration structure
 * 
 */
struct Configuration {
    String ssid;
    String pass;
    String ip;
    String lat;
    String lon;
    String key;
};