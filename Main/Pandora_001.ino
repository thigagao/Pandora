//Motores
/*
Cada motor usa dois pinos de controle:
  pwm: controla a potencia dos motores.
    -- Motor desligado = 0v no pwm
  drt: controla o sentido de rotação do motor.
    -- Frente = 1
    -- Re = 0
-2 = drt direito
-3 = pwm direito
-4 = drt esquerdo
-5 = pwm esquerdo
*/
int motor[4] = {2,3,4,5};

//Sensores de linha
/*
Serão usados 4 sensores de linha, um em cada lado da base inferior do robo.
O sensor é dito ativo quando se encontrar com uma linha branca no campo.
A ação do robo deve ser dada no sentido contrario do lado que contem o sensor ativo.
2 sensores ativos em sequecia circular representa uma quina, entao, mover-se na reta transversao entre os dois sensore inativos.
2 sensores opostos ativos representa metade do robo fora, entao, checar qual sensor qua nao foi ativado nesse turno e no anterior e se mover para esta direcao.
Em caso de 3 sensores ativos, mover-se na direção do unico inativo.
-6 = sensor de linha frontal
-7 = sensor de linha direito
-8 = sensor de linha esquerdo
-9 = sensor de linha traseiro
*/
int line[4] = {6,7,8,9};

//Ultrassom
/*
Tatica basica de busca: enviar um pulso e esperar sua reflexo.
-10 = trigger = pulso de comprimento escalado
-11 = echo = leitura de retorno
*/
int trigger = 10;
int echo = 11;
int pulse_time = 10;
int time_out = 100;
int dist = 0;

//IR
/*
Tatica basica de busca: emitir um feixe de infravermelho e esperar sua reflexao.
Permite identificar inimigos em curto alcance
-10 = esquerda
-11 = direita
*/
int ir[2] = {10,11};

//Armazenameto de leitura e movimentos
unsigned int history_line_sense = 0;  //Valores lidos dos sensores de linha nos ultimos 4 loops
unsigned int line_sense = 0;         //Valores lidos dos sensores de linha no loop atual
boolean motor_act_r_drt = 0;     //Direcao de rotacao do motor direito
int motor_act_r_pwm = 0;         //Potencia do motor direito
boolean motor_act_l_drt = 0;     //Direcao de rotacao do motor esquerdo
int motor_act_l_pwm = 0;         //Potencia do motor esquerdo
unsigned int ir_sense = 0;    //Valores lidos pelos ir

//***************************************************************************************************************
//***************************************************************************************************************

void setup() {
  //Declaraço de pinos
  for(int a = 0; a < 2; a++){
    pinMode(motor[a],OUTPUT);
    pinMode(motor[a+2],OUTPUT);
    pinMode(line[a], INPUT);
    pinMode(line[a+2], INPUT);
    pinMode(ir[a], INPUT);
  }
  pinMode(trigger, OUTPUT);
  pinMode(echo, INPUT);
  
  fight();
}

void loop() {
  no_safe_sense();
  think();
  delay(10);
}

//***************************************************************************************************************
//***************************************************************************************************************

//Tatica de inicio
void fight(){
  for(int a = 0; a < 8; a++){
    safe_sense();
    delay(450);
  }
  front_motor(255);
  time_to_walk(15);
}

//***************************************************************************************************************
//***************************************************************************************************************
//Act (interface do motor)
void break_act(){ //Freio
  motor_act_r_pwm = motor_act_r_pwm/5;
  motor_act_l_pwm = motor_act_l_pwm/5;
}
void turn_off_motor(){ //Desligar motores
  motor_act_r_drt = 1;
  motor_act_r_pwm =0;
  motor_act_l_drt = 1;
  motor_act_l_pwm = 0;
  delay(10);
}
void front_motor(int vel){ //mover para frente
  motor_act_r_drt = 1;
  motor_act_r_pwm =vel;
  motor_act_l_drt = 1;
  motor_act_l_pwm = vel;
  delay(10);
}
void back_motor(int vel){ //mover para tras
  motor_act_r_drt = 0;
  motor_act_r_pwm =vel;
  motor_act_l_drt = 0;
  motor_act_l_pwm = vel;
  delay(10);
}
void turn_r_motor(int vel){ //girar frente do robo para direita
  motor_act_r_drt = 0;
  motor_act_r_pwm =vel;
  motor_act_l_drt = 1;
  motor_act_l_pwm = vel;
  delay(10);
}
void turn_l_motor(int vel){ //girar frente do robo para esquerda
  motor_act_r_drt = 1;
  motor_act_r_pwm =vel;
  motor_act_l_drt = 0;
  motor_act_l_pwm = vel;
  delay(10);
} 
void time_to_walk(int time){ //Delay com verificacao
  int cont = 0;
  while(line_sense==0 || cont<time){
    no_safe_sense();
    cont++;
    delay(10);
  }
}
void search_motor(int ir_test){ //metodo de busca
  front_motor(100);
  time_to_walk(10);
  break_act();
  if(line_sense!=0) { turn_off_motor(); return; }
  turn_off_motor();
  if(ir_test==0 || ir_test==1) turn_r_motor(200);
  else turn_l_motor(200);
  time_to_walk(5);
  if(line_sense!=0) { turn_off_motor(); return; }
}
void act(){
  switch(ir_sense){
    //Sei la
    default: break;
    //Nenhum sensor ativo
    case 0: //0-0
      if(dist<150){
        front_motor(250-dist);
        time_to_walk(5);
      }
      else{
        search_motor(ir_sense);
        if(line_sense!=0) break;
        safe_sense();
      }
    break; 
    case 1: //0-1
      turn_r_motor(200);
      time_to_walk(5);
      if(line_sense!=0) { turn_off_motor(); break; }
      if(dist<150){ safe_sense(); }
      else{ safe_sense(); }
    break;
    case 2: //1-0
      turn_l_motor(200);
      time_to_walk(5);
      if(line_sense!=0) { turn_off_motor(); break; }
      if(dist<150){ safe_sense(); }
      else{ safe_sense(); }
    break;
    case 3: //1-1
      attack();
    break;
  }
}
void attack(){
  front_motor(255);
  time_to_walk(5);
}

//***************************************************************************************************************
//***************************************************************************************************************
//Sense
void no_safe_sense(){
  //Leitura dos sensores de linha
  history_line_sense <<= 4;
  line_sense = 0;
  if(digitalRead(line[0])) { history_line_sense = history_line_sense|1; line_sense = line_sense|1; }
  if(digitalRead(line[1])) { history_line_sense = history_line_sense|2; line_sense = line_sense|2; }
  if(digitalRead(line[2])) { history_line_sense = history_line_sense|4; line_sense = line_sense|4; }
  if(digitalRead(line[3])) { history_line_sense = history_line_sense|8; line_sense = line_sense|8; }
}

void safe_sense(){
  //Leitura do IR
  ir_sense = 0;
  if(digitalRead(ir[0])) { ir_sense = line_sense|1; }
  if(digitalRead(ir[1])) { ir_sense = line_sense|2; }
  //Leitura do ultrassom
  if(!((ir_sense&1) && (ir_sense&2))){
    digitalWrite(trigger, LOW); delayMicroseconds(2);
    digitalWrite(trigger, HIGH); delayMicroseconds(pulse_time);
    digitalWrite(trigger, LOW); 
    dist = ((pulseIn(echo,HIGH,time_out))*17);
    if (dist == 0) dist = time_out*17;
  }
}

//***************************************************************************************************************
//***************************************************************************************************************

void think(){
  switch(line_sense){
    //Sei la
    default: break;
    //Nenhum sensor ativo
    case 0: //0-0-0-0
      act();
    break;  
    //Sensor 1 ativo
    case 1: //0-0-0-1
      break_act();
      turn_off_motor();
      //turn_r_motor(255);
      //turn_l_motor(255);
      if(check_history()) turn_r_motor(255);
      else turn_l_motor(255);
    break;
    //Sensor 2 ativo
    case 2: //0-0-1-0
      break_act();
      turn_off_motor();
      turn_l_motor(255);
    break;
    //Sensores 2 e 1 ativos
    case 3: //0-0-1-1
      break_act();
      turn_off_motor();
      turn_l_motor(100);
    break;
    //Sesor 3 ativo
    case 4: //0-1-0-0
      break_act();
      turn_off_motor();
      front_motor(255);
    break;
    //Sensores 3 e 1 ativos
    case 5: //0-1-0-1
      break_act();
      turn_off_motor();
      //turn_r_motor(100);
      //turn_l_motor(100);
      if(check_history()) turn_r_motor(100);
      else turn_l_motor(100);
    break;
    //Sensores 3 e 2 ativos
    case 6: //0-1-1-0
      break_act();
      turn_off_motor();
      turn_l_motor(100);
    break;
    //Sensores 3, 2 e 1 ativos
    case 7: //0-1-1-1
      break_act();
      turn_off_motor();
      turn_l_motor(100);
    break;
    //Sensor 4 ativo
    case 8: //1-0-0-0
      break_act();
      turn_off_motor();
      turn_r_motor(255);
    break;
    //Sensores 4 e 1 ativos
    case 9: //1-0-0-1
      break_act();
      turn_off_motor();
      turn_r_motor(100);
    break;
    //Sesores 4 e 2 ativos
    case 10: //1-0-1-0
      break_act();
      turn_off_motor();
      front_motor(100);
    break;
    //Sesores 4, 2 e 1 ativos
    case 11: //1-0-1-1
      break_act();
      turn_off_motor();
      back_motor(255);
    break;
    //Sensores 4 e 3 ativos
    case 12: //1-1-0-0
      break_act();
      turn_off_motor();
      turn_r_motor(100);
    break;
    //Sensores 4,3 e 1 ativos
    case 13: //1-1-0-1
      break_act();
      turn_off_motor();
      turn_r_motor(100);
    break;
    //Sensores 4, 3 e 2 ativos
    case 14: //1-1-1-0
      break_act();
      turn_off_motor();
      front_motor(255);
    break;
    //Todos os sensores ativos
    case 15: //1-1-1-1
      //Nao sei 
    break;
  }
}
boolean check_history(){
  int cont_r = 0;
  int cont_l = 0;
  if((history_line_sense>>4)&2) cont_r++;
  if((history_line_sense>>8)&2) cont_r++;
  if((history_line_sense>>12)&2) cont_r++;
  if((history_line_sense>>4)&8) cont_l++;
  if((history_line_sense>>8)&8) cont_l++;
  if((history_line_sense>>12)&8) cont_l++;
  return cont_r >= cont_l;
}
