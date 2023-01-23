/*************************************************************************
 * Modified by mzashh, to support 1/8 scan panels.
 * Description: 
 * 
 * The underlying implementation of the ESP32-HUB75-MatrixPanel-I2S-DMA only
 * supports output to 1/16 or 1/32 scan panels - which means outputting 
 * two lines at the same time, 16 or 32 rows apart. This cannot be changed
 * at the DMA layer as it would require a messy and complex rebuild of the 
 * library's DMA internals.
 *
 * However, it is possible to connect 1/8 scan panels to this same library and
 * 'trick' the output to work correctly on these panels by way of adjusting the
 * pixel co-ordinates that are 'sent' to the ESP32-HUB75-MatrixPanel-I2S-DMA
 * library.
 * 
 **************************************************************************/
#define FILESYSTEM SPIFFS
#include <SPIFFS.h>
#include <AnimatedGIF.h>
#include "ESP32-HUB75-MatrixPanel-I2S-DMA.h"
#include "ESP32-VirtualMatrixPanel-I2S-DMA.h"

/* Use the Virtual Display class to re-map co-ordinates such that they draw 
 * correctly on a 32x16 1/8 Scan panel (or chain of such panels).
*/
 


  // Panel configuration
  #define PANEL_RES_X 32 // Number of pixels wide of each INDIVIDUAL panel module. 
  #define PANEL_RES_Y 32 // Number of pixels tall of each INDIVIDUAL panel module.
  
  
  #define NUM_ROWS 1 // Number of rows of chained INDIVIDUAL PANELS
  #define NUM_COLS 1 // Number of INDIVIDUAL PANELS per ROW
  
  // ^^^ NOTE: DEFAULT EXAMPLE SETUP IS FOR A SINGLE 1/8 SCAN PANEL , NEED TO REMAP CORDINATES FOR CHAINS
    
  // Change this to your needs, for details on VirtualPanel pls read the PDF!
  #define SERPENT true
  #define TOPDOWN false

  // placeholder for the matrix object
  MatrixPanel_I2S_DMA *dma_display = nullptr;

  // placeholder for the virtual display object
  VirtualMatrixPanel  *OneEightMatrixDisplay = nullptr;
  
 uint16_t myBLACK = dma_display->color565(0, 0, 0);
 uint16_t myWHITE = dma_display->color565(255, 255, 255);
 uint16_t myRED = dma_display->color565(255, 0, 0);
 uint16_t myGREEN = dma_display->color565(0, 255, 0);
 uint16_t myBLUE = dma_display->color565(0, 0, 255);


 AnimatedGIF gif;
 File f;
 int x_offset, y_offset;

  // Draw a line of image directly on the LED Matrix
 void GIFDraw(GIFDRAW *pDraw)
 {
    uint8_t *s;
    uint16_t *d, *usPalette, usTemp[320];
    int x, y, iWidth;

  iWidth = pDraw->iWidth;
  if (iWidth > MATRIX_WIDTH)
      iWidth = MATRIX_WIDTH;

    usPalette = pDraw->pPalette;
    y = pDraw->iY + pDraw->y; // current line
    
    s = pDraw->pPixels;
    if (pDraw->ucDisposalMethod == 2) // restore to background color
    {
      for (x=0; x<iWidth; x++)
      {
        if (s[x] == pDraw->ucTransparent)
           s[x] = pDraw->ucBackground;
      }
      pDraw->ucHasTransparency = 0;
    }
    // Apply the new pixels to the main image
    if (pDraw->ucHasTransparency) // if transparency used
    {
      uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
      int x, iCount;
      pEnd = s + pDraw->iWidth;
      x = 0;
      iCount = 0; // count non-transparent pixels
      while(x < pDraw->iWidth)
      {
        c = ucTransparent-1;
        d = usTemp;
        while (c != ucTransparent && s < pEnd)
        {
          c = *s++;
          if (c == ucTransparent) // done, stop
          {
            s--; // back up to treat it like transparent
          }
          else // opaque
          {
             *d++ = usPalette[c];
             iCount++;
          }
        } // while looking for opaque pixels
        if (iCount) // any opaque pixels?
        {
          for(int xOffset = 0; xOffset < iCount; xOffset++ ){
             OneEightMatrixDisplay->drawPixel(x + xOffset, y, usTemp[xOffset]); // 565 Color Format
          }
          x += iCount;
          iCount = 0;
        }
        // no, look for a run of transparent pixels
        c = ucTransparent;
        while (c == ucTransparent && s < pEnd)
        {
          c = *s++;
          if (c == ucTransparent)
             iCount++;
          else
             s--; 
        }
        if (iCount)
        {
          x += iCount; // skip these
          iCount = 0;
        }
      }
    }
    else // does not have transparency
    {
      s = pDraw->pPixels;
      // Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
      for (x=0; x<pDraw->iWidth; x++)
      {
         OneEightMatrixDisplay->drawPixel(x, y, usPalette[*s++]); // color 565
      }
    }
} /* GIFDraw() */


void * GIFOpenFile(const char *fname, int32_t *pSize)
{

  f = FILESYSTEM.open(fname);
  if (f)
  {
    *pSize = f.size();
    return (void *)&f;
  }
  return NULL;
} /* GIFOpenFile() */

void GIFCloseFile(void *pHandle)
{
  File *f = static_cast<File *>(pHandle);
  if (f != NULL)
     f->close();
} /* GIFCloseFile() */

int32_t GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen)
{
    int32_t iBytesRead;
    iBytesRead = iLen;
    File *f = static_cast<File *>(pFile->fHandle);
    // Note: If you read a file all the way to the last byte, seek() stops working
    if ((pFile->iSize - pFile->iPos) < iLen)
       iBytesRead = pFile->iSize - pFile->iPos - 1; // <-- ugly work-around
    if (iBytesRead <= 0)
       return 0;
    iBytesRead = (int32_t)f->read(pBuf, iBytesRead);
    pFile->iPos = f->position();
    return iBytesRead;
} /* GIFReadFile() */

int32_t GIFSeekFile(GIFFILE *pFile, int32_t iPosition)
{ 
  int i = micros();
  File *f = static_cast<File *>(pFile->fHandle);
  f->seek(iPosition);
  pFile->iPos = (int32_t)f->position();
  i = micros() - i;
//  Serial.printf("Seek time = %d us\n", i);
  return pFile->iPos;
} /* GIFSeekFile() */

unsigned long start_tick = 0;

void ShowGIF(char *name)
{
  start_tick = millis();
   
  if (gif.open(name, GIFOpenFile, GIFCloseFile, GIFReadFile, GIFSeekFile, GIFDraw))
  {
    x_offset = (MATRIX_WIDTH - gif.getCanvasWidth())/2;
    if (x_offset < 0) x_offset = 0;
    y_offset = (MATRIX_HEIGHT - gif.getCanvasHeight())/2;
    if (y_offset < 0) y_offset = 0;
    while (gif.playFrame(true, NULL))
    {      
      if ( (millis() - start_tick) > 8000) { // we'll get bored after about 8 seconds of the same looping gif
        break;
      }
    }
    gif.close();
  }

} /* ShowGIF()
  /******************************************************************************
   * Setup!
   ******************************************************************************/
  void setup()
  {
    delay(250);
/* 
     // 62x32 1/8 Scan Panels don't have a D and E pin!
     
     HUB75_I2S_CFG::i2s_pins _pins = {
      R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, 
      A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, 
      LAT_PIN, OE_PIN, CLK_PIN
     };
*/
    HUB75_I2S_CFG mxconfig(
                PANEL_RES_X*2,              // DO NOT CHANGE THIS
                PANEL_RES_Y/2,              // DO NOT CHANGE THIS
                NUM_ROWS*NUM_COLS           // DO NOT CHANGE THIS
                //,_pins            // Uncomment to enable custom pins
    );
    
    mxconfig.clkphase = false; // Change this if you see pixels showing up shifted wrongly by one column the left or right.
    
    //mxconfig.driver   = HUB75_I2S_CFG::FM6126A;     // in case that we use panels based on FM6126A chip, we can set it here before creating MatrixPanel_I2S_DMA object
  
    // OK, now we can create our matrix object
    dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  
    // let's adjust default brightness to about 75%
    dma_display->setBrightness8(50);    // range is 0-255, 0 - 0%, 255 - 100%
    dma_display->fillScreen(myWHITE);
    // Allocate memory and start DMA display
    if( not dma_display->begin() )
      Serial.println("****** !KABOOM! I2S memory allocation failed ***********");

   
    dma_display->clearScreen();
    delay(500);
    
    // create OneEightMatrixDisplaylay object based on our newly created dma_display object
    OneEightMatrixDisplay = new VirtualMatrixPanel((*dma_display), NUM_ROWS, NUM_COLS, PANEL_RES_X, PANEL_RES_Y, SERPENT, TOPDOWN);
    
	// THE IMPORTANT BIT BELOW!
    OneEightMatrixDisplay->setPhysicalPanelScanRate(ONE_EIGHT_32);

      Serial.println(" * Loading SPIFFS");
  if(!SPIFFS.begin()){
        Serial.println("SPIFFS Mount Failed");
  }
  
  gif.begin(LITTLE_ENDIAN_PIXELS);


  }
 String gifDir = "/gifs"; // play all GIFs in this directory on the SD card
 char filePath[256] = { 0 };
 File root, gifFile;
  

  
  void loop() {

      // What the panel sees from the DMA engine!
        while (1) // run forever
   {
      
      root = FILESYSTEM.open(gifDir);
      if (root)
      {
           gifFile = root.openNextFile();
            while (gifFile)
            {
              if (!gifFile.isDirectory()) // play it
              {
                
                // C-strings... urghh...                
                memset(filePath, 0x0, sizeof(filePath));                
                strcpy(filePath, gifFile.path());
                
                // Show it.
                ShowGIF(filePath);
               
              }
              //dma_display->setBrightness8(10);    // comment out 297-299 to play the first GIF in memory on loop
              //gifFile.close();
             //gifFile = root.openNextFile();
            }
         root.close();
      } // root
      
      delay(1000); // pause before restarting  
   }
  
  } // end loop

