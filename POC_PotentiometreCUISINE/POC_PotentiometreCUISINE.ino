#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#define WLAN_SSID "Iphone de Guillaume"
#define WLAN_PASS "GuiguiWcz77410"
#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883 // use 8883 for SSL
#define AIO_USERNAME "Guillaumewcz"
#define AIO_KEY "aio_XJYs07rUvDc9xVbFzIcR8FzAxZMh"

#define SENSOR_BUILTOUT   36 
#define SECONDE         1000

int Value;
int Consommation;

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
// Setup a feed called 'photocell' for publishing.
Adafruit_MQTT_Publish WWCInstant = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/WWCuisineInstant");
Adafruit_MQTT_Publish WWCConso = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/WWCuisineConso");
// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe ConsoCuisine = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/WWCuisineConso");
Adafruit_MQTT_Subscribe ConsoSDB = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/WWSDBConso");
void MQTT_connect();

void setup() 
{
  // initialize digital pin Buttonpin as an output.
  pinMode(SENSOR_BUILTOUT, INPUT);
  // Change the resolution for 12 bits
  analogReadResolution(12);

  Serial.begin(115200);
  delay(10);
  Serial.println(F("Adafruit MQTT demo"));
  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&ConsoCuisine);
  mqtt.subscribe(&ConsoSDB);
}

void loop() 
{
  Value = analogRead(SENSOR_BUILTOUT);
  int Instantannee = (Value/4095.0) * 20;
  /*
  // Print a message on the serial monitor
  Serial.print("The value is :");   
  // Print a message on the serial monitor
  Serial.println(Consommation);
  delay(SECONDE);
  */
  MQTT_connect();
  Adafruit_MQTT_Subscribe *subscription;

  while ((subscription = mqtt.readSubscription(10))) 
  {
    if (subscription == &ConsoCuisine) 
    {
      Serial.print(F(" >> Consommation totale cuisine : "));
      Serial.println((char *)ConsoCuisine.lastread);
      Consommation = (String((char *)ConsoCuisine.lastread).toInt()) + Instantannee;
    }
    if (subscription == &ConsoSDB) {
      Serial.print(F(" >> Consommation totale salle de bain : "));
      Serial.println((char *)ConsoSDB.lastread);
    }
  }

  // Now we can publish stuff!
  Serial.print(F("\n >> Sending valve value : "));
  Serial.print(Instantannee);
  Serial.print(" ... ");
  if (! WWCInstant.publish(Instantannee)) 
  {
    Serial.println(F(" Failed"));
  }
  else
  {
    Serial.println(F(" OK!"));
  }

  // Now we can publish stuff!
  Serial.print(F("\n >> Sending total valve value : "));
  Serial.print(Consommation);
  Serial.print(" ... ");
  if (! WWCConso.publish(Consommation)) 
  {
    Serial.println(F(" Failed"));
  } 
  else 
  {
    Serial.println(F(" OK!"));
  }
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() 
{
  int8_t ret;
  // Stop if already connected.
  if (mqtt.connected()) 
  {
    return;
  }

  Serial.print("Connecting to MQTT... ");
  uint8_t retries = 3;

  while ((ret = mqtt.connect()) != 0) 
  { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000); // wait 5 seconds
    retries--;
    if (retries == 0) 
    {
      // basically die and wait for WDT to reset me
      while (1);
    }
  }
  Serial.println("MQTT Connected!");
}
