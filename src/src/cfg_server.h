/**
 * @file cfg_server.h
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
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "file_sys.h"

/**
 * @brief Configuration WWW severver
 * 
 */
class CfgServer {

private:

    const char*     _cssid = "ssid";        ///> form param  
    const char*     _cpass = "pass";        ///> form param
    const char*     _cservice = "service";  ///> form param
    const char*     _clat = "lat";  ///> form param
    const char*     _clon = "lon";  ///> form param
    const char*     _ckey = "apikey";  ///> form param
    
    AsyncWebServer*     _as    {nullptr};
    ItemFS*             _fs    {nullptr};

 public: 

    /**
     * @brief Construct a new Cfg Server object
     * 
     * @param fs 
     */
    explicit CfgServer(ItemFS* fs) : _fs(fs) {
    } 

    /**
     * @brief Destroy the Cfg Server object
     * 
     */
    ~CfgServer(){
        done();
    }

    /**
     * @brief 
     * 
     * @param port - listen port
     * @return true - success
     * @return false 
     */
    bool init(uint16_t port = 80) {
        bool rc = false;
        do {
           if (_as)  done(); 
           _as = new AsyncWebServer(port);
           if (_as==nullptr) break;
         } while (false);
         return rc;
    }

    /**
     * @brief down server
     * 
     */
    void done() {
        if (_as) delete(_as);
        _as = nullptr; 
    }

    /**
     * @brief main cfg page & working with responses
     * 
     */
    void serveCfgPage() {
        do {
      
            if (!_as) break;
 
            _as->on("/", HTTP_GET, [this](AsyncWebServerRequest *request){
                request->send(*_fs->getFS(), "/index.html", "text/html");
            });

            _as->serveStatic("/", *_fs->getFS(), "/");

            _as->on("/", HTTP_POST, [this] (AsyncWebServerRequest *request) {
                auto params = request->params();
                for(auto i=0; i<params; i++) {
                    AsyncWebParameter* p = request->getParam(i);
                    if(p->isPost()){  
                        // HTTP POST ssid value
                        if (p->name() == _cssid) {
                            _fs->writeItem(ItemFS::Data::ssid,p->value().c_str());
                            
                        }

                        if (p->name() == _cpass) {
                            _fs->writeItem(ItemFS::Data::password, p->value().c_str());
                            
                        }

                        if (p->name() == _cservice) {
                            _fs->writeItem(ItemFS::Data::ip, p->value().c_str());
                            
                        }

                        if (p->name() == _clat) {
                            _fs->writeItem(ItemFS::Data::lat, p->value().c_str());
                            
                        }

                        if (p->name() == _clon) {
                            _fs->writeItem(ItemFS::Data::lon, p->value().c_str());
                            
                        }

                        if (p->name() == _ckey) {
                            _fs->writeItem(ItemFS::Data::apikey, p->value().c_str());
                            
                        }
                        
                    }
                }

                request->send(200, "text/plain", "Done. Restart....");
                delay(3000);
                ESP.restart();

            });

            _as->begin();
          

        } while(false);
    }

   
    
};