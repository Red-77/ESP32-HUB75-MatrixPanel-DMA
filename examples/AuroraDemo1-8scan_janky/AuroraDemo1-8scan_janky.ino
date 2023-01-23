// Modified by mzashh, to support 1/8 scan panels.
// In a janky state some effets are buggy (render for a 64x64 display basically cropped out), need to fix.

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "ESP32-VirtualMatrixPanel-I2S-DMA.h"
/*--------------------- MATRIX GPIO CONFIG  -------------------------*/
#define R1_PIN 25
#define G1_PIN 26
#define B1_PIN 27
#define R2_PIN 14
#define G2_PIN 12
#define B2_PIN 13
#define A_PIN 23
#define B_PIN 19 // Changed from library default
#define C_PIN 5
#define D_PIN 17
#define E_PIN -1
#define LAT_PIN 4
#define OE_PIN 15
#define CLK_PIN 16


/*--------------------- MATRIX PANEL CONFIG -------------------------*/
#define PANEL_RES_X 32      // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 32     // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1      // Total number of panels chained one to another
  #define NUM_ROWS 1 // Number of rows of chained INDIVIDUAL PANELS
  #define NUM_COLS 1 
    #define SERPENT true
  #define TOPDOWN false
/*
//Another way of creating config structure
//Custom pin mapping for all pins
HUB75_I2S_CFG::i2s_pins _pins={R1, G1, BL1, R2, G2, BL2, CH_A, CH_B, CH_C, CH_D, CH_E, LAT, OE, CLK};
HUB75_I2S_CFG mxconfig(
						64,   // width
						64,   // height
						 4,   // chain length
					 _pins,   // pin mapping
  HUB75_I2S_CFG::FM6126A      // driver chip
);

*/
MatrixPanel_I2S_DMA *dma_display = nullptr;
  VirtualMatrixPanel  *OneEightMatrixDisplay = nullptr;
  
// Module configuration
    HUB75_I2S_CFG mxconfig(
                PANEL_RES_X*2,              // DO NOT CHANGE THIS
                PANEL_RES_Y/2,              // DO NOT CHANGE THIS
                NUM_ROWS*NUM_COLS           // DO NOT CHANGE THIS
                //,_pins            // Uncomment to enable custom pins
    );
    



//mxconfig.gpio.e = -1; // Assign a pin if you have a 64x64 panel
//mxconfig.clkphase = false; // Change this if you have issues with ghosting.
//mxconfig.driver = HUB75_I2S_CFG::FM6126A; // Change this according to your pane.



#include <FastLED.h>

#include "Effects.h"
Effects effects;

#include "Drawable.h"
#include "Playlist.h"
//#include "Geometry.h"

#include "Patterns.h"
Patterns patterns;

/* -------------------------- Some variables -------------------------- */
unsigned long fps = 0, fps_timer; // fps (this is NOT a matrix refresh rate!)
unsigned int default_fps = 30, pattern_fps = 30;  // default fps limit (this is not a matrix refresh counter!)
unsigned long ms_animation_max_duration = 20000;  // 20 seconds
unsigned long last_frame=0, ms_previous=0;

void setup()
{
 /************** SERIAL **************/
  Serial.begin(115200);
  delay(250);
  
 /************** DISPLAY **************/
  Serial.println("...Starting Display");
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(90); //0-255
    OneEightMatrixDisplay = new VirtualMatrixPanel((*dma_display), NUM_ROWS, NUM_COLS, PANEL_RES_X, PANEL_RES_Y, SERPENT, TOPDOWN);
    
  // THE IMPORTANT BIT BELOW!
    OneEightMatrixDisplay->setPhysicalPanelScanRate(ONE_EIGHT_32);
  OneEightMatrixDisplay->fillScreenRGB888(255,255,255);
  delay(5000);
  OneEightMatrixDisplay->fillScreenRGB888(0,0,128);
  delay(1000);
  OneEightMatrixDisplay->clearScreen();  
  delay(1000);  
  Serial.println("**************** Starting Aurora Effects Demo ****************");


   // setup the effects generator
  effects.Setup();

  delay(500);
  Serial.println("Effects being loaded: ");
  listPatterns();
 

  patterns.moveRandom(1); // start from a random pattern

  Serial.print("Starting with pattern: ");
  Serial.println(patterns.getCurrentPatternName());
  patterns.start();
  ms_previous = millis();
  fps_timer = millis();
}

void loop()
{
    // menu.run(mainMenuItems, mainMenuItemCount);  

  if ( (millis() - ms_previous) > ms_animation_max_duration ) 
  {
       patterns.stop();      
       patterns.moveRandom(1);
       //patterns.move(1);
       patterns.start();  
       
       Serial.print("Changing pattern to:  ");
       Serial.println(patterns.getCurrentPatternName());
        
       ms_previous = millis();

       // Select a random palette as well
       effects.RandomPalette();
    }
 
    if ( 1000 / pattern_fps + last_frame < millis()){
      last_frame = millis();
      pattern_fps = patterns.drawFrame();
      if (!pattern_fps)
        pattern_fps = default_fps;

      ++fps;
    }

    if (fps_timer + 1000 < millis()){
       Serial.printf_P(PSTR("Effect fps: %ld\n"), fps);
       fps_timer = millis();
       fps = 0;
    }
       
}


void listPatterns() {
  patterns.listPatterns();
}

