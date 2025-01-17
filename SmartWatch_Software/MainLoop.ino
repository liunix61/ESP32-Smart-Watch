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
/*  everything that happens in the normal operation of the smartwatch happens here. this is kind of the user-interface part of the code.
*/
bool newNotificationData = false;
bool dontSleep = false;

void MainLoop()
{
  initBLE();
  while (lastTouchTime + screenOnTime > millis() || dontSleep || (readZAccel() > 2200 && wokenByAccelerometer)) {
    if (!newNotificationData && pRemoteCharacteristic) {
      updateNotificationData();
      newNotificationData = true;
    }
    if (touchDetected) {
      //handleTouch() operates in a similar manner to the MainLoop()
      //and simply switches to the correct touch handler
      handleTouch();
    } else {
      switch (currentPage)
      {
        case TEST_SCREEN:
          dontSleep = false;
          testScreen();
          break;
        case HOME:
          dontSleep = false;
          drawHome();
          break;
        case SETTINGS:
          dontSleep = false;
          drawSettings();
          break;
        case NOTIFICATIONS:
          dontSleep = false;
          drawNotifications();
          break;
        case CALCULATOR:
          dontSleep = true;
          drawCalculator();
          break;
        default:
          currentPage = 0;
          break;
      }
    }
  }
  digitalWrite(LCD_LED, LOW);
}

void testScreen()
{
  tft.setTextColor(TEXT_COLOR);
  tft.setCursor(0, 0);
  tft.println("ESP32 Smartwatch");

  tft.print("Battery Percentage: ");
  tft.print(getBatteryPercentage());
  tft.println("%");

  tft.print("Battery Voltage: ");
  tft.print(getBatteryVoltage());
  tft.println("V");

  pinMode(CHG_STAT, INPUT);
  tft.print("Charging Status: ");
  tft.println(digitalRead(CHG_STAT));

  tft.println("Starting device....");

}
