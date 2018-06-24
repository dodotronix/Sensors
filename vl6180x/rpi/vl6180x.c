/* vl6180x.c
 *
 * Datum: Wed Dec 30 12:14:01 CET 2015
 * Author: idle_rug (dodotronix)
 *
 * Comunication test of sensor vl6180x
 *
*/

//#define CRTSCTS  020000000000

// define the ID of the device on the I2C Bus
#define dID 0x29 
#define _BSD_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

int I2CFile;
char symbol;

void sensor_init(){
  I2CFile = open("/dev/i2c-1", O_RDWR);   // Open the I2C device
  ioctl(I2CFile, I2C_SLAVE, 0x29);   // Specify the address of the I2C Slave to communicate with
}

uint8_t ReadByte(uint16_t reg){
  uint8_t value[] = {0};
  uint8_t data[2];
  data[0] = (reg >> 8) & 0xff;  // reg high byte
  data[1] = reg & 0xff;         // reg low byte
  if(write(I2CFile, data, 2) != 2) {printf ("ERROR writing\n");}
  else {
      if(read(I2CFile, value, 1) != 1) {printf ("ERROR reading\n");}
  }

  return value[0];
}

void WriteByte(uint16_t reg, uint8_t value){
  uint8_t data[3];
  data[0] = (reg >> 8) & 0xff;  // reg high byte
  data[1] = reg & 0xff;         // reg low byte
  data[2] = value;
  if(write(I2CFile, data, 3) != 3) {printf ("ERROR writing\n");}
}

void closeCOM ()
{
  close(I2CFile);
}

void Settings (){
  // Mandatory : private registers
  WriteByte(0x0207, 0x01);
  WriteByte(0x0208, 0x01);
  WriteByte(0x0096, 0x00);
  WriteByte(0x0097, 0xfd);
  WriteByte(0x00e3, 0x00);
  WriteByte(0x00e4, 0x04);
  WriteByte(0x00e5, 0x02);
  WriteByte(0x00e6, 0x01);
  WriteByte(0x00e7, 0x03);
  WriteByte(0x00f5, 0x02);
  WriteByte(0x00d9, 0x05);
  WriteByte(0x00db, 0xce);
  WriteByte(0x00dc, 0x03);
  WriteByte(0x00dd, 0xf8);
  WriteByte(0x009f, 0x00);
  WriteByte(0x00a3, 0x3c);
  WriteByte(0x00b7, 0x00);
  WriteByte(0x00bb, 0x3c);
  WriteByte(0x00b2, 0x09);
  WriteByte(0x00ca, 0x09);
  WriteByte(0x0198, 0x01);
  WriteByte(0x01b0, 0x17);
  WriteByte(0x01ad, 0x00);
  WriteByte(0x00ff, 0x05);
  WriteByte(0x0100, 0x05);
  WriteByte(0x0199, 0x05);
  WriteByte(0x01a6, 0x1b);
  WriteByte(0x01ac, 0x3e);
  WriteByte(0x01a7, 0x1f);
  WriteByte(0x0030, 0x00);
}

void SpecSettings(){
  // Recommended : Public registers - See data sheet for more detail
  WriteByte(0x0011, 0x10); // Enables polling for ‘New Sample ready’
  // when measurement completes
  WriteByte(0x010a, 0x30); // Set the averaging sample period
  // (compromise between lower noise and
  // increased execution time)
  WriteByte(0x003f, 0x46); // Sets the light and dark gain (upper
  // nibble). Dark gain should not be
  // changed.
  WriteByte(0x0031, 0xFF); // sets the # of range measurements after
  // which auto calibration of system is
  // performed
  WriteByte(0x0040, 0x63); // Set ALS integration time to 100ms

  WriteByte(0x002e, 0x01); // perform a single temperature calibration
  // of the ranging sensor
  //Optional: Public registers - See data sheet for more detail
  WriteByte(0x001b, 0x09); // Set default ranging inter-measurement
  // period to 100ms
  WriteByte(0x003e, 0x31); // Set default ALS inter-measurement period
  // to 500ms
  WriteByte(0x0014, 0x24); // Configures interrupt on ‘New Sample
  // Ready threshold event’
}


void vlx_init(){
   while(1){
    WriteByte(0x010, 0x01); //set GPIO to "1"
    usleep(1000); //wait for 1ms [Device Boot]
    if(ReadByte(0x016) == 0x01) break; // register SYSTEM__FRESH_OUT_OF_RESET set to "1" ?
    else{
      WriteByte(0x010, 0x00); //set GPIO to "0"
      usleep(1000); //wait for 1ms
    }
  }
  Settings();
  SpecSettings();
  WriteByte(0x016, 0x00);
}

void AddrChng(uint8_t addr){
  WriteByte(0x212, addr);
}

void SingleShot(uint8_t dist){
  while(1){
    WriteByte(0x18, 0x01); // start measurement
    while ((ReadByte(0x04f) & 0x04) != 0x04); // wait for bit 2 in register RESULT__INTERRUPT_STATUS_GPIO
    dist = ReadByte(0x062); // read distance value

    WriteByte(0x015, 0x07); //clear interrupt
    printf("distance: %u\n", dist);
    //printf("do you want a new measurement?\n");
    usleep(50000);
    //scanf("%c", &symbol);
    //if(symbol=='n') break;
  }
}

void Continuous(uint8_t dist){

    WriteByte(0x18, 0x03); // start measurement
    while(1){
      while ((ReadByte(0x04f) & 0x04) != 0x04); // wait for bit 2 in register RESULT__INTERRUPT_STATUS_GPIO
      dist = ReadByte(0x062); // read distance value
      WriteByte(0x015, 0x07); //clear interrupt
      printf("distance: %u\n", dist);
      //printf("do you want a new measurement?\n");
      usleep(50000);
      //scanf("%c\n", &symbol);
      //if(symbol=='n'){
        //WriteByte(0x18, 0x01); // start measurement
        //break;
      //}
    }
}

int main() {
  //char symbol;
  //uint8_t distance = 0;

  sensor_init();
  vlx_init();
  //AddrChng(0x20); change addres for running time
  printf("MODEL_ID = %u\n", ReadByte(0x000));
  //SingleShot(distance);
  //Continuous(distance);
  
  closeCOM();
  
  return 0;
}
