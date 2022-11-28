
/**
 * @file file_sys.h
 * @author Petr Vanek (petr@fotoventus.cz)
 * @brief  SPIFFS Filesystem
 * @version 0.1
 * @date 2022-04-05
 * 
 * @copyright Copyright (c) 2022 Petr Vanek
 * 
 */

#pragma once

#include "FS.h"
#include "SPIFFS.h"

/**
 * @brief easy file sys wrapper, SPIFFS 
 * |--------------|-------|---------------|--|--|--|--|--|
 * ^              ^       ^               ^     ^
 * Sketch    OTA update   File system   EEPROM  WiFi config (SDK) 
 * 
 */
class ItemFS {
public:
    enum class Data { dblrst, ssid, password, ip, lat, lon, apikey };

private:
    const char* _dblrst = "/dblrst.txt";
    const char* _cssid = "/ssid.txt";
    const char* _cpasswd = "/password.txt";
    const char* _cip = "/ip.txt";
    const char* _cindex = "/index.html";
    const char* _cstyle = "/style.css";
    const char* _clat = "/lat.txt";
    const char* _clon = "/lon.txt";
    const char* _capikey = "/apikey.txt";
    
public:

    /**
    * @brief initialize file system
    * 
    * @return true - OK
    * @return false - the file system had to be formatted, the data will not be available
    */
    bool init() {
        return SPIFFS.begin(false);
    } 

    /**
     * @brief close FS
     * 
     */
    void done() {
        SPIFFS.end();
    } 

    /**
     * @brief 
     * 
     * @return true 
     * @return false 
     */
    bool isRequiredFileExists() {
       return ( SPIFFS.exists(_cindex) && SPIFFS.exists(_cstyle));
    }

    /**
     * @brief gets file system class
     * 
     * @return fs::FS* 
     */
    fs::FS* getFS() {
        return &SPIFFS;
    }

    /**
     * @brief mapping file item to file path
     * 
     * @param fileItem 
     * @return const char* 
     */
    const char* item2Path(Data fileItem) {
        const char* path = nullptr;
        switch (fileItem) {
            case Data::dblrst: path = _dblrst; break;
            case Data::ssid: path = _cssid; break;
            case Data::password: path = _cpasswd; break;
            case Data::ip: path = _cip; break;
            case Data::lat: path = _clat; break;
            case Data::lon: path = _clon; break;
            case Data::apikey: path = _capikey; break;
        }
        return path;
    }
 
    /**
     * @brief read file item content
     * 
     * @param fileItem - file item
     * @return String - file path
     */
    String readItem(Data fileItem){
        String rc;
        const char* path = item2Path(fileItem);
        if (path) {
            File file = SPIFFS.open(path);
            if(file) {
                 while(file.available()){
                    rc += file.readString();
                    break;     
                }
            }
            if (file) file.close();
        }
        // Serial.printf("READ ITEM  [%s]  [%s]  %s\n", path, rc.c_str(), (rc)?"OK":"ERROR");
        return rc;
    }

    

    /**
     * @brief write file item ito FS
     * 
     * @param fileItem - file item
     * @param content - content of file
     * @return true - succes
     * @return false - operation failed
     */
    bool writeItem(Data fileItem, const char * content){
        bool rc = false;
        const char* path = item2Path(fileItem);
        if (path) {
            File file = SPIFFS.open(path, FILE_WRITE);
            do {
                if (!file) {
                    break;
                }

                if (!file.print(content)) {
                    break;
                } 
       
                rc = true;
            } while(false);

            if (file) file.close();
        }

        //Serial.printf("WRITE ITEM  [%s]  [%s]  %s\n", path, content, (rc)?"OK":"ERROR");
        return rc;
    }

    

bool writeInt(Data fileItem, int32_t value) {
  bool rc = false;
  const char* path = item2Path(fileItem);
  File file = SPIFFS.open(path, FILE_WRITE);
  if (file) {
    char buff[10];
    snprintf(buff, sizeof(buff), "%i\n", value); 
    //Serial.printf("writeInt  %s\n", buff);
    rc = file.print(buff);
    file.close();
  }
  return rc;
}

int32_t readInt(Data fileItem) {
  int32_t rc = 0;
  const char* path = item2Path(fileItem);
  File file = SPIFFS.open(path);
  if (file) {
    char buff[10]; 
    int x = file.readBytesUntil('\n', buff, sizeof(buff) - 1);
    buff[x] = 0; 
    //Serial.printf("readInt  [%s]  \n",  buff);
    rc = atoi (buff);
    file.close();
  }

 
  return (rc);
}

    /**
     * @brief dignostic dump of FS
     * 
     */
    void dumpFiles() {
 
        File root = SPIFFS.open("/");       
        File file = root.openNextFile();
        while(file){
            Serial.printf("#FILE: %s\n", file.name());
            file = root.openNextFile();
        }
 
    }

};