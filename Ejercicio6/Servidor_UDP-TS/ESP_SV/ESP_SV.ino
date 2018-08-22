#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>

#define BAUD_RATE 115200
#define SERVER 50
#define N_CLIENTS 4
#define N_DIGITS 3

#define DEBUG_MODE 1
#define WIFI_ENABLED 1
#define UDP_OUTPORT 0

// Datos de thingspeak
#define CHANNEL_ID  396158
String apiKey = "IZ41288QYK0TRU82";
const char* logServer = "api.thingspeak.com";

// Usuario-contraseña WiFi hogar
#define SSID_HOME "Telecentro-cb70"
#define PASSWORD_HOME "VWN52Y4ETZ2Q"

// Usuario-Contraseña de Access Point
#define SSID_AP "CP_TBA"
#define PASSWORD_AP "ASDASDASD"

ESP8266WebServer server(80);

#define LOCALPORT 2000

IPAddress ServerIP(192, 168, 4, SERVER);
IPAddress ClientIP(192, 168, 4, 1);

IPAddress Gateway(192,168,4,SERVER);
IPAddress Subnet(255,255,255,0);

WiFiUDP udp; // Instancia UDP para enviar/recibir paquetes UDP

char packetBuffer[N_DIGITS];   // Buffer de datos UDP
char people[N_CLIENTS][N_DIGITS+1];

void setup(){
  Serial.begin(BAUD_RATE);
  setupArray();
  setup_begin();
}

void setup_begin(){
  setupAccessPoint();
  setupUDP();
}

void updateArray(char* t, char h) {
  //strcpy(people[(int)h-1], t);
  strcpy(people[h-'0'-1], t);
}

void setupArray(){
  size_t i;
  
  for(i=0;i<N_CLIENTS;i++)
  {
  strcpy(people[i],"000");
  }
  printArray();
}

void printArray(){
  size_t i;
  for(i=0;i<N_CLIENTS;i++)
  {
    Serial.printf("%s|",people[i]);
  }
  Serial.println("*");
}

void setupUDP() {
  #if(DEBUG_MODE)
  Serial.println("UDP > Configurando UDP");
  #endif
  udp.begin(LOCALPORT);
  #if(DEBUG_MODE)
  Serial.printf("UDP > Puerto local: %i\r\n",udp.localPort());
  #endif
}

void setupAccessPoint() {
  #if(DEBUG_MODE)
  Serial.println("AP > Configurando Access Point");
  #endif
  //delay(100);
  //WiFi.disconnect();
  //delay(100);
  //WiFi.persistent(false);
  delay(100);
  WiFi.mode(WIFI_OFF);
  delay(100);
  #if(DEBUG_MODE)
  Serial.println("AP > Desvinculando conexiones anteriores");
  #endif
  WiFi.mode(WIFI_AP);
  #if(DEBUG_MODE)
  Serial.printf("AP > Creando SSID: %s\r\n",SSID_AP);
  #endif
  WiFi.softAP(SSID_AP,PASSWORD_AP);
  WiFi.softAPConfig(ServerIP,Gateway,Subnet);
  IPAddress myIP = WiFi.softAPIP();
  #if(DEBUG_MODE)
  Serial.print("AP > Direccion IP: ");
  Serial.println(myIP);
  #endif
}

void connectStation(){
  int retry=0;
   WiFi.begin(SSID_HOME,PASSWORD_HOME);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    #if(DEBUG_MODE)
    Serial.print(".");
    #endif
    retry++;
    if(retry > 20){
      #if(DEBUG_MODE)
      Serial.println("No conectado");
      #endif
      return;
    }
  }

  #if(DEBUG_MODE)
  Serial.println("\r\nSTATION > Conexion exitosa");
  Serial.println("STATION > Iniciando cliente");
  #endif
}

void sendInformation(String t, char h){
  int i;
  WiFiClient client;
  #if(DEBUG_MODE)
  Serial.println("CLIENT > Conectandose a la base de datos de: " + String(logServer));
  #endif
  if (client.connect(logServer, 80)) {
    #if(DEBUG_MODE)
    Serial.println("CLIENT > Conexion exitosa");
    #endif
    //Serial.println(t + " " + h);
    String postStr = apiKey;
    
    for(i=0;i<N_CLIENTS;i++)
    {
      postStr += "&field";
      postStr += i+1;
      postStr += "=";
      postStr += people[i];
    }
    
    //postStr += "&field";
    //postStr += h;
    //postStr += "=";
    //postStr += String(t);
    
    postStr += "\r\n\r\n";
    #if(DEBUG_MODE)
    Serial.println("CLIENT > Enviando datos");
    #endif
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
    #if(DEBUG_MODE)
    Serial.print(postStr);
    #endif
  }
  delay(100);
  client.stop(); 
  #if(DEBUG_MODE)
  Serial.println("CLIENT > Apagando cliente");
  #endif
}

void setupStMode() {
  #if(DEBUG_MODE)
  Serial.println("STATION > Configurando modo estacion");
  #endif
  WiFi.softAPdisconnect();
  WiFi.disconnect();
  #if(DEBUG_MODE)
  Serial.println("STATION > Desvinculando conexiones anteriores");
  #endif
  WiFi.mode(WIFI_STA);
  #if(DEBUG_MODE)
  Serial.printf("STATION > Conectando al SSID: %s\r\n",SSID_HOME);
  #endif
  delay(100);
  
  /** If your ESP does not respond you can just
  *** reset after each request sending
    Serial.println("- trying reset");
    ESP.reset();
  **/
}

void loop(){
  String source = "";
  char room='0';
  int N_Bytes = udp.parsePacket();

  if (N_Bytes)
  {
    // UDP_rx -> Serial_tx
    udp.read(packetBuffer, N_Bytes); // read the packet into the buffer, we are reading only one byte
    source = udp.remoteIP().toString();
    room = source[source.length()-1];
    //Serial.printf("%s(%c)\r\n",packetBuffer,room);
    delay(100);
    updateArray(packetBuffer, room);
    #if(WIFI_ENABLED)
    setupStMode();
    connectStation();
    sendInformation(String(packetBuffer), room);
    #endif
    
    delay(100);
    setup_begin();
  }
  else {
    if (Serial.available() > 0)
    {
      #if(UDP_OUTPORT)
      // Serial_rx -> UDP_tx
      udp.beginPacket(ClientIP, LOCALPORT); //Send UDP requests are to port 2000

      char a[1];
      a[0] = char(Serial.read()); //Serial Byte Read
      udp.write(a, 1); //Send one byte to ESP8266
      udp.endPacket();
      #endif

      switch (Serial.read()) {
        case 'q':
        {
          printArray();
          break;
        };
      };
        
    }
  }
}
