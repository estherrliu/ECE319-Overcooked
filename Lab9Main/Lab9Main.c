// Lab9Main.c
// Runs on MSPM0G3507
// Lab 9 ECE319K
// Your name
// Last Modified: 12/31/2023

//=====TO DO +====
//PROGRESS NOTE: START HERE TMR


//more ideas:
//how to play -- more instructions
//fire

//things that could be added:
//loading line at overcooked screen
//additional instructions for buttons

//additional bugs to fix maybe if time  ====
// if two of same item exists in same page then it glitches
//while chopping, sprites glitch ane make duplicate

//notes:
//rn can't drop wrong ingredients into bowl, maybe modify so can move bowl to throw contents away if mess up order
#include <stdio.h>
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "../inc/ST7735.h"
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "../inc/TExaS.h"
#include "../inc/Timer.h"
#include "../inc/ADC1.h"
#include "../inc/ADC.h"
#include "../inc/DAC5.h"
#include "SmallFont.h"
#include "LED.h"
#include "Switch.h"
#include "Sound.h"
#include "images/images.h"
//#include "sprites.h"




// ****note to ECE319K students****
// the data sheet says the ADC does not work when clock is 80 MHz
// however, the ADC seems to work on my boards at 80 MHz
// I suggest you try 80MHz, but if it doesn't work, switch to 40MHz
void PLL_Init(void){ // set phase lock loop (PLL)
  // Clock_Init40MHz(); // run this line for 40MHz
  Clock_Init80MHz(0);   // run this line for 80MHz
}

uint32_t M=1;
uint32_t Random32(void){
  M = 1664525*M+1013904223;
  return M;
}
uint32_t Random(uint32_t n){
  return (Random32()>>16)%n;
}


#define oL (1<<13)
#define oU (1<<24)
#define oR (1<<25)
#define oD (1<<26)
#define zL (1<<31)
#define zU (1<<28)
#define zR (1<<9)
#define zD (1<<17)

#define P0XSTARTPT 35
#define P0YSTARTPT 75
#define P1XSTARTPT 75
#define P1YSTARTPT 75

#define P1PICKDROPSW sw_val_oL
#define P0PICKDROPSW sw_val_zL
#define P1CHOPSW sw_val_oU
#define P0CHOPSW sw_val_zU
int32_t j1hori, j1vert, j0hori, j0vert;

typedef struct Langu{
    int english, spanish;
} Langu;
Langu lang;
typedef struct Player0{
    int x, y;
    int dx, dy;
    const uint16_t *holt;
    int carry;
} Player0;
Player0 p0;

typedef struct Player1{
    int x, y;
    int dx, dy;
    const uint16_t *valbano;
    int carry;
} Player1;
Player1 p1;

typedef struct Tomato{
    int x, y;
    int dx, dy;
    const uint16_t *tomato;
    int v_carry, h_carry;
    int chopped;
    int choppingboard1, choppingboard2;
} Tomato;
Tomato tom;

typedef struct Bokchoy{
    int x, y;
    int dx, dy;
    const uint16_t *bokchoy;
    int v_carry, h_carry;
    int chopped;
    int choppingboard1, choppingboard2;
} Bokchoy;
Bokchoy bok;

typedef struct Beef{
    int x, y;
    int dx, dy;
    const uint16_t *meat;
    int v_carry, h_carry;
    int cooked;
    int in_bowl;
} Beef;
Beef bf;

typedef struct Dough{
    int x, y;
    int d, dy;
    const uint16_t *dough;
    int v_carry, h_carry;
    int noodled;
    int choppingboard1, choppingboard2;
} Dough;
Dough dgh;

typedef struct TomatoCut{
    int x, y;
    const uint16_t *tomatocut;
    int v_carry, h_carry;
    int in_bowl;
} TomatoCut;
TomatoCut tomcut;

typedef struct BokchoyCut{
    int x, y;
    const uint16_t *bokchoycut;
    int v_carry, h_carry;
    int in_bowl;
} BokchoyCut;
BokchoyCut bokcut;

typedef struct Noodle{
    int x, y;
    int cooked;
    const uint16_t *noodle;
    int v_carry, h_carry;
    int in_bowl;
} Noodle;
Noodle ndl;
typedef struct EmptyPot{
    int x, y;
    const uint16_t *emptypot;
    int v_carry, h_carry;
    int bf_filled, noodle_filled;
    int on_stove;
} EmptyPot;
EmptyPot emp_pot;

typedef struct NoodlePot{
    int x, y;
    const uint16_t *spr_progpot;
    int v_carry, h_carry;
    //int bf_filled, noodle_filled;
    //int beef_done;
    int on_stove; //not sure if i need
} NoodlePot;
NoodlePot noodle_pot;

typedef struct DonePot{
    int x, y;
    const uint16_t *spr_donepot;
    int v_carry, h_carry;
    //int done;
    int on_stove;
} DonePot;
DonePot d_pot;

typedef struct BeefNoodleSoup{
    int x, y;
    int v_carry, h_carry;
    int completed_bowl;
    int delivered;
    int erase;
} BeefNoodleSoup;
BeefNoodleSoup bns;

typedef struct Recipe1{
    int x, y;
    int v_carry, h_carry;
    int completed_bowl;
    int delivered;
    int erase
} Recipe1;
Recipe1 rec1;

typedef struct Recipe2{
    int x, y;
    int v_carry, h_carry;
    int completed_bowl;
    int delivered;
    int erase
} Recipe2;
Recipe2 rec2;

typedef struct Recipe3{
    int x, y;
    int v_carry, h_carry;
    int completed_bowl;
    int delivered;
    int erase
} Recipe3;
Recipe3 rec3;
// games  engine runs at 30Hz
int sw_val_oU, sw_val_oR, sw_val_oD, sw_val_oL, sw_val_zU, sw_val_zR, sw_val_zL, sw_val_zD;
int32_t j1hori, j1vert, j0hori, j0vert; //j1hori off by half idk why
int sema;
int timer_count;
int timer_display = 120;
int game_over;
void TIMG0_IRQHandler(void){
  if((TIMG0->CPU_INT.IIDX) == 1){ // this will acknowledge
    GPIOB->DOUTTGL31_0 = GREEN;   // toggle PB27
    GPIOB->DOUTTGL31_0 = GREEN;   // toggle PB27
    timer_count++;
    if((timer_count % 55) == 0){
        timer_display--;
        timer_count = 0;
    }
  }
}
void TIMG12_IRQHandler(void){uint32_t pos,msg;
  if((TIMG12->CPU_INT.IIDX) == 1){ // this will acknowledge
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
// game engine goes here
    // 1) sample joysticks
    ADC_InDual(ADC0, &j0vert, &j0hori);
    ADC_InDual(ADC1, &j1hori, &j1vert);
    sema = 1;


    // 2) read input switches
    sw_val_oU = Switch_In(oU); //P1 -- chop
    sw_val_oR = Switch_In(oR); //P1 -- pick up
    sw_val_zR = Switch_In(zR); //P0 -- chop
    sw_val_zU = Switch_In(zU); //P1 -- pick up
    sw_val_oL = Switch_In(oL); //test bc oR isn't working
    sw_val_zL = Switch_In(zL);
    // 3) move sprites
    // 4) start sounds
    // 5) set semaphore
    // NO LCD OUTPUT IN INTERRUPT SERVICE ROUTINES

    //SAMPLE VALVANO
     p1.dy = (256*(j1vert-2048))/2048; //-256 to 256
     if(p1.dy > 50){ //valvano up
         if(p1.y > 35){
             p1.y--;
             if(tom.v_carry == 1){
                 tom.y--;
             }else if(bok.v_carry == 1){
                 bok.y--;
             }else if(bf.v_carry == 1){
                 bf.y--;
             }else if(dgh.v_carry == 1){
                 dgh.y--;
             }else if(tomcut.v_carry == 1){
                 tomcut.y--;
             }else if(bokcut.v_carry == 1){
                 bokcut.y--;
             }else if(d_pot.v_carry == 1){
                 d_pot.y--;
             }else if(emp_pot.v_carry == 1){
                 emp_pot.y--;
             }else if(ndl.v_carry == 1){
                 ndl.y--;
             }else if(noodle_pot.v_carry == 1){
                 noodle_pot.y--;
             }else if(bns.v_carry == 1){
                 bns.y--;
             }else if(rec1.v_carry == 1){
                 rec1.y--;
             }else if(rec2.v_carry == 1){
                 rec2.y--;
             }else if(rec3.v_carry == 1){
                 rec3.y--;
             }
             //if other ingredient
         }
     }
     p1.dy = (256*(j1vert-2048))/2048; //-256 to 256
     if(p1.dy < -50){ //valvano down
         if(p1.y < 150){
             p1.y++;
             if(tom.v_carry == 1){
                 tom.y++;
             }else if(bok.v_carry == 1){
                 bok.y++;
             }else if(bf.v_carry == 1){
                 bf.y++;
             }else if(dgh.v_carry == 1){
                 dgh.y++;
             }else if(tomcut.v_carry == 1){
                 tomcut.y++;
             }else if(bokcut.v_carry == 1){
                 bokcut.y++;
             }else if(d_pot.v_carry == 1){
                 d_pot.y++;
             }else if(emp_pot.v_carry == 1){
                 emp_pot.y++;
             }else if(ndl.v_carry == 1){
                 ndl.y++;
             }else if(noodle_pot.v_carry == 1){
                 noodle_pot.y++;
             }else if(bns.v_carry == 1){
                 bns.y++;
             }else if(rec1.v_carry == 1){
                 rec1.y++;
             }else if(rec2.v_carry == 1){
                 rec2.y++;
             }else if(rec3.v_carry == 1){
                 rec3.y++;
             }
         }
     }
     p1.dx = (256*(j1hori-2048))/2048; //-256 to 256
     if (p1.dx < -50){ //valvano right
         if(p1.x < 95){
             p1.x++;
             if(tom.v_carry == 1){
                 tom.x++;
             }else if(bok.v_carry == 1){
                 bok.x++;
             }else if(bf.v_carry == 1){
                 bf.x++;
             }else if(dgh.v_carry == 1){
                 dgh.x++;
             }else if(tomcut.v_carry == 1){
                 tomcut.x++;
             }else if(bokcut.v_carry == 1){
                 bokcut.x++;
             }else if(d_pot.v_carry == 1){
                 d_pot.x++;
             }else if(emp_pot.v_carry == 1){
                 emp_pot.x++;
             }else if(ndl.v_carry == 1){
                 ndl.x++;
             }else if(noodle_pot.v_carry == 1){
                 noodle_pot.x++;
             }else if(bns.v_carry == 1){
                 bns.x++;
             }else if(rec1.v_carry == 1){
                 rec1.x++;
             }else if(rec2.v_carry == 1){
                 rec2.x++;
             }else if(rec3.v_carry == 1){
                 rec3.x++;
             }
         }
     }
     p1.dx = (256*(j1hori-2048))/2048; //-256 to 256
     if (p1.dx > 50){ //valvano left
         if(p1.x > 20){
             p1.x--;
             if(tom.v_carry == 1){
                 tom.x--;
             }else if(bok.v_carry == 1){
                 bok.x--;
             }else if(bf.v_carry == 1){
                 bf.x--;
             }else if(dgh.v_carry == 1){
                 dgh.x--;
             }else if(tomcut.v_carry == 1){
                 tomcut.x--;
             }else if(bokcut.v_carry == 1){
                 bokcut.x--;
             }else if(d_pot.v_carry == 1){
                 d_pot.x--;
             }else if(emp_pot.v_carry == 1){
                 emp_pot.x--;
             }else if(ndl.v_carry == 1){
                 ndl.x--;
             }else if(noodle_pot.v_carry == 1){
                 noodle_pot.x--;
             }else if(bns.v_carry == 1){
                 bns.x--;
             }else if(rec1.v_carry == 1){
                 rec1.x--;
             }else if(rec2.v_carry == 1){
                 rec2.x--;
             }else if(rec3.v_carry == 1){
                 rec3.x--;
             }
         }
     }

     //SAMPLE HOLT
     p0.dy = (256*(j0vert-2048))/2048; //-256 to 256
     if (p0.dy > 50){ //holt up
          if(p0.y > 35){
              p0.y--;
              if(tom.h_carry == 1){
                  tom.y--;
              }else if(bok.h_carry == 1){
                  bok.y--;
              }else if(bf.h_carry == 1){
                  bf.y--;
              }else if(dgh.h_carry == 1){
                  dgh.y--;
              }else if(tomcut.h_carry == 1){
                  tomcut.y--;
              }else if(bokcut.h_carry == 1){
                  bokcut.y--;
              }else if(d_pot.h_carry == 1){
                  d_pot.y--;
              }else if(emp_pot.h_carry == 1){
                  emp_pot.y--;
              }else if(ndl.h_carry == 1){
                  ndl.y--;
              }else if(noodle_pot.h_carry == 1){
                  noodle_pot.y--;
              }else if(bns.h_carry == 1){
                  bns.y--;
              }else if(rec1.h_carry == 1){
                  rec1.y--;
              }else if(rec2.h_carry == 1){
                  rec2.y--;
              }else if(rec3.h_carry == 1){
                  rec3.y--;
              }
          }
     }

     p0.dy = (256*(j0vert-2048))/2048; //-256 to 256
     if (p0.dy < -50){ //holt down
         if(p0.y < 150){
             p0.y++;
             if(tom.h_carry == 1){
                 tom.y++;
             }else if(bok.h_carry == 1){
                 bok.y++;
             }else if(bf.h_carry == 1){
                 bf.y++;
             }else if(dgh.h_carry == 1){
                 dgh.y++;
             }else if(tomcut.h_carry == 1){
                 tomcut.y++;
             }else if(bokcut.h_carry == 1){
                 bokcut.y++;
             }else if(d_pot.h_carry == 1){
                 d_pot.y++;
             }else if(emp_pot.h_carry == 1){
                 emp_pot.y++;
             }else if(ndl.h_carry == 1){
                 ndl.y++;
             }else if(noodle_pot.h_carry == 1){
                 noodle_pot.y++;
             }else if(bns.h_carry == 1){
                 bns.y++;
             }else if(rec1.h_carry == 1){
                 rec1.y++;
             }else if(rec2.h_carry == 1){
                 rec2.y++;
             }else if(rec3.h_carry == 1){
                 rec3.y++;
             }
         }
     }
     p0.dx = (256*(j0hori-2048))/2048; //-256 to 256
     if (p0.dx < -50){ //holt right
         if(p0.x < 95){
             p0.x++;
             if(tom.h_carry == 1){
                 tom.x++;
             }else if(bok.h_carry == 1){
                 bok.x++;
             }else if(bf.h_carry == 1){
                 bf.x++;
             }else if(dgh.h_carry == 1){
                 dgh.x++;
             }else if(tomcut.h_carry == 1){
                 tomcut.x++;
             }else if(bokcut.h_carry == 1){
                 bokcut.x++;
             }else if(d_pot.h_carry == 1){
                 d_pot.x++;
             }else if(emp_pot.h_carry == 1){
                 emp_pot.x++;
             }else if(ndl.h_carry == 1){
                 ndl.x++;
             }else if(noodle_pot.h_carry == 1){
                 noodle_pot.x++;
             }else if(bns.h_carry == 1){
                 bns.x++;
             }else if(rec1.h_carry == 1){
                 rec1.x++;
             }else if(rec2.h_carry == 1){
                 rec2.x++;
             }else if(rec3.h_carry == 1){
                 rec3.x++;
             }
         }
     }
     p0.dx = (256*(j0hori-2048))/2048; //-256 to 256
     if (p0.dx > 50){ //holt left
         if(p0.x > 20){
             p0.x--;
             if(tom.h_carry == 1){
                 tom.x--;
             }else if(bok.h_carry == 1){
                 bok.x--;
             }else if(bf.h_carry == 1){
                 bf.x--;
             }else if(dgh.h_carry == 1){
                 dgh.x--;
             }else if(tomcut.h_carry == 1){
                 tomcut.x--;
             }else if(bokcut.h_carry == 1){
                 bokcut.x--;
             }else if(d_pot.h_carry == 1){
                 d_pot.x--;
             }else if(emp_pot.h_carry == 1){
                 emp_pot.x--;
             }else if(ndl.h_carry == 1){
                 ndl.x--;
             }else if(noodle_pot.h_carry == 1){
                 noodle_pot.x--;
             }else if(bns.h_carry == 1){
                 bns.x--;
             }else if(rec1.h_carry == 1){
                 rec1.x--;
             }else if(rec2.h_carry == 1){
                 rec2.x--;
             }else if(rec3.h_carry == 1){
                 rec3.x--;
             }
         }
     }
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
  }
}
uint8_t TExaS_LaunchPadLogicPB27PB26(void){
  return (0x80|((GPIOB->DOUT31_0>>26)&0x03));
}

typedef enum {English, Spanish, Portuguese, French} Language_t;
Language_t myLanguage=English;
typedef enum {HELLO, GOODBYE, LANGUAGE} phrase_t;
const char Hello_English[] ="Hello";
const char Hello_Spanish[] ="\xADHola!";
const char Hello_Portuguese[] = "Ol\xA0";
const char Hello_French[] ="All\x83";
const char Goodbye_English[]="Goodbye";
const char Goodbye_Spanish[]="Adi\xA2s";
const char Goodbye_Portuguese[] = "Tchau";
const char Goodbye_French[] = "Au revoir";
const char Language_English[]="English\x20\x20";
const char Language_Spanish[]="Espa\xA4ol\x20\x20";
const char Language_Portuguese[]="Portugu\x88s";
const char Language_French[]="Fran\x87" "ais";
const char *Phrases[3][4]={
  {Hello_English,Hello_Spanish,Hello_Portuguese,Hello_French},
  {Goodbye_English,Goodbye_Spanish,Goodbye_Portuguese,Goodbye_French},
  {Language_English,Language_Spanish,Language_Portuguese,Language_French}
};
// use main1 to observe special characters
int main1(void){ // main1
    char l;
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf();
  ST7735_FillScreen(0x0000);            // set screen to black
  for(phrase_t myPhrase=HELLO; myPhrase<= GOODBYE; myPhrase++){
    for(Language_t myL=English; myL<= French; myL++){
         ST7735_OutString((char *)Phrases[LANGUAGE][myL]);
      ST7735_OutChar(' ');
         ST7735_OutString((char *)Phrases[myPhrase][myL]);
      ST7735_OutChar(13);
    }
  }
  Clock_Delay1ms(3000);
  ST7735_FillScreen(0x0000);       // set screen to black
  l = 128;
  while(1){
    Clock_Delay1ms(2000);
    for(int j=0; j < 3; j++){
      for(int i=0;i<16;i++){
        ST7735_SetCursor(7*j+0,i);
        ST7735_OutUDec(l);
        ST7735_OutChar(' ');
        ST7735_OutChar(' ');
        ST7735_SetCursor(7*j+4,i);
        ST7735_OutChar(l);
        l++;
      }
    }
  }
}

int val = 0;
int main2(void){ // main2
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf();
  //ST7735_InvertDisplay(1);
    //note: if you colors are weird, see different options for
    // ST7735_InitR(INITR_REDTAB);inside ST7735_InitPrintf()
  //ST7735_DrawBitmap(0, 160, intropage, 128, 160);
  //Clock_Delay1ms(400);
  ST7735_DrawBitmap(0, 159, backgroundedit, 128, 160);
  ST7735_DrawSprite(87, 110, backgroundedit, valbano, 0xFFFF);
  ST7735_DrawSprite(25, 50, backgroundedit, bokchoy, 0xFFFF);
  ST7735_DrawSprite(88, 159, backgroundedit, tomato, 0xFFFF);
  ST7735_DrawSprite(25, 85, backgroundedit, meat, 0xFFFF);
  ST7735_DrawSprite(25, 110, backgroundedit, dough, 0xFFFF);
  ST7735_DrawSprite(25, 130, backgroundedit, noodle, 0xFFFF);
  ST7735_DrawSprite(25, 150, backgroundedit, tomatocut, 0xFFFF);
  ST7735_DrawSprite(0, 65, backgroundedit, emptypot, 0xFFFF);
  ST7735_DrawSprite(107, 110, backgroundedit, emptybowl, 0xFFFF);
  ST7735_DrawSprite(70, 159, backgroundedit, knife, 0xFFFF);
  ST7735_DrawSprite(0, 65, backgroundedit, beefnoodlesoup, 0xFFFF);

  /*ST7735_FillScreen(ST7735_BLACK);
  ST7735_SetCursor(1, 2);
  ST7735_OutString(GameOver1);
  ST7735_SetCursor(1, 4);
  ST7735_OutString(GameOver2);
  ST7735_SetCursor(19, 4);
  ST7735_OutUDec(2);
  ST7735_SetCursor(1, 6);
  ST7735_OutString(GameOver3);
  ST7735_SetCursor(8, 6);
  ST7735_OutUDec(60);
*/

 /* for (uint32_t x = 0, y = 79; x < 128; x++, y++) {
      ST7735_DrawBitmap(x, y, carr, 100, 79);
      Clock_Delay1ms(50);
  }
*/
  /*
  ST7735_DrawBitmap(22, 159, PlayerShip0, 18,8); // player ship bottom
  //ST7735_DrawBitmap(53, 151, Bunker0, 18,5);
  ST7735_DrawBitmap(42, 159, PlayerShip1, 18,8); // player ship bottom
  ST7735_DrawBitmap(62, 159, PlayerShip2, 18,8); // player ship bottom
  ST7735_DrawBitmap(82, 159, PlayerShip3, 18,8); // player ship bottom
  ST7735_DrawBitmap(0, 9, SmallEnemy10pointA, 16,10);
  ST7735_DrawBitmap(20,9, SmallEnemy10pointB, 16,10);
  ST7735_DrawBitmap(40, 9, SmallEnemy20pointA, 16,10);
  ST7735_DrawBitmap(60, 9, SmallEnemy20pointB, 16,10);
  ST7735_DrawBitmap(80, 9, SmallEnemy30pointA, 16,10);

  for(uint32_t t=500;t>0;t=t-5){
   SmallFont_OutVertical(t,104,6); // top left
   Clock_Delay1ms(50);              // delay 50 msec
  }
  ST7735_FillScreen(0x0000);   // set screen to black
  ST7735_SetCursor(1, 1);
//  ST7735_OutString("GAME OVER");
//  ST7735_SetCursor(1, 2);
//  ST7735_OutString("Nice try,");
//  ST7735_SetCursor(1, 3);
//  ST7735_OutString("Earthling!");
//  ST7735_SetCursor(2, 4);
  ST7735_OutUDec(1234);
  */
  while(1){
      val = Random(5);
      ST7735_FillRect(25, 0, 16, 17, 0xFFFF);
      ST7735_DrawSmallCircle(26, 4, 0x118B); //beef
      ST7735_DrawSmallCircle(34, 4, 0x8E9F); //noodle
      ST7735_DrawSmallCircle(26, 10, 0x0019); //tomato
      ST7735_DrawSmallCircle(34, 10, 0x6423); //bok

  }
}




// use main3 to test switches and LEDs
int main3(void){ // main3
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  Switch_Init(); // initialize switches

  ADC_InitDual(ADC0, 0, 5, ADCVREF_VDDA);
  ADC_InitDual(ADC1, 3, 4, ADCVREF_VDDA);
  while(1){


  }

}
// use main4 to test sound outputs
int main4(void){ uint32_t last=0,now;
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  Switch_Init(); // initialize switches
  LED_Init(); // initialize LED
  Sound_Init();  // initialize sound
  TExaS_Init(ADC0,6,0); // ADC1 channel 6 is PB20, TExaS scope
  __enable_irq();
  while(1){
    //now = Switch_In(); // one of your buttons
    if((last == 0)&&(now == 1)){
      Sound_Shoot(); // call one of your sounds
    }
    if((last == 0)&&(now == 2)){
      Sound_Killed(); // call one of your sounds
    }
    if((last == 0)&&(now == 4)){
      Sound_Explosion(); // call one of your sounds
    }
    if((last == 0)&&(now == 8)){
      Sound_Fastinvader1(); // call one of your sounds
    }
    // modify this to test all your sounds
  }
}





const char Welcome[] = "Welcome!";
const char ChooseL1[] = "Please choose a";
const char ChooseL2[] = "language:";
const char Language_English_Sel[] = "English\x20\x11";
const char Language_Spanish_Sel[] = "Espa\xA4ol\x20\x11";

//english instructions
const char EngInstr1[] = "Complete recipes";
const char EngInstr2[] = "to earn coins.";
const char EngInstr3[] = "Bokchoy should";
const char EngInstr4[] = "be cut.";
const char EngInstr5[] = "Tomato should";
const char EngInstr7[] = "Dough should";
const char EngInstr8[] = "rolled and";
const char EngInstr9[] = "cooked.";
const char EngInstr10[] = "Beef should";
const char EngInstr11[] = "be cooked.";
const char EngInstr12[] = "Click UP to";
const char EngInstr13[] = "repeat";
const char EngInstr14[] = "instructions.";

//spanish instructions
const char SpanInstr1[] = "Recetas completas";
const char SpanInstr2[] = "para ganar monedas.";
const char SpanInstr3[] = "El bokchoy debe";
const char SpanInstr4[] = "estar picado.";
const char SpanInstr5[] = "El tomate debe";
const char SpanInstr6[] = "La masa debe";
const char SpanInstr7[] = "enrollarse y";
const char SpanInstr8[] = "cocinarse";
const char SpanInstr9[] = "La carne debe";
const char SpanInstr10[] = "Haga clic ARRIBA";
const char SpanInstr11[] = "para repetir las";
const char SpanInstr12[] = "instrucciones.";

//game over
const char EngGameOver1[] = "KITCHEN COMPLETE";
const char EngGameOver2[] = "Orders Delivered:";
const char EngGameOver3[] = "Total:";
const char SpGameOver1[] = "COCINA COMPLETA";
const char SpGameOver2[] = "Pedidos Entregados:";

int recipe;
// ALL ST7735 OUTPUT MUST OCCUR IN MAIN
int main(void){ // final main
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf();
  //ST7735_InvertDisplay(1);
    //note: if you colors are weird, see different options for
    // ST7735_InitR(INITR_REDTAB); inside ST7735_InitPrintf()
  ADC_InitDual(ADC0, 0, 5, ADCVREF_VDDA);
  ADC_InitDual(ADC1, 3, 4, ADCVREF_VDDA);
  Switch_Init(); // initialize switches
  //LED_Init();    // initialize LED
  Sound_Init();  // initialize sound
  TExaS_Init(0,0,&TExaS_LaunchPadLogicPB27PB26); // PB27 and PB26
    // initialize interrupts on TimerG12 at 30 Hz
  TimerG12_IntArm(40000000/30,2); //0xffff
  TimerG0_IntArm(10000, 80, 1); //interrupt at 100Hz?
  ST7735_FillScreen(ST7735_BLACK);
  // initialize all data structures
  __enable_irq();
  ST7735_DrawBitmap(0, 160, intropage, 128, 160);
  Clock_Delay1ms(1000); //change to button maybe


  //ST7735_DrawBitmap(0, 160, intropage2, 128, 160);
  ST7735_FillScreen(ST7735_BLACK);
  ST7735_SetTextColor(0xFFFF);
  ST7735_SetCursor(2, 2);
  ST7735_OutString(Welcome);
  ST7735_SetCursor(2, 3);
  ST7735_OutString(ChooseL1);
  ST7735_SetCursor(2, 4);
  ST7735_OutString(ChooseL2);
  ST7735_SetCursor(4, 6);
  ST7735_OutString(Language_English_Sel);
  ST7735_SetCursor(4, 8);
  ST7735_OutString(Language_Spanish);
  lang.english = 1;
  lang.spanish = 0;

  while(P1PICKDROPSW == 0){
      p1.dy = (256*(j1vert-2048))/2048; //-256 to 256
       if(p1.dy > 50){//up
           ST7735_SetCursor(4, 6);
           ST7735_OutString(Language_English_Sel);
           ST7735_SetCursor(4, 8);
           ST7735_OutString(Language_Spanish);
           lang.english = 1;
           lang.spanish = 0;
       }else if(p1.dy < -50){
           ST7735_SetCursor(4, 6);
           ST7735_OutString(Language_English);
           ST7735_SetCursor(4, 8);
           ST7735_OutString(Language_Spanish_Sel);
           lang.english = 0;
           lang.spanish = 1;
       }
  }

  Clock_Delay1ms(250);


  while(P1PICKDROPSW == 0){
      if(lang.english == 1){
enginstr1:ST7735_FillScreen(ST7735_BLACK); //first instruction page
          ST7735_SetCursor(2, 2);
          ST7735_OutString(EngInstr1);
          ST7735_SetCursor(2, 3);
          ST7735_OutString(EngInstr2);
          ST7735_SetCursor(4, 5);
          ST7735_OutString(EngInstr3);
          ST7735_SetCursor(4, 6);
          ST7735_OutString(EngInstr4);
          ST7735_SetCursor(4, 8);
          ST7735_OutString(EngInstr5);
          ST7735_SetCursor(4, 9);
          ST7735_OutString(EngInstr4);
          ST7735_SetCursor(4, 11);
          ST7735_OutString(EngInstr7);
          ST7735_SetCursor(4, 12);
          ST7735_OutString(EngInstr8);
          ST7735_SetCursor(4, 13);
          ST7735_OutString(EngInstr9);
          Clock_Delay1ms(2000);
          ST7735_FillScreen(ST7735_BLACK); //second instruction page
enginstr2:ST7735_SetCursor(4, 2);
          ST7735_OutString(EngInstr10);
          ST7735_SetCursor(4, 3);
          ST7735_OutString(EngInstr11);
          ST7735_SetCursor(2, 5);
          ST7735_OutString(EngInstr12);
          ST7735_SetCursor(2, 6);
          ST7735_OutString(EngInstr13);
          ST7735_SetCursor(2, 7);
          ST7735_OutString(EngInstr14);
          if(P1CHOPSW != 0){
              goto enginstr1;
          }else if(P1PICKDROPSW != 0){
              break;
          }else{
              goto enginstr2;
          }
      }else if(lang.spanish == 1){
spinstr1: ST7735_FillScreen(ST7735_BLACK);
          ST7735_SetCursor(2, 2);
          ST7735_OutString(SpanInstr1);
          ST7735_SetCursor(2, 3);
          ST7735_OutString(SpanInstr1);
          ST7735_SetCursor(4, 5);
          ST7735_OutString(SpanInstr3);
          ST7735_SetCursor(4, 6);
          ST7735_OutString(SpanInstr4);
          ST7735_SetCursor(4, 8);
          ST7735_OutString(SpanInstr5);
          ST7735_SetCursor(4, 9);
          ST7735_OutString(SpanInstr4);
          ST7735_SetCursor(4, 11);
          ST7735_OutString(SpanInstr6);
          ST7735_SetCursor(4, 12);
          ST7735_OutString(SpanInstr7);
          ST7735_SetCursor(4, 13);
          ST7735_OutString(SpanInstr8);
          Clock_Delay1ms(2000);
          ST7735_FillScreen(ST7735_BLACK);
spinstr2: ST7735_SetCursor(4, 2);
          ST7735_OutString(SpanInstr9);
          ST7735_SetCursor(4, 3);
          ST7735_OutString(SpanInstr8);
          ST7735_SetCursor(2, 5);
          ST7735_OutString(SpanInstr10);
          ST7735_SetCursor(2, 6);
          ST7735_OutString(SpanInstr11);
          ST7735_SetCursor(2, 7);
          ST7735_OutString(SpanInstr12);
          if(P1CHOPSW != 0){
              goto spinstr1;
          }else if(P1PICKDROPSW != 0){
              break;
          }else{
              goto spinstr2;
          }
      }
  }

  ST7735_DrawBitmap(0, 160, backgroundedit, 128, 160);
  ST7735_DrawSprite(0, 65, backgroundedit, emptypot, 0xFFFF);
  ST7735_DrawSprite(P1XSTARTPT, P1YSTARTPT, backgroundedit, valbano, 0xFFFF);
  ST7735_DrawSprite(P0XSTARTPT, P0YSTARTPT, backgroundedit, holt, 0xFFFF);
  ST7735_DrawSprite(110, 110, backgroundedit, emptybowl, 0xFFFF);
  ST7735_FillRect(25, 0, 16, 17, 0xFFFF);
  //uint32_t x;
  p1.x = P1XSTARTPT;
  p1.y = P1YSTARTPT;
  p0.x = P0XSTARTPT;
  p0.y = P0YSTARTPT;
  emp_pot.on_stove = 1;
  bns.erase = 1, rec1.erase = 1, rec2.erase = 1, rec3.erase = 1;
  int en_greenline = 0;
  int greenline = 2;
  int time_check = 0;
  timer_count = 0;
  timer_display = 160;
  //rec: 0 = bns, 1 = bns w/o tom, 2 = bns w/o bok, 3 = bokchoy beef
  //recipe = Random(4); //generates range form 0-3 (4 items)
  recipe = 3;
  while(1){
      while(!sema){}
      sema = 0;

      if(game_over){
          ST7735_SetTextColor(0xFFFF);
          if(lang.english == 1){
              while(1){
                  //ST7735_FillScreen(ST7735_BLACK);
                  ST7735_SetCursor(1, 2);
                  ST7735_OutString(EngGameOver1);
                  ST7735_SetCursor(1, 4);
                  ST7735_OutString(EngGameOver2);
                  ST7735_SetCursor(19, 4);
                  ST7735_OutUDec(bns.delivered + rec1.delivered + rec2.delivered + rec3.delivered);
                  ST7735_SetCursor(1, 6);
                  ST7735_OutString(EngGameOver3);
                  ST7735_SetCursor(8, 6);
                  ST7735_OutUDec((bns.delivered * 30) + (rec1.delivered * 25) + (rec2.delivered * 25) + (rec3.delivered * 20));
              }
          }else if(lang.spanish == 1){
              while(1){
                  ST7735_SetCursor(1, 2);
                  ST7735_OutString(SpGameOver1);
                  ST7735_SetCursor(1, 4);
                  ST7735_OutString(SpGameOver2);
                  ST7735_SetCursor(19, 4);
                  ST7735_OutUDec(bns.delivered + rec1.delivered + rec2.delivered + rec3.delivered);
                  ST7735_SetCursor(1, 6);
                  ST7735_OutString(EngGameOver3);
                  ST7735_SetCursor(8, 6);
                  ST7735_OutUDec((bns.delivered * 30) + (rec1.delivered * 25) + (rec2.delivered * 25) + (rec3.delivered * 20));
              }
          }
      }

      //update sprites
      ST7735_DrawSprite(p1.x, p1.y, backgroundedit, valbano, 0xFFFF);
      ST7735_DrawSprite(p0.x, p0.y, backgroundedit, holt, 0xFFFF);
      //ST7735_FillRect(25, 0, 16, 17, 0xFFFF);
      if((tom.v_carry == 1) || (tom.h_carry == 1)){
          ST7735_DrawSprite(tom.x, tom.y, backgroundedit, tomato, 0xFFFF);
      }
      if((bok.v_carry == 1) || (bok.h_carry == 1)){
          ST7735_DrawSprite(bok.x, bok.y, backgroundedit, bokchoy, 0xFFFF);
      }
      if((bf.v_carry == 1) || (bf.h_carry == 1)){
          ST7735_DrawSprite(bf.x, bf.y, backgroundedit, meat, 0xFFFF);
      }
      if((dgh.v_carry == 1) || (dgh.h_carry == 1)){
          ST7735_DrawSprite(dgh.x, dgh.y, backgroundedit, dough, 0xFFFF);
      }
      if((tomcut.v_carry == 1) || (tomcut.h_carry == 1)){
          ST7735_DrawSprite(tomcut.x, tomcut.y, backgroundedit, tomatocut, 0xFFFF);
      }
      if((bokcut.v_carry == 1) || (bokcut.h_carry == 1)){
          ST7735_DrawSprite(bokcut.x, bokcut.y, backgroundedit, bokchoycut, 0xFFFF);
      }
      if((d_pot.v_carry == 1) || (d_pot.h_carry == 1)){
          ST7735_DrawSprite(d_pot.x, d_pot.y, backgroundedit, spr_donepot, 0xFFFF);
      }
      if((emp_pot.v_carry == 1) || (emp_pot.h_carry == 1)){
          ST7735_DrawSprite(emp_pot.x, emp_pot.y, backgroundedit, emptypot, 0xFFFF);
      }
      if((ndl.v_carry == 1) || (ndl.h_carry == 1)){
          ST7735_DrawSprite(ndl.x, ndl.y, backgroundedit, noodle, 0xFFFF);
      }
      if((noodle_pot.v_carry == 1) || (noodle_pot.h_carry == 1)){
          ST7735_DrawSprite(noodle_pot.x, noodle_pot.y, backgroundedit, spr_progpot, 0xFFFF);
      }
      if((bns.v_carry == 1) || (bns.h_carry == 1)){
          ST7735_DrawSprite(bns.x, bns.y, backgroundedit, beefnoodlesoup, 0xFFFF);
      }
      if((rec1.v_carry == 1) || (rec1.h_carry == 1)){
                ST7735_DrawSprite(rec1.x, rec1.y, backgroundedit, spr_rec1, 0xFFFF);
      }
      if((rec2.v_carry == 1) || (rec2.h_carry == 1)){
                ST7735_DrawSprite(rec2.x, rec2.y, backgroundedit, spr_rec2, 0xFFFF);
      }
      if((rec3.v_carry == 1) || (rec3.h_carry == 1)){
                ST7735_DrawSprite(rec3.x, rec3.y, backgroundedit, spr_rec3, 0xFFFF);
      }
      if((bns.completed_bowl == 0) && (rec1.completed_bowl == 0) && (rec2.completed_bowl == 0) && (rec3.completed_bowl == 0)){
          ST7735_DrawSprite(110, 110, backgroundedit, emptybowl, 0xFFFF);
      }

      //score
      ST7735_DrawCircle(0, 0, 0x5F7F);
      ST7735_SetCursor(2, 0);
      ST7735_SetTextColor(0x5F7F);
      ST7735_OutUDec((bns.delivered * 30) + (rec1.delivered * 25) + (rec2.delivered * 25) + (rec3.delivered * 20));


      //recipe
      if(recipe == 0){ //bns
          ST7735_DrawSmallCircle(26, 4, 0x118B); //beef
          ST7735_DrawSmallCircle(34, 4, 0x8E9F); //noodle
          ST7735_DrawSmallCircle(26, 10, 0x0019); //tomato
          ST7735_DrawSmallCircle(34, 10, 0x6423); //bok
      }else if(recipe == 1){ //bns w/o tom
          ST7735_DrawSmallCircle(26, 4, 0x118B); //beef
          ST7735_DrawSmallCircle(34, 4, 0x8E9F); //noodle
          ST7735_DrawSmallCircle(34, 10, 0x6423); //bok
      }else if(recipe == 2){ //bns w/o bok
          ST7735_DrawSmallCircle(26, 4, 0x118B); //beef
          ST7735_DrawSmallCircle(34, 4, 0x8E9F); //noodle
          ST7735_DrawSmallCircle(26, 10, 0x0019); //tomato
      }else if(recipe == 3){ //bokchoy beef
          ST7735_DrawSmallCircle(26, 4, 0x118B); //beef
          ST7735_DrawSmallCircle(34, 10, 0x6423); //bok
      }

      //timer
      if(game_over == 0){
          if(timer_display < 0){
              game_over = 1;
              ST7735_FillScreen(ST7735_BLACK);
          }else if(timer_display < 10){
              ST7735_EraseSprite(90, 15, backgroundedit); //maybe don't need
              ST7735_EraseSprite(94, 15, backgroundedit);
              ST7735_SetCursor(19, 0);
              ST7735_OutUDec(timer_display);
          }else if(timer_display < 100){
              ST7735_EraseSprite(90, 15, backgroundedit);
              ST7735_SetCursor(18, 0);
              ST7735_OutUDec(timer_display);
          }else{
              ST7735_SetCursor(17, 0);
              ST7735_OutUDec(timer_display);
          }
      }




//========PICKING UP=========
 //should i implement picking pot up from stove? not sure if there's a purpose if we can't put it down somewhere random and add ingredients or whatev

      //check if valvano picks smth up //CHANGE SWITCh FOR REAL THIN IDK WHY IT'S UGGING RN
      if((P1PICKDROPSW != 0) && (p1.carry == 0)){
          if((40 < p1.x) && (p1.x < 60) && (p1.y == 35)){ //tomato region
              p1.carry = 1;
              tom.v_carry = 1;
              tom.x = p1.x;
              tom.y = p1.y - 20;
              ST7735_DrawSprite(tom.x, tom.y, backgroundedit, tomato, 0xFFFF);
          }else if((62 < p1.x) && (p1.x < 73) && (p1.y == 35)){ //bokchoy region
              p1.carry = 1;
              bok.v_carry = 1;
              bok.x = p1.x;
              bok.y = p1.y - 20;
              ST7735_DrawSprite(bok.x, bok.y, backgroundedit, bokchoy, 0xFFFF);
          }else if((117 > p1.y) && (p1.y > 102) && (p1.x == 20)){ //meat region
              p1.carry = 1;
              bf.v_carry = 1;
              bf.x = p1.x;
              bf.y = p1.y - 20;
              ST7735_DrawSprite(bf.x, bf.y, backgroundedit, meat, 0xFFFF);
          }else if((124 < p1.y) && (p1.y < 130) && (p1.x == 20)){ //dough region
              p1.carry = 1;
              dgh.v_carry = 1;
              dgh.x = p1.x;
              dgh.y = p1.y - 20;
              ST7735_DrawSprite(dgh.x, dgh.y, backgroundedit, dough, 0xFFFF);
          }else if((67 < p1.x) && (p1.x < 81) && (p1.y == 150) && (tom.chopped == 1)){ //tomatocut
              p1.carry = 1;
              tomcut.v_carry = 1;
              tomcut.x = p1.x;
              tomcut.y = p1.y - 20;
              tom.chopped = 0; //chopped tomato no longer on board
              ST7735_DrawSprite(tomcut.x, tomcut.y, backgroundedit, tomatocut, 0xFFFF);
              ST7735_EraseSprite(70, 159, backgroundedit);
          }else if((67 < p1.x) && (p1.x < 81) && (p1.y == 150) && (bok.chopped == 1)){ //bokchoycut
              p1.carry = 1;
              bokcut.v_carry = 1;
              bokcut.x = p1.x;
              bokcut.y = p1.y - 20;
              bok.chopped = 0; //chopped bokchoy no longer on board
              ST7735_DrawSprite(bokcut.x, bokcut.y, backgroundedit, bokchoycut, 0xFFFF);
              ST7735_EraseSprite(70, 159, backgroundedit);
          }else if((p1.x == 20) && (55 < p1.y) && (p1.y < 77) && (bf.cooked == 1) && (d_pot.on_stove == 1)){ //beefcooked
              emp_pot.on_stove = 0;
              p1.carry = 1;
              d_pot.v_carry = 1;
              d_pot.x = p1.x;
              d_pot.y = p1.y - 20;
              d_pot.on_stove = 0;
              bf.cooked = 0;
              ST7735_EraseSprite(0, 64, backgroundedit);
              ST7735_EraseSprite(0, 45, backgroundedit);
              ST7735_DrawSprite(d_pot.x, d_pot.y, backgroundedit, spr_donepot, 0xFFFF);
          }else if((67 < p1.x) && (p1.x < 81) && (p1.y == 150) && (dgh.noodled == 1)){ //dough noodled -- NOT DONE EDITING
              p1.carry = 1;
              ndl.v_carry = 1;
              ndl.x = p1.x;
              ndl.y = p1.y - 20;
              dgh.noodled = 0; //rolled dough no longer on board
              ST7735_DrawSprite(ndl.x, ndl.y, backgroundedit, noodle, 0xFFFF);
              ST7735_EraseSprite(70, 159, backgroundedit);
          }else if((p1.x == 20) && (55 < p1.y) && (p1.y < 77) && (ndl.cooked == 1) && (d_pot.on_stove == 1)){ //noodlecooked
              emp_pot.on_stove = 0;
              p1.carry = 1;
              noodle_pot.v_carry = 1;
              noodle_pot.x = p1.x;
              noodle_pot.y = p1.y - 20;
              ndl.cooked = 0;
              d_pot.on_stove = 0;
              ST7735_EraseSprite(0, 64, backgroundedit);
              ST7735_EraseSprite(0, 45, backgroundedit);
              ST7735_DrawSprite(noodle_pot.x, noodle_pot.y, backgroundedit, spr_progpot, 0xFFFF);
          }else if((bns.completed_bowl == 1) && (p1.x == 95) && (107 < p1.y) && (p1.y < 121) && (p1.carry == 0)){ //pick up completed bowl
              p1.carry = 1;
              bns.completed_bowl = 0;
              bns.v_carry = 1;
              bns.x = p1.x;
              bns.y = p1.y - 20;
              bns.erase = 1;
              ST7735_EraseSprite(110, 110, backgroundedit);
              ST7735_DrawSprite(bns.x, bns.y, backgroundedit, beefnoodlesoup, 0xFFFF);
          }else if((rec1.completed_bowl == 1) && (p1.x == 95) && (107 < p1.y) && (p1.y < 121) && (p1.carry == 0)){
              p1.carry = 1;
              rec1.completed_bowl = 0;
              rec1.v_carry = 1;
              rec1.x = p1.x;
              rec1.y = p1.y - 20;
              rec1.erase = 1;
              ST7735_EraseSprite(110, 110, backgroundedit);
              ST7735_DrawSprite(rec1.x, rec1.y, backgroundedit, spr_rec1, 0xFFFF);
          }else if((rec2.completed_bowl == 1) && (p1.x == 95) && (107 < p1.y) && (p1.y < 121) && (p1.carry == 0)){
              p1.carry = 1;
              rec2.completed_bowl = 0;
              rec2.v_carry = 1;
              rec2.x = p1.x;
              rec2.y = p1.y - 20;
              rec2.erase = 1;
              ST7735_EraseSprite(110, 110, backgroundedit);
              ST7735_DrawSprite(rec2.x, rec2.y, backgroundedit, spr_rec2, 0xFFFF);
          }else if((rec3.completed_bowl == 1) && (p1.x == 95) && (107 < p1.y) && (p1.y < 121) && (p1.carry == 0)){
              p1.carry = 1;
              rec3.completed_bowl = 0;
              rec3.v_carry = 1;
              rec3.x = p1.x;
              rec3.y = p1.y - 20;
              rec3.erase = 1;
              ST7735_EraseSprite(110, 110, backgroundedit);
              ST7735_DrawSprite(rec3.x, rec3.y, backgroundedit, spr_rec3, 0xFFFF);
          }
      }

      //check if holt picks smth up //CHANGE SWITHCH IDK WHY ITS HUGGING
      if((P0PICKDROPSW != 0) && (p0.carry == 0)){
          if((40 < p0.x) && (p0.x < 60) && (p0.y == 35)){
              p0.carry = 1;
              tom.h_carry = 1;
              tom.x = p0.x;
              tom.y = p0.y - 20;
              ST7735_DrawSprite(tom.x, tom.y, backgroundedit, tomato, 0xFFFF);
          }else if((62 < p0.x) && (p0.x < 73) && (p0.y == 35)){ //bokchoy region
              p0.carry = 1;
              bok.h_carry = 1;
              bok.x = p0.x;
              bok.y = p0.y - 20;
              ST7735_DrawSprite(bok.x, bok.y, backgroundedit, bokchoy, 0xFFFF);
          }else if((117 > p0.y) && (p0.y > 102) && (p0.x == 20)){ //meat region
              p0.carry = 1;
              bf.h_carry = 1;
              bf.x = p0.x;
              bf.y = p0.y - 20;
              ST7735_DrawSprite(bf.x, bf.y, backgroundedit, meat, 0xFFFF);
          }else if((124 < p0.y) && (p0.y < 130) && (p0.x == 20)){ //dough region
              p0.carry = 1;
              dgh.h_carry = 1;
              dgh.x = p0.x;
              dgh.y = p0.y - 20;
              ST7735_DrawSprite(dgh.x, dgh.y, backgroundedit, dough, 0xFFFF);
          }else if((67 < p0.x) && (p0.x < 81) && (p0.y == 150) && (tom.chopped == 1)){ //tomatocut from board
              p0.carry = 1;
              tomcut.h_carry = 1;
              tomcut.x = p0.x;
              tomcut.y = p0.y - 20;
              tom.chopped = 0;
              ST7735_DrawSprite(tomcut.x, tomcut.y, backgroundedit, tomatocut, 0xFFFF);
              ST7735_EraseSprite(70, 159, backgroundedit);
          }else if((67 < p0.x) && (p0.x < 81) && (p0.y == 150) && (bok.chopped == 1)){
              p0.carry = 1;
              bokcut.h_carry = 1;
              bokcut.x = p0.x;
              bokcut.y = p0.y - 20;
              bok.chopped = 0;
              ST7735_DrawSprite(bokcut.x, bokcut.y, backgroundedit, bokchoycut, 0xFFFF);
              ST7735_EraseSprite(70, 159, backgroundedit);
          }else if((p0.x == 20) && (55 < p0.y) && (p0.y < 77) && (bf.cooked == 1) && (d_pot.on_stove == 1)){ //beefcooked
              emp_pot.on_stove = 0;;
              p0.carry = 1;
              d_pot.on_stove = 0;
              d_pot.h_carry = 1;
              d_pot.x = p0.x;
              d_pot.y = p0.y - 20;
              bf.cooked = 0;
              ST7735_EraseSprite(0, 64, backgroundedit);
              ST7735_EraseSprite(0, 45, backgroundedit);
              ST7735_DrawSprite(d_pot.x, d_pot.y, backgroundedit, spr_donepot, 0xFFFF);
          }else if((67 < p0.x) && (p0.x < 81) && (p0.y == 150) && (dgh.noodled == 1)){ //dough noodled -- NOT DONE EDITING
              p0.carry = 1;
              ndl.h_carry = 1;
              ndl.x = p0.x;
              ndl.y = p0.y - 20;
              dgh.noodled = 0; //rolled dough no longer on board
              ST7735_DrawSprite(ndl.x, ndl.y, backgroundedit, noodle, 0xFFFF);
              ST7735_EraseSprite(70, 159, backgroundedit);
          }else if((p0.x == 20) && (55 < p0.y) && (p0.y < 77) && (ndl.cooked == 1) && (d_pot.on_stove == 1)){ //noodlecooked
              emp_pot.on_stove = 0;
              p0.carry = 1;
              noodle_pot.h_carry = 1;
              noodle_pot.x = p0.x;
              noodle_pot.y = p0.y - 20;
              //noodle_pot.on_stove = 0; //valvano doesn't have
              d_pot.on_stove = 0;
              ndl.cooked = 0;
              ST7735_EraseSprite(0, 64, backgroundedit);
              ST7735_EraseSprite(0, 45, backgroundedit);
              ST7735_DrawSprite(noodle_pot.x, noodle_pot.y, backgroundedit, spr_progpot, 0xFFFF);
          }else if((bns.completed_bowl == 1) && (p0.x == 95) && (107 < p0.y) && (p0.y < 121) && (p0.carry == 0)){ //pick up completed bowl
              p0.carry = 1;
              bns.completed_bowl = 0;
              bns.h_carry = 1;
              bns.erase = 1;
              bns.x = p0.x;
              bns.y = p0.y - 20;
              ST7735_EraseSprite(110, 110, backgroundedit);
              ST7735_DrawSprite(bns.x, bns.y, backgroundedit, beefnoodlesoup, 0xFFFF);
          }else if((rec1.completed_bowl == 1) && (p0.x == 95) && (107 < p0.y) && (p0.y < 121) && (p0.carry == 0)){
              p0.carry = 1;
              rec1.completed_bowl = 0;
              rec1.h_carry = 1;
              rec1.x = p0.x;
              rec1.y = p0.y - 20;
              rec1.erase = 1;
              ST7735_EraseSprite(110, 110, backgroundedit);
              ST7735_DrawSprite(rec1.x, rec1.y, backgroundedit, spr_rec1, 0xFFFF);
          }else if((rec2.completed_bowl == 1) && (p0.x == 95) && (107 < p0.y) && (p0.y < 121) && (p0.carry == 0)){
              p0.carry = 1;
              rec2.completed_bowl = 0;
              rec2.h_carry = 1;
              rec2.x = p0.x;
              rec2.y = p0.y - 20;
              rec2.erase = 1;
              ST7735_EraseSprite(110, 110, backgroundedit);
              ST7735_DrawSprite(rec2.x, rec2.y, backgroundedit, spr_rec2, 0xFFFF);
          }else if((rec3.completed_bowl == 1) && (p0.x == 95) && (107 < p0.y) && (p0.y < 121) && (p0.carry == 0)){
              p0.carry = 1;
              rec3.completed_bowl = 0;
              rec3.h_carry = 1;
              rec3.x = p0.x;
              rec3.y = p0.y - 20;
              rec3.erase = 1;
              ST7735_EraseSprite(110, 110, backgroundedit);
              ST7735_DrawSprite(rec3.x, rec3.y, backgroundedit, spr_rec3, 0xFFFF);
          }
      }

//========DROPPING========
      //check valvano drop condition
      if((P1PICKDROPSW != 0) && (p1.carry == 1)){
          if((tom.v_carry == 1) && (59 < p1.x) && (p1.x < 81) && (p1.y == 150)){ //drop tomato
              if((59 < p1.x) && (p1.x < 81)){ //cutting board 1
                  p1.carry = 0;
                  tom.v_carry = 0;
                  tom.choppingboard1 = 1;
                  ST7735_EraseSprite(tom.x, tom.y, backgroundedit);
                  tom.x = 70;
                  tom.y = 159;
                  ST7735_DrawSprite(tom.x, tom.y, backgroundedit, tomato, 0xFFFF); //draw tomato on board
              }/*else if((81 < p1.x) && (p1.x < 95)){ //cutting board 2 -- no idea why it's not working
                  p1.carry = 0;
                  tom.v_carry = 0;
                  tom.choppingboard2 = 1;
                  ST7735_EraseSprite(tom.x, tom.y, backgroundedit);
                  tom.x = 88;
                  tom.y = 159;
                  ST7735_DrawSprite(tom.x, tom.y, backgroundedit, tomato, 0xFFFF);
              }*/
          }else if((bok.v_carry == 1) && (59 < p1.x) && (p1.x < 81) && (p1.y == 150)){ //drop bokchoy
              if((59 < p1.x) && (p1.x < 81)){ //cutting board 1
                  p1.carry = 0;
                  bok.v_carry = 0;
                  bok.choppingboard1 = 1;
                  ST7735_EraseSprite(bok.x, bok.y, backgroundedit);
                  bok.x = 70;
                  bok.y = 159;
                  ST7735_DrawSprite(bok.x, bok.y, backgroundedit, bokchoy, 0xFFFF); //draw bokchoy on board
              }/*else if((81 < p1.x) && (p1.x < 95)){ //cutting board 2 -- no idea why it's not working
                  p1.carry = 0;
                  bok.v_carry = 0;
                  bok.choppingboard2 = 1;
                  ST7735_EraseSprite(bok.x, bok.y, backgroundedit);
                  bok.x = 88;
                  bok.y = 159;
                  ST7735_DrawSprite(bok.x, bok.y, backgroundedit, bokchoy, 0xFFFF);
              }*/
          }else if((dgh.v_carry == 1) && (59 < p1.x) && (p1.x < 81) && (p1.y == 150)){ //drop dough
              if((67 < p1.x) && (p1.x < 81)){ //cuting board 1
                  p1.carry = 0;
                  dgh.v_carry = 0;
                  dgh.choppingboard1 = 1;
                  ST7735_EraseSprite(dgh.x, dgh.y, backgroundedit);
                  dgh.x = 70;
                  dgh.y = 159;
                  ST7735_DrawSprite(dgh.x, dgh.y, backgroundedit, dough, 0xFFFF);
              }
          }else if((tomcut.v_carry == 1) && (p1.x == 95) && (107 < p1.y) && (p1.y < 121) && (recipe != 1) && (recipe != 3)){ //drop cut tomato in bowl
                  p1.carry = 0;
                  tomcut.v_carry = 0;
                  tomcut.in_bowl = 1;
                  bns.erase = 0, rec1.erase = 0, rec2.erase = 0, rec3.erase = 0;
                  ST7735_EraseSprite(tomcut.x, tomcut.y, backgroundedit);
                  ST7735_DrawSmallCircle(114, 85, 0x0019);
          }else if((bokcut.v_carry == 1) && (p1.x == 95) && (107 < p1.y) && (p1.y < 121) && (recipe !=2)){ //drop cut bokchoy in bowl
              p1.carry = 0;
              bokcut.v_carry = 0;
              bokcut.in_bowl = 1;
              bns.erase = 0, rec1.erase = 0, rec2.erase = 0, rec3.erase = 0;
              ST7735_EraseSprite(bokcut.x, bokcut.y, backgroundedit);
              ST7735_DrawSmallCircle(120, 85, 0x6423);
          }else if((bf.v_carry == 1) && (emp_pot.on_stove == 1) && (emp_pot.bf_filled == 0) && (emp_pot.noodle_filled == 0) && (p1.x == 20) && (55 < p1.y) && (p1.y < 77)){//drop beef in pot
              p1.carry = 0;
              bf.v_carry = 0;
              emp_pot.bf_filled = 1;
              ST7735_EraseSprite(bf.x, bf.y, backgroundedit);
              ST7735_EraseSprite(0, 65, backgroundedit);
              ST7735_DrawSprite(0, 65, backgroundedit, spr_progpot, 0xFFFF);
              en_greenline = 1;
              time_check = 0;
          }else if((d_pot.v_carry == 1) && (p1.x == 95) && (107 < p1.y) && (p1.y < 121)){ //drop beef broth in bowl
              //p1.carry = 0;
              d_pot.v_carry = 0;
              emp_pot.v_carry = 1;
              bf.in_bowl = 1;
              bns.erase = 0, rec1.erase = 0, rec2.erase = 0, rec3.erase = 0;
              emp_pot.x = p1.x;
              emp_pot.y = p1.y - 20;
              ST7735_DrawSprite(emp_pot.x, emp_pot.y, backgroundedit, emptypot, 0xFFFF);
              ST7735_DrawSmallCircle(114, 78, 0x118B);
          }else if((emp_pot.v_carry == 1) && (emp_pot.on_stove == 0) && (p1.x == 20) && (55 < p1.y) && (p1.y < 77)){ //place pot back on stove
              p1.carry = 0;
              emp_pot.v_carry = 0;
              emp_pot.on_stove = 1;
              emp_pot.bf_filled = 0;
              emp_pot.noodle_filled = 0;
              ST7735_EraseSprite(emp_pot.x, emp_pot.y, backgroundedit);
              emp_pot.x = 0;
              emp_pot.y = 65;
              ST7735_DrawSprite(emp_pot.x, emp_pot.y, backgroundedit, emptypot, 0xFFFF);
          }else if((ndl.v_carry == 1) && (emp_pot.on_stove == 1) && (emp_pot.bf_filled == 0) && (emp_pot.noodle_filled == 0) && (p1.x == 20) && (55 < p1.y) && (p1.y < 77)){ //drop noodle in pot
              p1.carry = 0;
              ndl.v_carry = 0;
              emp_pot.noodle_filled = 1;
              ST7735_EraseSprite(ndl.x, ndl.y, backgroundedit);
              ST7735_EraseSprite(0, 65, backgroundedit);
              ST7735_DrawSprite(0, 65, backgroundedit, spr_progpot, 0xFFFF);
              en_greenline = 1;
              time_check = 0;
          }else if((noodle_pot.v_carry == 1) && (p1.x == 95) && (107 < p1.y) && (p1.y < 121) && (recipe != 3)){ //drop noodle in bowl
              //p1.carry = 0;
              noodle_pot.v_carry = 0;
              emp_pot.v_carry = 1;
              ndl.in_bowl = 1;
              bns.erase = 0, rec1.erase = 0, rec2.erase = 0, rec3.erase = 0;
              emp_pot.x = p1.x; //NEW CARRYING COORDINATES
              emp_pot.y = p1.y - 20;
              ST7735_DrawSprite(emp_pot.x, emp_pot.y, backgroundedit, emptypot, 0xFFFF);
              ST7735_DrawSmallCircle(120, 78, 0x8E9F);
          }else if((bns.v_carry == 1) && (20 < p1.x) && (p1.x < 50) && (p1.y == 150) && (recipe == 0)){
              bns.delivered++;
              bns.v_carry = 0;
              ST7735_EraseSprite(p1.x, p1.y - 20, backgroundedit);
              p1.carry = 0, tomcut.in_bowl = 0, bokcut.in_bowl = 0, bf.in_bowl = 0, ndl.in_bowl = 0, emp_pot.bf_filled = 0, emp_pot.noodle_filled = 0;
              ST7735_DrawSprite(110, 110, backgroundedit, emptybowl, 0xFFFF); //don't forget to reset variables at top of main
              recipe = Random(4);
              ST7735_FillRect(25, 0, 16, 17, 0xFFFF);
          }else if((rec1.v_carry == 1) && (20 < p1.x) && (p1.x < 50) && (p1.y == 150) && (recipe == 1)){
              rec1.delivered++;
              rec1.v_carry = 0;
              ST7735_EraseSprite(p1.x, p1.y - 20, backgroundedit);
              p1.carry = 0, tomcut.in_bowl = 0, bokcut.in_bowl = 0, bf.in_bowl = 0, ndl.in_bowl = 0, emp_pot.bf_filled = 0, emp_pot.noodle_filled = 0;
              ST7735_DrawSprite(110, 110, backgroundedit, emptybowl, 0xFFFF); //don't forget to reset variables at top of main
              recipe = Random(4);
              ST7735_FillRect(25, 0, 16, 17, 0xFFFF);
          }else if((rec2.v_carry == 1) && (20 < p1.x) && (p1.x < 50) && (p1.y == 150) && (recipe == 2)){
              rec2.delivered++;
              rec2.v_carry = 0;
              ST7735_EraseSprite(p1.x, p1.y - 20, backgroundedit);
              p1.carry = 0, tomcut.in_bowl = 0, bokcut.in_bowl = 0, bf.in_bowl = 0, ndl.in_bowl = 0, emp_pot.bf_filled = 0, emp_pot.noodle_filled = 0;
              ST7735_DrawSprite(110, 110, backgroundedit, emptybowl, 0xFFFF); //don't forget to reset variables at top of main
              recipe = Random(4);
              ST7735_FillRect(25, 0, 16, 17, 0xFFFF);
          }else if((rec3.v_carry == 1) && (20 < p1.x) && (p1.x < 50) && (p1.y == 150) && (recipe == 3)){
              rec3.delivered++;
              rec3.v_carry = 0;
              ST7735_EraseSprite(p1.x, p1.y - 20, backgroundedit);
              p1.carry = 0, tomcut.in_bowl = 0, bokcut.in_bowl = 0, bf.in_bowl = 0, ndl.in_bowl = 0, emp_pot.bf_filled = 0, emp_pot.noodle_filled = 0;
              ST7735_DrawSprite(110, 110, backgroundedit, emptybowl, 0xFFFF); //don't forget to reset variables at top of main
              recipe = Random(4);
              ST7735_FillRect(25, 0, 16, 17, 0xFFFF);
          }else if((p1.carry == 1) && (140 < p1.y) && (p1.y < 151) && (p1.x == 95)){ //drop into trash
              ST7735_EraseSprite(p1.x, p1.y-20, backgroundedit);
              if(tom.v_carry == 1){
                  p1.carry = 0;
                  tom.v_carry = 0;
              }else if(bok.v_carry == 1){
                  p1.carry = 0;
                  bok.v_carry = 0;
              }else if(bf.v_carry == 1){
                  p1.carry = 0;
                  bf.v_carry = 0;
              }else if(dgh.v_carry == 1){
                  p1.carry = 0;
                  dgh.v_carry = 0;
              }else if(tomcut.v_carry == 1){
                  p1.carry = 0;
                  tomcut.v_carry = 0;
              }else if(bokcut.v_carry == 1){
                  p1.carry = 0;
                  bokcut.v_carry = 0;
              }else if(ndl.v_carry == 1){
                  p1.carry = 0;
                  ndl.v_carry = 0;
              }else if(d_pot.v_carry == 1){
                  d_pot.v_carry = 0;
                  emp_pot.v_carry = 1;
                  emp_pot.x = p1.x;
                  emp_pot.y = p1.y - 20;
                  ST7735_DrawSprite(emp_pot.x, emp_pot.y, backgroundedit, emptypot, 0xFFFF);
              }else if(noodle_pot.v_carry == 1){
                  noodle_pot.v_carry = 0;
                  emp_pot.v_carry = 1;
                  emp_pot.x = p1.x;
                  emp_pot.y = p1.y - 20;
                  ST7735_DrawSprite(emp_pot.x, emp_pot.y, backgroundedit, emptypot, 0xFFFF);
              }
          }

      }

      //check holt drop condition
      if((P0PICKDROPSW != 0) && (p0.carry == 1)){
          if((tom.h_carry == 1) && (59 < p0.x) && (p0.x < 81) && (p0.y == 150)){ //drop tomato
              if((59 < p0.x) && (p0.x < 81)){ //cutting board 1 -- same range as above bc no secon board
                  p0.carry = 0;
                  tom.h_carry = 0;
                  tom.choppingboard1 = 1;
                  ST7735_EraseSprite(tom.x, tom.y, backgroundedit);
                  tom.x = 70;
                  tom.y = 159;
                  ST7735_DrawSprite(tom.x, tom.y, backgroundedit, tomato, 0xFFFF); //draw tomato on board
              }/*else if((81 < p0.x) && (p0.x < 95)){ //cutting board 2 -- no idea why it's not working
                  p0.carry = 0;
                  tom.h_carry = 0;
                  tom.choppingboard2 = 1;
                  ST7735_EraseSprite(tom.x, tom.y, backgroundedit);
                  tom.x = 88;
                  tom.y = 159;
                  ST7735_DrawSprite(tom.x, tom.y, backgroundedit, tomato, 0xFFFF);
              }*/
          }else if((bok.h_carry == 1) && (59 < p0.x) && (p0.x < 81) && (p0.y == 150)){ //drop bokchoy
              if((59 < p0.x) && (p0.x < 81)){ //cutting board 1
                  p0.carry = 0;
                  bok.h_carry = 0;
                  bok.choppingboard1 = 1;
                  ST7735_EraseSprite(bok.x, bok.y, backgroundedit);
                  bok.x = 70;
                  bok.y = 159;
                  ST7735_DrawSprite(bok.x, bok.y, backgroundedit, bokchoy, 0xFFFF); //draw bokchoy on board
              }/*else if((81 < p0.x) && (p0.x < 95)){ //cutting board 2 -- no idea why it's not working
                  p0.carry = 0;
                  bok.h_carry = 0;
                  bok.choppingboard2 = 1;
                  ST7735_EraseSprite(bok.x, bok.y, backgroundedit);
                  bok.x = 88;
                  bok.y = 159;
                  ST7735_DrawSprite(bok.x, bok.y, backgroundedit, bokchoy, 0xFFFF);
              }*/
          }else if((dgh.h_carry == 1) && (59 < p0.x) && (p0.x < 81) && (p0.y == 150)){ //drop dough
              if((65 < p0.x) && (p0.x < 75)){ //cuting board 1
                  p0.carry = 0;
                  dgh.h_carry = 0;
                  dgh.choppingboard1 = 1;
                  ST7735_EraseSprite(dgh.x, dgh.y, backgroundedit);
                  dgh.x = 70;
                  dgh.y = 159;
                  ST7735_DrawSprite(dgh.x, dgh.y, backgroundedit, dough, 0xFFFF);
              }
          }else if((tomcut.h_carry == 1) && (p0.x == 95) && (97 < p0.y) && (p0.y < 117) && (recipe != 1) && (recipe != 3)){ //drop cut tomato in bowl
              p0.carry = 0;
              tomcut.h_carry = 0;
              tomcut.in_bowl = 1;
              bns.erase = 0, rec1.erase = 0, rec2.erase = 0, rec3.erase = 0;
              ST7735_EraseSprite(tomcut.x, tomcut.y, backgroundedit);
              ST7735_DrawSmallCircle(114, 85, 0x0019);
          }else if((bokcut.h_carry == 1) && (p0.x == 95) && (107 < p0.y) && (p0.y < 121) && (recipe != 2)){ //drop cut bokchoy in bowl
              p0.carry = 0;
              bokcut.h_carry = 0;
              bokcut.in_bowl = 1;
              bns.erase = 0, rec1.erase = 0, rec2.erase = 0, rec3.erase = 0;
              ST7735_EraseSprite(bokcut.x, bokcut.y, backgroundedit);
              ST7735_DrawSmallCircle(120, 85, 0x6423);
          }else if((bf.h_carry == 1) && (emp_pot.bf_filled == 0) && (emp_pot.noodle_filled == 0) && (emp_pot.on_stove == 1) && (p0.x == 20) && (55 < p0.y) && (p0.y < 77)){//drop beef in pot
              p0.carry = 0;
              bf.h_carry = 0;
              emp_pot.bf_filled = 1;
              ST7735_EraseSprite(bf.x, bf.y, backgroundedit);
              ST7735_EraseSprite(0, 65, backgroundedit);
              ST7735_DrawSprite(0, 65, backgroundedit, spr_progpot, 0xFFFF);
              en_greenline = 1;
              time_check = 0;
          }else if((d_pot.h_carry == 1) && (p0.x == 95) && (107 < p0.y) && (p0.y < 121)){ //drop beef broth in bowl
              d_pot.h_carry = 0;
              emp_pot.h_carry = 1;
              emp_pot.bf_filled = 0;
              bf.in_bowl = 1;
              bns.erase = 0, rec1.erase = 0, rec2.erase = 0, rec3.erase = 0;
              emp_pot.x = p0.x;
              emp_pot.y = p0.y - 20;
              ST7735_DrawSprite(emp_pot.x, emp_pot.y, backgroundedit, emptypot, 0xFFFF);
              ST7735_DrawSmallCircle(114, 78, 0x118B);
          }else if((emp_pot.h_carry == 1) && (emp_pot.on_stove == 0) && (p0.x == 20) && (55 < p0.y) && (p0.y < 77)){ //drop empty pot back on stove
              p0.carry = 0;
              emp_pot.h_carry = 0;
              emp_pot.on_stove = 1;
              emp_pot.bf_filled = 0;
              emp_pot.noodle_filled = 0;
              ST7735_EraseSprite(emp_pot.x, emp_pot.y, backgroundedit);
              emp_pot.x = 0;
              emp_pot.y = 65;
              ST7735_DrawSprite(emp_pot.x, emp_pot.y, backgroundedit, emptypot, 0xFFFF);
          }else if((ndl.h_carry == 1) && (emp_pot.on_stove == 1) && (emp_pot.bf_filled == 0) && (emp_pot.noodle_filled == 0) && (p0.x == 20) && (55 < p0.y) && (p0.y < 77)){ //drop noodle in pot
              p0.carry = 0;
              ndl.h_carry = 0;
              emp_pot.noodle_filled = 1;
              ST7735_EraseSprite(ndl.x, ndl.y, backgroundedit);
              ST7735_EraseSprite(0, 65, backgroundedit);
              ST7735_DrawSprite(0, 65, backgroundedit, spr_progpot, 0xFFFF);
              en_greenline = 1;
              time_check = 0;
          }else if((noodle_pot.h_carry == 1) && (p0.x == 95) && (107 < p0.y) && (p0.y < 121) && (recipe != 3)){ //drop noodle in bowl
              //p0.carry = 0;
              noodle_pot.h_carry = 0;
              emp_pot.h_carry = 1;
              emp_pot.noodle_filled = 0;
              ndl.in_bowl = 1;
              bns.erase = 0, rec1.erase = 0, rec2.erase = 0, rec3.erase = 0;
              emp_pot.x = p0.x;
              emp_pot.y = p0.y - 20;
              ST7735_DrawSprite(emp_pot.x, emp_pot.y, backgroundedit, emptypot, 0xFFFF);
              ST7735_DrawSmallCircle(120, 78, 0x8E9F);
          }else if((bns.h_carry == 1) && (20 < p0.x) && (p0.x < 50) && (p0.y == 150) && (recipe == 0)){
              bns.delivered++;
              bns.h_carry = 0;
              ST7735_EraseSprite(p0.x, p0.y - 20, backgroundedit);
              p0.carry = 0, tomcut.in_bowl = 0, bokcut.in_bowl = 0, bf.in_bowl = 0, ndl.in_bowl = 0, emp_pot.bf_filled = 0, emp_pot.noodle_filled = 0;
              ST7735_DrawSprite(110, 110, backgroundedit, emptybowl, 0xFFFF);
              recipe = Random(4);
              ST7735_FillRect(25, 0, 16, 17, 0xFFFF);
          }else if((rec1.h_carry == 1) && (20 < p0.x) && (p0.x < 50) && (p0.y == 150) && (recipe == 1)){
              rec1.delivered++;
              rec1.h_carry = 0;
              ST7735_EraseSprite(p0.x, p0.y - 20, backgroundedit);
              p0.carry = 0, tomcut.in_bowl = 0, bokcut.in_bowl = 0, bf.in_bowl = 0, ndl.in_bowl = 0, emp_pot.bf_filled = 0, emp_pot.noodle_filled = 0;
              ST7735_DrawSprite(110, 110, backgroundedit, emptybowl, 0xFFFF);
              recipe = Random(4);
              ST7735_FillRect(25, 0, 16, 17, 0xFFFF);
          }else if((rec2.h_carry == 1) && (20 < p0.x) && (p0.x < 50) && (p0.y == 150) && (recipe == 2)){
              rec2.delivered++;
              rec2.h_carry = 0;
              ST7735_EraseSprite(p0.x, p0.y - 20, backgroundedit);
              p0.carry = 0, tomcut.in_bowl = 0, bokcut.in_bowl = 0, bf.in_bowl = 0, ndl.in_bowl = 0, emp_pot.bf_filled = 0, emp_pot.noodle_filled = 0;
              ST7735_DrawSprite(110, 110, backgroundedit, emptybowl, 0xFFFF);
              recipe = Random(4);
              ST7735_FillRect(25, 0, 16, 17, 0xFFFF);
          }else if((rec3.h_carry == 1) && (20 < p0.x) && (p0.x < 50) && (p0.y == 150) && (recipe == 3)){
              rec3.delivered++;
              rec3.h_carry = 0;
              ST7735_EraseSprite(p0.x, p0.y - 20, backgroundedit);
              p0.carry = 0, tomcut.in_bowl = 0, bokcut.in_bowl = 0, bf.in_bowl = 0, ndl.in_bowl = 0, emp_pot.bf_filled = 0, emp_pot.noodle_filled = 0;
              ST7735_DrawSprite(110, 110, backgroundedit, emptybowl, 0xFFFF);
              recipe = Random(4);
              ST7735_FillRect(25, 0, 16, 17, 0xFFFF);
          }else if((p0.carry == 1) && (140 < p0.y) && (p0.y < 151) && (p0.x == 95)){ //drop into trash
              ST7735_EraseSprite(p0.x, p0.y-20, backgroundedit);
              if(tom.h_carry == 1){
                  p0.carry = 0;
                  tom.h_carry = 0;
              }else if(bok.h_carry == 1){
                  p0.carry = 0;
                  bok.h_carry = 0;
              }else if(bf.h_carry == 1){
                  p0.carry = 0;
                  bf.h_carry = 0;
              }else if(dgh.h_carry == 1){
                  p0.carry = 0;
                  dgh.h_carry = 0;
              }else if(tomcut.h_carry == 1){
                  p0.carry = 0;
                  tomcut.h_carry = 0;
              }else if(bokcut.h_carry == 1){
                  p0.carry = 0;
                  bokcut.h_carry = 0;
              }else if(ndl.h_carry == 1){
                  p0.carry = 0;
                  ndl.h_carry = 0;
              }else if(d_pot.h_carry == 1){
                  d_pot.h_carry = 0;
                  emp_pot.h_carry = 1;
                  emp_pot.x = p0.x;
                  emp_pot.y = p0.y - 20;
                  ST7735_DrawSprite(emp_pot.x, emp_pot.y, backgroundedit, emptypot, 0xFFFF);
              }else if(noodle_pot.v_carry == 1){
                  noodle_pot.h_carry = 0;
                  emp_pot.h_carry = 1;
                  emp_pot.x = p0.x;
                  emp_pot.y = p0.y - 20;
                  ST7735_DrawSprite(emp_pot.x, emp_pot.y, backgroundedit, emptypot, 0xFFFF);
              }
          }//else if(another ingredient

      }

//=======CHOPPING=======

      //check valvano chop condition
      if((P1CHOPSW != 0) && (p1.carry == 0)){ //if press chop
          if((59 < p1.x) && (p1.x < 81) && (p1.y == 150)){
              if((59 < p1.x) && (p1.x < 81)){ //cutting board 1
                  ST7735_DrawSprite(70, 159, backgroundedit, knife, 0xFFFF);
                  while(sw_val_oU != 0){ //while pressing chop
                      Sound_Chop();
                      for(int i=0; i<30; i++){
                          ST7735_DrawSprite(70, 159, backgroundedit, knife, 0xFFFF);
                          ST7735_DrawSprite(70, 153, backgroundedit, knife, 0xFFFF);
                          if(sw_val_oU == 0){
                              break;
                          }
                      }
                  }
                  if(tom.choppingboard1 == 1){
                      tom.chopped = 1;
                      tom.choppingboard1 = 0;
                      ST7735_EraseSprite(70, 150, backgroundedit);
                      ST7735_DrawSprite(p1.x, p1.y, backgroundedit, valbano, 0xFFFF);
                      ST7735_DrawSprite(70, 159, backgroundedit, tomatocut, 0xFFFF);
                  }else if(bok.choppingboard1 == 1){
                      bok.chopped = 1;
                      bok.choppingboard1 = 0;
                      ST7735_EraseSprite(70, 150, backgroundedit);
                      ST7735_DrawSprite(p1.x, p1.y, backgroundedit, valbano, 0xFFFF);
                      ST7735_DrawSprite(70, 159, backgroundedit, bokchoycut, 0xFFFF);
                  }else if(dgh.choppingboard1 == 1){
                      dgh.noodled = 1;
                      dgh.choppingboard1 = 0;
                      ST7735_EraseSprite(70, 150, backgroundedit);
                      ST7735_DrawSprite(p1.x, p1.y, backgroundedit, valbano, 0xFFFF);
                      ST7735_DrawSprite(70, 159, backgroundedit, noodle, 0xFFFF);
                  }
              }
          }
      }


      //check holt chop condition
      if((P0CHOPSW != 0) && (p0.carry == 0)){ //if press chop
          if((59 < p0.x) && (p0.x < 81) && (p0.y == 150)){
              if((59 < p0.x) && (p0.x < 81)){ //cutting board 1
                  ST7735_DrawSprite(70, 159, backgroundedit, knife, 0xFFFF);
                  while(sw_val_zU != 0){ //while pressing chop
                      Sound_Chop();
                      for(int i=0; i<30; i++){
                          ST7735_DrawSprite(70, 159, backgroundedit, knife, 0xFFFF);
                          ST7735_DrawSprite(70, 153, backgroundedit, knife, 0xFFFF);
                          if(sw_val_zU == 0){
                              break;
                          }
                      }
                  }
                  if(tom.choppingboard1 == 1){
                      tom.chopped = 1;
                      tom.choppingboard1 = 0;
                      ST7735_EraseSprite(70, 150, backgroundedit);
                      ST7735_DrawSprite(p0.x, p0.y, backgroundedit, holt, 0xFFFF);
                      ST7735_DrawSprite(70, 159, backgroundedit, tomatocut, 0xFFFF);
                  }else if(bok.choppingboard1 == 1){
                      bok.chopped = 1;
                      bok.choppingboard1 = 0;
                      ST7735_EraseSprite(70, 150, backgroundedit);
                      ST7735_DrawSprite(p0.x, p0.y, backgroundedit, holt, 0xFFFF);
                      ST7735_DrawSprite(70, 159, backgroundedit, bokchoycut, 0xFFFF);
                  }else if(dgh.choppingboard1 == 1){
                      dgh.noodled = 1;
                      dgh.choppingboard1 = 0;
                      ST7735_EraseSprite(70, 150, backgroundedit);
                      ST7735_DrawSprite(p0.x, p0.y, backgroundedit, holt, 0xFFFF);
                      ST7735_DrawSprite(70, 159, backgroundedit, noodle, 0xFFFF);
                  }
              }
          }
      }


//things that need to keep drawing while others are moving
      //timing still weird -- moving spritrs glitchy while cokkin
      if(en_greenline == 1){
          time_check++;
          if((time_check % 50) == 0){
              ST7735_DrawPixel(greenline, 45, 0x8656);
              greenline++;
              if(greenline == 15){
                  ST7735_EraseSprite(0, 64, backgroundedit); //erase line
                  en_greenline = 0;
                  greenline = 2;
                  if(emp_pot.bf_filled == 1){
                      bf.cooked = 1;
                      d_pot.on_stove = 1;
                      ST7735_EraseSprite(0, 74, backgroundedit); //erase old pot
                      ST7735_DrawSprite(0, 45, backgroundedit, greencheck, 0xFFFF); //greencheck
                      ST7735_DrawSprite(0, 64, backgroundedit, spr_donepot, 0xFFFF);//replace with beef done pot
                      Sound_Done();
                  }else if(emp_pot.noodle_filled == 1){
                      ndl.cooked = 1;
                      d_pot.on_stove = 1;
                      ST7735_EraseSprite(0, 74, backgroundedit);
                      ST7735_DrawSprite(0, 45, backgroundedit, greencheck, 0xFFFF);
                      ST7735_DrawSprite(0, 64, backgroundedit, spr_progpot, 0xFFFF);
                      Sound_Done();
                  }
              }
          }
      }

//WIN CONDITIONS =========
      if((tomcut.in_bowl == 1) && (bokcut.in_bowl == 1) && (bf.in_bowl == 1) && (ndl.in_bowl == 1) && (bns.completed_bowl == 0) && (bns.erase == 0) && (recipe == 0)){
          ST7735_EraseSprite(114, 92, backgroundedit);
          ST7735_DrawSprite(110, 110, backgroundedit, beefnoodlesoup, 0xFFFF);
          bns.completed_bowl = 1;
      }else if((bokcut.in_bowl == 1) && (bf.in_bowl == 1) && (ndl.in_bowl == 1) && (recipe == 1) && (rec1.completed_bowl == 0) && (rec1.erase == 0)){
          ST7735_EraseSprite(114, 92, backgroundedit);
          ST7735_DrawSprite(110, 110, backgroundedit, spr_rec1, 0xFFFF);
          rec1.completed_bowl = 1;
      }else if((tomcut.in_bowl == 1) && (bf.in_bowl == 1) && (ndl.in_bowl == 1) && (recipe == 2) && (rec2.completed_bowl == 0) && (rec2.erase == 0)){
          ST7735_EraseSprite(114, 92, backgroundedit);
          ST7735_DrawSprite(110, 110, backgroundedit, spr_rec2, 0xFFFF);
          rec2.completed_bowl = 1;
      }else if((bf.in_bowl == 1) && (bokcut.in_bowl == 1) && (recipe == 3) && (rec3.completed_bowl == 0) && (rec3.erase == 0)){
          ST7735_EraseSprite(114, 92, backgroundedit);
          ST7735_DrawSprite(110, 110, backgroundedit, spr_rec3, 0xFFFF);
          rec3.completed_bowl = 1;
      }

  }
}
