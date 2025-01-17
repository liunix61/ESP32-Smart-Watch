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
boolean correctTime = true;


//header file for the time.h library https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/esp32-hal-timer.h (closest thing to documentation I've found so far)


//found here https://www.esp32.com/viewtopic.php?t=5398 (with some changes for timezone)
void mjd_set_timezone_est()
{
  ESP_LOGD(TAG, "%s()", __FUNCTION__);

  // @doc https://remotemonitoringsystems.ca/time-zone-abbreviations.php
  // @doc timezone UTC = UTC
  setenv("TZ", "EST5EDT", 1);
  tzset();
}

//referenced time update from https://www.esp32.com/viewtopic.php?t=6043
void updateTimeFromNotificationData() {
  //check that the notification data we have is complete
  String lastNotificationLine = parseFromNotifications(getNotificationLines(), 0);
  if (lastNotificationLine[0] == '*' &&
      lastNotificationLine[1] == '*' &&
      lastNotificationLine[2] == '*') {

    //get the last line of the notification string, that will contain the current time,
    //note that this function should only be called directly after receiving a notification update
    //otherwise the time will be inaccurate
    String timeString = parseFromNotifications(getNotificationLines() - 1, 0);
    printDebug("Parsed Time String - " + timeString);

    //format "HH:mm:ss dd-MM-yyyy"
    //simply parse out the data from the string, some adjustments need to be made for year and month.
    struct tm tim;

    //time of year
    tim.tm_year = (String(timeString[15]) + String(timeString[16]) + String(timeString[17]) + String(timeString[18])).toInt() - 1900;
    tim.tm_mon = (String(timeString[12]) + String(timeString[13])).toInt() - 1; //month is zero indexed...
    tim.tm_mday = (String(timeString[9]) + String(timeString[10])).toInt();

    //time of day
    tim.tm_hour = (String(timeString[0]) + String(timeString[1])).toInt() + readDataField(DAYLIGHT_SAVINGS);
    tim.tm_min = (String(timeString[3]) + String(timeString[4])).toInt();
    tim.tm_sec = (String(timeString[6]) + String(timeString[7])).toInt();

    //make sure timezone is set to EST
    mjd_set_timezone_est();

    //convert the time struct into time_t variable (essentially an integer representation of time)
    time_t t = mktime(&tim);

    printDebug("Setting time: " + String(asctime(&tim)));


    //place the time_t variable into a timeval and update the RTC clock so that the time can be continued in deep sleep
    struct timeval tv = { .tv_sec = t };
    settimeofday(&tv, NULL);

  } else {
    printDebug("Notification data not complete, cannot parse time");

  }
}


struct

String getInternetTime()
{
  //connect to WiFi
  int wifiCounter = 0;

  mjd_set_timezone_est();

  //i need these values to be strings so some extra code is required
  WiFi.mode(WIFI_STA);
  char ssid0[60];
  char password0[60];
  ssid.toCharArray(ssid0, ssid.length() + 1);
  password.toCharArray(password0, password.length() + 1);

  printDebug("Connecting to " + ssid);


  WiFi.begin(ssid0, password0);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    printDebug(".");

    if (++wifiCounter > 30)
    {
      printDebug("COULD NOT CONNECT TO WIFI ");

      break;
    }
  }
#ifdef DEBUG
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println(" CONNECTED");
  }
#endif
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  delay(1000);
  printLocalTime();
  unsigned long tm0 = micros();
  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  //  bootCount++;
  //  beginTimedSleep (tm0);
  return "00:00:00AM";
}

void printLocalTime()
{
  time(&now);
  timeinfo = localtime(&now);
#ifdef DEBUG
  Serial.printf("%s\n", asctime(timeinfo));
  delay(2); // 26 bytes @ 115200 baud is less than 2 ms
#endif
}

void drawDate(int x, int y, int textSize)
{
  //configure current timezone (this information gets lost in deep sleep)
  mjd_set_timezone_est();
  time(&now);
  timeinfo = localtime(&now);

  String weekday;

  switch (timeinfo->tm_wday)
  {
    case 0:
      weekday = "Sunday";
      break;
    case 1:
      weekday = "Monday";
      break;
    case 2:
      weekday = "Tuesday";
      break;
    case 3:
      weekday = "Wednesday";
      break;
    case 4:
      weekday = "Thursday";
      break;
    case 5:
      weekday = "Friday";
      break;
    case 6:
      weekday = "Saturday";
      break;
    default:
      weekday = "error";
      break;
  }

  String Date = weekday + ", " + String(timeinfo->tm_mon + 1) + "/" + String(timeinfo->tm_mday);

  frameBuffer->setTextSize(textSize);
  frameBuffer->setTextColor(TEXT_COLOR);
  for (int a = 0; a < Date.length(); a++)
  {
    frameBuffer->setCursor(x + a * 6 * textSize, y);
    frameBuffer->print(Date[a]);
  }
}

void drawDateCentered(int y, int textSize)
{
  //configure current timezone (this information gets lost in deep sleep)
  mjd_set_timezone_est();
  time(&now);
  timeinfo = localtime(&now);

  String weekday;

  switch (timeinfo->tm_wday)
  {
    case 0:
      weekday = "Sunday";
      break;
    case 1:
      weekday = "Monday";
      break;
    case 2:
      weekday = "Tuesday";
      break;
    case 3:
      weekday = "Wednesday";
      break;
    case 4:
      weekday = "Thursday";
      break;
    case 5:
      weekday = "Friday";
      break;
    case 6:
      weekday = "Saturday";
      break;
    default:
      weekday = "error";
      break;
  }

  String Date = weekday + ", " + String(timeinfo->tm_mon + 1) + "/" + String(timeinfo->tm_mday);

  int x = (160 - (Date.length() * 6 * textSize)) / 2;

  frameBuffer->setTextSize(textSize);
  frameBuffer->setTextColor(TEXT_COLOR);
  for (int a = 0; a < Date.length(); a++)
  {
    frameBuffer->setCursor(x + a * 6 * textSize, y);
    frameBuffer->print(Date[a]);
  }
}


void drawTime(int x, int y, int textSize)
{
  //configure current timezone (this information gets lost in deep sleep)
  mjd_set_timezone_est();
  time(&now);
  timeinfo = localtime(&now);

  String Hour = String(timeinfo->tm_hour, DEC);
  String Minute = String(timeinfo->tm_min, DEC);
  String Second = String(timeinfo->tm_sec, DEC);

  byte hour, minute, second = 0;
  hour = timeinfo->tm_hour;
  minute = (timeinfo->tm_min);
  second = timeinfo->tm_sec;

  char timestr[12] = "00:00:00 XM";
  if (timeinfo->tm_hour > 12)
  {
    timestr[0] = '0' + ((hour - 12) / 10);
    timestr[1] = '0' + ((hour - 12) % 10);
    timestr[9] = 'P';
  }
  else if (timeinfo->tm_hour == 12)
  {
    timestr[0] = '1';
    timestr[1] = '2';
    timestr[9] = 'P';
  }
  else if (timeinfo->tm_hour == 0)
  {
    timestr[0] = '1';
    timestr[1] = '2';
    timestr[9] = 'A';
  }
  else
  {
    timestr[0] = '0' + (timeinfo->tm_hour / 10);
    timestr[1] = '0' + (timeinfo->tm_hour % 10);
    timestr[9] = 'A';
  }

  timestr[3] = '0' + (timeinfo->tm_min / 10);
  timestr[4] = '0' + (timeinfo->tm_min % 10);

  timestr[6] = '0' + (timeinfo->tm_sec / 10);
  timestr[7] = '0' + (timeinfo->tm_sec % 10);

  /*  when writing the time we assume that we're writing over something, so for each character
       we fill in a black box behind it exactly the required size. we do this to try and prevent character "flashing"
       as much as possible.  */
  frameBuffer->setTextSize(textSize);
  if (correctTime)
  {
    frameBuffer->setTextColor(TEXT_COLOR);
  }
  else
  {
    frameBuffer->setTextColor(ERROR_COLOR);
  }
  for (int a = 0; a < 11; a++)
  {
    frameBuffer->setCursor(x + a * 6 * textSize, y);
    frameBuffer->print(timestr[a]);
  }
}
