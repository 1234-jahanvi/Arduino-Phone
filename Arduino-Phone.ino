#include <SoftwareSerial.h>
SoftwareSerial Serial1(3, 2); // RX, TX

#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

String msg="";
String str1="";
String number="";
String str_sms="";
String instr="";
int ring=0;
int i=0;
int sms_flag=0;
char sms_num[3];
boolean at_flag=1;

int rec_read=0;
int temp=0;
int temp1=0;
char ok[2] = {'O','K'};
char sim_ready[]={'+','C','P','I','N',':','R','E','A','D','Y'};

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char hexaKeys[ROWS][COLS] = 
{
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {11, 10, 9, 8}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {7, 6, 5, 4};

Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

String ch="1,.?!@2abc3def4ghi5jkl6mno7pqrs8tuv9wxyz0 ";

void setup() 
{
  Serial.begin(9600);
  Serial1.begin(9600);
  lcd.init();
  lcd.home();
  lcd.backlight();
  lcd.print("Arduino-Phone");
  delay(1000);
  lcd.clear();
  lcd.print("System Ready...");
  gsm_init();
  delay(2000);
  Serial1.println("AT");
  Serial1.println("AT+CLIP=1");
}

void loop() 
{
  serialEvent();

  if(sms_flag==1)
  {
    lcd.clear();
    lcd.print("New Message");
    int ind=instr.indexOf("+CMTI: \"SM\",") + 12;
    int k=0;
    lcd.setCursor(0,1);
    lcd.print(ind);
    while(1)
    {
      while(instr[ind]!= 0x0D)
      {
        sms_num[k]=instr[ind];
        k++;
        ind++;
      }
      break;
    }
    ind=0;
    sms_flag=0;
    lcd.setCursor(0,1);
    lcd.print("Read SMS --> C");
    delay(4000);
    instr="";
    rec_read=1;
    temp1=1;
    i=0;
  }

  if(ring==1)
  {
    number="";
    int loc=instr.indexOf("+CLIP: \"");
    if(loc>0)
    {
      number+=instr.substring(loc+8,loc+13+8);
    }
    lcd.setCursor(0,0);
    lcd.print("Incoming.Press 0");
    lcd.setCursor(0,1);
    lcd.print(number);
    instr="";
    i=0;
  }
  else
  {
    serialEvent();
    lcd.setCursor(0,0);
    lcd.print("Call --> *      ");
    lcd.setCursor(0,1);
    lcd.print("SMS  --> #   ");
    lcd.print(" ");
  }
    char key=customKeypad.getKey();
    if(key)
    {
      if(key=='0')
      {
        if(ring==1)
        {
          lcd.clear();
          lcd.print("Answering");
          Serial1.println("ATA");
          delay(5000);
          lcd.clear();
        }
      }
      else if(key=='*')
      {
        call();
      }
      else if(key=='#')
      {
        sms();
      }
      else if(key=='C')
      {
        rec_read=0;
        lcd.clear();
        lcd.print("Please wait...");
        Serial1.print("AT+CMGR=");
        Serial1.println(sms_num);
        int sms_read=1;
        str_sms="";
        while(sms_read)
        {
          while(Serial1.available()>0)
          {
            char ch = Serial1.read();
            str_sms+=ch;
            if(str_sms.indexOf("OK")>0)
            {
              sms_read=0;
            }
          }
        }
        int l1=str_sms.indexOf("\"\r\n");
        int l2=str_sms.indexOf("OK");
        String sms=str_sms.substring(l1+3,l2-4);
        lcd.clear();
        lcd.print(sms);
        delay(5000);
      }
      delay(1000);
    }
  
  
}


void call()
{
  number="";
  lcd.clear();
  lcd.print("After Enter No.");
  lcd.setCursor(0,1);
  lcd.print("Press * to Call");
  delay(2000);
  lcd.clear();
  lcd.print("Enter Number:");
  lcd.setCursor(0,1);
  while(1)
  { 
      serialEvent();
      char key=customKeypad.getKey();
      if(key)
      {
        if(key=='*')
        {
          lcd.clear();
          lcd.print("Calling........");
          lcd.setCursor(0,1);
          lcd.print(number);
          Serial1.print("ATD+ +91");
          Serial1.print(number);
          Serial1.println(";");
          long stime=millis()+5000;
          int ans=1;
          while(ans==1)
          {   
             while(Serial1.available()>0)
            {
              if(Serial1.find(ok))
              {
                lcd.clear();
                lcd.print("Ringing....");
                str1="";
                while(ans==1)
                {   
                    while(Serial1.available()>0)
                    {
                      char ch=Serial1.read();
                      str1+=ch;
                      if(str1.indexOf("NO CARRIER")>0)
                      {
                        lcd.clear();
                        lcd.print("Call End");
                        delay(2000);
                        ans=0;
                        return;
                      }
                    }
                    char key=customKeypad.getKey();
                    if(key == '#')
                    {
                      lcd.clear();
                      lcd.print("Call End");
                      delay(2000);
                      ans=0;
                      return;
                    }
                    if(ans==0)
                    break;
                 }
               }
             }
          }
        }
        else
        {
          number+=key;
          lcd.print(key);
        }
      } 
  }
}

void sms()
{
  lcd.clear();
  lcd.print("Initialising...");
  Serial1.println("AT+CMGF=1");   // Configuring TEXT mode
  delay(400);
  lcd.clear();
  lcd.print("After Enter No.");
  lcd.setCursor(0,1);
  lcd.print("Press #        ");
  delay(2000);
  lcd.clear();
  lcd.print("Enter Rcpt No.:");
  lcd.setCursor(0,1);
  Serial1.print("AT+CMGS=\"+91");
  while(1)
  {
    serialEvent();
    char key = customKeypad.getKey();
    if(key)
    {
      if(key=='#')
      {
        number+="";
        Serial1.println("\"");
        break;
      }
      else
      {
        number+=key;
        Serial1.print(key);
        lcd.print(key);
      }
    }
  }
  lcd.clear();
  lcd.print("After Enter MSG ");
  lcd.setCursor(0,1);
  lcd.print("Press # to Send ");
  delay(2000);
  lcd.clear();
  lcd.print("Enter Your Msg");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0,0);
  alfakey();
}

void serialEvent()
{
   while(Serial1.available())
  {
    char ch=Serial1.read();
    instr+=ch;
    i++;

    if(instr[i-4] == 'R' && instr[i-3] == 'I' && instr[i-2] == 'N' && instr[i-1] == 'G' )
    {
      //Indicates Incoming Call
       ring=1;
    }

    if(instr.indexOf("NO CARRIER")>=0)
    {
       ring=0;
       i=0;
    }
    if(instr.indexOf("+CMTI: \"SM\"")>=0)
    {
      sms_flag=1;
    }
  }
}

void alfakey()
{
 int x=0,y=0;
 int num=0;
  while(1)
  {
    lcd.cursor();
    char key=customKeypad.getKey();
    if(key)
    {
       if(key=='1')
       {
         num=0;
         lcd.setCursor(x,y);
         lcd.print(ch[num]);
         for(int i=0;i<3000;i++)
         { 
            lcd.noCursor();
            char key=customKeypad.getKey();
            if(key=='1')
            {
               num++;
               if(num>5)
               num=0;
               lcd.setCursor(x,y);
               lcd.print(ch[num]);
               i=0;
               //delay(200);
            } 
         }
         x++;
         if(x>15)
         {
           x=0;
           y++;
           y%=2;
         }
        msg+=ch[num];
        }

         else if(key=='2')
       {
           num=6;
           lcd.setCursor(x,y);
           lcd.print(ch[num]);
           for(int i=0;i<3000;i++)
           { 
              lcd.noCursor();
              char key=customKeypad.getKey();
              if(key=='2')
              {
                 num++;
                 if(num>9)
                 num=6;
                 lcd.setCursor(x,y);
                 lcd.print(ch[num]);
                 i=0;
                 //delay(200);
              } 
           }
           x++;
           if(x>15)
           {
               x=0;
               y++;
               y%=2;
           }
           msg+=ch[num];
        }

        else if(key=='3')
       {
         num=10;
         lcd.setCursor(x,y);
         lcd.print(ch[num]);
         for(int i=0;i<3000;i++)
         { 
          lcd.noCursor();
          char key=customKeypad.getKey();
          if(key=='3')
          {
           num++;
           if(num>13)
           num=10;
           lcd.setCursor(x,y);
           lcd.print(ch[num]);
           i=0;
           //delay(200);
          } 
         }
         x++;
          if(x>15)
         {
           x=0;
           y++;
           y%=2;
         }
         msg+=ch[num];
        }

        else if(key=='4')
       {
         num=14;
         lcd.setCursor(x,y);
         lcd.print(ch[num]);
         for(int i=0;i<3000;i++)
         { 
          lcd.noCursor();
          char key=customKeypad.getKey();
          if(key=='4')
          {
           num++;
           if(num>17)
           num=14;
           lcd.setCursor(x,y);
           lcd.print(ch[num]);
           i=0;
           //delay(200);
          } 
         }
         x++;
          if(x>15)
         {
           x=0;
           y++;
           y%=2;
         }
         msg+=ch[num];
        }

       else if(key=='5')
       {
         num=18;
         lcd.setCursor(x,y);
         lcd.print(ch[num]);
         for(int i=0;i<3000;i++)
         { 
          lcd.noCursor();
          char key=customKeypad.getKey();
          if(key=='5')
          {
           num++;
           if(num>21)
           num=18;
           lcd.setCursor(x,y);
           lcd.print(ch[num]);
           i=0;
           //delay(200);
          } 
         }
         x++;
          if(x>15)
         {
           x=0;
           y++;
           y%=2;
         }
         msg+=ch[num];
        }

        else if(key=='6')
       {
         num=22;
         lcd.setCursor(x,y);
         lcd.print(ch[num]);
         for(int i=0;i<3000;i++)
         { 
          lcd.noCursor();
          char key=customKeypad.getKey();
          if(key=='6')
          {
           num++;
           if(num>25)
           num=22;
           lcd.setCursor(x,y);
           lcd.print(ch[num]);
           i=0;
           //delay(200);
          } 
         }
         x++;
          if(x>15)
         {
           x=0;
           y++;
           y%=2;
         }
         msg+=ch[num];
        }

       else if(key=='7')
       {
         num=26;
         lcd.setCursor(x,y);
         lcd.print(ch[num]);
         for(int i=0;i<3000;i++)
         { 
          lcd.noCursor();
          char key=customKeypad.getKey();
          if(key=='7')
          {
           num++;
           if(num>30)
           num=26;
           lcd.setCursor(x,y);
           lcd.print(ch[num]);
           i=0;
           //delay(200);
          } 
         }
         x++;
          if(x>15)
         {
           x=0;
           y++;
           y%=2;
         }
         msg+=ch[num];
        }

       else if(key=='8')
       {
         num=31;
         lcd.setCursor(x,y);
         lcd.print(ch[num]);
         for(int i=0;i<3000;i++)
         { 
          lcd.noCursor();
          char key=customKeypad.getKey();
          if(key=='8')
          {
           num++;
           if(num>34)
           num=31;
           lcd.setCursor(x,y);
           lcd.print(ch[num]);
           i=0;
           //delay(200);
          } 
         }
         x++;
          if(x>15)
         {
           x=0;
           y++;
           y%=2;
         }
         msg+=ch[num];
        }

       else if(key=='9')
       {
         num=35;
         lcd.setCursor(x,y);
         lcd.print(ch[num]);
         for(int i=0;i<3000;i++)
         { 
          lcd.noCursor();
          char key=customKeypad.getKey();
          if(key=='9')
          {
           num++;
           if(num>39)
           num=35;
           lcd.setCursor(x,y);
           lcd.print(ch[num]);
           i=0;
           //delay(200);
          } 
         }
         x++;
          if(x>15)
         {
           x=0;
           y++;
           y%=2;
         }
         msg+=ch[num];
        }

        else if(key=='0')
       {
         num=40;
         lcd.setCursor(x,y);
         lcd.print(ch[num]);
         for(int i=0;i<3000;i++)
         { 
          lcd.noCursor();
          char key=customKeypad.getKey();
          if(key=='0')
          {
           num++;
           if(num>41)
           num=40;
           lcd.setCursor(x,y);
           lcd.print(ch[num]);
           i=0;
           //delay(200);
          } 
         }
         x++;
          if(x>15)
         {
           x=0;
           y++;
           y%=2;
         }
         msg+=ch[num];
        }
        else if(key=='#')
        {
          lcd.clear();
          lcd.print("Sending SMS....");
          Serial1.print(msg);
          Serial1.write(26);
          delay(5000);
          lcd.clear();
          lcd.print("SMS Sent to");
          lcd.setCursor(0,1);
          lcd.print(number);
          delay(2000);
          number="";
          break;
        }
    }
  }
}

void updateSerial()
{
  delay(500);
  while (Serial.available()) 
  {
    Serial1.write(Serial.read());//Forward what Serial received to Software Serial Port
    
  }
  while(Serial1.available()) 
  {
    Serial.write(Serial1.read());//Forward what Software Serial received to Serial Port
    
  }
}

void gsm_init()
{
  lcd.clear();
  lcd.print("Finding Module..");
  boolean at_flag=1;
  while(at_flag)
  {
    Serial1.println("AT");  
    updateSerial();//Test command checks the communication with the module
    delay(500);
    at_flag=0;
    break;
    delay(1000);
  }
  lcd.clear();
  lcd.print("Module Connected..");
  delay(1000);
  lcd.clear();
  lcd.print("Disabling ECHO");
  boolean echo_flag=1;
  while(echo_flag)
  {
    Serial1.println("ATE1");      //Echo Command which causes the modem to display a command as you type it
    while(Serial1.available()>0)
    {
      if(Serial1.find(ok))
      {
        echo_flag=0;
      }
    }
    break;
    delay(1000);
  }
  lcd.clear();
  lcd.print("Echo OFF");
  delay(1000);
  lcd.clear();
  lcd.print("Finding Network..");
  boolean net_flag=1;
  while(net_flag)
  {
    Serial1.println("AT+CPIN?");      //Command for SIM detection
    while(Serial1.available()>0)
    {
      if(Serial1.find(sim_ready)) //When the SIM card is ready, above AT response will be received from the module
      {
        net_flag=0;
      }
    }
    break;
    delay(1000);
  }
  lcd.clear();
  lcd.print("Network Found..");
  delay(1000);
  lcd.clear();
}
