/* hmc5883l.c
 *
 * datum: Wed Jan 12 09:14:01 CET 2016
 * Author: idlerug (dodotronix)
 *
 * Comunication test of sensor hmc5883l adafruit
 *
*/

//#define CRTSCTS  020000000000
#define ADDRESS 0x1e // define the ID of the device on the I2C Bus
#define _BSD_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <math.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

int I2CFile;

void compass_init(){
  I2CFile = open("/dev/i2c-1", O_RDWR);		// Open the I2C device
  ioctl(I2CFile, I2C_SLAVE, ADDRESS);   // Specify the address of the I2C Slave to communicate with
}

void ReadByte(unsigned char reg, int16_t axis[3]){
  unsigned char data[] ={reg};
  unsigned char output[] ={0, 0, 0, 0, 0, 0, 0}; //output save array
  if(write(I2CFile, data, 1) !=1) {printf ("ERROR writing\n");}
  else {
    if(read(I2CFile, output, 6) !=6) {printf ("ERROR writing\n");}
  }
  axis[0] =(output[0]<<8)|output[1]; // axis X
  axis[1] =(output[4]<<8)|output[5]; // axis Y
  axis[2] =(output[2]<<8)|output[3]; // axis Z
  /*
  for(int i =0; i<3; ++i){
    axis[i] =(output[i*2+1] <<8)|output[2*i];
  }
  */
}

void WriteByte(unsigned char reg, unsigned char value){
  unsigned char data[2];
  data[0] = reg;
  data[1] = value;
  if(write(I2CFile, data, 2) != 2) {printf ("ERROR writing\n");}
}

void point_to_reg(unsigned char reg){
  unsigned char data[] ={reg};
  if(write(I2CFile, data, 1) != 1) {printf ("ERROR writing\n");}
}

void closeCOM(){
  close(I2CFile);
}

void cont_mode_set(){
  WriteByte(0x00, 0x70); //8-average, 15Hz default, normal meassure
  //WriteByte(0x00, 0x74);
  //WriteByte(0x00, 0x72);
  //WriteByte(0x01, 0x20); //default gain
  WriteByte(0x01, 0xa0); //gain =5
  WriteByte(0x02, 0x00); //continous mode set
  usleep(7000); //wait 6ms
}

void sing_mode_set(){
  WriteByte(0x00, 0x70); //8-average, 15Hz default, normal meassure
  WriteByte(0x01, 0xa0); //gain =5
  //WriteByte(0x02, 0x01); //single shot
  usleep(6000); //wait 6ms
}

void display(int16_t pole[3], float angle){
  printf("X: %i\n", pole[0]);
  printf("Y: %i\n", pole[1]);
  printf("Z: %i\n", pole[2]);
  printf("Angle: %f\n", angle);
}
uint8_t read_byte(uint8_t reg){
  uint8_t array[] ={reg};
  uint8_t output[] ={0};
  if(write(I2CFile, array, 1) !=1) {printf ("ERROR writing\n");}
  else {
    if(read(I2CFile, output, 1) !=1) {printf ("ERROR writing\n");}
  }
  return output[0];
}

int main() {
  short x, y, z;
  //int16_t axis[] ={0, 0, 0};
  float rad;
  uint8_t byte =0;
  //float angle =0;
  compass_init();
  cont_mode_set();
  //sing_mode_set();
  while(1){
    //WriteByte(0x02, 0x01); //single shot
    //ReadByte(0x03, axis);
    byte = read_byte(0x03);
    x = byte<<8;
    byte = read_byte(0x04);
    x |= byte; 
    byte = read_byte(0x05);
    z = byte<<8;
    byte = read_byte(0x06);
    z |= byte; 
    byte = read_byte(0x07);
    y = byte<<8;
    byte = read_byte(0x08);
    y |= byte;

    point_to_reg(0x03); //point to reg 0x03
    rad =atan2(y, x);
    if(rad <0)
      rad +=2*M_PI;
    usleep(67000); //wait 67ms
    //display(axis, angle); //print all axis
    
    printf("X: %d\n", x);
    printf("Y: %d\n", y);
    printf("Z: %d\n",z);
    printf("rad: %f\n", rad);
    printf("angle: %f\n", rad*(180/M_PI));


  }

  close(I2CFile);
  return 0;
}
