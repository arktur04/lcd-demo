#ifndef __LED_SENSOR_H
#define __LED_SENSOR_H

#define OUT_O       0
#define OUT_1       1
#define DISCR_INPUT 2
#define AN_INPUT    3

struct Coord
{
  int x, y;
};

//Coord getCoord(void);

//(void)(int x, int y) LcdPressedCallback;

//void setLcdPressedCallback(LcdPressedCallback lcdPressedCallback);

void delay(int value);

void initAdc(void);

char getXLpullup(void);
char getXL(void);
void setXLout(char value);

void setYUasAdc(void);
int getYUadc(void);
void setYUout(char value);

void setXRasAdc(void);
int getXRadc(void);
void setXRout(char value);
char getXR(void);

void setYDout(char value);
char getYD(void);
//------------------------------------------------------------------------------

char getScreenPressed(void);
void setState1(void);
void setState2(void);

//test functions
//void testXL(char state);
//void testXR(char state);
//void testYU(char state);
//void testYD(char state);

//------------------------------------------------------------------------------
#endif // __LED_SENSOR_H