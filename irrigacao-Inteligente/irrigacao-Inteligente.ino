#define BLYNK_TEMPLATE_ID "xxxxxxxxxxxxx"
#define BLYNK_TEMPLATE_NAME "xxxxxxxxxxxxxxxxxxxx"
#define BLYNK_AUTH_TOKEN "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"

#define BAUD_RATE 9600
#define BLYNK_PRINT Serial

#define PINO_BOTAO 23    //define a porta do botão físico
#define PINO_BOIA 5      //define a porta da boia de nível d'água
#define PINO_SOLO_A0 34  //define a porta da saída analogica so sensor de umidade do solo
#define PINO_SOLO_D0 12  //define a porta da saída digital so sensor de umidade do solo
#define PINO_RELE 13
#define LED_VERMELHO 18
#define LED_VERDE 19
#define VIRTUAL_PIN_1 V1
#define VIRTUAL_PIN_2 V2
#define VIRTUAL_PIN_3 V2

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "WIFI-ESP";
char pass[] = "1A2b3c4d";

bool ESTADO_SOLO = HIGH;
bool ESTADO_BOIA = HIGH;
bool ESTADO_BOTAO = LOW;
bool ESTADO_APP = LOW;
bool ESTADO_RELE = LOW;

bool ESTADO_LG = HIGH;
bool ESTADO_LR = LOW;

unsigned long ULTIMO_ESTADO_BOTAO = LOW;
unsigned long MILLIS_REAIS;

int VALOR_UMIDADE;

/*Função para monitorar o sensor de umidade do solo*/
void monitorarSolo() {
  //ler a saida analogica do sensor de umidade do solo
  VALOR_UMIDADE = analogRead(PINO_SOLO_A0);             //Realiza a leitura analógica do sensor e armazena em valorumidade
  VALOR_UMIDADE = map(VALOR_UMIDADE, 4095, 0, 0, 100);  //Transforma os valores analógicos em uma escala de 0 a 100
  Blynk.virtualWrite(VIRTUAL_PIN_1, VALOR_UMIDADE);
  Serial.print("Umidade encontrada: ");  //Imprime mensagem
  Serial.print(VALOR_UMIDADE);           //Imprime no monitor serial o valor de umidade em porcentagem
  Serial.println(" % ");

  //ler a saida digital do sensor de umidade do solo
  ESTADO_SOLO = digitalRead(PINO_SOLO_D0);
  Serial.println(ESTADO_SOLO == HIGH ? "Solo Seco" : "Solo Úmido");
}

/*Função para monitorar a boia de nível de água*/
void monitorarBoia() {
  ESTADO_BOIA = digitalRead(PINO_BOIA);
  Serial.println(ESTADO_BOIA == HIGH ? "Água no reservatorio" : "Sem Água no reservatorio");
}

/*Função para monitorar o botão físico*/
void monitorBotao() {
  MILLIS_REAIS = millis();  // Atualiza o tempo atual
  if (digitalRead(PINO_BOTAO) == LOW && MILLIS_REAIS - ULTIMO_ESTADO_BOTAO > 1000) {
    ULTIMO_ESTADO_BOTAO = MILLIS_REAIS;
    ESTADO_BOTAO = (ESTADO_BOTAO == HIGH) ? LOW : HIGH;
    Serial.println("Botão Pressionado");
    Serial.println(ESTADO_BOTAO);
  }
}

/*Função para receber os dados do aplicativo Blynk IoT*/
BLYNK_WRITE(VIRTUAL_PIN_2) {
  ESTADO_APP = param.asInt();
  ESTADO_BOTAO = (ESTADO_BOTAO == HIGH) ? LOW : HIGH;
}

void controlarMotor() {
  if (((ESTADO_SOLO == HIGH) && (ESTADO_BOIA == HIGH) && (ESTADO_BOTAO == HIGH) && (ESTADO_APP == LOW)) || ((ESTADO_SOLO == HIGH) && (ESTADO_BOIA == HIGH) && (ESTADO_BOTAO == LOW) && (ESTADO_APP == HIGH))) {
    digitalWrite(PINO_RELE, HIGH);
    digitalWrite(LED_VERMELHO, HIGH);
    digitalWrite(LED_VERDE, LOW);
    Blynk.virtualWrite(VIRTUAL_PIN_2, HIGH);
    Serial.println("Relé Ligado");
  } else {
    digitalWrite(PINO_RELE, LOW);
    digitalWrite(LED_VERMELHO, LOW);
    digitalWrite(LED_VERDE, HIGH);
    Blynk.virtualWrite(VIRTUAL_PIN_2, LOW);
    Serial.println("Relé Desligado");
  }
}


void setup() {
  Serial.begin(BAUD_RATE);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  pinMode(PINO_BOTAO, INPUT_PULLUP);
  pinMode(PINO_BOIA, INPUT_PULLUP);
  pinMode(PINO_SOLO_A0, INPUT);
  pinMode(PINO_SOLO_D0, INPUT);
  pinMode(PINO_RELE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);

  digitalWrite(PINO_RELE, ESTADO_RELE);
  digitalWrite(LED_VERMELHO, ESTADO_LR);
  digitalWrite(LED_VERDE, ESTADO_LG);
}

void loop() {
  Blynk.run();
  monitorarSolo();
  monitorarBoia();
  monitorBotao();
  controlarMotor();
}
