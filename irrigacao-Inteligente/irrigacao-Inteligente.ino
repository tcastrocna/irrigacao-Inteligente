#define BLYNK_TEMPLATE_ID "TMPL2GJQq5HPf"
#define BLYNK_TEMPLATE_NAME "IRRIGAÇÂO SMART"
#define BLYNK_AUTH_TOKEN "qHBYYCBpttz7LXKLSU0cHp3ifcpFjwUR"

#define BAUND_RATE 115200
#define BLYNK_PRINT Serial

//Define as saídas
#define LED_VERMELHO 18  //define a porta para o led vermelho
#define LED_VERDE 19     //define a porta para o led verde
#define MOTOR_PIN 13     // define a porta para o controle de liga e desliga da bomba (M)

//Define as entradas
#define BUTTON_PIN 23       // define o botão liga e desiga (B)
#define BOIA_AGUA_PIN 05    // define a porta do sensor de água na porta 19 (A)
#define LDR_SENSOR_PIN 35   // define a porta 35 Analogica para sensor de luminosidade (L)
#define SOIL_SENSOR_PIN 34  // define a porta 34 Analogica para sendor umidade de solo (S)

//Pinos virtuais da plataforma Blynk IoT Cloud 2.0.
#define VIRTUAL_PIN_1 V1  //Pino virtual da umidade (funcção Y)
#define VIRTUAL_PIN_2 V2  //Pino virtual da Luminosidade  (funcção Y)
#define VIRTUAL_PIN_3 V3  //Pino virtual do Motor  (funcção Y)

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Motoe";
char pass[] = "1A2b3c4d";


bool FETCH_BLYNK_STATE = true;
BlynkTimer timer;
int WiFi_Flag = 0;

// Constantes para limites do sensor de luminosidade LDR
const int LDR_SENSOR_MIN = 0;
const int LDR_SENSOR_MAX = 4095;
const int LUMINOSIDADE_MIN = 0;
const int LUMINOSIDADE_MAX = 100;
const int LIMITE_NOTURNO = 30;

// Constantes para limites do sensor de umidade do solo
const int SOIL_SENSOR_MIN = 0;
const int SOIL_SENSOR_MAX = 1023;
const int UMIDADE_MIN = 0;
const int UMIDADE_MAX = 100;
const int LIMITE_UMIDADE_ALTA = 80;
const int LIMITE_UMIDADE_BAIXA = 30;

// Variáveis para os sensores
float luminosidade;
int valorLdr = 0;
int valorNoite = 0;

float umidadeSolo;
int valorUmidade = 0;
int valorUmidadeSolo = 0;

// Variável para armazenar o estado da boia
int estadoBoia = 0;

// Variável para armazenar o estado do botão
int estadoBotao = 0;

// Variável para armazenar o estado do motor
int estadoMotor = 0;

void ldrSensor() {
  valorLdr = analogRead(LDR_SENSOR_PIN);
  luminosidade = map(valorLdr, LDR_SENSOR_MIN, LDR_SENSOR_MAX, LUMINOSIDADE_MIN, LUMINOSIDADE_MAX);
  Blynk.virtualWrite(VIRTUAL_PIN_2, luminosidade);
  if (luminosidade <= LIMITE_NOTURNO) {
    valorNoite = 1;
  } else {
    valorNoite = 0;
  }
  Serial.print("Luminosidade: ");
  Serial.print(luminosidade);
  Serial.println("%");
  delay(100);
}

void umidadeSoloSensor() {
  valorUmidade = analogRead(SOIL_SENSOR_PIN);
  umidadeSolo = map(valorUmidade, SOIL_SENSOR_MIN, SOIL_SENSOR_MAX, UMIDADE_MIN, UMIDADE_MAX);
  if (umidadeSolo >= LIMITE_UMIDADE_ALTA) {
    valorUmidadeSolo = 1;
  } else if (umidadeSolo <= LIMITE_UMIDADE_BAIXA) {
    valorUmidadeSolo = 0;
  }
  Serial.print("Umidade do Solo: ");
  Serial.print(umidadeSolo);
  Serial.println("%");
  delay(100);
}

void monitorarBoia() {
  estadoBoia = digitalRead(BOIA_AGUA_PIN);
  estadoBoia = !estadoBoia;
  Serial.print("Estado da Boia: ");
  Serial.println(estadoBoia);
  delay(100);
}

void monitorarBotao() {
  estadoBotao = digitalRead(BUTTON_PIN);
  estadoBotao = !estadoBotao;
  Serial.print("Estado do Botão: ");
  Serial.println(estadoBotao);

  if (estadoBotao == HIGH) {
    if (estadoMotor == 0 && estadoBoia == 0 && valorUmidadeSolo == 0) {
      digitalWrite(MOTOR_PIN, HIGH);
      estadoMotor = 1;
      digitalWrite(LED_VERMELHO, HIGH);
      digitalWrite(LED_VERDE, LOW);
      Serial.println("Motor Ligado!");
    } else {
      digitalWrite(MOTOR_PIN, LOW);
      estadoMotor = 0;
      digitalWrite(LED_VERMELHO, LOW);
      digitalWrite(LED_VERDE, HIGH);
      Serial.println("Motor Desligado!");
    }
  }

  delay(100);
}

void controlarMotor() {
  if (valorNoite == 1 && valorUmidadeSolo == 0 && estadoBoia == 0) {
    digitalWrite(MOTOR_PIN, HIGH);
    estadoMotor = 1;
    digitalWrite(LED_VERMELHO, HIGH);
    digitalWrite(LED_VERDE, LOW);
    Serial.println("Motor Ligado!");
  } else if (valorNoite == 1 && valorUmidadeSolo == 1 && estadoBoia == 0) {
    digitalWrite(MOTOR_PIN, LOW);
    estadoMotor = 0;
    digitalWrite(LED_VERMELHO, LOW);
    digitalWrite(LED_VERDE, HIGH);
    Serial.println("Motor Desligado - Solo úmido!");
  } else if (valorNoite == 0 && valorUmidadeSolo == 1 && estadoBoia == 0) {
    digitalWrite(MOTOR_PIN, LOW);
    estadoMotor = 0;
    digitalWrite(LED_VERMELHO, LOW);
    digitalWrite(LED_VERDE, HIGH);
    Serial.println("Motor Desligado - Dia e Solo úmido!");
  } else if (valorNoite == 1 && valorUmidadeSolo == 0 && estadoBoia == 1) {
    digitalWrite(MOTOR_PIN, LOW);
    estadoMotor = 0;
    digitalWrite(LED_VERMELHO, LOW);
    digitalWrite(LED_VERDE, HIGH);
    Serial.println("Motor Desligado - Reservatório Sem Água!");
  } else {
    digitalWrite(MOTOR_PIN, LOW);
    estadoMotor = 0;
    digitalWrite(LED_VERMELHO, LOW);
    digitalWrite(LED_VERDE, HIGH);
    Serial.println("Motor Desligado - Condições Não Atendidas!");
  }
}

BLYNK_WRITE(VIRTUAL_PIN_3) {
  int estadoMotorBlynk = param.asInt();

  if (estadoMotorBlynk == 1) {
    // Botão no aplicativo Blynk pressionado
    if (estadoMotor == 0 && estadoBoia == 0 && valorUmidadeSolo == 0) {
      digitalWrite(MOTOR_PIN, HIGH);
      estadoMotor = 1;
      digitalWrite(LED_VERMELHO, HIGH);
      digitalWrite(LED_VERDE, LOW);
      Serial.println("Motor Ligado!");
    } else {
      digitalWrite(MOTOR_PIN, LOW);
      estadoMotor = 0;
      digitalWrite(LED_VERMELHO, LOW);
      digitalWrite(LED_VERDE, HIGH);
      Serial.println("Motor Desligado!");
    }
  }
}

void setup() {
  Serial.begin(BAUND_RATE);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BOIA_AGUA_PIN, INPUT_PULLUP);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(MOTOR_PIN, OUTPUT);
}

void loop() {
  Blynk.run();
  ldrSensor();
  umidadeSoloSensor();
  monitorarBoia();
  monitorarBotao();
  controlarMotor();
}
