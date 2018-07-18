/*
 * Sistema de controle da estufa 
 * Version: 2.0
 * 24/Novembro/2017
 * Hardware: 
 *            - Arduino Uno
 *            - RTC
 *            - LCD 16x2 I2C
 *            - Shield Rele 4 
 *            - Potenciometro 10k
 *            - Button
 *            - LM35
 * Software:
 *          O sistema ira ter uma tela de inicialão, após essa tela teremos uma que mostra dia, mes ano e horas.
 * 
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"

// Inicializa o display no endereco 0x27
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
byte zero = 0x00;


// Modulo RTC no endereco 0x68
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Domingo", "Segunda", "Terca", "Quarta", "Quinta", "Sexta", "Sabado"};


// Definição de Variaveis e Porta utilizadas
const int LM35    = A1;  // Define o pino que lera a saída do LM35
const int Rele    = 5;   // Define o pino que é usado para desativar o rele
float temperatura = 0;   // Variável que armazenará a temperatura medida
int Potenciometro = A2;  // center pin of the potentiometer
const int Enter   = 2;   // Botão de Seleção
int buttonState   = 0;   // variable for reading the pushbutton status
int StatusEnter   = 0;
const int ledPin  = 13;  // the number of the LED pin
int opcaoSwitch   = 0;
int meuArray[3] = {NULL};

/*
 * CapturaTemperatura
 * Lê a porta A0 (sensor lm35) 20 vezes, após essa captura faz os calculos e divide pela quantidade de vezes capturada
 */
float CapturaTemperatura()
{
   float leitura[50] = {0}; // Armazena valores recolhidos pelo sensor
   float armazena = 0;

   //Faz a captura de valores
   for (int i = 0; i < 50; i++) 
   {
      delay(100);
      // leitura[i] = ((float(analogRead(LM35)) * 5 / (1023)) / 0.01);
      leitura[i] = (float( 5.0 * analogRead(LM35) * 100.0) / 1024.0);
      delay(100);
  }
  
  //Soma o array de valores capturados
  for (int i = 0; i < 50; i++) 
  {
    armazena = armazena + leitura[i];
  }
  // Media de temperatura (todos os valores somados dividido pela quantidade)
  armazena = (armazena / 50);
  return armazena;
}

byte ConverteParaBCD(byte val)
{ 
  //Converte o número de decimal para BCD
  return ( (val/10*16) + (val%10) );
}

byte ConverteparaDecimal(byte val)  
{ 
  //Converte de BCD para decimal
  return ( (val/16*10) + (val%16) );
}


int pegaHora ()
{
  
  meuArray[3] = {NULL};
  // Le os valores (data e hora) do modulo DS1307
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_ADDRESS, 7);
  meuArray[0] = ConverteparaDecimal(Wire.read());
  meuArray[1] = ConverteparaDecimal(Wire.read());
  meuArray[2] = ConverteparaDecimal(Wire.read() & 0b111111);
  
}


void MostraHora()
{
  // Refazer esta função - Jeito mais facil que pensei no momento, passar os valores como array, e trabalhar com eles 
  // tomara que não estrague tudo.
  // Fazer backup dessa versão antes de alterar.
  
  // Le os valores (data e hora) do modulo DS3231
  DateTime now = rtc.now();
  pegaHora ();
  int horas        = meuArray[2];
  int minutos      = meuArray[1]; 
  int segundos     = meuArray[0];
  int controleTela = 0;

  //Comprimenta
  if((horas >= 1) && (horas <=12))
  {
     if(controleTela == 1)
     {
        lcd.clear();
     }
     lcd.setCursor(0,0);
     lcd.print("Bom dia!");
  }
  else if((horas >= 13) && (horas <=18))
  {
     
     lcd.setCursor(0,0);
     lcd.print("Boa Tarde!");
     controleTela = 0;
  }
  else
  {
     lcd.setCursor(0,0);
     lcd.print("Boa Noite!");
     controleTela = 1;
  }


  // Mostra hora no display
  lcd.setCursor(10,0);
  lcd.print(" ");
  //Acrescenta o 0 (zero) se a hora for menor do que 10
  if (horas <10)
    lcd.print("0");
    lcd.print(horas);
    lcd.print(":");
  //Acrescenta o 0 (zero) se minutos for menor do que 10
  if (minutos < 10)
     lcd.print("0");
     lcd.print(minutos);
     lcd.setCursor(2,1);
     
  //Mostra o dia da semana

  lcd.setCursor(3,1);
  lcd.print(now.day(), DEC);
  lcd.print("/");
  lcd.print(now.month(), DEC);
  lcd.print("/");
  lcd.print(now.year(), DEC);
}

int MostraPosicao()
{
    int sensorValue = map(analogRead(Potenciometro), 0, 1025, 1, 50);
    // int pot = map(analogRead(potPin), 0, 1025, 1, 50);
    delay (300);
    return (sensorValue);
}


int ajusteManual(int limiteMenor, int limiteMaior)
{
    int sensorValue = map(analogRead(Potenciometro), 0, 1025, limiteMenor, limiteMaior);
    // int pot = map(analogRead(potPin), 0, 1025, 1, 1000);
    delay (100);
    return (sensorValue);
}

int ajusteTempo()
{
    int sensorValue = map(analogRead(Potenciometro), 0, 1025, 1, 11);
    // int pot = map(analogRead(potPin), 0, 1025, 1, 1000);
    delay (300);
    return (sensorValue);
}






void montaTelaAquecimento( int temperaturaMaxima, int temperaturaMinima, int tempoAquecimento)
{
  // https://forum.arduino.cc/index.php?PHPSESSID=d30237rgtla7qogj3i94e616j7&topic=86907.15
    
  int horaFinal[3]    = {NULL};
  int horaInicial[3]  = {NULL};
  long diferencaTempo = NULL;
  long inicioTempo    = NULL;
  long fimTempo       = NULL;
  
  

  //Liga o Rele
  digitalWrite(Rele, HIGH); // Liga o Rele para esfriar a estufa
  digitalWrite(LED_BUILTIN, LOW);
  lcd.clear();


  //Acrescenta o tempo de secagem ao horario atual
  pegaHora();
  horaFinal[0] = meuArray[0];
  horaFinal[1] = meuArray[1]; 
  horaFinal[2]= meuArray[2] + tempoAquecimento;
  
  //Verifica a primeira vez a hora antes de entrar em loop
  pegaHora();
  horaInicial[0] = meuArray[0];
  horaInicial[1] = meuArray[1]; 
  horaInicial[2] = meuArray[2];
  

  // Converte o tempo para segundos e calcula a diferença
  inicioTempo = horaInicial[2]*3600 + horaInicial[1] * 60 + horaInicial[0];
  fimTempo    = horaFinal[2]*3600 + horaFinal[1] * 60 + horaFinal[0];
  diferencaTempo= fimTempo- inicioTempo;

  while(diferencaTempo >= 0)
  {
    
    digitalWrite(Rele, HIGH); // Inicio do aquecimento

    //Zera as variaveis utilizadas
    temperatura    = NULL;
    horaInicial[0] = NULL;
    horaInicial[1] = NULL; 
    horaInicial[2] = NULL;

    
    // Le os valores (data e hora) do modulo DS1307
    pegaHora();
    horaInicial[0] = meuArray[0];
    horaInicial[1] = meuArray[1]; 
    horaInicial[2] = meuArray[2];

    // Captura a temperatura atual para mostrar no display  
    temperatura = CapturaTemperatura();
    
    lcd.setCursor(0,0);
    lcd.print("Temp: ");
    lcd.setCursor(5, 0);
    lcd.print(temperatura);
    lcd.setCursor(14, 0);
    lcd.print("AQ");
    lcd.setCursor(0,1);
    lcd.print("A:");
    lcd.setCursor(2,1);
    
    //Acrescenta o 0 (zero) se a hora for menor do que 10
    if (horaInicial[2] <10)
    lcd.print("0");
    lcd.print(horaInicial[2]);
    lcd.print(":");

    //Acrescenta o 0 (zero) se minutos for menor do que 10
    if (horaInicial[1] < 10)
    lcd.print("0");
    lcd.print(horaInicial[1]);
    lcd.setCursor(9,1);
    
    // Mostra o horario de termino da secagem
    lcd.print("F:");
    //Acrescenta o 0 (zero) se a hora for menor do que 10
    if (horaFinal[2] <10)
    lcd.print("0");
    lcd.print(horaFinal[2]);
    lcd.print(":");
    //Acrescenta o 0 (zero) se minutos for menor do que 10
    if (horaFinal[1] <10)
    lcd.print("0");
    lcd.print(horaFinal[1]);

    if (temperatura >= temperaturaMaxima)
    {
      temperatura = 0;
      digitalWrite(Rele, LOW); // Desliga o Rele para esfriar a estufa
      digitalWrite(LED_BUILTIN, LOW);
      lcd.setBacklight(HIGH);
      lcd.setCursor(14, 0);
      lcd.print("ES");
      
      do{
          temperatura = CapturaTemperatura();
          lcd.setCursor(0,0);
          lcd.print("Temp: ");
          lcd.setCursor(5, 0);
          lcd.print(temperatura);
          
          //Atualiza a hora atua
          horaInicial[0] = NULL;
          horaInicial[1] = NULL; 
          horaInicial[2] = NULL;
          // Le os valores (data e hora) do modulo DS1307
          pegaHora();
          horaInicial[0] = meuArray[0];
          horaInicial[1] = meuArray[1]; 
          horaInicial[2] = meuArray[2];
          //Mostra no Display a hora atual
          
          
          lcd.setCursor(0,1);
          lcd.print("A:");
          lcd.setCursor(2,1);
          //Acrescenta o 0 (zero) se a hora for menor do que 10
          if (horaInicial[2] <10)
              lcd.print("0");
          lcd.print(horaInicial[2]);
          lcd.print(":");

          //Acrescenta o 0 (zero) se minutos for menor do que 10
          if (horaInicial[1] < 10)
              lcd.print("0");
          lcd.print(horaInicial[1]);
          delay(200);

          //Atualiza valores para verificar se continua em loop
          horaInicial[0] = NULL;
          horaInicial[1] = NULL; 
          horaInicial[2] = NULL;
          diferencaTempo = NULL;
    
          // Le os valores (data e hora) do modulo DS1307
          pegaHora();
          horaInicial[0] = meuArray[0];
          horaInicial[1] = meuArray[1]; 
          horaInicial[2] = meuArray[2];
          inicioTempo = horaInicial[2]*3600 + horaInicial[1] * 60 + horaInicial[0];
          diferencaTempo= fimTempo- inicioTempo;

          if(diferencaTempo <= 0)
          {
            break;
          }




          
          
      }while(temperatura > temperaturaMinima);
    }
    //Atualiza valores para verificar se continua em loop
    horaInicial[0] = NULL;
    horaInicial[1] = NULL; 
    horaInicial[2] = NULL;
    diferencaTempo = NULL;
    
    // Le os valores (data e hora) do modulo DS1307
    pegaHora();
    horaInicial[0] = meuArray[0];
    horaInicial[1] = meuArray[1]; 
    horaInicial[2] = meuArray[2];
    inicioTempo = horaInicial[2]*3600 + horaInicial[1] * 60 + horaInicial[0];
    diferencaTempo= fimTempo- inicioTempo;
   
    
  }// Fim do While
    
    lcd.clear();
    lcd.print("Fim do aquecimento....");
    lcd.setCursor(0,1);
    lcd.print("Desligando estufa!");
    digitalWrite(Rele, LOW); // Desliga o Rele para esfriar a estufa
    digitalWrite(LED_BUILTIN, LOW);
    // Implentar função do buzzer

    // Limpeza de todas as variaveis usadas
    horaInicial[0]  = NULL;
    horaInicial[1]  = NULL; 
    horaInicial[2]  = NULL;
    diferencaTempo  = NULL;
    horaFinal[3]    = {NULL};
    horaInicial[3]  = {NULL};
    diferencaTempo  = NULL;
    inicioTempo     = NULL;
    fimTempo        = NULL;
    
    delay(500);
}


void montaTelaAjusteManual()
{
  int prendeLoop       = 3;
  int potenciometro    = 0;
  int temperaturaSel   = 0;
  int tempoSelecionado = 0;
  
  while(prendeLoop != 0)
  {
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Temperatura:");
    potenciometro = ajusteManual(0,250);
    lcd.setCursor(0,1);
    lcd.print(potenciometro);
    delay(1800);     
     // Verifica se o botão foi pressionado
    buttonState = digitalRead(Enter);
    //Se sim a entrada do arduino muda para Ligada (HIGH)
    if (buttonState == HIGH) 
    {
      
      temperaturaSel = potenciometro;
      prendeLoop     = 0;
    }
  }

  
  potenciometro = NULL;
  prendeLoop     = 4;
  
  while(prendeLoop != 0)
  {
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Tempo (h):");
    potenciometro = ajusteTempo();
    lcd.setCursor(0,1);
    lcd.print(potenciometro);
    delay(1800);     
     // Verifica se o botão foi pressionado
    buttonState = digitalRead(Enter);
    //Se sim a entrada do arduino muda para Ligada (HIGH)
    if (buttonState == HIGH) 
    {
      
      tempoSelecionado = potenciometro;
      prendeLoop     = 0;
    }
  }

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Temperatura:");
  lcd.setCursor(12,0);
  lcd.print(temperaturaSel);
  lcd.setCursor(0,1);
  lcd.print("Tempo (H):");
  lcd.setCursor(12,1);
  lcd.print(tempoSelecionado);
  delay(3000);
  montaTelaAquecimento(temperaturaSel,(temperaturaSel-10),tempoSelecionado);
  
}


void ajustaDataHora()
{
  int ajusteHora    = 0;
  int ajusteminuto  = 0;
  int ajusteSegundo = 0;
  int ajusteDia     = 0;
  int ajusteMes     = 0;
  int ajusteAno     = 0;
  int prendeLoop    = 6;
  int potenciometro = 0;


  while(prendeLoop != 0)
  {
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Hora Atual:");
    potenciometro = ajusteManual(0,27);
    lcd.setCursor(0,1);
    lcd.print(potenciometro);
    delay(1800);     
     // Verifica se o botão foi pressionado
    buttonState = digitalRead(Enter);
    //Se sim a entrada do arduino muda para Ligada (HIGH)
    if (buttonState == HIGH) 
    {
      
      ajusteHora = potenciometro;
      prendeLoop     = 0;
    }
  }
  potenciometro = NULL;
  prendeLoop     = 4;

  while(prendeLoop != 0)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Hora Atual:");
    potenciometro = ajusteManual(0,69);
    lcd.setCursor(0,1);
    lcd.print(ajusteHora);
    lcd.print(":");    
    lcd.print(potenciometro);
    delay(1800);     
     // Verifica se o botão foi pressionado
    buttonState = digitalRead(Enter);
    //Se sim a entrada do arduino muda para Ligada (HIGH)
    if (buttonState == HIGH) 
    {
      
      ajusteminuto = potenciometro;
      prendeLoop     = 0;
    }
  }
  potenciometro = NULL;
  prendeLoop     = 6;
  lcd.clear();

  while(prendeLoop != 0)
  {
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Data Atual:");
    potenciometro = ajusteManual(1,36);
    lcd.setCursor(0,1);
    lcd.print(potenciometro);
    delay(1800);     
     // Verifica se o botão foi pressionado
    buttonState = digitalRead(Enter);
    //Se sim a entrada do arduino muda para Ligada (HIGH)
    if (buttonState == HIGH) 
    {
      
      ajusteDia  = potenciometro;
      prendeLoop = 0;
    }
  }

  potenciometro = NULL;
  prendeLoop     = 9;

  while(prendeLoop != 0)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Data Atual:");
    potenciometro = ajusteManual(1,14);
    lcd.setCursor(0,1);
    lcd.print(ajusteDia);
    lcd.print("/");
    lcd.print(potenciometro);
    delay(1800);     
     // Verifica se o botão foi pressionado
    buttonState = digitalRead(Enter);
    //Se sim a entrada do arduino muda para Ligada (HIGH)
    if (buttonState == HIGH) 
    {
      
      ajusteMes  = potenciometro;
      prendeLoop = 0;
    }
  }

  potenciometro = NULL;
  prendeLoop     = 26;

  while(prendeLoop != 0)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Data Atual:");

    lcd.setCursor(0,1);
    lcd.print(ajusteDia);
    lcd.print("/");
    lcd.print(ajusteMes);
    lcd.print("/");
    potenciometro = ajusteManual(2018,2040);
    lcd.print(potenciometro);
    delay(1800);     
     // Verifica se o botão foi pressionado
    buttonState = digitalRead(Enter);
    //Se sim a entrada do arduino muda para Ligada (HIGH)
    if (buttonState == HIGH) 
    {
      
      ajusteAno  = potenciometro;
      prendeLoop = 0;
    }
  }

  potenciometro = NULL;
  prendeLoop    = 0;

  lcd.clear();
  rtc.begin();
  rtc.lostPower();
  rtc.adjust(DateTime(ajusteAno, ajusteMes, ajusteDia, ajusteHora, ajusteminuto, 0));

  delay(3000);
  StatusEnter = 0;
  lcd.clear();
  MostraHora();
}


int meuMenuOperacional(int Opcao)
{
  switch (Opcao) {
    case 1:
      // Filamentos
      montaTelaAquecimento(60,55,2);
      delay(500);
      StatusEnter = 0;
      lcd.clear();
      break;
    case 2:
      //Granulos;      
      montaTelaAquecimento(80,75,4);
      delay(500);
      StatusEnter = 0;
      lcd.clear();
      break;
    case 3:
      montaTelaAjusteManual();
      delay(500);
      StatusEnter = 0;
      lcd.clear();
      break;
    case 4:
      ajustaDataHora();
      delay(500);
      StatusEnter = 0;
      lcd.clear();
    
    break;
    case 5:
    default:
      //Atribui o valor a variavel StatusEnter
      StatusEnter = 0;
      lcd.clear();
      MostraHora();
      break;
  }

  
}



void setup()
{
  //A linha é usada para acertar a Data e a Hora
  #ifndef ESP8266
    while (!Serial); // for Leonardo/Micro/Zero
  #endif

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  // Inicializa o botão de seleção (ENTER)
  pinMode(Enter, OUTPUT);
  pinMode(ledPin, OUTPUT);
  
  pinMode(Rele, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  //analogReference(INTERNAL);
  lcd.setBacklight(HIGH);
  
  pinMode(7, OUTPUT);
  lcd.begin(20,4); //Inicialização do LCD 
  lcd.noBacklight();
  delay(250);
  lcd.backlight();
  lcd.setCursor(0,2);
  lcd.write(" Controle Estufa"); 
  lcd.setCursor(0,3);
  lcd.write("**PrintGreen3D**"); 
  //lcd.blink();
  delay(3000);
  lcd.noBlink(); 
  
  for (int i = 0; i < 20; i++) {
   delay(200);
   lcd.scrollDisplayLeft();
  }
  delay(2500);
  lcd.clear(); // Turns off the blinking cursor
}



void loop()
{
    int potencia = 0;
    //Inicia mostrando a Hora
    MostraHora();
    
    // Verifica se o botão foi pressionado
    buttonState = digitalRead(Enter);
    //Se sim a entrada do arduino muda para Ligada (HIGH)
    if (buttonState == HIGH) 
    {
      //Limpo o LCD    
      lcd.clear();
      //Atribui o valor a variavel StatusEnter
      StatusEnter = 1;
      delay (200);
      while(StatusEnter == 1)
      {
          potencia = MostraPosicao();

          if((potencia >=0) && (potencia <= 10))
          {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("> Filamentos");
            lcd.setCursor(0,1);
            lcd.print("  Granulos");
            opcaoSwitch = 1;
          }
          else if ((potencia >=11) && (potencia <= 20))
          {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("  Filamentos");
            lcd.setCursor(0,1);
            lcd.print("> Granulos");
            opcaoSwitch = 2;
          }

          else if ((potencia >=21) && (potencia <= 30))
          {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("  Granulos");
            lcd.setCursor(0,1);
            lcd.print("> Def. Manual");
            opcaoSwitch = 3;
          }
          else if ((potencia >=31) && (potencia <= 40))
          {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("  Def. Manual");
            lcd.setCursor(0,1);
            lcd.print("> Ajuste hora");
            opcaoSwitch = 4;
          }
          else
          {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("  Ajuste hora");
            lcd.setCursor(0,1);
            lcd.print("> Sair");
            opcaoSwitch = 5;
          }

            
           //Verifica o botão foi precionado novamente
          delay(200);
          buttonState = digitalRead(Enter);
          if (buttonState == HIGH) 
          {
            meuMenuOperacional(opcaoSwitch);
            delay(200); 
          }
      }// Fim do while          
    
    }   
}

