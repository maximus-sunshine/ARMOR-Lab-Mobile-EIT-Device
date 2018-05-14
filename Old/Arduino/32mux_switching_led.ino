//Barebones switching for a 32 node configuration


char h = HIGH;
char l = LOW;

char chan[32][5] = {{l,l,l,l,l},{l,l,l,l,h},{l,l,l,h,l},{l,l,l,h,h},
                      {l,l,h,l,l},{l,l,h,l,h},{l,l,h,h,l},{l,l,h,h,h},
                      {l,h,l,l,l},{l,h,l,l,h},{l,h,l,h,l},{l,h,l,h,h},
                      {l,h,h,l,l},{l,h,h,l,h},{l,h,h,h,l},{l,h,h,h,h},
                      {h,l,l,l,l},{h,l,l,l,h},{h,l,l,h,l},{h,l,l,h,h},
                      {h,l,h,l,l},{h,l,h,l,h},{h,l,h,h,l},{h,l,h,h,h},
                      {h,h,l,l,l},{h,h,l,l,h},{h,h,l,h,l},{h,h,l,h,h},
                      {h,h,h,l,l},{h,h,h,l,h},{h,h,h,h,l},{h,h,h,h,h}};

int demux1[32] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32};
int demux2[32] = {24,23,22,21,20,19,18,17,32,31,30,29,28,27,26,25,8,7,6,5,4,3,2,1,16,15,14,13,12,11,10,9};
int mux[32][30];
int k = 0;

                   

int demux1_a0  = 24;
int demux1_a1 = 26;
int demux1_a2 = 28;
int demux1_a3 = 30;
int demux1_a4 = 32;

int demux2_a0 = 34;
int demux2_a1 = 36;
int demux2_a2 = 38;
int demux2_a3 = 40;
int demux2_a4 = 42;

int mux_a0 = 44;
int mux_a1 = 46;
int mux_a2 = 48;
int mux_a3 = 50;
int mux_a4 = 52;




void setup() {

  for( int i = 0; i<=31; i++){
    
    for(int j=0;j<=31;j++){ 
      
      if (demux1[i] != demux1[j] && demux2[i] != demux1[j]){
        
        mux[i][k] = demux1[j];
        k++;
      }
    }
    k = 0; 
  }
                   


  Serial.begin(115200);
  pinMode(demux1_a0, OUTPUT);
  pinMode(demux1_a1, OUTPUT);
  pinMode(demux1_a2, OUTPUT);
  pinMode(demux1_a3, OUTPUT);
  pinMode(demux1_a4, OUTPUT);

  pinMode(demux2_a0, OUTPUT);
  pinMode(demux2_a1, OUTPUT);
  pinMode(demux2_a2, OUTPUT);
  pinMode(demux2_a3, OUTPUT);
  pinMode(demux2_a4, OUTPUT);

  pinMode(mux_a0, OUTPUT);
  pinMode(mux_a1, OUTPUT);
  pinMode(mux_a2, OUTPUT);
  pinMode(mux_a3, OUTPUT);
  pinMode(mux_a4, OUTPUT);
  
 
}

void loop() {

  
  
  for( int i = 0; i<=31; i++){
    
    
    //Power and Ground Distribution
    digitalWrite(demux1_a0,chan[demux1[i]-1][4]);
    digitalWrite(demux1_a1,chan[demux1[i]-1][3]);
    digitalWrite(demux1_a2,chan[demux1[i]-1][2]);
    digitalWrite(demux1_a3,chan[demux1[i]-1][1]);
    digitalWrite(demux1_a4,chan[demux1[i]-1][0]);

    digitalWrite(demux2_a0,chan[demux1[i]-1][4]);
    digitalWrite(demux2_a1,chan[demux1[i]-1][3]);
    digitalWrite(demux2_a2,chan[demux1[i]-1][2]);
    digitalWrite(demux2_a3,chan[demux1[i]-1][1]);
    digitalWrite(demux2_a4,chan[demux1[i]-1][0]);
    
    
    //Inner Loop controls sampling
    for(int j = 0; j <= 29; j++){
      digitalWrite(mux_a0, chan[mux[i][j]-1][4]);
      digitalWrite(mux_a1, chan[mux[i][j]-1][3]);
      digitalWrite(mux_a2, chan[mux[i][j]-1][2]);
      digitalWrite(mux_a3, chan[mux[i][j]-1][1]);
      digitalWrite(mux_a4, chan[mux[i][j]-1][0]);
      
      delay(1);
      
    }
   
    
    
  }
  
}
