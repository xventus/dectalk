/**
 * @file talk_server.h
 * @author Petr Vanek (petr@fotoventus.cz)
 * @brief Talk server 
 * @version 0.1
 * @date 2022-11-27
 * 
 * @copyright Copyright (c) 2022 Petr Vanek
 * 
 */

#pragma once

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "file_sys.h"
#include "build_in_led.h"

/**
 * @brief Talk WWW severver
 * 
 */
class TalkServer {

private:

    AsyncWebServer*     _as    {nullptr};
    ItemFS*             _fs    {nullptr};
    S1V30120*           _talker{nullptr};
    BuildInLed*         _binled{nullptr};  

    const char*         _talkstr = "talk";   
    const char*         _txtstr  = "text/html";
    const char*         _txtplainstr  = "text/plain";
    const char*         _talkhtmstr  = "/talk.html";

 public: 

    /**
     * @brief Construct a new Talk server object
     * 
     * @param fs 
     */
    explicit TalkServer(ItemFS* fs, S1V30120 *talker, BuildInLed*  binled) : 
        _fs(fs), 
        _talker(talker), 
        _binled(binled) {
    } 

    /**
     * @brief Destroy the Talk Server object
     * 
     */
    ~TalkServer(){
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

    /// @brief non blocking speach
    /// @param txt test to speach
    /// @return 
    bool nonBlockingTalk(const String& txt) {
        bool rc = false;
        if (_talker && _binled && !txt.isEmpty() && txt.length() <= _talker->maximumMsgSize) {
            if (!_talker->isRunning()) {
                _talker->speak(txt, false, true);
                _binled->setState(BuildInLed::State::on);
                rc = true;
            }
            _binled->update();
        }
        return rc;
    }

    /// @brief main TALK & working with responses
    void serveTalkPage() {
        do {
      
            if (!_as) break;
            if (!_talker) break;
            if (!_binled) break;
            if (!_fs) break;
            
            // root same as /talk 
            _as->on("/", HTTP_GET, [this](AsyncWebServerRequest *request){
                request->send(*_fs->getFS(), _talkhtmstr, _txtstr);
            });

            // for static access
            _as->serveStatic("/", *_fs->getFS(), "/");

            // specific talk POST page
            _as->on("/talk", HTTP_POST, [this] (AsyncWebServerRequest *request) {
                auto parnum = request->params();
                for(auto i=0; i<parnum; i++) {
                    AsyncWebParameter* p = request->getParam(i);
                    if(p->isPost()){  
                        if (p->name()==_talkstr) {
                            // ignore RC
                            nonBlockingTalk(p->value());
                            break;
                        }
                    }
                }
                request->send(*_fs->getFS(), _talkhtmstr, _txtstr);               
            });

            // specific talk GET page
            _as->on("/talk", HTTP_GET, [this] (AsyncWebServerRequest *request) {
                auto isOK= false;
                auto parnum = request->params(); 
                for(auto i=0; i<parnum; i++) {
                    AsyncWebParameter* p = request->getParam(i); 
                    if (p->name()==_talkstr) {
                        isOK = nonBlockingTalk(p->value());
                        break;
                    }  
                }

                request->send(200, _txtplainstr, isOK?"OK":"ERROR");
                
            });

            _as->begin();
        } while(false);
    }   
};