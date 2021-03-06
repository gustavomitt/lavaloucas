git#include "Adafruit_GFX.h" // Biblioteca Gráfica Adafruit
#include <MCUFRIEND_kbv.h> // Bibllioteca para controle do lcd 
//#include <TouchScreen.h>
#include "max6675.h"
//#include <arduino-timer.h>
#include <TimerOne.h>
//#include <TimerThree.h>
//#include <TimerFour.h>

// Definicao de estados
#define DESLIGADO 0
#define CICLO 1
#define ENCHER_1 2
#define LAVAR 3
#define AQUECER_1 4
#define ESVAZIAR_1 5
#define ENCHER_2 6
#define ENXAGUE_1 7
#define ESVAZIAR_2 8
#define ENCHER_3 9
#define ENXAGUE_2 10
#define AQUECER_2 11
#define ESVAZIAR_3 12
#define VENTILAR 13
#define PAUSADO 14
#define ERRO 15

// Definicao de ciclos
#define CICLO_LAVAR 0
#define CICLO_PRELAVAR 1
#define CICLO_ESVAZIAR 2

int ciclo;

#define ARRAYSIZE 20

String estados[ARRAYSIZE] = {
  "DESLIGADO",
  "CICLO",
  "ENCHER_1",
  "LAVAR",
  "AQUECER_1",
  "ESVAZIAR_1",
  "ENCHER_2",
  "ENXAGUE_1",
  "ESVAZIAR_2",
  "ENCHER_3",
  "ENXAGUE_2",
  "AQUECER_2",
  "ESVAZIAR_3",
  "VENTILAR",
  "PAUSADO",
  "ERRO"
};


// Comandos da serial
int incomeByte = 0; // 1: encher - 2: pausar - 3: esvaziar


// Cores que iremos utilizar em nosso projeto
#define PRETO   0x0000
#define VERMELHO     0xF800
#define VERDE   0x07E0
#define BRANCO 0xFFFF
#define AZUL 0x001F

// Objetos do touchscreen
//const int XP=7,XM=A1,YP=A2,YM=6; //320x480 ID=0x0000
//const int TS_LEFT=496,TS_RT=467,TS_TOP=439,TS_BOT=464;

//TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
//TSPoint tp;

#define MINPRESSURE 200
#define MAXPRESSURE 1000


MCUFRIEND_kbv tft; // Objeto  de comunicação com  display

// Inicializa termopar
//int thermo_gnd_pin = 45;
//int thermo_vcc_pin = 47;
//int thermo_so_pin  = 53;
//int thermo_cs_pin  = 51;
//int thermo_sck_pin = 49;

int thermo_gnd_pin = 53;
int thermo_vcc_pin = 51;
int thermo_sck_pin = 49;
int thermo_cs_pin  = 47;
int thermo_so_pin  = 45;


//int thermo_gnd_pin = 44;
//int thermo_vcc_pin = 46;
//int thermo_so_pin  = 52;
//int thermo_cs_pin  = 50;
//int thermo_sck_pin = 48;
  
MAX6675 sensor(thermo_sck_pin, thermo_cs_pin, thermo_so_pin);


//String tempString;
//String tempStringOld;
volatile byte temperatura;
volatile byte temperaturaOld;

#define MINTEMPERATURE 45.0
#define MAXTEMPERATURE 55.0
#define TEMPERATURADEEMERGENCIA 60.0
//#define MINTEMPERATURE 30.0
//#define MAXTEMPERATURE 35.0

// Nivel
int nivel = 21;
volatile bool volatileCheio;
bool cheio;
bool cheioOld;
// Tempo de enchimento medido: 37s
const unsigned long tempoMaximoDeEnchimento = 120000; // Tempo maximo em qua a maquina estara enchendo em milisegundos
volatile unsigned long volatileTimerDeEnchimento; // Timer a ser disparado quando a maquina comecar a encher
const int numeroMaximoDeLeiturasDeNivelCheio = 3; // Maximo de leituras de nivel cheio apos o primeiro nivel cheio
volatile int contadorDeLeiturasDeNivelCheio; //

// Porta aberta
int portaPin = 20;
volatile bool volatilePortaAberta;
bool portaAberta;
bool portaAbertaOld;

// Botao Direito
int botaoDireitoPin = 19;
int buttonStateDireito = HIGH;
bool botaoDireitoPressionado = false;

// Botao Esquerdo
int botaoEsquerdoPin = 18;
int buttonStateEsquerdo = HIGH;
bool botaoEsquerdoPressionado = false;


// pino dos atuadores
const int bombaCirculacao = 22;
const int bombaExaustao = 24;
const int ebulidor = 26;
const int valvula = 28;
const int ventilador = 30;

// Estado dos atuadores
bool bombaDeCirculacaoFuncionando;
bool bombaDeExaustaoFuncionando;
bool ebulidorFuncionando;
bool valvulaFuncionando;
bool ebulidorFuncionandoOld;
bool ventiladorFuncionando;

// Estado do time
bool timerLigado;
unsigned int tempoAquecido = 300; // 5 minutos
unsigned int tempoExaustao  = 20; // 20 segundos
unsigned int tempoExague = 300; // 5 minutos
unsigned int tempoVentilacao = 900; // 15 minutos
volatile unsigned int volatileContadorTimerAquecido; 
volatile unsigned int volatileContadorTimerExaustao;
volatile unsigned int volatileContadorTimerExague;
volatile unsigned int volatileContadorTimerVentilacao;
volatile unsigned int volatileContadorPortaAberta;
volatile unsigned int volatileContadorBotaoPretoPressionado;
volatile unsigned int volatileContadorBotaoAmareloPressionado;
unsigned int contadorTimerAquecido; 
unsigned int contadorTimerExaustao;
unsigned int contadorTimerExague;
unsigned int contadorTimerVentilacao;
bool timerAquecido; 
bool timerExaustao;
bool timerExague;
bool timerVentilacao;

// variaveis de estado
byte estadoAtual;
byte estadoAnterior;

// Timer
// auto timer = timer_create_default(); // create a timer with default settings

// liga e desliga atuadores
void desligaBombaDeCirculacao(){
  digitalWrite(bombaCirculacao, HIGH);
  //Serial.print("Bomba de circulacao deslidada.\n");
}

void ligaBombaDeCirculacao(){
  digitalWrite(bombaCirculacao, LOW);
  //Serial.print("Bomba de circulacao deslidada.\n");
}

void desligaBombaDeExaustao(){
  digitalWrite(bombaExaustao, HIGH);
  //Serial.print("Bomba de exaustao deslidada.\n");
}

void ligaBombaDeExaustao(){
  digitalWrite(bombaExaustao, LOW);
  //Serial.print("Bomba de exaustao lidada.\n");
}

void desligaEbulidor(){
  digitalWrite(ebulidor, HIGH);
  ebulidorFuncionando = false;
  //Serial.print("Ebulidor deslidado.\n");
}

void ligaEbulidor(){
  digitalWrite(ebulidor, LOW);
  ebulidorFuncionando = true;
  //Serial.print("Ebulidor lidado.\n");
}

void desligaValvula(){
  digitalWrite(valvula, HIGH);
  //Serial.print("Valvula deslidada.\n");
}

void ligaValvula(){
  digitalWrite(valvula, LOW);
  //Serial.print("Valvula lidada.\n");
}

void desligaVentilador(){
  digitalWrite(ventilador, HIGH);
  //Serial.print("Ventilador deslidado.\n");
}

void ligaVentilador(){
  digitalWrite(ventilador, LOW);
  //Serial.print("Ventilador lidado.\n");
}

void setState(int estado){
  estadoAnterior = estadoAtual;
  estadoAtual = estado;
  Serial.print("Novo estado: ");
  Serial.print(estadoAtual);
  Serial.print("\n");
}

// funcoes de entradas dos estados
void entraEstadoDesligado(){
  
  Serial.print("Entrando no estado desligado\n");
  // seta estados
  setState(DESLIGADO);
  
  telaDesligado();

  // acoes
  desligaBombaDeCirculacao();
  desligaBombaDeExaustao();
  desligaEbulidor();
  desligaValvula();
  desligaVentilador();
  

  // Variaveis no default
  // Timers
  timerAquecido = false; 
  timerExaustao = false;
  timerExague = false;
  timerVentilacao = false;
  
  contadorTimerAquecido = 0;
  volatileContadorTimerAquecido = 0;
  
  // Nivel
  cheio = false;
  cheioOld = false;
  volatileCheio = false;
  //timer.every(1000, lerNivel);
  volatileTimerDeEnchimento = 0;
  contadorDeLeiturasDeNivelCheio = 0;
  

  // Porta Aberta
  portaAberta = false;
  portaAbertaOld = false;
  volatilePortaAberta = false;
  volatileContadorPortaAberta = 0;

  // Ebulidor
  ebulidorFuncionando = false;
  ebulidorFuncionandoOld = false;
  
  delay(1000);

}

void entraEstadoCiclo(){
  
  Serial.print("Entrando no estado de escolha de ciclo\n");
  // seta estados
  setState(CICLO);
  
  telaCiclo();

  // acoes
  desligaBombaDeCirculacao();
  desligaBombaDeExaustao();
  desligaEbulidor();
  desligaValvula();
  desligaVentilador();
  
  delay(1000);

}

void entraEstadoErro(String mensagem){
  
  Serial.print("Entrando no estado de Erro\n");
  Serial.print("Mensagem de erro: ");
  Serial.println(mensagem);
  // seta estados
  setState(ERRO);
  
  telaErro(mensagem);

  // acoes
  desligaBombaDeCirculacao();
  desligaBombaDeExaustao();
  desligaEbulidor();
  desligaValvula();
  desligaVentilador();
  
  delay(1000);

}

void entraEstadoPausado(){
  Serial.print("Entrando no estado pausado\n");
  // seta estados
  setState(PAUSADO);

  telaPausado();

  // acoes
  desligaBombaDeCirculacao();
  desligaBombaDeExaustao();
  desligaEbulidor();
  desligaValvula();
  desligaVentilador();

  delay(1000);
}

void entraEstadoEncher(int estado){
  Serial.print("Entrando no estado Encher: ");
  Serial.print(estado);
  Serial.print("\n");
  // configura o timer como desligado
  if (estado == ENCHER_1){
    timerLigado = false;
    Serial.print("timerLigado = false\n");
  }
  // Configura os timers de emergencia
  contadorDeLeiturasDeNivelCheio = 0;
  volatileTimerDeEnchimento = millis();
  
  // configura estados
  setState(estado);

  telaLigado(estadoAtual);
    
  // acoes
  desligaBombaDeCirculacao();
  desligaBombaDeExaustao();
  desligaEbulidor();
  ligaValvula();
  desligaVentilador();

  delay(1000);

}

void entraEstadoAspergir(int estado){
  Serial.print("Entrando no estado Aspergir: ");
  Serial.print(estado);
  Serial.print("\n");

  switch(estado){
    case ENXAGUE_1:
      Serial.print("Ligando timer para acabar enxague\n");
      timerExague = true;
      volatileContadorTimerExague = 0;
      //timer.in(tempoExague, entraEstadoEsvaziar,ESVAZIAR_2);
      //primeiraChamadaDoTimer3 = true;
      //Timer3.initialize(tempoExague);
      //Timer3.attachInterrupt(entraEstadoEsvaziar_2_Timer3);
      break;
  }
  
  // configura estados
  setState(estado);

  telaLigado(estadoAtual);
  
  // acoes
  ligaBombaDeCirculacao();
  desligaBombaDeExaustao();
  desligaEbulidor();
  desligaValvula();
  desligaVentilador();

  delay(1000);

}

void entraEstadoAquecer(int estado){
  Serial.print("Entrando no estado Aquecer: ");
  Serial.print(estado);
  Serial.print("\n");

  // configura estados
  setState(estado);

  telaLigado(estadoAtual);
  
  // acoes
  ligaBombaDeCirculacao();
  desligaBombaDeExaustao();
  ligaEbulidor();
  desligaValvula();
  desligaVentilador();

  delay(1000);
  
}

void entraEstadoEsvaziar(int estado){
  Serial.print("Entrando no estado Esvaziar: ");
  Serial.print(estado);
  Serial.print("\n");

  Serial.print("Ligando timer para desligar bomba de exaustao\n");
  timerExaustao = true;
  volatileContadorTimerExaustao = 0;
  /*switch(estado){
    case ESVAZIAR_1:
      primeiraChamadaDoTimer4 = true;
      Timer4.initialize(tempoExaustao);
      Timer4.attachInterrupt(entraEstadoEncher_2_Timer4);
      //timer.in(tempoExaustao, entraEstadoEncher,ENCHER_2);
      break;
    case ESVAZIAR_2:
      primeiraChamadaDoTimer4 = true;
      Timer4.initialize(tempoExaustao);
      Timer4.attachInterrupt(entraEstadoEncher_3_Timer4);
      //timer.in(tempoExaustao, entraEstadoEncher,ENCHER_3);
      break;
    case ESVAZIAR_3:
      primeiraChamadaDoTimer4 = true;
      Timer4.initialize(tempoExaustao);
      Timer4.attachInterrupt(entraEstadoDesligado_Timer4);
      Serial.print("timer de ");
      Serial.print(tempoExaustao/1000000);
      Serial.print(" ativado\n");
      //timer.in(tempoExaustao, entraEstadoDesligado);
      break;
  }*/

  // configura estados
  setState(estado);
  
  telaLigado(estadoAtual);
  
  // acoes
  desligaBombaDeCirculacao();
  ligaBombaDeExaustao();
  desligaEbulidor();
  desligaValvula();
  desligaVentilador();

  delay(1000);
}

void entraEstadoVentilar(int estado){
  Serial.print("Entrando no estado Ventilar: ");
  Serial.print(estado);
  Serial.print("\n");

  Serial.print("Ligando timer para desligar o vantilador\n");
  timerVentilacao = true;
  volatileContadorTimerVentilacao = 0;

  // configura estados
  setState(estado);
  
  telaLigado(estadoAtual);

  // acoes
  desligaBombaDeCirculacao();
  desligaBombaDeExaustao();
  desligaEbulidor();
  desligaValvula();
  desligaVentilador();
  ligaVentilador();

  delay(1000);
}

bool verificaBotaoDireitoPressionado(){
  /*tp = ts.getPoint();
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  if (tp.z > MINPRESSURE && tp.z < MAXPRESSURE){
    Serial.print("tp.x=" + String(tp.x) + "\n");
    Serial.print("tp.y=" + String(tp.y) + "\n");
    Serial.print("tp.z=" + String(tp.z) + "\n");

    if (tp.x > 650 && tp.x < 900  && tp.y > 750 && tp.y < 870) {
      //tft.fillScreen(PRETO);
      //tft.setTextSize(2);
      //tft.setCursor(50, 50);
      //tft.print("tp.x=" + String(tp.x) + " tp.y=" + String(tp.y) + "   ");
      Serial.print("Botao direito pressionado\n");
      return true;
    }
  }
  return false;*/
  buttonStateDireito = digitalRead(botaoDireitoPin);

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonStateDireito == LOW) {
    delay(500);
    buttonStateDireito = digitalRead(botaoDireitoPin);
    if(buttonStateDireito == LOW) {
      Serial.println("Botao direito pressionado");
      //delay(1000);
      return true;
    }
  } else {
    return false;
  }
}

bool verificaBotaoEsquerdoPressionado(){
  /*tp = ts.getPoint();
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  if (tp.z > MINPRESSURE && tp.z < MAXPRESSURE){
    Serial.print("tp.x=" + String(tp.x) + "\n");
    Serial.print("tp.y=" + String(tp.y) + "\n");
    Serial.print("tp.z=" + String(tp.z) + "\n");
    if (tp.x > 0 && tp.x < 500  && tp.y > 750 && tp.y < 870) {
      //tft.fillScreen(PRETO);
      //tft.setTextSize(2);
      //tft.setCursor(50, 50);
      //tft.print("tp.x=" + String(tp.x) + " tp.y=" + String(tp.y) + "   ");
      Serial.print("Botao esquerdo pressionado\n");
      return true;
    }
  }
  return false;*/
  /*Serial.print("Estado do botao amarelo na funcao botaoEsquerdoPressionado: ");
  Serial.println(botaoAmareloPressionado);
  Serial.print("Estado do botao amarelo old na funcao botaoEsquerdoPressionado: ");
  Serial.println(botaoAmareloPressionadoOld);
  Serial.print("Estado atual diferente do anterior? ");
  Serial.println(botaoAmareloPressionado != botaoAmareloPressionadoOld);
  */
  
  buttonStateEsquerdo = digitalRead(botaoEsquerdoPin);

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonStateEsquerdo == LOW) {
    delay(500);
    buttonStateEsquerdo = digitalRead(botaoEsquerdoPin);
    if(buttonStateEsquerdo == LOW) {
      Serial.println("Botao esquerdo pressionado");
      //delay(1000);
      return true;
    }
  } else {
    return false;
  }

}


void telaDesligado(){
  tft.setRotation(3); // Display é rotacionado para modo paisagem
  tft.fillScreen(PRETO); // Tela  é preenchida pela cor Preta
  escreveTexto(50,0,"Desligado",3,BRANCO); // Texto é escrito na posição (50,0)
  escreveTexto(50,50,"Temperatura:",2,VERDE); // Texto é escrito na posição (50,0)
  escreveTexto(50,70,"Nivel: baixo",2,VERDE); // Texto é escrito na posição (50,0)
  escreveTexto(50,90,"Aquecimento: desligado",2,VERDE); // Texto é escrito na posição (50,0)
  criarBotao(10,200,150,30,"Esvaziar",VERMELHO); // Esquerda
  criarBotao(170,200,120,30,"Ligar",VERMELHO); // Direita
}

void telaCiclo(){
  tft.setRotation(3); // Display é rotacionado para modo paisagem
  tft.fillScreen(PRETO); // Tela  é preenchida pela cor Preta
  escreveTexto(50,0,"Escolha de Ciclo",3,BRANCO); // Texto é escrito na posição (50,0)
  escreveTexto(50,50,"Temperatura:",2,VERDE); // Texto é escrito na posição (50,0)
  criarBotao(10,200,150,30,"PreLav",VERMELHO); // Esquerda
  criarBotao(170,200,120,30,"Lavar",VERMELHO); // Direita
}

void telaErro(String mensagem){
  tft.setRotation(3); // Display é rotacionado para modo paisagem
  tft.fillScreen(PRETO); // Tela  é preenchida pela cor Preta
  escreveTexto(50,0,"Erro:",3,BRANCO);
  escreveTexto(50,20,mensagem,3,BRANCO);
  escreveTexto(50,50,"Temperatura:",2,VERDE);
  criarBotao(10,200,150,30,"PreLav",VERMELHO); // Esquerda
  criarBotao(170,200,120,30,"Lavar",VERMELHO); // Direita
}

void telaLigado(int estado){
  tft.setRotation(3); // Display é rotacionado para modo paisagem
  tft.fillScreen(PRETO); // Tela  é preenchida pela cor Branca
  escreveTexto(50,0,estados[estado],3,BRANCO); // Texto é escrito na posição (50,0)
  escreveTexto(50,50,"Temperatura:",2,VERDE); // Texto é escrito na posição (50,0)
  escreveTexto(50,70,"Nivel: baixo",2,VERDE); // Texto é escrito na posição (50,0)
  escreveTexto(50,90,"Aquecimento: desligado",2,VERDE); // Texto é escrito na posição (50,0)
  criarBotao(160,200,120,30,"Pausar",VERMELHO); // Criação do botão Logar
}

void telaPausado(){
  tft.setRotation(3); // Display é rotacionado para modo paisagem
  tft.fillScreen(PRETO); // Tela  é preenchida pela cor Branca
  escreveTexto(40,0,"Ciclo Pausado",3,BRANCO); // Texto é escrito na posição (50,0)
  escreveTexto(50,50,"Temperatura: 35.0",2,VERDE); // Texto é escrito na posição (50,0)
  escreveTexto(50,70,"Nivel: alto",2,VERDE); // Texto é escrito na posição (50,0)
  escreveTexto(50,90,"Aquecimento: ligado",2,VERDE); // Texto é escrito na posição (50,0)
  criarBotao(10,200,150,30,"Retomar",VERMELHO); // Criação do botão Logar
  criarBotao(160,200,150,30,"Desligar",VERMELHO); // Criação do botão Logar
}

void escreveTexto(int posx,int posy, String texto,int tamanho,int cor){ // Função criada para facilitar escrita de texto
  tft.setCursor(posx,posy); // Cursor é deslocado para posição passada como parâmetro
  tft.setTextColor(cor); // Cor a ser escrita é alterada conforme valor recebido como parâmetro
  tft.setTextSize(tamanho); // Tamanho da fonte é  alterado conforme parâmetro recebido
  tft.print(texto); // Texto passado por parâmetro é escrito na tela
}

void lerTemperatura(){
  temperatura = (byte) sensor.readCelsius();
  Serial.print("Estado atual: ");
  Serial.print(estados[estadoAtual]);
  Serial.print("\n");
  volatileContadorPortaAberta++;
  volatileContadorBotaoPretoPressionado++;
  volatileContadorBotaoAmareloPressionado++;
  if (timerAquecido){
      volatileContadorTimerAquecido++;
      Serial.print("Timer Aquecido : ");
      Serial.print(volatileContadorTimerAquecido);
      Serial.print("\n");
  }
  if (timerExaustao){
      volatileContadorTimerExaustao++;
      Serial.print("Timer Exaustao : ");
      Serial.print(volatileContadorTimerExaustao);
      Serial.print("\n");
  }
  if (timerExague){
      volatileContadorTimerExague++;
      Serial.print("Timer Enxague : ");
      Serial.print(volatileContadorTimerExague);
      Serial.print("\n");
  }
  if (timerVentilacao){
      volatileContadorTimerVentilacao++;
      Serial.print("Timer Ventilacao : ");
      Serial.print(volatileContadorTimerVentilacao);
      Serial.print("\n");
  }

  // Verificacoes de seguranca
  lerNivel();
  Serial.print("Nível está cheio? ");
  Serial.println(volatileCheio);
  if ( (estadoAtual == ENCHER_1) || (estadoAtual == ENCHER_2) || (estadoAtual == ENCHER_3) ){
    if(millis() > (volatileTimerDeEnchimento + tempoMaximoDeEnchimento)){
      entraEstadoErro("Excedido tempo maximo de enchimento.");
    }
    if( volatileCheio == true ){
      if (contadorDeLeiturasDeNivelCheio > numeroMaximoDeLeiturasDeNivelCheio) {
        entraEstadoErro("Excedido numero maximo de leituras de nivel cheio.");
      } else {
        contadorDeLeiturasDeNivelCheio++;
      }
    }
    
  }
  if ( temperatura > TEMPERATURADEEMERGENCIA ) {
    entraEstadoErro("Temperatura acima no nivel de emergencia");
  }
}

void lerNivel(){
  volatileCheio = (digitalRead(nivel) == HIGH);
}

void lerPortaAberta(){
  Serial.print("Estado do pino Porta Aberta: ");
  Serial.println(String(digitalRead(portaPin)));
  Serial.print("Contador Porta Aberta: ");
  Serial.println(volatileContadorPortaAberta);
  if (volatileContadorPortaAberta > 2){
    volatileContadorPortaAberta = 0;
    delay(100);
    volatilePortaAberta = (digitalRead(portaPin) == HIGH);
    if(volatilePortaAberta){
      Serial.println("Porta aberta" );
    } else{
      Serial.println("Porta fechada" );
    }
  }
}

/*void funcaoBotaoPreto() {
  if (volatileContadorBotaoPretoPressionado > 2){
    volatileContadorBotaoPretoPressionado = 0;
    volatileContadorBotaoAmareloPressionado = 0;
    Serial.println("Botao preto (direito) disparado ");
    delay(200);
    volatileBotaoPretoPressionado = digitalRead(botaoPretoPin);
    Serial.print("Estado do botao preto: ");
    Serial.println(volatileBotaoPretoPressionado);
  }
}

void funcaoBotaoAmarelo() {
  if (volatileContadorBotaoAmareloPressionado > 2){
    volatileContadorBotaoPretoPressionado = 0;
    volatileContadorBotaoAmareloPressionado = 0;
    Serial.println("Botao amarelo (esquerdo) disparado ");
    delay(200);
    volatileBotaoAmareloPressionado = digitalRead(botaoAmareloPin);
    Serial.print("Estado do botao amarelo: ");
    Serial.println(volatileBotaoAmareloPressionado);
  }
}
*/




void atualizaTemperatura(){
  if (temperatura != temperaturaOld){
    //Serial.print("Temperatura: " + tempString + "\n");
    temperaturaOld = temperatura;
    tft.fillRect(200,50, 60, 20, PRETO);
    escreveTexto(200,50,String(temperatura),2,VERDE); // Texto é escrito na posição (50,0)
  }

}

void atualizaPortaAberta(){
  if (portaAberta != portaAbertaOld){
    Serial.print("Porta Aberta: " + String(portaAberta) + "\n");
    portaAbertaOld = portaAberta;
    if (portaAberta) {
      entraEstadoPausado();
    } else {
      retornaDeEstadoPausado();
    }
  }

}

void atualizaNivel(boolean cheioPar){
  if (cheioPar != cheioOld){
    Serial.print("Estado do nível: ");
    Serial.print(digitalRead(nivel));
    Serial.print("\n");
    if (cheio) {
      Serial.print("Nivel alto\n");
    } else {
      Serial.print("Nivel baixo\n");
    }
    cheioOld = cheioPar;
    tft.fillRect(50,70, 200, 20, PRETO);
    if (cheio){
      escreveTexto(50,70,"Nivel: alto",2,VERDE); 
    }else{
      escreveTexto(50,70,"Nivel: baixo",2,VERDE);
    }
  }
}

void atualizaAquecimento(){
  if (ebulidorFuncionando != ebulidorFuncionandoOld){
    ebulidorFuncionandoOld = ebulidorFuncionando;
    tft.fillRect(50,90, 300, 20, PRETO);
    if (ebulidorFuncionando){
      escreveTexto(50,90,"Aquecimento: ligado",2,VERDE); 
    }else{
      escreveTexto(50,90,"Aquecimento: desligado",2,VERDE);
    }
  }
}

void criarBotao(int posx,int posy, int largura, int altura,String texto,int cor) //
{
    //Create Red Button
  tft.fillRect(posx,posy, largura, altura, cor); // um quadrado começando em (posx,posy) é renderizado conforme parâmetros passados
  tft.drawRect(posx,posy, largura, altura,PRETO); // um quadrado de cor preta é desenhado ao redor do quadrado vermelho 
  tft.setCursor(posx+8,posy+4); // Cursor é deslocado para o pixel de posição (posx+8,posy+4)
  tft.setTextColor(BRANCO); // Cor do texto é alterada para Branco
  tft.setTextSize(3); // Fonte é alterada para tamanho 3
  tft.print(texto); // Texto é escrito em posição determinada
}

void retornaDeEstadoPausado(){
  switch (estadoAnterior){
    case DESLIGADO:
      entraEstadoDesligado();
      break;
    case ENCHER_1:
      entraEstadoEncher(ENCHER_1);
      break;
    case LAVAR:
      entraEstadoAspergir(LAVAR);
      break;
    case AQUECER_1:
      entraEstadoAquecer(AQUECER_1);
      break;
    case ESVAZIAR_1:
      entraEstadoEsvaziar(ESVAZIAR_1);
      break;
    case ENCHER_2:
      entraEstadoEncher(ENCHER_2);
      break;
    case ENXAGUE_1:
      entraEstadoAspergir(ENXAGUE_1);
      break;
    case ESVAZIAR_2:
      entraEstadoEsvaziar(ESVAZIAR_2);
      break;
    case ENCHER_3:
      entraEstadoEncher(ENCHER_3);
      break;
    case ENXAGUE_2:
      entraEstadoAspergir(ENXAGUE_2);
      break;
    case AQUECER_2:
      entraEstadoAquecer(AQUECER_2);
      break;
    case ESVAZIAR_3:
      entraEstadoEsvaziar(ESVAZIAR_3);
      break;
    case VENTILAR:
      entraEstadoVentilar(VENTILAR);
      break;
  }
}


void setup() {
  Serial.begin(9600);
  delay(500);


  // Inicia termopar
  pinMode(thermo_vcc_pin, OUTPUT); 
  pinMode(thermo_gnd_pin, OUTPUT); 
  digitalWrite(thermo_vcc_pin, HIGH);
  digitalWrite(thermo_gnd_pin, LOW);

  // Inicia botoes
  pinMode(botaoDireitoPin, INPUT_PULLUP);
  pinMode(botaoEsquerdoPin, INPUT_PULLUP);

  
  
  // chama ler temperatura a cada 1000 millis (1 second)
  Timer1.initialize(1000000);
  Timer1.attachInterrupt(lerTemperatura);  
  // Inicia o Timer3 para ser usado em temporizadores da lavagem
  //Timer3.initialize(36000000000); // uma hora, tempo infinito
  // Inicia o Timer4 para ser usado em temporizadores do enchague
  //Timer4.initialize(36000000000); // uma hora, tempo infinito
  timerAquecido = false; 
  timerExaustao = false;
  timerExague = false;
  timerVentilacao = false;

  // Nivel
  cheio = false;
  cheioOld = false;
  volatileCheio = false;
  pinMode(nivel, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(nivel), lerNivel, CHANGE);
  //timer.every(1000, lerNivel);
  volatileTimerDeEnchimento = 0;
  contadorDeLeiturasDeNivelCheio = 0;

  // Porta Aberta
  portaAberta = false;
  portaAbertaOld = false;
  volatilePortaAberta = false;
  pinMode(portaPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(portaPin), lerPortaAberta, CHANGE);
  volatileContadorPortaAberta = 0;

  // Ebulidor
  ebulidorFuncionando = false;
  ebulidorFuncionandoOld = false;

  // configura pinos
  pinMode(bombaCirculacao, OUTPUT);
  pinMode(bombaExaustao, OUTPUT);
  pinMode(ebulidor, OUTPUT);
  pinMode(valvula, OUTPUT);
  desligaBombaDeCirculacao();
  desligaBombaDeExaustao();
  desligaEbulidor();
  desligaValvula();
  desligaVentilador();
  

  // Inicia LCD
  uint16_t ID = tft.readID(); // Leitura do código de identificação do controlador
  if (ID == 0x0000) ID = 0x9481; //force reg(0x00D3) 00 00 31 29 ILI9341, ILI9488
  tft.begin(ID); // Inicialização da tela

  // Entra no estado desligado
  estadoAtual = DESLIGADO;
  entraEstadoDesligado();
  
}

void loop() {

  // Copia valores lidos pelas interrupcoes
  noInterrupts();
  cheio = volatileCheio;
  portaAberta = volatilePortaAberta;
  contadorTimerAquecido = volatileContadorTimerAquecido;
  contadorTimerExaustao = volatileContadorTimerExaustao;
  contadorTimerExague = volatileContadorTimerExague;
  contadorTimerVentilacao = volatileContadorTimerVentilacao;
  //float temperaturaCopy = temperatura;
  interrupts();

  botaoEsquerdoPressionado = verificaBotaoEsquerdoPressionado();  
  botaoDireitoPressionado = verificaBotaoDireitoPressionado();  

  // Atualiza tela com valores que mudaram
  atualizaTemperatura();
  atualizaNivel(cheio);
  atualizaAquecimento();
  atualizaPortaAberta();
  
  // put your main code here, to run repeatedly:
  /*Serial.print("Graus C = "); 
  Serial.print(tempString);
  Serial.print("\n"); 
  delay(500);*/

  //timer.tick(); // tick the timer

  // Le comandos da serial
  if (Serial.available() > 0) {
    incomeByte = int(Serial.read());
    Serial.println("Entrada na serial: " + String(incomeByte));
    switch (incomeByte){
      case 49: // caractere "1"
        entraEstadoEncher(ENCHER_1);
        break;
      case 50: // caractere "2"
        entraEstadoPausado();
        break;
      case 51: // caractere "3"
        entraEstadoEsvaziar(ESVAZIAR_3);
        break;
      case 52: // caractere "4"
        entraEstadoVentilar(VENTILAR);
        break;
    }
  }
  
  //Serial.print("Estado: " + String(estado) + "\n");
  switch (estadoAtual){
    case DESLIGADO:
      //Serial.print("Case Desligado\n");
      if (botaoEsquerdoPressionado){
        entraEstadoEsvaziar(ESVAZIAR_3);
        ciclo = CICLO_ESVAZIAR;
      }
      if (botaoDireitoPressionado){
        entraEstadoCiclo();
      }
      break;
    case CICLO:
      if (botaoEsquerdoPressionado){
        entraEstadoEncher(ENCHER_2);
        ciclo = CICLO_PRELAVAR;
      }
      if (botaoDireitoPressionado){
        entraEstadoEncher(ENCHER_1);
        ciclo = CICLO_LAVAR;
      }
      break;
     case ENCHER_1:
      if(cheio){
        entraEstadoAspergir(LAVAR);
      }
      if (botaoDireitoPressionado){
        entraEstadoPausado();
      }
      break;
     case LAVAR:
      if ( temperatura < MINTEMPERATURE ){
        entraEstadoAquecer(AQUECER_1);
      }
      if (contadorTimerAquecido > tempoAquecido) {
          timerAquecido = false;
          entraEstadoEsvaziar(ESVAZIAR_1);
      }
      if (botaoDireitoPressionado){
        entraEstadoPausado();
      }
      break;
     case AQUECER_1:
      if ( temperatura > MAXTEMPERATURE ){
        if( !timerLigado ) {
          timerLigado = true;
          Serial.print("timerligado = true\n");
          timerAquecido = true;
          volatileContadorTimerAquecido = 0;
          //primeiraChamadaDoTimer3 = true;
          //Timer3.initialize(tempoAquecido);
          //Timer3.attachInterrupt(entraEstadoEsvaziar_1_Timer3);
          //timer.in(tempoAquecido, entraEstadoEsvaziar,ESVAZIAR_1);
        }
        entraEstadoAspergir(LAVAR);
      } else {
        if (contadorTimerAquecido > tempoAquecido) {
            timerAquecido = false;
            entraEstadoEsvaziar(ESVAZIAR_1);
        }
      }
      if (botaoDireitoPressionado){
        entraEstadoPausado();
      }
      break;
     case ESVAZIAR_1:
      if (contadorTimerExaustao > tempoExaustao) {
          timerExaustao = false;
          entraEstadoEncher(ENCHER_2);
      }
      if (botaoDireitoPressionado){
        entraEstadoPausado();
      }
      break;
     case ENCHER_2:
      if(cheio){
        entraEstadoAspergir(ENXAGUE_1);
      }
      if (botaoDireitoPressionado){
        entraEstadoPausado();
      }
      break;
     case ENXAGUE_1:
      /*Serial.print("contadorTimerExague : ");
      Serial.print(contadorTimerExague);
      Serial.print(" - tempoExague : ");
      Serial.print(tempoExague);
      Serial.print("\n");*/
      if (contadorTimerExague > tempoExague) {
          timerExague = false;
          entraEstadoEsvaziar(ESVAZIAR_2);
      }
      if (botaoDireitoPressionado){
        entraEstadoPausado();
      }
      break;
     case ESVAZIAR_2:
      if (contadorTimerExaustao > tempoExaustao) {
          timerExaustao = false;
          if(ciclo == CICLO_PRELAVAR){
            entraEstadoDesligado();
          } else {
            entraEstadoEncher(ENCHER_3);
          }
      }
      if (botaoDireitoPressionado){
        entraEstadoPausado();
      }
      break;
     case ENCHER_3:
      if(cheio){
        entraEstadoAspergir(ENXAGUE_2);
      }
      if (botaoDireitoPressionado){
        entraEstadoPausado();
      }
      break;
     case ENXAGUE_2:
      if ( temperatura < MINTEMPERATURE ){
        entraEstadoAquecer(AQUECER_2);
      }
      if (botaoDireitoPressionado){
        entraEstadoPausado();
      }
      break;
     case AQUECER_2:
      if ( temperatura > MAXTEMPERATURE ){
        entraEstadoEsvaziar(ESVAZIAR_3);
      }
      if (botaoDireitoPressionado){
        entraEstadoPausado();
      }
      break;
     case ESVAZIAR_3:
      if (contadorTimerExaustao > tempoExaustao) {
          timerExaustao = false;
          if(ciclo == CICLO_ESVAZIAR){
            entraEstadoDesligado();
          } else {
            entraEstadoVentilar(VENTILAR);
          }
      }
      if (botaoDireitoPressionado){
        entraEstadoPausado();
      }
      break;
     case VENTILAR:
      if (contadorTimerVentilacao > tempoVentilacao) {
          timerVentilacao = false;
          entraEstadoDesligado();
      }
      if (botaoDireitoPressionado){
        entraEstadoPausado();
      }
      break;
     case PAUSADO:
      if (botaoEsquerdoPressionado){
         retornaDeEstadoPausado();
      }
      if (botaoDireitoPressionado){
        entraEstadoDesligado();
      }
      break;
  }
}
