#include "SPIFFS.h"
#include "esp_onboarding.h"
#include "esp_config.h"

boolean initialized = false;

// Prototypes
char* getNextLine(File file);
char* getKey(char* line);
char* getValue(char* line);
boolean lineMatchesKey(char* line, char* key);
static char* getSsidFromValue(char* value);
static char* getWifiPasswordFromValue(char* value);
boolean keyValueExists(char* key, char* value);
boolean saveKeyValue(char* key, char* value, boolean replaceExisting);
void deleteKeyValue(char* key, char* value);
void deleteKey(char* key);

/**
 *  espConfigInit
 */
boolean espConfigInit(void) {
  boolean status = false;
      
  if (!initialized) {
//    Serial.printf("\nespConfigInit: Initializing\n");
//    delay(1000);
    
    if (SPIFFS.begin(true)){
      if(!SPIFFS.exists(CONFIG_FILE_NAME)) {
        Serial.println("Config file doesn't exist. Creating it...");
        delay(1000);
        File file = SPIFFS.open(CONFIG_FILE_NAME, FILE_WRITE);
        if (file){
          initialized = true;
          status = true;
          file.close();
        } else {
          Serial.print("Failed to open file ");
          Serial.print(CONFIG_FILE_NAME);
          Serial.println(" for reading.");
        }
      } else {
        Serial.println("espConfigInit: Config file exists");
        delay(1000);
        initialized = true;
        status = true;
      }
    } else {
      Serial.println("An Error has occurred while mounting SPIFFS");
      delay(1000);
    }
  } else {
//    Serial.println("espConfigInit: Already initialized");
//    delay(1000);
    status = true;
  }
//  Serial.println("espConfigInit: Finished");
//  delay(1000);
  return status;
}

char* getNextLine(File file) {
  // TODO: ignore blank lines
  boolean eolDetected = false;
  char* line = (char*)malloc(MAX_LINE_SIZE * sizeof(char));
  int index = 0;
  char tempBuf[8];
  
  while (file.available() && !eolDetected) {
    char currentChar = file.read();
//    sprintf(tempBuf, "0x%02x", currentChar);
//    Serial.print("getNextLine: Read character: [");
//    Serial.print(currentChar);
//    Serial.print("], hex:[");
//    Serial.print(tempBuf);
//    Serial.println("]");
    line[index] = currentChar;

//    file.read((uint8_t*)&line[index], sizeof(char));
    if (currentChar == '\n' || currentChar == '\0' || currentChar=='\r') {
      line[index] = '\0';
      eolDetected = true;
    }
    if (eolDetected) {
//      Serial.println("getNextLine: End of line detected.");
    }
    index++;
  }
  if (index == 0) {
    free(line);
    line = NULL;
  }
//  Serial.printf("getNextLine: line size: %d\n", strlen(line));
  return line;
}

char* getKey(char* line) {
  char tempBuf[256];
  strncpy(tempBuf, line, sizeof(line)+1);
  return strtok(tempBuf, KVP_DELIMITER);
}

char* getValue(char* line) {
  char* result = NULL;
  
  if (line != NULL) {
  char* tempBuf = (char*)malloc(MAX_LINE_SIZE * sizeof(char));
    strcpy(tempBuf, line);
    char* foo = strtok(tempBuf, KVP_DELIMITER);
    foo = strtok(NULL, KVP_DELIMITER);
    result = foo;
  }
  return result;
}

boolean lineMatchesKey(char* line, char* key) {
  boolean status = false;
  if (strlen(line) > strlen(key)) {
    if (0 == (strncmp(line, key, strlen(key)))) {
      status = true;
    }
  }
  return status;
}

/**
 * getDeviceName
 */
char* getDeviceName(void) {
  char* result = NULL;
  if (espConfigInit()) {
    File file = SPIFFS.open(CONFIG_FILE_NAME, FILE_READ);
    if (file){
      while (file.available()) {
        char* line = getNextLine(file);
        if (line) {
          if (lineMatchesKey(line, DEVICE_NAME_KEY)) {
            result = getValue(line);
          }
          free(line);
        }
        
      }
      file.close();
    } else {
      Serial.print("Failed to open file ");
      Serial.print(CONFIG_FILE_NAME);
      Serial.println(" for reading.");
    }
  }
  return result;
}


/**
 * getMqttTopicRoot
 */
char* getMqttTopicRoot(void) {
  char* result = NULL;
  if (espConfigInit()) {
    File file = SPIFFS.open(CONFIG_FILE_NAME, FILE_READ);
    if (file){
      while (file.available()) {
        char* line = getNextLine(file);
        if (line) {
          if (lineMatchesKey(line, MQTT_TOPIC_ROOT_KEY)) {
            result = getValue(line);
          }
          free(line);
        }
        
      }
      file.close();
    } else {
      Serial.print("Failed to open file ");
      Serial.print(CONFIG_FILE_NAME);
      Serial.println(" for reading.");
    }
  }
  return result;
}

/**
 * getBrightness
 */
char* getBrightness(void) {
  char* result = NULL;
  if (espConfigInit()) {
    File file = SPIFFS.open(CONFIG_FILE_NAME, FILE_READ);
    if (file){
      while (file.available()) {
        char* line = getNextLine(file);
        if (line) {
          if (lineMatchesKey(line, BRIGHTNESS_KEY)) {
            result = getValue(line);
          }
          free(line);
        }
        
      }
      file.close();
    } else {
      Serial.print("Failed to open file ");
      Serial.print(CONFIG_FILE_NAME);
      Serial.println(" for reading.");
    }
  }
  return result;
}

/**
 * getOrientation
 */
char* getOrientation(void) {
  char* result = NULL;
  if (espConfigInit()) {
    File file = SPIFFS.open(CONFIG_FILE_NAME, FILE_READ);
    if (file){
      while (file.available()) {
        char* line = getNextLine(file);
        if (line) {
          if (lineMatchesKey(line, ORIENTATION_KEY)) {
            result = getValue(line);
          }
          free(line);
        }
        
      }
      file.close();
    } else {
      Serial.print("Failed to open file ");
      Serial.print(CONFIG_FILE_NAME);
      Serial.println(" for reading.");
    }
  }
  return result;
}

/*
 * getSsidFromValue
 */
char* getSsidFromValue(char* value) {
  char* tempBuf = (char*)malloc(MAX_LINE_SIZE * sizeof(char));
  char* result = NULL;
  if (NULL != value) {
    strcpy(tempBuf, value);
    char* foo = strtok(tempBuf, VALUE_DELIMITER);
    if (NULL != foo) {
      result = foo;
    }
  }
  return result;
}

/*
 * getSsids
 */
WifiInfo* getSsids(void) {
  WifiInfo* result = (wifiInfo_t*)malloc(sizeof(struct wifiInfo_t));
  if (espConfigInit()) {    
    char** savedSSIDs = (char**)malloc(MAX_WIFI_ENTRIES * sizeof(char*));
    int ssidCount = 0;
    
    File file = SPIFFS.open(CONFIG_FILE_NAME, FILE_READ);

    if (file){
      while (file.available()) {
        char* line = getNextLine(file);        
        if (NULL != line) {
          if (lineMatchesKey(line, WIFI_KEY)) {
            char* knownSsid = getSsidFromValue(getValue(line));
            savedSSIDs[ssidCount] = knownSsid;
            ssidCount++;
          }
        }
      }
      file.close();
    } else {
      Serial.printf("Failed to open file %s for reading.\n", CONFIG_FILE_NAME);
    }
    Serial.printf("Found %d saved SSID", ssidCount);
    if (ssidCount >1 ) {
      Serial.printf("s");
    }
    Serial.printf("\n");

    result->ssid = savedSSIDs;
    result->numberOfSSIDsFound = ssidCount;
  }
  return result;
}

/*
 * getWifiPasswordFromValue
 */
static char* getWifiPasswordFromValue(char* value) {
  char* result = NULL;
  if (NULL != value) {
    char* tempBuf = (char*)malloc(strlen(value)+1 * sizeof(char));
    strncpy(tempBuf, value, strlen(value)+1);
    char* tmpPtr=NULL;
    tmpPtr = strtok(tempBuf, VALUE_DELIMITER);
    tmpPtr = strtok(NULL, VALUE_DELIMITER);
    result = tmpPtr;        
  }
  return result;
}

/*
 * getWifiPassword
 */
char* getWifiPassword(char* ssid) {
  char* result = NULL;
  if (espConfigInit()) {
    File file = SPIFFS.open(CONFIG_FILE_NAME, FILE_READ);
    if (file){
      while (result == NULL && file.available()) {
        char* line = getNextLine(file);
        if (line) {
          if (lineMatchesKey(line, WIFI_KEY)) {
            char* currentKnownSsid = getSsidFromValue(getValue(line));
            if(0 == strcmp(ssid, currentKnownSsid)) {
              result = getWifiPasswordFromValue(getValue(line));
            } else {
              free(currentKnownSsid);
            }
          }
        }
      }
      file.close();
    } else {
      Serial.printf("Failed to open file %s for reading\n", CONFIG_FILE_NAME);
    }
  }
  return result;
}

/*
 * keyExists
 */
boolean keyExists(char* key) {
  boolean result = false;
  File file = SPIFFS.open(CONFIG_FILE_NAME, FILE_READ);
    if (file){
      while (result == false && file.available()) {
        char* line = getNextLine(file);
        if (line) {
          if (lineMatchesKey(line, key)) {
              result = true;
          }
        }
      }
      file.close();
    } else {
      Serial.print("Failed to open file ");
      Serial.print(CONFIG_FILE_NAME);
      Serial.println(" for reading.");
    }
  return result;
}

/*
 * keyValueExists
 */
boolean keyValueExists(char* key, char* value) {
  boolean result = false;
  File file = SPIFFS.open(CONFIG_FILE_NAME, FILE_READ);
    if (file){
      while (result == false && file.available()) {
        char* line = getNextLine(file);
        if (line) {
          if (lineMatchesKey(line, key)) {
            if (0 == strcmp(value, getValue(line))) {
              result = true;
            }
          }
        }
      }
      file.close();
    } else {
      Serial.print("Failed to open file ");
      Serial.print(CONFIG_FILE_NAME);
      Serial.println(" for reading.");
    }
  return result;  
}

boolean saveKeyValue(char* key, char* value, boolean replaceExisting) {
  boolean result = false;
  Serial.printf("saveKeyValue: key: [%s], value: [%s]\n", key, value);

  if (espConfigInit()) {
//    Serial.println("saveKeyValue: CP 0.1");
    if (!keyExists(key)) {
//      Serial.println("saveKeyValue: CP 1");
      File file = SPIFFS.open(CONFIG_FILE_NAME, FILE_APPEND);
      if (file){
//        Serial.println("saveKeyValue: CP 2");
        file.print(key);
        file.print(KVP_DELIMITER);
        file.println(value);
        file.print("\0");
        file.println("");
        file.close();
//        dumpConfig();
//        Serial.println("saveKeyValue: CP 2.1");
      } else {
        Serial.printf("Failed to open file [%s] for reading\n", CONFIG_FILE_NAME);
      }
    } else {
      Serial.printf("WARNING: key/value [%s]/[%s] already exists\n", key, value);
      if (replaceExisting) {
        Serial.println("Replacing existing key/value with new one");
        deleteKey(key);
//        dumpConfig();
        saveKeyValue(key, value, false);
      } else {
        Serial.println("replaceExisting = false. Not replacing existing key/value.");
      }
    }
  } else {
    Serial.println("saveKeyValue: CP WTF");
  }
  return result;
}

void deleteKey(char* key) {
  char* tempFileBuffer[MAX_LINES_IN_CONFIG_FILE];
  int index = 0;
//  Serial.printf("deleteKey: key: [%s]\n", key);
  if (espConfigInit()) {
    // read all lines into buffer EXCEPT the matching one (there should be no duplicates)
    if (keyExists(key)) {
//      Serial.println("deleteKey: opening the file");
      File file = SPIFFS.open(CONFIG_FILE_NAME, FILE_READ);
      if (file){
        int lineCount = 0;
        while (file.available()) {
          char* line = getNextLine(file);
//          Serial.printf("deleteKey: Current line[%d] = [%s], length = %d\n", lineCount++, line, strlen(line));
          if (!(lineMatchesKey(line, key)) && 0 != strlen(line)) {
            tempFileBuffer[index++] = line;
          } else {
//            Serial.printf("deleteKey: skipping line = [%s]\n",line);
          }
        }
//        Serial.println("deleteKey: closing the file");
        file.close();

        // delete the file
//        Serial.printf("deleteKey: deleting the config file\n");
        SPIFFS.remove(CONFIG_FILE_NAME);
        
        // re-write the file from the buffer
//        Serial.println("deleteKey: opening the file");
        File file = SPIFFS.open(CONFIG_FILE_NAME, FILE_WRITE);
        for (int currentLine=0 ; currentLine<index ; currentLine++) {
//          Serial.printf("deleteKey: writing line = [%s]\n", tempFileBuffer[currentLine]);
          file.println(tempFileBuffer[currentLine]);
        }
//        Serial.println("deleteKey: closing the file");
        file.close();
      }
    }
  }
//  Serial.printf("deleteKey: Dump the config file with the key [%s] removed\n", key);
//  dumpConfig();
//  Serial.println("deleteKey: End");
}

void deleteKeyValue(char* key, char* value) {
  char* tempFileBuffer[MAX_LINES_IN_CONFIG_FILE];
  int index = 0;
  Serial.printf("deleteKeyValue: key: [%s] value[%s]\n", key, value);
  if (espConfigInit()) {
    // read all lines into buffer EXCEPT the matching one (there should be no duplicates)
    if (keyValueExists(key, value)) {
      File file = SPIFFS.open(CONFIG_FILE_NAME, FILE_READ);
      if (file){
        while (file.available()) {
          char* line = getNextLine(file);
          Serial.printf("deleteKeyValue: Current line = [%s]\n", line);
          if (!(lineMatchesKey(line, key) && (0 == strcmp(getValue(line), value)))) {
            tempFileBuffer[index++] = getNextLine(file);
          }
        }
        file.close();

        // delete the file
        SPIFFS.remove(CONFIG_FILE_NAME);
        
        // re-write the file from the buffer
        File file = SPIFFS.open(CONFIG_FILE_NAME, FILE_WRITE);
        for (int currentLine=0 ; currentLine<index ; currentLine++) {
          file.println(tempFileBuffer[currentLine]);
        }
        file.close();
      }
    }
  }
//  Serial.println("deleteKeyValue: CP END");
}

void deleteConfigData(void) {
  Serial.println("=============== DELETE CONFIG FILE ===============");
  if (espConfigInit()) {
    SPIFFS.remove(CONFIG_FILE_NAME);
  }
  Serial.println("=============== DELETE CONFIG FILE COMPLETE ===============");
}

/*
 * setDeviceName
 */
void setDeviceName(char* deviceName) {
  saveKeyValue(DEVICE_NAME_KEY, deviceName, true);
}
 
/*
 * SaveWifiInfo
 */
void saveWifiInfo(char* ssid, char* password) {
  char* value = (char*) malloc((strlen(ssid)+strlen(password)+2) * sizeof(char)); // +2 because of delimiter and null terminator
  sprintf(value, "%s%s%s", ssid, VALUE_DELIMITER, password);
  saveKeyValue( WIFI_KEY, value, false);
}

char* getCustomValue(char* key) {
  char* result = (char*)malloc(sizeof(char*));
  result = NULL;
//  Serial.printf("getCustomValue: [%s]\n",key);
//  delay(500);
  if (espConfigInit()) {
//    Serial.printf("getCustomValue: passed espConfigInit\n",key);
//    delay(500);
    File file = SPIFFS.open(CONFIG_FILE_NAME, FILE_READ);
    if (file){
//      Serial.printf("getCustomValue: got a file handle\n",key);
//      delay(500);
      while (file.available()) {
        char* line = getNextLine(file);
        if (line) {
          if (lineMatchesKey(line, key)) {
            result = getValue(line);
          }
          free(line);
        }
        
      }
      file.close();
    } else {
      Serial.print("Failed to open file ");
      Serial.print(CONFIG_FILE_NAME);
      Serial.println(" for reading.");
    }
  } else {
//    Serial.printf("getCustomValue: wtf!\n",key);
//    delay(500);
  }
  return result;
}

float getCustomValueFloat(char* key) {
  char* result = getCustomValue(key);
  float value = 0;
  if (result) {
    value = atof(result);
  }
  return value;
}

int getCustomValueInt(char* key) {
  char* result = getCustomValue(key);
  int value = 0;
  if (result) {
    value = atoi(result);
  }
  return value;
}

void dumpConfig(void) {
//  Serial.println("dumpConfig: starting");
//  delay(500);
  if (espConfigInit()) {
    File file = SPIFFS.open(CONFIG_FILE_NAME, FILE_READ);
    if (NULL != file) {
      Serial.println("File Contents:");
      Serial.println("---------------------------------------");
      File file = SPIFFS.open(CONFIG_FILE_NAME, FILE_READ);
      if (file){
        if (file.available()) {
          Serial.print("    ");
        } else {
          Serial.printf("    File is empty\n");
        }
        while (file.available()) {
          char currentChar = file.read();
          Serial.write(currentChar);
          if (currentChar == '\n' && file.available()) {
            Serial.print("    ");
          }
        }
        file.close();
      } else {
        Serial.print("Failed to open file ");
        Serial.print(CONFIG_FILE_NAME);
        Serial.println(" for reading.");
      }
      Serial.println("---------------------------------------");
    } else {
      Serial.println("dumpConfig: Config file does not exist");
    }
  }
}
