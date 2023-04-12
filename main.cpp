#include "mbed.h"
#include "TextLCD.h"
#include <cstdint>
#include <cstdio>

// define the textLCD commands
#define LOCATE 0x01
#define CLS 0x02
#define PUTC 0x03

Thread main_thread;
Thread device_thread;

SPI spi(D11, D12, D13); // mosi, miso, sclk
DigitalOut cs(D9);

SPISlave device(PD_4, PD_3, PD_1, PD_0); //mosi, miso, sclk, cs; PMOD pins

I2C i2c_lcd(D14, D15); // SDA, SCL
TextLCD_I2C lcd(&i2c_lcd, 0x4E, TextLCD::LCD16x2);

int store[256] = {0};

int slave()
{
   device.format(16, 3);
   device.frequency(1000000);
   //device.reply(0x00); // Prime SPI with first reply
   while (1)
   {
      
      if (device.receive())
      {
            int v = device.read(); // Read byte from master
            printf("%d\n", v);
            if ((v & 0XFF00) == 0X0100)
            {                    
               int column = (v & 0X00F0) / 16;
               int row = v % 16;
               
               lcd.locate(column, row); 
            }
            else if ((v & 0XFF00) == 0X0200) 
            {
                lcd.cls();
            } 
            else if ((v & 0XFF00) == 0X0300) 
            {
                char input = (char)(v & 0X00FF);
                lcd.putc(input);
            }
            else
            {
               printf("Default reply to master: 0x00\n");
               device.reply(0x00); //Reply default value
            };
      }
   }
}

void master()
{
   // Setup the spi for 8 bit data, high steady state clock,
   // second edge capture, with a 1MHz clock rate
   spi.format(16, 3);
   spi.frequency(1000000);
       cs = 0;
       cs = 1;
       cs = 0;
       

      
       int command = 0X0100;
       int column = 0X0070;
       int row = 0X0001;
       spi.write(0x0000 | command | column | row);
       ThisThread::sleep_for(1000ms);
       cs = 1;

       cs = 0;
       command = 0X0300;
       char data = 'c';
       spi.write(0x0000 | command | (int)data);
       ThisThread::sleep_for(1000ms);
       cs = 1;

       cs = 0;
       command = 0X0200;
       spi.write(command);
       ThisThread::sleep_for(1000ms);
       cs = 1;
}

int main()
{
   main_thread.start(master);
   device_thread.start(slave);
}
