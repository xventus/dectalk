/**
 * @file ap.h
 * @author Petr Vanek (petr@fotoventus.cz)
 * @brief AP & config Server 
 * @version 0.1
 * @date 2022-04-05
 * 
 * @copyright Copyright (c) 2022 Petr Vanek
 * 
 */

#pragma once

#include <WiFi.h>

/**
 * @brief encapsulates the access point
 * 
 */
class AP {

private:
    const char *ssid = "DECTALK CONFIG";     ///>  hard coded SSID
    const char *passphrase = nullptr;    ///>  hard coded password, if required, min. 8, max. 63 characters, optional
    IPAddress _local;                    ///>  AP IP
    IPAddress _gateway;                  ///>  gateway IP
    IPAddress _subnet;                   ///>  net mask

 public: 

    /**
     * @brief return name of ssid
     * 
     * @return const char*  ssid
     */
    const char* getSSID() const {
        return ssid;
    }

    /**
     * @brief Initialisation AP
     * 
     * @param ip - IP address of the device
     * @param gtw - gateway
     * @param net - net mask
     * @return true - success
     * @return false 
     */
    bool init(const char* ip="192.168.4.100", const char* gtw="192.168.4.1", const char* net="255.255.255.0") {
        bool rc = false;
        do {
            if (!_local.fromString(ip)) break;
            if (!_gateway.fromString(gtw)) break;
            if (!_subnet.fromString(net)) break;
            if (!WiFi.softAP(ssid,passphrase)) break;
            if (!WiFi.softAPConfig(_local, _gateway, _subnet)) break;

            rc = true;
         } while (false);
         return rc;
    }

    /**
     * @brief Address of the connection point
     * 
     * @return IPAddress 
     */
    IPAddress getIP() {
        return WiFi.softAPIP();
    }
};