WiFiServer server(8180);

void setupServer(String ssid, String password) {
  if (!SPIFFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  displayNewMessage(WiFi.localIP().toString());
  server.begin();
}

void serverLoop() {
  WiFiClient client = server.available();
  while (client)
  {
    client.setTimeout(100);
    if (client.connected())
    {
      displayNewMessage("Client connected");
    }
    while (client.connected())
    {
      String readBuffer;
      while (client.available() > 0)
      {
        readBuffer = client.readStringUntil('\r');
        Serial.println(readBuffer);
        if (readBuffer.startsWith("profile"))
        {
          String messages[] = { "Recieved profile message", readBuffer };
          displayNewMessages(messages, 2);
          handleNewProfile(readBuffer);
        }
        else
        {
          displayNewMessage(readBuffer);
        }
      }
    }
    client.stop();

    displayNewMessage("Client Disconnected");
  }
  for (int i = 0; i < 6; i++)
  {
    readStoredProfile(i);
    //readStoredTestProfile(i);
    delay(500);
  }
}
