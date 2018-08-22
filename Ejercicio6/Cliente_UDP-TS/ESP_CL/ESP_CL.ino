/*
    Wireless Serial using UDP ESP8266
    Hardware: NodeMCU
    Circuits4you.com
    2018
    Slave Board connects to Access Point
*/
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define SERIAL_INPUT FALSE

//const char *ssid = "Telecentro-cb70";
//const char *pass = "VWN52Y4ETZ2Q";
const char* ssid = "pepito";
const char* pass = "ASDASD";

unsigned int localPort = 2000; // local port to listen for UDP packets

IPAddress ServerIP(192, 168, 4, 1);
IPAddress ClientIP(192, 168, 4, 2);

IPAddress Gateway(192, 168, 4, 1);
IPAddress Subnet(255, 255, 255, 0);

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

char packetBuffer[3];   //Where we get the UDP data
//======================================================================
//                Setup
//======================================================================
void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("Iniciando modo STATION");
  WiFi.mode(WIFI_STA);
  Serial.println("Conectando a AP");
  WiFi.begin(ssid, pass);   //Connect to access point
  WiFi.config(ClientIP, Gateway, Subnet);
  int i = 0;
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    ESP.wdtFeed();
    if (i > 40) {
      Serial.println("");
      WiFi.begin(ssid, pass);   //Connect to access point
      i = 0;
    }
    i++;

    Serial.print(".");
  }
  Serial.print("\r\n Conectado a ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  //Start UDP
  Serial.println("Iniciando UDP");
  udp.begin(localPort);
  Serial.print("Puerto local: ");
  Serial.println(udp.localPort());
}
//======================================================================
//                MAIN LOOP
//======================================================================
void loop()
{
  while (WiFi.status() != WL_CONNECTED)
  {
    int packetSize = udp.parsePacket();
    if (!packetSize)
    {
      #if(SERIAL_INPUT)
      // Serial_rx -> UDP_tx
      if (Serial.available() > 0)
      {
        udp.beginPacket(ServerIP, localPort);  // Enviar UDP request a maestro

        char a[2];
        a[0] = char(Serial.read()); // Leey byte de serial
        udp.write(a, 1); // Enviar 1 byte por UDP
        udp.endPacket();
      }
      #endif
      #if(!SERIAL_INPUT)
        udp.beginPacket(ServerIP, localPort);  // Enviar UDP request a maestro
        int people=random(1,300);
        
        //char* persona="102";
        //String str = String(people);
        //Serial.println(people);
        //char cstr[str.length()];
        char cstr[intDigits(people)];

        Serial.printf("#:%i\r\n",intDigits(people));
        //str.toCharArray(cstr,str.length());
         //itoa(people,cstr,10);
         sprintf(cstr,"%03d",people);
        
        udp.write(cstr);
        udp.endPacket();
        delay(20*1000);
      #endif
    }
    else {
      // UDP_rx -> Serial_tx
      udp.read(packetBuffer, 1); // Leer el paquete en el buffer de a 1 byte
      Serial.print(packetBuffer);
      delay(20);
    }
  }
  WiFi.begin(ssid, pass);
}


int intDigits(int number)
{
  if(number!=0)
  {
    return (int)floor(log10(abs(number))) + 1;
  }
  return 1;
}


