int const r_bdr = 3;
int const g_bdr = 5;
int const b_bdr = 6;

int const r_lvr = 9;
int const g_lvr = 10;
int const b_lvr = 11;

int intLoopCounter = 0;

String strSerialInput = "";
String h = "";
String s = "";
String l = "";
String r = "";

int rgb[3];

void setup() 
{
  pinMode(r_bdr, OUTPUT);
  pinMode(g_bdr, OUTPUT);
  pinMode(b_bdr, OUTPUT);

  pinMode(r_lvr, OUTPUT);
  pinMode(g_lvr, OUTPUT);
  pinMode(b_lvr, OUTPUT);
  
  Serial.begin(115200);
}

void loop() 
{
  delay(1);
  CheckAndExecuteSerialCommand();  
}

void CheckAndExecuteSerialCommand()
{
  String serialData = GetPossibleSerialData();
  if (serialData != ""){

    h = getValue(serialData, ';', 0);
    s = getValue(serialData, ';', 1);
    l = getValue(serialData, ';', 2);
    r = getValue(serialData, ';', 3);

    hsi2rgb(h.toFloat(), s.toFloat(), l.toFloat());
    
    if (r.startsWith("lvr")){
      analogWrite(r_lvr, rgb[0]);
      analogWrite(g_lvr, rgb[1]);
      analogWrite(b_lvr, rgb[2]);
    }
    else if (r.startsWith("bdr")){
      analogWrite(r_bdr, rgb[0]);
      analogWrite(g_bdr, rgb[1]);
      analogWrite(b_bdr, rgb[2]);
    }   
  }
}

String GetPossibleSerialData()
{
  String retVal;
  int iteration = 10;
  if (strSerialInput.length() > 0){
    if (intLoopCounter > strSerialInput.length() + iteration){
        retVal = strSerialInput;
        strSerialInput = "";
        intLoopCounter = 0;
    } 
    intLoopCounter++;
  }
  return retVal;
}

void serialEvent(){  
  while (Serial.available()){    
    strSerialInput.concat((char) Serial.read());
  } 
}

void hsi2rgb(float H, float S, float I){
  int r, g, b;
  H = fmod(H,360); // cycle H around to 0-360 degrees
  H = 3.14159*H/(float)180; // Convert to radians.
  S = S>0?(S<1?S:1):0; // clamp S and I to interval [0,1]
  I = I>0?(I<1?I:1):0;
    
  // Math! Thanks in part to Kyle Miller.
  if(H < 2.09439){
    r = 255*I/3*(1+S*cos(H)/cos(1.047196667-H));
    g = 255*I/3*(1+S*(1-cos(H)/cos(1.047196667-H)));
    b = 255*I/3*(1-S);
  } else if (H < 4.188787){
    H = H - 2.09439;
    g = 255*I/3*(1+S*cos(H)/cos(1.047196667-H));
    b = 255*I/3*(1+S*(1-cos(H)/cos(1.047196667-H)));
    r = 255*I/3*(1-S);
  } else {
    H = H - 4.188787;
    b = 255*I/3*(1+S*cos(H)/cos(1.047196667-H));
    r = 255*I/3*(1+S*(1-cos(H)/cos(1.047196667-H)));
    g = 255*I/3*(1-S);
  }

 rgb[0]=r;
 rgb[1]=g;
 rgb[2]=b;
}

String getValue(String data, char separator, int index)
{
 int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;
  for(int i=0; i<=maxIndex && found<=index; i++){
  if(data.charAt(i)==separator || i==maxIndex){
  found++;
  strIndex[0] = strIndex[1]+1;
  strIndex[1] = (i == maxIndex) ? i+1 : i;
  }
 }
  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}


