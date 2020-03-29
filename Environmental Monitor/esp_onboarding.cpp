
#include <WebServer.h>
#include "esp_onboarding.h"
#include "esp_config.h"


boolean timeToEraseFlash;
boolean timeToReadFlash;
boolean timeToWriteFlash;
boolean flashSetupComplete = false;

/* IP Address details for access point */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

WebServer server(80);

char discoveredSSIDs[20][64];
int numberOfUniqueSSIDsDiscovered = 0;

// ----- Prototypes ---------------------------
char* getSuggestedDeviceName(void);
void setupOnboardingAP(void);
void setupWebServer(void);
void handle_OnConnect(void);
void handle_PostForm(void);
void handle_NotFound(void);
String SendHTML(void);
String SendSuccessHTML(void);
void displayConfigMode(char*, IPAddress);

//----------------------------------------------
// Public 
//----------------------------------------------
void performOnboarding()
{  
  Serial.println("performOnboarding");
  scanForAPs();
  Serial.println("Finished scanForAPs...");
  setupOnboardingAP();
  Serial.println("Finished setupOnboardingAP...");
  setupWebServer();
  Serial.println("Finished setupWebServer...");
  while (1) { server.handleClient(); };
}

//----------------------------------------------

void setupOnboardingAP()
{
  char APName[32];
  char* currentDeviceName = getDeviceName();

  if (NULL == currentDeviceName || strlen(currentDeviceName) == 0) {
    char* suggestedDeviceName;
    suggestedDeviceName = getSuggestedDeviceName();
    strcpy(APName, suggestedDeviceName);
  } else {
    strcpy(APName, currentDeviceName);
  }

  Serial.println("Setting soft-AP ... ");
  boolean result = WiFi.softAP(APName,"");
  if(result == true)
  {
    WiFi.softAPConfig(local_ip, gateway, subnet);
    delay(100);
    Serial.printf("AP %s Ready\n", APName);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
    displayConfigMode(APName, IP);
  } else {
    Serial.println("Failed to set up AP!");
  }
}

void appendMacToName(char *name) {
    String myMacAddress = WiFi.macAddress();
    if (strlen(name) > (31 - 5)) { // Max name size minus mac addr portion added below
      name[26] = '\0'; // cut it short enough to fit the mac addr portion
    }
    String macSubString = myMacAddress.substring(myMacAddress.length()-5);
    macSubString.toCharArray(&name[strlen(name)-1], 6);
}

WifiInfo* scanForAPs() {
  WifiInfo* result = (WifiInfo*)malloc(sizeof(WifiInfo));
  wifi_mode_t savedMode = WiFi.getMode();

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.printf("scanForAPs: Detected %d network access points\n", n);
    
    // Sort the indexes of the SSIDs discovered based on signal strength
    int* sortedIndexes = (int*)malloc(n * sizeof(int));
    for (int i=0;i<n;i++) {
      sortedIndexes[i] = 0;
    }
    
    for (int i=0;i<n;i++) {
      for (int j=0;j<n;j++) {
        if (WiFi.RSSI(j) > WiFi.RSSI(i)) {
          sortedIndexes[i] = j;
        }
      }
    }

    // Remove duplicate SSID entries
    char** resultSsidList = (char**)malloc(n * sizeof(char*));
    
    for (int i=0;i<n;i++) {
      boolean alreadyInList = false;
      int size = WiFi.SSID(sortedIndexes[i]).length();
      char * tempBuf = (char*) malloc((size + 1) * sizeof(char));
      WiFi.SSID(sortedIndexes[i]).toCharArray(tempBuf, size + 1);
          
      // Check to see if this AP has the same name as one already in our list
      if (numberOfUniqueSSIDsDiscovered > 0) {
        for (int i=0;i<numberOfUniqueSSIDsDiscovered;i++) {
          if (0 == strcmp(discoveredSSIDs[i], tempBuf)) {
            alreadyInList = true;
          }
        }
      }
      
      // If it isn't in our list, add it
      if (!alreadyInList) {
        resultSsidList[numberOfUniqueSSIDsDiscovered] = tempBuf;
        strcpy(discoveredSSIDs[numberOfUniqueSSIDsDiscovered], tempBuf);
        numberOfUniqueSSIDsDiscovered++;
      } else {
        free(tempBuf);
      }
    }

    result->ssid = resultSsidList;
    result->numberOfSSIDsFound = numberOfUniqueSSIDsDiscovered;
    
    Serial.printf("Found %d unique SSID", result->numberOfSSIDsFound);
    if (result->numberOfSSIDsFound >1 ) {
      Serial.printf("s");
    }
    Serial.printf("\n");
    
    WiFi.mode(savedMode);
  }
  return result;
}

/* --------------------------------------------------------------------------------------
 *  Web Server code
 */

void setupWebServer() {
  Serial.println("setupWebServer");
  server.on("/", handle_OnConnect);
  server.on("/postForm", handle_PostForm);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void handle_OnConnect() {
  Serial.println("handle_OnConnect");
  delay(1000);
  server.send(200, "text/html", SendHTML()); 
}

void handle_PostForm()
{
  char tempBuf[32];
  
  Serial.println("handle_PostForm");

  String deviceName = server.arg("deviceName");
  Serial.print("    Device name: ");
  Serial.println(deviceName);
  deviceName.toCharArray(tempBuf, 32);
  setDeviceName(tempBuf);

  String ssidString = server.arg("ssid");
  Serial.printf("    SSID: [%s]\n", ssidString);
  ssidString.toCharArray(tempBuf, 32);  
  char* ssid = (char*)malloc((strlen(tempBuf)+1) * sizeof(char));
  strcpy(ssid, tempBuf);

  String passwordString = server.arg("password");
  Serial.printf("    Password: [%s]\n", passwordString);
  passwordString.toCharArray(tempBuf, 32);
  char* password = (char*)malloc((strlen(tempBuf)+1) * sizeof(char));
  strcpy(password, tempBuf);
  
  saveWifiInfo(ssid, password);
  free(ssid);
  free(password);
  
  server.send(200, "text/html", SendSuccessHTML()); 

  // Now reset the board
  delay(1000);
  ESP.restart();
  for(;;) {}; // Not really needed I think.
}

void handle_NotFound(){
  Serial.println("handle_NotFound");
  server.send(404, "text/plain", "Not found");
}

String SendHTML(){
  Serial.println("SendHTML");
  delay(1000);
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>ESP32 OnBoarding Portal</title>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>ESP32 OnBoarding Portal</h1>\n";
  ptr +="<h3></h3>\n";
  ptr +="<form METHOD=\"POST\"action=\"/postForm\">\n";
  ptr +="Device Name:<br>\n";
  ptr +="<input type=\"text\" name=\"deviceName\" value=\"";
  char* deviceName = getDeviceName();
  
  if (NULL != deviceName && strlen(deviceName) > 0) {
    ptr +=deviceName;
  } else {
    ptr += getSuggestedDeviceName();
  }
  Serial.println("SendHTML: CP 1");
  delay(1000);
  ptr +="\"><br>\n";
  ptr +="SSID:<br>\n";
  ptr +="<input type=\"text\" name=\"ssid\" value=\"";
  if (numberOfUniqueSSIDsDiscovered > 0) {
    ptr +=discoveredSSIDs[0];
  }
  Serial.println("SendHTML: CP 2");
  delay(1000);
  ptr +="\"><br>\n";
  ptr +="SSIDs found:<br>\n";
  for (int i=0;i<numberOfUniqueSSIDsDiscovered;i++) {
    ptr +="    ";
    ptr +=discoveredSSIDs[i];
    ptr +="<br>\n";
  }
  Serial.println("SendHTML: CP 3");
  delay(1000);
  ptr +="<br>\n";
  ptr +="Password:<br>\n";
  ptr +="<input type=\"text\" name=\"password\" value=\"\"><br><br>\n";
  ptr +="<input type=\"submit\" value=\"Submit\">\n";
  ptr +="</form>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  Serial.println("SendHTML: CP 4");
  delay(1000);
  return ptr;
}

String SendSuccessHTML(){
  Serial.println("SendHTML");
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>ESP32 OnBoarding Portal</title>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>ESP32 OnBoarding Portal</h1>\n";
  ptr +="<h3>Success</h3>\n";
  ptr +="<br><br><br>";
  ptr +="Resetting in 5 seconds...\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

char* getSuggestedDeviceName() {  
  char* suggestedDeviceName = (char*) malloc(32 * sizeof(char));
  String myMacAddress = WiFi.macAddress();
  strcpy(suggestedDeviceName, "Thing-");
  String macSubString = myMacAddress.substring(myMacAddress.length()-5);
  int length = strlen(suggestedDeviceName);  
  macSubString.toCharArray(&suggestedDeviceName[length], 6);
  return suggestedDeviceName;
}
