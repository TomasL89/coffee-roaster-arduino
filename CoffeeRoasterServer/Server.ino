WiFiServer server(8180);

void setupServer(String ssid, String password) {
  if (!SPIFFS.begin()) {
    return;
  }

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }

  displayNewMessage(WiFi.localIP().toString());
  server.begin();
}

void serverLoop() {
  WiFiClient client = server.available();
  bool profileRunning = false;
  bool chargeRunning = false;
  while (client)
  {

    client.setTimeout(100);
    if (client.connected())
    {
      displayNewMessage(F("Client connected"));
      client.flush();
    }
    while (client.connected())
    {

      if (client.available()) {
        String readBuffer;
 
        readBuffer = client.readStringUntil('\n');
       //Serial.println(readBuffer);
        if (readBuffer.startsWith("profile"))
        {
          String messages[] = { "Recieved profile message", readBuffer };
          displayNewMessages(messages, 2);
          handleNewProfile(readBuffer);
        }
       
        else if (readBuffer.startsWith("Start"))
        {
          timer = 0;
          profileRunning = true;
        	drawHomeScreen();

          continue;
        }
        else if (readBuffer.startsWith("Stop"))
        {
          chargeRunning = false;
          profileRunning = false;
          displayNewMessage("Stopping Roast");
          timer = 0;
        }
        else if (readBuffer.startsWith("Profile Get"))
        {
            //String currentProfile = getStoredProfile();
            StaticJsonDocument<2048> profileBufferJson;
            //deserializeJson(profileBufferJson, currentProfile);
            deserializeJson(profileBufferJson, getStoredProfile());
            String roastName = profileBufferJson["RoastName"];
            int roastLength = profileBufferJson["RoastLengthTotalInSeconds"];
            client.println("4:{\"RoastName\":\"" + roastName + "\"," + "\"RoastLengthTotalInSeconds\":" + (String)roastLength + ",\"RoastPoints\":[]}");
            JsonArray roastPoints = profileBufferJson["RoastPoints"];
            int profileToSendSize = roastPoints.size();
            Serial.println("Profile Size" + (String)profileToSendSize);
            for(int i = 0; i < profileToSendSize; i++)
            {
              String profileStageName = roastPoints[i]["StageName"];
              int profiletemperature = roastPoints[i]["Temperature"];
              int profileStartSeconds = roastPoints[i]["StartSeconds"];
              int profileEndSeconds = roastPoints[i]["EndSeconds"];
              int profileStagePoint = roastPoints[i]["StagePoint"];
              client.println("4:{\"StagePoint\":" + (String)profileStagePoint + ",\"StageName\":\"" + profileStageName + "\",\"Temperature\":" + profiletemperature + ",\"StartSeconds\":" + profileStartSeconds +",\"EndSeconds\":" + profileEndSeconds + "}"); 
              // Serial.println(profilePoint);
               client.flush();
              // todo debugging only, should'nt be in production code
            }
            char endProfile[] = "4:end transmission";
            client.println(endProfile);
            delay(10);

            client.flush();
        }
        //else if (readBuffer.length() > 50)
        else if (readBuffer.startsWith("Profile Set"))
        {
          readBuffer.remove(0, 11);
          Serial.println(readBuffer);
          handleNewProfile(readBuffer);
          delay(10);
          bool profileIsValid = readStoredProfile();
          delay(50);
          readBuffer = "";

          if (profileIsValid)
          {
            delay(0);
            setupLoadedProfile();

            client.println(F("valid_profile"));
          }
          else 
          {
            client.println(F("invalid_profile"));
          }
          client.flush();
        }
        }
        
        if (profileRunning)
        {
            
            int temperaturePoint = roastPoints[timer];
            int temperatureReading = getTemperature();

            checkHeater(temperaturePoint);
            drawTemperatureLimit(temperaturePoint);
            updateTime();
            client.println("1:" + (String)temperatureReading + ":" + (String)timer + ":"+ (String)heaterOn);
            delay(1000);
            continue;
        }
        
        }
        client.stop();
        displayNewMessage(F("Client Disconnected"));

    }
  }
