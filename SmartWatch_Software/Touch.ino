/*****************************************************************************
The MIT License (MIT)

Copyright (c) 2020 Matthew James Bellafaire

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

//attach IRQ interrupt
void initTouch()
{

  //dummy read the touch controller, bits 2 and 3 determine the power down state, 00 enables touch IRQ and puts the device into power down
  struct point p;
  p.xPos = map(readRegister(TOUCH_ADDR, 0b11010010) >> 8, X_MIN, X_MAX, 0, SCREEN_WIDTH);
  p.yPos = map(readRegister(TOUCH_ADDR, 0b11000010) >> 8, Y_MIN, Y_MAX, SCREEN_HEIGHT, 0);

  attachInterrupt(TOUCH_IRQ, TOUCH_ISR, FALLING);
}

int rapidTouchCount = 0;

void IRAM_ATTR TOUCH_ISR()
{
  printDebug("Touch detected from interrupt");

  if (millis() - lastTouchTime < 200) {
    rapidTouchCount++;
    printDebug("rapidTouchCount: " + String(rapidTouchCount));

    if (rapidTouchCount > 50) {
#ifdef DEBUG
      Serial.println("**** Rapid Touch shutdown registered ****");
      Serial.flush();
#endif
      deviceSleep();
    }
  } else {
    rapidTouchCount = 0;
  }

  //something's happening and if the user touches the screen we should just
  //reset the device because they want to use the smartwatch
  //  if (nonCriticalOperation) {
  //      if(connected){
  //        pClient->disconnect();
  //  }
  //    esp_sleep_enable_timer_wakeup(10);
  //    esp_deep_sleep_start();
  //  }
  // readTouch();
  lastTouchTime = millis();
  touchDetected = true;




  // handleTouch();
}

//reads touch from the touch controller and enables the Touch interrupt pin
struct point readTouch()
{
  struct point p;
  if (!digitalRead(TOUCH_IRQ))
  {
    //since we already have a readRegister function for the 16 bit battery monitor we just reuse it here
    //and shift the result to the right. the values read from the touch controller are then
    //mapped according to the values defined in declarations.h
    p.xPos = map(readRegister(TOUCH_ADDR, 0b11010010) >> 8, X_MIN, X_MAX, 0, SCREEN_WIDTH);
    p.yPos = map(readRegister(TOUCH_ADDR, 0b11000010) >> 8, Y_MIN, Y_MAX, SCREEN_HEIGHT, 0);
#ifdef DEBUG
    printPoint(p);
    tft.drawCircle(p.xPos, p.yPos, 2, TEXT_COLOR);
#endif
  }
  else
  {
    p.xPos = -1;
    p.yPos = -1;
  }
  return p;
}

void printPoint(struct point p)
{
  Serial.print("Point (x=");
  Serial.print(p.xPos);
  Serial.print(", y=");
  Serial.print(p.yPos);
  Serial.println(")");
}

//All touch handling for individual pages are switched here
void handleTouch()
{
  if (touchDetected)
  {
    switch (currentPage)
    {
      case HOME:
        HomeTouchHandler(readTouch());
        break;
      case SETTINGS:
        SettingsTouchHandler(readTouch());
        break;
      case NOTIFICATIONS:
        NotificationsTouchHandler(readTouch());
        break;
      case CALCULATOR:
        calculatorTouchHandler(readTouch());
        break;
      default:
        break;
    }
    touchDetected = false;
  }
}
