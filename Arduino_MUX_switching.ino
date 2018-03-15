/*
 *  Thruth table for MUX/DEMUX, 'Chan on' column is pin on IC not
 *  Arduino pins
 *  |   E   |   s2  |   s1  |   s0  | Chan On  | 
 *  |  Low  |  Low  |  Low  |  Low  |  pin 13  |
 *  |  Low  |  Low  |  Low  |  High |  pin 14  |
 *  |  Low  |  Low  |  High |  Low  |  pin 15  |
 *  |  Low  |  Low  |  High |  High |  pin 12  |
 *  |  Low  |  High |  Low  |  Low  |  pin 1   |
 *  |  Low  |  High |  Low  |  High |  pin 5   |
 *  |  Low  |  High |  High |  Low  |  pin 2   |
 *  |  Low  |  High |  High |  High |  pin 4   |
 *  |  High |  NA   |  NA   |  NA   |  all off |
 */



char h = HIGH;
char l = LOW;
float data = 0;
char chan[8][3] = {{h,l,l},{h,h,l},{h,h,h},
                   {h,l,h},{l,l,h},{l,h,l},
                   {l,h,h},{l,l,l}}; 
int demux1[8] = {1,2,3,4,5,6,7,8};
int demux2[8] = {6,5,8,7,2,1,4,3};
int mux[8][6] = {{2,3,4,5,7,8},{1,3,4,6,7,8},
                 {1,2,4,5,6,7},{1,2,3,5,6,8},
                 {1,3,4,6,7,8},{2,3,4,5,7,8},
                 {1,2,3,5,6,8},{1,2,4,5,6,7}}; 
                   
int volt_read = 0;// pin A0 on Arduino, pin 3 on MUX
int demux1_s0 = 5;// set s0 to pin 5 on Arduino, pin 11 on MUX1
int demux1_s1 = 6;// set s0 to pin 6 on Arduino, pin 10 on MUX1
int demux1_s2 = 7;// set s0 to pin 7 on Arduino, pin 9 on MUX1
int demux2_s0 = 8;// set s0 to pin 8 on Arduino, pin 11 on MUX2
int demux2_s1 = 9;// set s0 to pin 9 on Arduino, pin 10 on MUX2
int demux2_s2 = 10;// set s0 to pin 10 on Arduino, pin 9 on MUX2
int mux_s0 = 2;// set s0 to pin 2 on Arduino, pin 11 on MUX3
int mux_s1 = 3;// set s0 to pin 3 on Arduino, pin 10 on MUX3
int mux_s2 = 4;// set s0 to pin 4 on Arduino, pin 9 on MUX3
int EE = 11; //Enable (curr and gnd MUX) to pin 8 on Arduino, pin 6 on MUX 1&2 
int EE_2 = 12;//Enable (volt read MUX) to pin 9 on Arduino, pin 6 on MUX3

float BITS_TO_VOLTS = 3.3/1024;

void setup() {
  // put your setup code here, to run once:
  //Serial.begin(115200);
  pinMode(demux1_s0, OUTPUT);
  pinMode(demux1_s1, OUTPUT);
  pinMode(demux1_s2, OUTPUT);
  pinMode(demux2_s0, OUTPUT);
  pinMode(demux2_s1, OUTPUT);
  pinMode(demux2_s2, OUTPUT);
  pinMode(mux_s0, OUTPUT);
  pinMode(mux_s1, OUTPUT);
  pinMode(mux_s2, OUTPUT);
  pinMode(EE, OUTPUT);
  pinMode(EE_2, OUTPUT);
//  digitalWrite(EE,LOW); //Enable pin set low
//  digitalWrite(EE_2, LOW);
}

void loop() {
  

  for( int i = 0; i<=7; i++){
    //Serial.print("Configuration #");
    //Serial.println(i+1);
    
    digitalWrite(demux1_s0,chan[demux1[i]-1][2]);
    digitalWrite(demux1_s1,chan[demux1[i]-1][1]);
    digitalWrite(demux1_s2,chan[demux1[i]-1][0]);
    digitalWrite(demux2_s0,chan[demux2[i]-1][2]);
    digitalWrite(demux2_s1,chan[demux2[i]-1][1]);
    digitalWrite(demux2_s2,chan[demux2[i]-1][0]);
    digitalWrite(EE,LOW); //Enable pin set low
  
    for(int j = 0; j <= 5; j++){
      digitalWrite(mux_s0, chan[mux[i][j]-1][2]);
      digitalWrite(mux_s1, chan[mux[i][j]-1][1]);
      digitalWrite(mux_s2, chan[mux[i][j]-1][0]);
      digitalWrite(EE_2, LOW);
      data = analogRead(volt_read)*BITS_TO_VOLTS;
      //Serial.println(data);
      digitalWrite(EE_2, HIGH);
    }
    //Serial.println(' ');

    //delay(10);
    digitalWrite(EE,HIGH); //Enable pin set high
    //delay(10);
    
  }



}
