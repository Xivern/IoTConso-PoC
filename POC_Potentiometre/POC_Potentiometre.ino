#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
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
#define uS_TO_mS_FACTOR 1000ULL
#define TIME_TO_SLEEP 10000

int Value;
int Consommation = 0;

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
// Setup a feed called 'photocell' for publishing.
Adafruit_MQTT_Publish WWSDBInstant = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/WWSDBInstant");
Adafruit_MQTT_Publish WWSDBConso = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/WWSDBConso");
// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe ConsoCuisine = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/WWCuisineConso");
Adafruit_MQTT_Subscribe ConsoSDB = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/WWSDBConso");
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

void MQTT_connect();

void setup() 
{
  // initialize digital pin Buttonpin as an output.
  pinMode(SENSOR_BUILTOUT, INPUT);
  // Change the resolution for 12 bits
  analogReadResolution(12);

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_mS_FACTOR);

  // initialize the lcd 
  lcd.init();

  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print(" Consommation");
  lcd.setCursor(0,1);
  lcd.print(" Totale : ");

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
    }
    if (subscription == &ConsoSDB) {
      Serial.print(F(" >> Consommation totale salle de bain : "));
      Serial.println((char *)ConsoSDB.lastread);
      Consommation = (String((char *)ConsoSDB.lastread).toInt()) + Instantannee;
    }
  }

  // Now we can publish stuff!
  Serial.print(F("\n >> Sending valve value : "));
  Serial.print(Instantannee);
  Serial.print(" ... ");
  if (! WWSDBInstant.publish(Instantannee)) 
  {
    Serial.println(F("Failed"));
  } 
  else 
  {
    Serial.println(F(" OK!"));
  }

  // Now we can publish stuff!
  Serial.print(F("\n >> Sending total valve value : "));
  Serial.print(Consommation);
  Serial.print(" ... ");
  if (! WWSDBConso.publish(Consommation)) 
  {
    Serial.println(F(" Failed"));
  } 
  else 
  {
    Serial.println(F(" OK!"));
  }

  int Consototale = String((char *)ConsoCuisine.lastread).toInt() + String((char *)ConsoSDB.lastread).toInt();

  lcd.setCursor(11,1);
  lcd.print(Consototale);
  delay(10000);
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
