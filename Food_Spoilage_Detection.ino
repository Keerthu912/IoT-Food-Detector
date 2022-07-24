#include <DHT.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>;
#include <ThingSpeak.h>;;
#define RL 10.0 // the value of RL is 10K
#define m -0.36848 // using formula y = mx+c and the graph in the datasheet
#define c 1.105 // btained by before calculation
#define R0 1.9
int buzzer = D2; // set the
const char * myWriteAPIKey = "ZI81ELI6BMT6BOGN";
unsigned long myChannelNumber = 1706828; //Replace
const char* ssid = "Keerthi";
const char* password = "keerthi789";
void send_event(const char *event);
const char *host = "maker.ifttt.com";
const char *privateKey = "c8h3rE2Vquv6EMN8Y1V_PF";
double gas;
#define Gas_Pin A0
#define DHTPIN D4 // CONNECT THE DHT11 SENSOR TO PIN D4 OF THE NODEMCU
DHT dht(DHTPIN, DHT11, 15);
WiFiServer server(80);
WiFiClient client;
void setup() {
  Serial.begin(9600);
  Serial.begin(9600);
  pinMode(buzzer, OUTPUT);
  delay(10);
  dht.begin();
  ThingSpeak.begin(client);
  WiFi.begin(ssid, password);
  pinMode(Gas_Pin, INPUT);
  Serial.print("Connecting to Wifi Network");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Successfully connected to WiFi.");
  Serial.println("IP address is : ");
  Serial.println(WiFi.localIP());
  server.begin();
  Serial.println("Server started");
}
int ppm1() {
  float sensor_volt; //Define variable for sensor voltage
  float RS_gas; //Define variable for sensor resistance
  float ratio; //Define variable for ratio
  float sensorValue = analogRead(Gas_Pin); //Read analog values of sensor
  sensor_volt = sensorValue * (5.0 / 1023.0); //Convert analog values to voltage
  RS_gas = ((5.0 * 10.0) / sensor_volt) - 10.0; //Get value of RS in a gas
  ratio = RS_gas / R0; // Get ratio RS_gas/RS_air
  double ppm_log = (log10(ratio) - c) / m; //Get ppm value in linear scale according to the the ratio value
  float ppm = pow(10, ppm_log); //Convert ppm value to log scale
  Serial.println(ppm);
  ThingSpeak.writeField(myChannelNumber, 3, ppm, myWriteAPIKey);
  delay(200);
  return (ppm);
  if (ppm >= 100)
  {
    digitalWrite(buzzer, HIGH); // send high signal to buzzer
    delay(1000); // delay 1ms
    digitalWrite(buzzer, LOW); // send low signal to buzzer
    delay(1000);
  }
}
void loop() {
  float t = dht.readTemperature();
  delay(200);
  ThingSpeak.writeField(myChannelNumber, 1, t, myWriteAPIKey);
  Serial.print("Temp = ");
  Serial.println(t);
  float h = dht.readHumidity();
  delay(2000);
  ThingSpeak.writeField(myChannelNumber, 2, h, myWriteAPIKey);
  if (h >= 70)
  {
    digitalWrite(buzzer, HIGH); // send high signal to buzzer
    delay(1000); // delay 1ms
    digitalWrite(buzzer, LOW); // send low signal to buzzer
    delay(1000);
  }
  Serial.print("Humidity = ");
  Serial.println(h);
  gas = ppm1();
  delay(2000);
  WiFiClient client = server.available();
  if (client)
  {
    Serial.println("Web Client connected ");
    String request = client.readStringUntil('\r');
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");  // the connection will be closed after completion of the response
    client.println("Refresh: 10");  // update the page after 10 sec
    client.println();
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<style>html { font-family: Cairo; display: block; margin: 0px auto; text-align: center;color: #333333; background-color: ##f3ffee;}");
    client.println("body{margin-top: 50px;}");
    client.println("h1 {margin: 50px auto 30px; font-size: 50px;  text-align: center;}");
    client.println(".side_adjust{display: inline-block;vertical-align: middle;position: relative;}");
    client.println(".text1{font-weight: 180; padding-left: 5px; font-size: 50px; width: 170px; text-align: left; color: #3498db;}");
    client.println(".data1{font-weight: 180; padding-left: 1px; font-size: 50px;color: #3498db;}");
    client.println(".data{padding: 1px;}");
    client.println("</style>");
    client.println("</head>");
    client.println("<body>");
    client.println("<div id=\"webpage\">");
    client.println("<h1>IoT Based Jar</h1>");
    client.println("<div class=\"data\">");
    client.println("<div class=\"side_adjust text1\">Status:</div>");
    client.println("<div class=\"side_adjust data1\">");
    client.print(t);
    client.println("<div class=\"side_adjust text1\">% filled</div>");
    client.println("</div>");
    client.println("</div>");
    client.println("</body>");
    client.println("</html>");
    //client.println("<h1>Level Indicator</h1>");
    if ( t >= 10) {
      send_event("temp_event");
    }
  }
}
void send_event(const char *event)
{
  Serial.print("Connecting to ");
  Serial.println(host);
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("Connection failed");
    return;
  }
  // We now create a URI for the request
  String url = "/trigger/";
  url += event;
  url += "/with/key/";
  url += privateKey;
  Serial.print("Requesting URL: ");
  Serial.println(url);
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  while (client.connected())
  {
    if (client.available())
    {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    } else {
      // No data yet, wait a bit
      delay(50);
    };
  }
  Serial.println();
  Serial.println("closing connection");
  client.stop();
}
