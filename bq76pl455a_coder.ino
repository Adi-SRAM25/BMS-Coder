#include<SoftwareSerial.h>
SoftwareSerial mSerial = SoftwareSerial(2,3);

void setup() {
  
Serial.begin(9600); // For Serial Monitor
mSerial.begin(9600); // For bq76pl455a-q1

delay(1000); // Optional
  
char a0[5]= {0x92,0x00,0x10,0x10,0x80}; // Communication configure 
write2(a0,sizeof(a0));

char a1[5] = {0x92,0x00,0x52,0xFF,0xC0}; // Clearing Faults
write2(a1,sizeof(a1));

char a2[4] = {0x91,0x00,0x3D,0x00}; // Sampling Delay set to 0 us
write2(a2,sizeof(a2));

char a3[4] = {0x91,0x00,0x3E,0xBC}; // Sampling Period 
write2(a3,sizeof(a3));

char a4[4] = {0x91,0x00,0x07,0x00}; // Oversampling rate
write2(a4,sizeof(a4));

char a5[4] = {0x91,0x00,0x51,0x38}; // Clear faults
write2(a5,sizeof(a5));

char a6[5] = {0x92,0x00,0x52,0xFF,0xC0}; // Clear faults
write2(a6,sizeof(a6));

char a7[4] = {0x91,0x00,0x0D,0x10}; // Number of cells
write2(a7,sizeof(a7));

char a8[7] = {0x94,0x00,0x03,0xFF,0xFF,0x00,0xC0}; // No. of Channels select
write2(a8,sizeof(a8));

char a9[5] = {0x92,0x00,0x90,0xD1,0xEC}; // Cell Overvoltage threshold
write2(a9,sizeof(a9));

char a10[5] = {0x92,0x00,0x8E,0x61,0x48}; // Cell Undervoltage Threshold
write2(a10,sizeof(a10));

char a11[5] = {0x91,0x00,0x12,0x10}; // 2 Bytes Transmitter Hold off
write2(a11,sizeof(a11));

char a12[5] = {0x91,0x00,0x28,0x00}; // CTO disabled
write2(a12,sizeof(a12));

char a13[5] = {0x91,0x00,0x0E,0x01}; // Faults have become unlatched
write2(a13,sizeof(a13));
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------

void loop() 
{
char b0[4] = {0x81,0x00,0x02,0x00}; // Command frame for sampling request
write2(b0,sizeof(b0));
readp();

char b1[4]={0x81,0x00,0x51,0x00};// Reading Fault or Stack Fault 
write2(b1,sizeof(b1));
unsigned int g = readval();

if ((g>>7)&0x01 == 1 | (g>>6)&0x01 == 1)
{
char b2[4]={0x81,0x00,0x52,0x01}; // Reading Fault Summary
write2(b2,sizeof(b2));
unsigned int s = readsum();
  
     for (int i =6;i<16;i++)
  {
       if(((s>>i) & 1) == 1)
    {
      type(i);
      int ax = addx(i);
      int bti = byt(i);
      char b3[4]={0x81,0x00,ax,byt(i)};
           
            if(bti==1)
        { 
          write2(b3,sizeof(b3));
          unsigned int z = readsum();

              if (ax<=93)
            { 
                  for (int j=0;j<16;j++)
                {
                  
                  if(((z>>j) & 0x01) == 1)
                 {
                  Serial.print("  CELL- ");
                  Serial.println(j); 
                 } 
                }   
            }

              if (ax==94)
            { 
                 for (int j=0;i<16;i++)
                {
                  unsigned int a1 = z>>j;
                  if((a1 & 0x01) == 1)
                {
                   comm(j);
                } 
               } 
         
            }

               if (ax==97)
            { 
                 for (int j=0;i<16;i++)
              {
                 unsigned int a2 = z>>j;
                 if((a2 & 0x01) == 1)
                {
                    dev(j);
                } 
              } 
         
             }
         
       }
      
           else 
     { 
          write2(b3,sizeof(b3));
          unsigned int y = readval();

            if(ax <= 89 )
          {
               for (int j=0;j<8;j++)
            {
               unsigned int ab = y>>j;
               if((ab & 0x01) == 1)
              {
                 Serial.print("     AUX-");
                 Serial.println(j); 
              }
            }
          }

            if (ax==96)
          {
                for (int j=0;j<8;j++)
             {
                 unsigned int ab1 = y>>j;
                 if((ab1 & 0x01) == 1)
                 {
                    sys(j);
                 }
              
             }
           }

             if (ax==99)
           {
               for (int j=0;j<8;j++)
            {
              unsigned int ab2 = y>>j;
              if((ab2 & 0x01) == 1)
             {
                gpio();
              }
            }
           }    
    }


}//if
}//for
}//sup

} //void loop   

// FUNCTIONS ---------------------------------------------------------------------------------------------------------------------------------------------------------------

void write2(char diff[],int len) // Function for writing and Generation of CRC
{ 
unsigned int crc = 0;
for (int j = 0; j < len; j++)
  {
    unsigned int b = diff[j];
    for (int i = 0; i < 8; i++)
    {
      crc = ((b ^ crc) & 1) ? ((crc >> 1) ^ 0xA001) : (crc >> 1);
      b >>= 1;
    }
  }

unsigned int a = crc;
diff[len]=a&0xff; // adds crc lsb to command frame
diff[len+1]= a>>8; // adds crc msb to command frame/
mSerial.write(diff,len+2); // writes array of bytes to bq76pl455a
}

void readp() // Function for reading arrived frame
{
 char buff[80];
 int i =0;
 //while(mSerial.available()==0){} // to wait for response to arrive (method to introduce variable delay)

 while(mSerial.available()>=2)  // Reading Response frame
{
  i = mSerial.read();// reads response frame initialisation byte = (no.of bytes-1)
  mSerial.readBytes(buff,i+1); // stores rest frame excluding crc bytes into buffer (1st byte excluded)
  
  for(int f=0;f<=i/2;f++)
 {
  Serial.print((i/2) - f +1);     // printing cell number
  Serial.print(" -> ");
  Serial.println(int((buff[2*f]<<8)|buff[2*f+1])); //printing sampled parameters from buffer by concating two bytes of same cell data 
 }

 Serial.println(""); // Separator
 delay(50);
}


}

int readval () 
{
 char buff1[5];
 int i =0;
 while(mSerial.available()==0){} // to wait for response to arrive (method to introduce variable delay)

 while(mSerial.available()>=2)  // Reading Response frame
{
  i = mSerial.read();// reads response frame initialisation byte = (no.of bytes-1)
  mSerial.readBytes(buff1,i+1); // stores rest frame excluding crc bytes into buffer (1st byte excluded)
}

return int(buff1[0]);
}

int readsum () 
{
 char buff2[5];
 int i =0;
 while(mSerial.available()==0){} // to wait for response to arrive (method to introduce variable delay)

 while(mSerial.available()>=2)  // Reading Response frame
{
  i = mSerial.read();// reads response frame initialisation byte = (no.of bytes-1)
  mSerial.readBytes(buff2,i+1); // stores rest frame excluding crc bytes into buffer (1st byte excluded)

}

return int((buff2[0]<<8)|buff2[1]);
}

int addx (int i)
{
  int r;
  switch(i)
 {
    case 15: r=0x54;
    break;
    case 14: r=0x56;
    break;
    case 13: r=0x58;
    break;
    case 12: r=0x59;
    break;
    case 11: r=0x5A;
    break;
    case 10: r=0x5C;
    break;
    case 9: r=0x5E;
    break;
    case 8: r=0x60;
    break;
    case 7: r=0x61;
    break;
    case 6: r=0x63;
    break;
  }

  return r;
}

int byt (int i)
{
  int r;
  switch(i)
 {
    case 15: r=0x01;
    break;
    case 14: r=0x01;
    break;
    case 13: r=0x00;
    break;
    case 12: r=0x00;
    break;
    case 11: r=0x01;
    break;
    case 10: r=0x01;
    break;
    case 9: r=0x01;
    break;
    case 8: r=0x00;
    break;
    case 7: r=0x01;
    break;
    case 6: r=0x00; 
    break;
  }

  return r;
}

void type(int i)
{
  switch(i)
  { 
    case 15: Serial.println("CELL UV FAULT : ");
    break;
    case 14: Serial.println("CELL OV FAULT : ");
    break;
    case 13: Serial.println("AUX UV FAULT : ");
    break;
    case 12: Serial.println("AUX OV FAULT : ");
    break;
    case 11: Serial.println("CMP UV FAULT : ");
    break;
    case 10: Serial.println("CMP OV FAULT : ");
    break;
    case 9: Serial.println("COMM FAULT : ");
    break;
    case 8: Serial.println("SYS FAULT : ");
    break;
    case 7: Serial.println("CHIP FAULT : ");
    break;
    case 6: Serial.println("GPI FAULT : "); 
    break;
   } 
}

void comm(int i)
{
  switch (i)
  {
    case 15: Serial.println("HIGH SIDE COMPLEMENT FAILURE!");
    break;
    case 14: Serial.println("LOW SIDE COMPLEMENT FAILURE!");
    break;
    case 13: Serial.println("HIGH SIDE COMM STOP");
    break;
    case 12: Serial.println("LOW SIDE COMM STOP");
    break;
    case 11: Serial.println("HIGH SIDE EDGE ERR");
    break;
    case 10: Serial.println("LOW SIDE EDGE ERR");
    break;
    case 9: Serial.println("ABORT HIGH");
    break;
    case 8: Serial.println("ABORT LOW");
    break;
    case 7: Serial.println("CRC HIGH");
    break;
    case 6: Serial.println("CRC LOW"); 
    break;
    case 5: Serial.println("FRAME ERR");
    break;
    case 3: Serial.println("INVALID STOP BIT-BASE");
    break;
    case 0: Serial.println("STACK FAULT"); 
    break;
  }
}

void sys(int i)
{ 
  switch (i)
  { 
    case 7: Serial.println("SYS RESET");
    break;
    case 6: Serial.println("CTO");
    break;
    case 5: Serial.println("VDIG LEAKAGE CURRENT");
    break;
    case 4: Serial.println("DIGI DIE OVTEMP");
    break;
    case 3: Serial.println("VDIG FAULT"); 
    break;
    case 2: Serial.println("VM FAULT");
    break;
    case 1: Serial.println("VP FAULT");
    break;
    case 0: Serial.println("VP CLAMP");
    break; 
  }
}

void dev(int i)
{  
  switch (i)
  {
    case 15: Serial.println("USER CHECKSUM ERR");
    break;
    case 14: Serial.println("FACT CHECKSUM ERR");
    break;
    case 13: Serial.println("ANALOG DIE FAULT");
    break;
    case 12: Serial.println("4.5 VREF2 OUTOR");
    break;
    case 11: Serial.println("ANA DIE GROUND REF ERR");
    break;
    case 4: Serial.println("ADC TEST FAIL");
    break;
    case 3: Serial.println("ECC CORRECTED (USER)");
    break; 
    case 2: Serial.println("ECC UNCORRECTABLE (USER)");
    break;
    case 1: Serial.println("ECC CORRECTED (FACT)");
    break;
    case 0: Serial.println("ECC UNCORRECTABLE (FACT)"); 
    break;
  }
}

void gpio()
{
  Serial.println("FAULT TRIGGERED BY GPIO");
}
