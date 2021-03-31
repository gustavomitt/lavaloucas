#include "Adafruit_GFX.h" // Biblioteca Gráfica Adafruit
#include <MCUFRIEND_kbv.h> // Bibllioteca para controle do lcd 
#include <TouchScreen.h>
#include "max6675.h"
//#include <arduino-timer.h>
#include <TimerOne.h>
#include <TimerThree.h>
#include <TimerFour.h>

// Definicao de estados
#define DESLIGADO 1
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
#define PAUSADO 13

// Cores que iremos utilizar em nosso projeto
#define PRETO   0x0000
#define VERMELHO     0xF800
#define VERDE   0x07E0
#define BRANCO 0xFFFF
#define AZUL 0x001F

// Objetos do touchscreen
const int XP=7,XM=A1,YP=A2,YM=6; //320x480 ID=0x0000
const int TS_LEFT=496,TS_RT=467,TS_TOP=439,TS_BOT=464;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
TSPoint tp;

#define MINPRESSURE 200
#define MAXPRESSURE 1000


volatile MCUFRIEND_kbv tft; // Objeto  de comunicação com  display

// Inicializa termopar
int thermo_gnd_pin = 45;
int thermo_vcc_pin = 47;
int thermo_so_pin  = 53;
int thermo_cs_pin  = 51;
int thermo_sck_pin = 49;
  
MAX6675 sensor(thermo_sck_pin, thermo_cs_pin, thermo_so_pin);


//String tempString;
//String tempStringOld;
volatile byte temperatura;
volatile byte temperaturaOld;

//#define MINTEMPERATURE 45.0
//#define MAXTEMPERATURE 55.0
#define MINTEMPERATURE 30.0
#define MAXTEMPERATURE 35.0

// Nivel
int nivel = 21;
volatile bool cheio;
volatile bool cheioOld;


// pino dos atuadores
const int bombaCirculacao = 22;
const int bombaExaustao = 24;
const int ebulidor = 26;
const int valvula = 28;


// Estado dos atuadores
boolean bombaDeCirculacaoFuncionando;
boolean bombaDeExaustaoFuncionando;
boolean ebulidorFuncionando;
boolean valvulaFuncionando;
boolean ebulidorFuncionandoOld;

// Estado do time
boolean timerLigado;
#define tempoAquecido 300000000 // 5 minutos
#define tempoExaustao  45000000 // 45 segundos
#define tempoExague   300000000 // 5 minutos

// variaveis de estado
volatile byte estadoAtual;
volatile byte estadoAnterior;

// Timer
//auto timer = timer_create_default(); // create a timer with default settings
volatile bool primeiraChamadaDoTimer3;
volatile bool primeiraChamadaDoTimer4;

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

void setState(int estado){
  estadoAnterior = estadoAtual;
  estadoAtual = estado;
  Serial.print("Novo estado: ");
  Serial.print(estadoAtual);
  Serial.print("\n");
}

void entraEstadoDesligado_Timer4(){
  if(primeiraChamadaDoTimer4) {
    primeiraChamadaDoTimer4 = false;
  }else {
    Timer4.detachInterrupt();
    entraEstadoDesligado();
  }

}

// funcoes de entradas dos estados
void entraEstadoDesligado(){
  
  Serial.print("Entrando no estado desligado\n");
  // seta estados
  setState(DESLIGADO);
  
  // acoes
  desligaBombaDeCirculacao();
  desligaBombaDeExaustao();
  desligaEbulidor();
  desligaValvula();
  
  telaDesligado();
}

void entraEstadoPausado(){
  Serial.print("Entrando no estado pausado\n");
  // seta estados
  setState(PAUSADO);

  // acoes
  desligaBombaDeCirculacao();
  desligaBombaDeExaustao();
  desligaEbulidor();
  desligaValvula();

  telaPausado();
}

void entraEstadoEncher_2_Timer4(){
  if(primeiraChamadaDoTimer4) {
    primeiraChamadaDoTimer4 = false;
  }else {
    Timer4.detachInterrupt();
    entraEstadoEncher(ENCHER_2);
  }
}

void entraEstadoEncher_3_Timer4(){
  if(primeiraChamadaDoTimer4) {
    primeiraChamadaDoTimer4 = false;
  }else {
    Timer4.detachInterrupt();
    entraEstadoEncher(ENCHER_3);
  }
}

void entraEstadoEncher(int estado){
  Serial.print("Entrando no estado Encher: ");
  Serial.print(estado);
  Serial.print("\n");
  // configura o timer como desligado
  if (estado = ENCHER_1){
    timerLigado = false;
    Serial.print("timerLigado = false\n");
  }
  
  // configura estados
  setState(estado);
    
  // acoes
  desligaBombaDeCirculacao();
  desligaBombaDeExaustao();
  desligaEbulidor();
  ligaValvula();

  telaLigado();
}

void entraEstadoAspergir(int estado){
  Serial.print("Entrando no estado Aspergir: ");
  Serial.print(estado);
  Serial.print("\n");

  switch(estado){
    case ENXAGUE_1:
      Serial.print("Ligando timer para acabar enxague\n");
      //timer.in(tempoExague, entraEstadoEsvaziar,ESVAZIAR_2);
      primeiraChamadaDoTimer3 = true;
      Timer3.initialize(tempoExague);
      Timer3.attachInterrupt(entraEstadoEsvaziar_2_Timer3);
      break;
  }
  
  // configura estados
  setState(estado);
  
  // acoes
  ligaBombaDeCirculacao();
  desligaBombaDeExaustao();
  desligaEbulidor();
  desligaValvula();

  telaLigado();
}

void entraEstadoAquecer(int estado){
  Serial.print("Entrando no estado Aquecer: ");
  Serial.print(estado);
  Serial.print("\n");

  // configura estados
  setState(estado);
  
  // acoes
  ligaBombaDeCirculacao();
  desligaBombaDeExaustao();
  ligaEbulidor();
  desligaValvula();
  
  telaLigado();
}

void entraEstadoEsvaziar_1_Timer3(){
  if(primeiraChamadaDoTimer3) {
    primeiraChamadaDoTimer3 = false;
  }else {
    Timer3.detachInterrupt();
    entraEstadoEsvaziar(ESVAZIAR_1);
  }
}

void entraEstadoEsvaziar_2_Timer3(){
  if(primeiraChamadaDoTimer3) {
    primeiraChamadaDoTimer3 = false;
  }else {
    Timer3.detachInterrupt();
    entraEstadoEsvaziar(ESVAZIAR_2);
  }
}

void entraEstadoEsvaziar(int estado){
  Serial.print("Entrando no estado Esvaziar: ");
  Serial.print(estado);
  Serial.print("\n");

  Serial.print("Ligando timer para desligar bomba de exaustao\n");
  switch(estado){
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
  }

  // configura estados
  setState(estado);
  
  // acoes
  desligaBombaDeCirculacao();
  ligaBombaDeExaustao();
  desligaEbulidor();
  desligaValvula();

  telaLigado();
}

bool botaoDireitoPressionado(){
  tp = ts.getPoint();
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  if (tp.z > MINPRESSURE && tp.z < MAXPRESSURE){
    /*Serial.print("tp.x=" + String(tp.x) + "\n");
    Serial.print("tp.y=" + String(tp.y) + "\n");
    Serial.print("tp.z=" + String(tp.z) + "\n");*/

    if (tp.x > 650 && tp.x < 900  && tp.y > 750 && tp.y < 870) {
      //tft.fillScreen(PRETO);
      //tft.setTextSize(2);
      //tft.setCursor(50, 50);
      //tft.print("tp.x=" + String(tp.x) + " tp.y=" + String(tp.y) + "   ");
      Serial.print("Botao direito pressionado\n");
      return true;
    }
  }
  return false;
}

bool botaoEsquerdoPressionado(){
  tp = ts.getPoint();
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  if (tp.z > MINPRESSURE && tp.z < MAXPRESSURE){
    /*Serial.print("tp.x=" + String(tp.x) + "\n");
    Serial.print("tp.y=" + String(tp.y) + "\n");
    Serial.print("tp.z=" + String(tp.z) + "\n");*/
    if (tp.x > 0 && tp.x < 500  && tp.y > 750 && tp.y < 870) {
      //tft.fillScreen(PRETO);
      //tft.setTextSize(2);
      //tft.setCursor(50, 50);
      //tft.print("tp.x=" + String(tp.x) + " tp.y=" + String(tp.y) + "   ");
      Serial.print("Botao esquerdo pressionado\n");
      return true;
    }
  }
  return false;
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

void telaLigado(){
  tft.setRotation(3); // Display é rotacionado para modo paisagem
  tft.fillScreen(PRETO); // Tela  é preenchida pela cor Branca
  escreveTexto(50,0,"Em Operacao",3,BRANCO); // Texto é escrito na posição (50,0)
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
  Serial.print(estadoAtual);
  Serial.print("\n");
}

void lerNivel(){
  cheio = (digitalRead(nivel) == HIGH);
}

void atualizaTemperatura(){
  if (temperatura != temperaturaOld){
    //Serial.print("Temperatura: " + tempString + "\n");
    temperaturaOld = temperatura;
    tft.fillRect(200,50, 60, 20, PRETO);
    escreveTexto(200,50,String(temperatura),2,VERDE); // Texto é escrito na posição (50,0)
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
  
  // chama ler temperatura a cada 1000 millis (1 second)
  Timer1.initialize(1000000);
  Timer1.attachInterrupt(lerTemperatura);  
  // Inicia o Timer3 para ser usado em temporizadores da lavagem
  Timer3.initialize(36000000000); // uma hora, tempo infinito
  // Inicia o Timer4 para ser usado em temporizadores do enchague
  Timer4.initialize(36000000000); // uma hora, tempo infinito

  // Nivel
  cheio = false;
  cheioOld = false;
  pinMode(nivel, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(nivel), lerNivel, CHANGE);
  //timer.every(1000, lerNivel);

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
  //noInterrupts();
  //float temperaturaCopy = temperatura;
  //interrupts();

  // Atualiza tela com valores que mudaram
  atualizaTemperatura();
  atualizaNivel(cheio);
  atualizaAquecimento();
  
  // put your main code here, to run repeatedly:
  /*Serial.print("Graus C = "); 
  Serial.print(tempString);
  Serial.print("\n"); 
  delay(500);*/

  //timer.tick(); // tick the timer
  
  //Serial.print("Estado: " + String(estado) + "\n");
  switch (estadoAtual){
    case DESLIGADO:
      //Serial.print("Case Desligado\n");
      if (botaoEsquerdoPressionado()){
        entraEstadoEsvaziar(ESVAZIAR_3);
      }
      if (botaoDireitoPressionado()){
        entraEstadoEncher(ENCHER_1);
      }
      break;
     case ENCHER_1:
      if(cheio){
        entraEstadoAspergir(LAVAR);
      }
      if (botaoDireitoPressionado()){
        entraEstadoPausado();
      }
      break;
     case LAVAR:
      if ( temperatura < MINTEMPERATURE ){
        entraEstadoAquecer(AQUECER_1);
      }
      if (botaoDireitoPressionado()){
        entraEstadoPausado();
      }
      break;
     case AQUECER_1:
      if ( temperatura > MAXTEMPERATURE ){
        if( !timerLigado ) {
          timerLigado = true;
          Serial.print("timerligado = true\n");
          primeiraChamadaDoTimer3 = true;
          Timer3.initialize(tempoAquecido);
          Timer3.attachInterrupt(entraEstadoEsvaziar_1_Timer3);
          //timer.in(tempoAquecido, entraEstadoEsvaziar,ESVAZIAR_1);
        }
        entraEstadoAspergir(LAVAR);
      }
      if (botaoDireitoPressionado()){
        entraEstadoPausado();
      }
      break;
     case ESVAZIAR_1:
      if (botaoDireitoPressionado()){
        entraEstadoPausado();
      }
      break;
     case ENCHER_2:
      if(cheio){
        entraEstadoAspergir(ENXAGUE_1);
      }
      if (botaoDireitoPressionado()){
        entraEstadoPausado();
      }
      break;
     case ENXAGUE_1:
      if (botaoDireitoPressionado()){
        entraEstadoPausado();
      }
      break;
     case ESVAZIAR_2:
      if (botaoDireitoPressionado()){
        entraEstadoPausado();
      }
      break;
     case ENCHER_3:
      if(cheio){
        entraEstadoAspergir(ENXAGUE_2);
      }
      if (botaoDireitoPressionado()){
        entraEstadoPausado();
      }
      break;
     case ENXAGUE_2:
      if ( temperatura < MINTEMPERATURE ){
        entraEstadoAquecer(AQUECER_2);
      }
      if (botaoDireitoPressionado()){
        entraEstadoPausado();
      }
      break;
     case AQUECER_2:
      if ( temperatura > MAXTEMPERATURE ){
        entraEstadoEsvaziar(ESVAZIAR_3);
      }
      if (botaoDireitoPressionado()){
        entraEstadoPausado();
      }
      break;
     case ESVAZIAR_3:
      if (botaoDireitoPressionado()){
        entraEstadoPausado();
      }
      break;
     case PAUSADO:
      if (botaoEsquerdoPressionado()){
         retornaDeEstadoPausado();
      }
      if (botaoDireitoPressionado ()){
        entraEstadoDesligado();
      }
      break;
  }
}
