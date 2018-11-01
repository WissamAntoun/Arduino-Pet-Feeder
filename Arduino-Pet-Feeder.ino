#include <SoftwareSerial.h> 
#include <Hx711.h>
#include <Servo.h> 

int servoPin = 2;
int IR=7; 
int power=3;
int modem=4;
int waterled=5;
int foodled=6;
const int RX1 = 10;
const int TX1 = 11;
int pump=8;

SoftwareSerial Serial2(RX1,TX1);
Hx711 wscale(A4, A5);
Hx711 fscale(A2, A3);
Servo Servo1; 

String GSMRead;
String currentNumber;
String message;
boolean GSMstatus=0;
boolean water;
boolean food;
boolean bowlfood=0;
boolean bowlwater=0;

void setup() {
  Serial2.begin(9600);  //Baud rate of the GSM/GPRS Module 
  Serial.begin(9600);
  
  Servo1.attach(servoPin);
  closeservo();

  pinMode(IR,INPUT);//IR pin
  pinMode(power, OUTPUT);
  pinMode(modem, OUTPUT);
  pinMode(waterled, OUTPUT);
  pinMode(foodled, OUTPUT);
  pinMode(pump,OUTPUT);

  digitalWrite(power, HIGH);
  Serial.println("Delaying 10s till the modem is powered");
  delay(20000);
  digitalWrite(modem, HIGH);
  

  Serial.println("set SMS mode to txt");  // set SMS mode to text
 Serial2.print("AT+CSCS=\"GSM\"\r");
  delay(500);
  Serial.println("set SMS mode to txt");  // set SMS mode to text
Serial2.print("AT+CMGF=1\r");  // set SMS mode to text
  delay(500);
  Serial.println("SMS is send to arduino when it arrives");
Serial2.print("AT+CNMI=2,2,0,0,0\r");//SMS is send to arduino when it arrives
  delay(500);
  Serial.println("delete all SMS"); // delete all SMS
Serial2.println("AT+CMGD=1,4"); // delete all SMS
  
  
}

void loop() {

  int i=0;
  char commandbuffer[200];

  if(Serial2.available()){    
     delay(10);
    
     while( Serial2.available() && i< 199) {
        commandbuffer[i++] = Serial2.read();     
     }
     commandbuffer[i++]='\0';
     GSMRead = (char*)commandbuffer;
      String serial;
      serial=GSMRead;
      Serial.println(serial);
      serial=GSMRead.substring(2,6);
      Serial.println(serial);
      serial=getNumber(GSMRead);
      Serial.println(serial);
      serial=getMessage(GSMRead);
      Serial.println(serial);
      Serial.println("imm here");
     if(GSMRead.substring(2,6)=="+CMT")
     {
     String temp=getNumber(GSMRead);
     Serial.print("extracted number is ");
     Serial.println(temp);
      if(!currentNumber.equals(temp))
      {  currentNumber=temp;
         Serial.println("im here 2");
         delay(100);
        }
      message=getMessage(GSMRead);
      Serial.print("extracted message is");
      Serial.println(message);
      execute(message);
      }      
          
  }//if(Serial2.available())
  delay(200);
  contcheckwater();//check for water in container
  contcheckfood();//checl for food in container
  //bowlcheckwater();
  bowlcheckfood();
}//LOOP end


void closeservo()
{
  Servo1.write(15);
}

String getNumber(String r)
{
  int i=6;
  while(r.substring(i, i+1) != ("+")&&i<50)
  {
      i++;
  }
  if(i<45)
  {
    return r.substring(i,i+12);
  }
  else
  {
    return "none";
  }    
}

String getMessage(String r)
{   int i=0;
   while(r.substring(i, i+3) != ("+08")&&i<50)
  {
      i++;
  }
  if(i<45)
  {
    return r.substring(i+2+2,r.length());
  }
  else
  {
    return "none2";
  }  
}

void contcheckwater()
{
  if(analogRead(A0)>150)
  {
    water=1;
    Serial.println("water not empty");
    digitalWrite(waterled, LOW);    
    }
    else
    {
      water=0;
     // sendmessage("No water left");
      Serial.println("No water left");
      digitalWrite(waterled, HIGH);
      }
}

void contcheckfood()
{
  if(digitalRead(IR))
  {
    food=0;//no food detected
    //sendmessage("No food left");
    Serial.println("NO food left");
    digitalWrite(foodled, HIGH);
  }
  else
  {
    food=1;//food detected
    Serial.println("food not empty");
    digitalWrite(foodled, LOW);
    }
}

void bowlcheckfood()
{
  if(fscale.getGram()>200)
  {
   bowlfood=1;
   Serial.println("there is food in the bowl");
  }
  else
  {
    if(bowlfood==1)
    {
      sendmessage("the dog ate his food");
      Serial.println("the dog ate his food");
      bowlfood=0;
      }
  }
}

void bowlcheckwater()
{
  if(wscale.getGram()>200)
  {
    bowlwater=1;
    Serial.println("there is water in the bowl");
  }
  else
  {
    if(bowlwater==0)
    {
      sendmessage("the dog drank his water");
      bowlwater=0;
      Serial.println("dog drank his water");
      }
  }
}

void sendmessage(String s)
{
  Serial2.print("AT+CMGS=\"");
  Serial2.print(currentNumber);
  Serial2.print("\"\r");
  delay(500);
   Serial2.print(s);
   Serial2.print("\r");   //The text of the message to be sent
  delay(1000);
  Serial2.write(0x1A);
  delay(5000); 
}

void execute(String m)
{
  String seriall;
   seriall=m.substring(2,7);
   Serial.print("substring 2 7 = ");
    Serial.println(seriall);
     seriall=m.substring(8,9);
       Serial.print("substring 8 9= ");
    Serial.println(seriall);
    delay(2000);
  if(m.substring(2,6).equals("Feed")||m.substring(2,6).equals("feed"))
  {
    if(m.substring(7,8).equals("1"))//dispense water
    { if(water)
      {
        digitalWrite(pump, HIGH);
        delay(5000);
        digitalWrite(pump, LOW);
      }
      else
      {
        sendmessage("The water container is empty");
      }
     
     }
      if(m.substring(7,8).equals("2"))//dispense food
    { 
      
      if(food)
      {
        Servo1.write(45); 
        delay(3000); 
        Servo1.write(15);
      }
      else
      {
        sendmessage("The fodd container is empty");
      }
         
     }
     if(m.substring(7,8).equals("3"))//dispense food
    { 
      if(water)
      {
        if(food)
        {
          Servo1.write(0);
          digitalWrite(pump, HIGH); 
          delay(3000); 
          Servo1.write(60); 
          delay(2000);
          digitalWrite(pump, LOW);
        }
        else
        {
          digitalWrite(pump, HIGH);
          delay(5000);
          digitalWrite(pump, LOW);
          sendmessage("The water container isnt empty but the food container is");
        }
      }
      else
      {
        if(food)
        {
           Servo1.write(45); 
          delay(3000); 
          Servo1.write(15);
          sendmessage("The water container is empty but the food container isn't");
        }
        else
        {
          sendmessage("The water and the food container are both empty");
        }
      }
       
     }     
  }
  else if(m.substring(2,7).equals("check")||m.substring(2,7).equals("Check"))
  {
    if(m.substring(8,9).equals("1"))
    {
      if(water)
      {
        sendmessage("The water container isnt empty");
      }
      else
      {
        sendmessage("The water container is empty");
      }
    }

    if(m.substring(8,9).equals("2"))
    {
      if(food)
      {
        sendmessage("The food container isnt empty");
      }
      else
      {
        sendmessage("The fodd container is empty");
      }
    }

    if(m.substring(8,9).equals("3"))
    {
      if(water)
      {
        if(food)
        {
          sendmessage("The water and the food containers aren't empty");
        }
        else
        {
          sendmessage("The water container isnt empty but the food container is");
        }
      }
      else
      {
        if(food)
        {
          sendmessage("The water container is empty but the food container isn't");
        }
        else
        {
          sendmessage("The water and the food container are both empty");
        }
      }
    }
  }
  else
  {
    sendmessage("Unknown Command, Enter Feed-X or Check-X");
  }
}




