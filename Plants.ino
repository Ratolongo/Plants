#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

const char* ssid = "@_2.4";
const char* password = "monteiro";

WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  delay(10);

  WiFi.begin(ssid, password);
  Serial.print("Conectando-se ao Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // Iniciar mDNS
  if (MDNS.begin("plants")) {
    Serial.println("mDNS iniciado: http://plants.local");
  } else {
    Serial.println("Erro ao iniciar mDNS");
  }

  server.begin();
}

void loop() {
  MDNS.update();  // Atualiza o serviço mDNS

  WiFiClient client = server.available();
  if (!client) return;

  Serial.println("Novo cliente conectado");

  // Esperar até que o cliente envie dados
  while (!client.available()) {
    delay(1);
  }

  String request = client.readStringUntil('\r');
  Serial.println("Requisição: " + request);
  client.flush();

  // Rota para JSON (dados dinâmicos)
  if (request.indexOf("GET /dados") >= 0) {
    int temperatura = random(28, 33);
    int umidade = random(40, 71);
    int luz = random(60, 91);

    String json = "{";
    json += "\"temperatura\": " + String(temperatura) + ",";
    json += "\"umidade\": " + String(umidade) + ",";
    json += "\"luz\": " + String(luz);
    json += "}";

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();
    client.println(json);
    return;
  }

  // Página HTML com atualização dinâmica
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-br">
<head>
  <meta charset="UTF-8">
  <title>Plants</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <style>
    :root {
      --header-cor: #8BBC45;
      --basa-cor: #594432;
      --info-cor: #8FA63F;
      --titulo-cor: #A68C5B;
    }
    * {
      margin: 0; padding: 0; border: 0;
    }
    body {
      display: flex;
      flex-direction: column;
      align-items: center;
      background-color: #262626;
      font-family: sans-serif;
      color: white;
    }
    header {
      display: flex;
      width: 100vh;
      height: 60px;
      font-size: 30px;
      justify-content: center;
      align-items: center;
      box-shadow: 2px 2px 10px #2f2f2f;
      background-color: var(--header-cor);
    }
    .base {
      display: flex;
      flex-direction: column;
      gap: 10px;
      align-items: center;
      width: 97%;
      height: 300px;
      margin-top: 20px;
      border-radius: 20px;
      box-shadow: 2px 2px 10px #2f2f2f;
      background-color: var(--basa-cor);
    }
    .titulo {
      width: 96%;
      height: 25px;
      text-align: center;
      border-radius: 20px;
      margin: 15px auto;
      box-shadow: 2px 2px 10px #2f2f2f;
      text-shadow: 0.1px 0.1px 0.7px #2c2c2c;
      background-color: var(--titulo-cor);
    }
    .info {
      display: flex;
      font-size: 90px;
      width: 93%;
      height: 70%;
      justify-content: center;
      align-items: center;
      margin: 10px auto;
      border-radius: 10px;
      box-shadow: 2px 2px 10px #2f2f2f;
      text-shadow: 2px 2px 1px #2c2c2c;
      background-color: var(--info-cor);
    }
    footer {
      display: flex;
      align-items: center;
      justify-content: center;
      position: fixed;
      bottom: 0;
      font-size: 10px;
      width: 100vh;
      height: 30px;
      background-color: var(--header-cor);
    }
  </style>
</head>
<body>
  <section>
    <header>
      <h1>Plants</h1>
    </header>
  </section>

  <div class="base">
    <div class="titulo"><h3>Temperatura Ideal: 28&deg; - 32&deg;</h3></div>
    <div class="info"><p id="temp">--</p><p>&deg;C</p></div>
  </div>

  <div class="base">
    <div class="titulo"><h3>Umidade Ideal: 40% - 70%</h3></div>
    <div class="info"><p id="umidade">--</p><p>%</p></div>
  </div>

  <div class="base">
    <div class="titulo"><h3>Luminosidade Ideal: 60% a 90%</h3></div>
    <div class="info"><p id="luminosidade">--</p><p>%</p></div>
  </div>

  <footer><p>Plants</p></footer>

  <script>
    function atualizarDados() {
      fetch("/dados")
        .then(response => response.json())
        .then(data => {
          document.getElementById("temp").textContent = data.temperatura;
          document.getElementById("umidade").textContent = data.umidade;
          document.getElementById("luminosidade").textContent = data.luz;
        })
        .catch(error => console.error("Erro ao buscar dados:", error));
    }

    setInterval(atualizarDados, 5000);
    atualizarDados(); // Carrega na primeira vez
  </script>
</body>
</html>
)rawliteral";

  // Enviar página HTML
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println(html);

  delay(1);
  Serial.println("Cliente desconectado");
}
