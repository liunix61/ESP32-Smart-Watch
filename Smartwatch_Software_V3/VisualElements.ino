/****************************************************
 *                   Background
 ****************************************************/
class BoxesBackground : public Drawable
{
public:
    BoxesBackground(GFXcanvas16 *buffer_ptr)
        : Drawable(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, buffer_ptr, "Background")
    {
        row_height = _height / rows;
        for (int a = 0; a < columns; a++)
            for (int b = 0; b < rows; b++)
                widths[a][b] = _width / columns;
    }

    void draw()
    {

        // get random index in widths
        if (widthModification == 0)
        {
            // int index = random(0, rows * columns);
            // int add = random(-1, 2); // random is min to max-1
            // widths[index % columns][index / columns] += add;

            // pick a random index
            current_row = random(0, rows);
            current_column = random(0, columns);
            widthModification = random(0, 15); // random is min to max-1

            if (widths[current_column][current_row] > _width / columns)
                widthModification *= -1;
        }

        widths[current_column][current_row] += widthModification > 0 ? 1 : -1;
        if (current_column < columns - 1)
            widths[current_column + 1][current_row] += widthModification > 0 ? -1 : 1;
        else
            widths[current_column - 1][current_row] += widthModification > 0 ? -1 : 1;
        widthModification += (widthModification > 0) ? -1 : 1;

        // _buffer_ptr->drawRGBBitmap(0, 0, _img, _width, _height);
        for (int b = 0; b < rows; b++)
        {
            int current_pos = 0;
            for (int a = 0; a < columns; a++)
            {

                _buffer_ptr->fillRect(_x + current_pos, _y + b * row_height, widths[a][b], row_height, colors[(b * columns + a) % 6]);
                current_pos += widths[a][b];
                if (current_pos < _width)
                    _buffer_ptr->fillRect(_x + current_pos, _y + b * row_height, _width - current_pos, row_height, colors[(b * columns + a) % 6]);
                // Serial.printf("Drawing rectangle at x:%d y:%d with width:%d and height:%d\n", _x + current_pos, _y + b * row_height, widths[a][b], row_height);
            }
        }
    }

private:
    const int arr_size = (sizeof(colors) / sizeof((colors)[0]));
    const static int rows = 8;
    const static int columns = 4;
    int row_height;
    uint16_t colors[6] = {
        RGB_TO_BGR565(40, 10, 10),
        RGB_TO_BGR565(50, 40, 60),
        RGB_TO_BGR565(56, 40, 40),
        RGB_TO_BGR565(50, 50, 50),
        RGB_TO_BGR565(45, 30, 40),
        RGB_TO_BGR565(35, 35, 35)};
    int widths[columns][rows];

    int current_row = 0;
    int current_column = 0;
    int widthModification = 0;
};

class ImageBackground : public Drawable
{
public:
    ImageBackground(const uint16_t *img, GFXcanvas16 *buffer_ptr)
        : Drawable(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, buffer_ptr, "Background")
    {
        _img = img;
    }

    void draw()
    {
        _buffer_ptr->drawRGBBitmap(0, 0, _img, _width, _height);
    }

private:
    const uint16_t *_img;
};

/****************************************************
 *                    Scrollbox
 ****************************************************/
class Scrollbox : public Drawable
{
public:
    Scrollbox(int x, int y, int width, int height, GFXcanvas16 *buffer_ptr)
        : Drawable(x, y, width, height, buffer_ptr, "Scrollbox")
    {
        setTouchable(true);
    }

    void setString(String str)
    {
        _str = str;
    }

    void onTouch(int x, int y)
    {
        // printDebug("onTouch called in " + toString());
        _touched = (x >= _x) && (y >= _y) && (x <= _x + _width) && (y <= _y + _height) && _touchable;
        if (_touched)
        {
            if (y > (_y + _height / 2))
                _scroll += 2;
            else
                _scroll -= 2;
            // printDebug("Scroll: " + String(_scroll));
        }
    }

    void draw()
    {
        int characters_per_line = ((_width - 2 * _padding) / _character_spacing);
        int xpos = _x + _padding;
        int ypos = _y;

        int length = _str.length();
        int character_position = (characters_per_line) * (_scroll / _line_spacing);

        if (character_position < 0)
            character_position = 0;

        while (character_position < length && ypos < _y + _height)
        {

            _buffer_ptr->setCursor(xpos, ypos);
            _buffer_ptr->print(_str[character_position]);
            xpos += _character_spacing;
            if ((xpos >= (_x + _width) - _padding) || _str[character_position] == '\n')
            {
                xpos = _x + _padding;
                ypos += _line_spacing;
            }
            character_position++;
        }
    }

    void resetScroll()
    {
        _scroll = 0;
    }

private:
    String _str = "";
    static const int _padding = 6; // this could be a define but it's better here.
    static const int _character_spacing = 6;
    static const int _line_spacing = 8;
    int _scroll = 0;
};

/****************************************************
 *               connection status
 ****************************************************/
class ConnStatus : public Drawable
{
public:
    ConnStatus(int x, int y, GFXcanvas16 *buffer_ptr)
        : Drawable(x, y, 10, 4, buffer_ptr, "BatteryIcon")
    {
    }

    void draw()
    {

        if (connected)
            if (!notificationsUpdated || !timeUpdated)
                _buffer_ptr->fillRect(_x, _y, _width, _height, RGB_TO_BGR565(0, 0, 255));
            else
                _buffer_ptr->fillRect(_x, _y, _width, _height, RGB_TO_BGR565(0, 255, 0));
        else
            _buffer_ptr->fillRect(_x, _y, _width, _height, RGB_TO_BGR565(255, 0, 0));
        _buffer_ptr->drawRect(_x, _y, _width, _height, RGB_TO_BGR565(0, 0, 0));
    }

private:
};

/****************************************************
 *                     Battery
 ****************************************************/
class BatteryIcon : public Drawable
{
public:
    BatteryIcon(int x, int y, GFXcanvas16 *buffer_ptr)
        : Drawable(x, y, 10, 6, buffer_ptr, "BatteryIcon")
    {
        batteryPercentage = getBatteryPercentage();
    }

    void draw()
    {
        // debug code!!!
        // batteryPercentage--;
        // if (batteryPercentage < 0)
        //     batteryPercentage = 100;

        _buffer_ptr->fillRect(_x + 1, _y, _width - 1, _height, RGB_TO_BGR565(50, 45, 50));
        _buffer_ptr->fillRect(_x, _y + 2, _width, _height - 4, RGB_TO_BGR565(50, 45, 50));

        int batsliver = batteryPercentage * (_width - 3) / 100;
        if (batsliver < 1)
            batsliver = 1; // always want at least a little on screen

        if (isCharging() && batteryPercentage < 100)
            batsliver = batsliver - ((animation_count / 25) % 2);

        uint16_t color;

        if (batteryPercentage < 20)
            color = RGB_TO_BGR565(255, 0, 0); // red
        else if (batteryPercentage < 40)
            color = RGB_TO_BGR565(252, 186, 3); // yellow
        else
            color = RGB_TO_BGR565(0, 255, 0); // green

        _buffer_ptr->fillRect(_x + (_width - batsliver - 1), _y + 1, batsliver, _height - 2, color);
        animation_count++;
    }

    void updateBatteryPercentage()
    {
        batteryPercentage = getBatteryPercentage();
        printDebug("Battery Percentage " + String(batteryPercentage));
    }

private:
    int batteryPercentage = 0;
    int animation_count = 0;
};

/****************************************************
 *                      Time
 ****************************************************/
class Time : public Drawable
{
public:
    Time(int x, int y, GFXcanvas16 *buffer_ptr)
        : Drawable(x, y, 32, 16, buffer_ptr, "Time Display")
    {
        mjd_set_timezone_est();
    }

    void draw()
    {
        time_t s = getRTCTime();
        _tmi = localtime(&s);

        String Hour = String(_tmi->tm_hour, DEC);
        String Minute = String(_tmi->tm_min, DEC);
        String Second = String(_tmi->tm_sec, DEC);

        byte hour, minute, second = 0;
        hour = _tmi->tm_hour;
        minute = (_tmi->tm_min);
        second = _tmi->tm_sec;

        char timestr[12] = "00:00:00 XM";
        if (_tmi->tm_hour > 12)
        {
            timestr[0] = '0' + ((hour - 12) / 10);
            timestr[1] = '0' + ((hour - 12) % 10);
            timestr[9] = 'P';
        }
        else if (_tmi->tm_hour == 12)
        {
            timestr[0] = '1';
            timestr[1] = '2';
            timestr[9] = 'P';
        }
        else if (_tmi->tm_hour == 0)
        {
            timestr[0] = '1';
            timestr[1] = '2';
            timestr[9] = 'A';
        }
        else
        {
            timestr[0] = '0' + (_tmi->tm_hour / 10);
            timestr[1] = '0' + (_tmi->tm_hour % 10);
            timestr[9] = 'A';
        }

        timestr[3] = '0' + (_tmi->tm_min / 10);
        timestr[4] = '0' + (_tmi->tm_min % 10);

        timestr[6] = '0' + (_tmi->tm_sec / 10);
        timestr[7] = '0' + (_tmi->tm_sec % 10);

        _buffer_ptr->setTextSize(textSize);

        _buffer_ptr->setTextColor(color);

        for (int a = 0; a < 11; a++)
        {
            _buffer_ptr->setTextColor(BACKGROUND_COLOR);
            _buffer_ptr->setCursor(_x + a * 6 * textSize + shadowOffset, _y + shadowOffset);
            _buffer_ptr->print(timestr[a]);

            _buffer_ptr->setTextColor(color);
            _buffer_ptr->setCursor(_x + a * 6 * textSize, _y);
            _buffer_ptr->print(timestr[a]);
        }
        _buffer_ptr->setTextSize(1);
    }

private:
    struct tm *_tmi;
    int shadowOffset = 1;
    int textSize = 2;
    int color = TEXT_COLOR;
};

/****************************************************
 *                 Notifications
 ****************************************************/
class NotificationPrinter : public Drawable
{
public:
    NotificationPrinter(int x, int y, int width, int height, String *notificationData, GFXcanvas16 *buffer_ptr)
        : Drawable(x, y, width, height, buffer_ptr, "Notification Area")
    {
        data = notificationData;
    }

    void draw()
    {
        _buffer_ptr->setCursor(_x, _y);
        _buffer_ptr->setTextSize(1);

        int lines = getLineCount(*data);

        for (int a = 0; a < lines; a++)
        {
            String line = parseField(*data, '\n', a);
            String name = parseField(line, ';', 0);
            _buffer_ptr->println(parseField(line, ',', 0));
        }
    }

private:
    String *data;
};

class NotificationPage : public Drawable
{
public:
    NotificationPage(String *notificationData, GFXcanvas16 *buffer_ptr)
        : Drawable(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, buffer_ptr, "Notification Page")
    {
        data = notificationData;
        setTouchable(true);
    }

    void draw()
    {
        // draw all the notifications on the left side first
        _buffer_ptr->setCursor(_x, _y);
        _buffer_ptr->setTextSize(1);

        notification_line_count = getLineCount(*data);

        for (int a = 0; a < notification_line_count; a++)
        {
            String line = parseField(*data, '\n', a);
            String appName = parseField(line, ',', NOTIFICATION_APP_NAME);

            if (a == currentSelection)
            {
                _buffer_ptr->setTextColor(BACKGROUND_COLOR);
                _buffer_ptr->fillRect(_x, _y + a * 8, app_name_width, 8, INTERFACE_COLOR);
            }
            else
                _buffer_ptr->setTextColor(INTERFACE_COLOR);

            int b = 0;
            while (b < appName.length() && b * 6 < app_name_width)
            {
                _buffer_ptr->setCursor(b * 6, a * 8);
                _buffer_ptr->print(appName[b]);
                b++;
            }

            if (a == currentSelection)
            {
                String title = parseField(line, ';', NOTIFICATION_APP_NAME);
                String description = parseField(line, ';', NOTIFICATION_DESCRIPTION);
                String extra = parseField(line, ';', NOTIFICATION_EXTRA_TEXT);

                _buffer_ptr->setTextColor(INTERFACE_COLOR);
                _buffer_ptr->setCursor(_x + app_name_width, _y);
                _buffer_ptr->print(title.substring(0, ((_width) - (_x + app_name_width)) / 6));

                scroll.setString("Subtitle:\n" + extra + "\nDescription:\n" + description);
                scroll.draw();
            }
        }

        if (notification_line_count == 0)
        {
            _buffer_ptr->setCursor(0, 0);
            _buffer_ptr->println("No Notifications...");
        }
        else
        {
            _buffer_ptr->drawFastVLine(app_name_width, 0, SCREEN_HEIGHT, INTERFACE_COLOR);
        }
    }

    void onTouch(int x, int y)
    {
        scroll.onTouch(x, y);
    }

    void nextItem()
    {
        scroll.resetScroll();
        if (currentSelection < notification_line_count)
            currentSelection++;
        printDebug("Next notification item " + String(currentSelection));
    }

    void previousItem()
    {
        scroll.resetScroll();
        if (currentSelection > 0)
            currentSelection--;
        printDebug("Previous notification item " + String(currentSelection));
    }

private:
    String *data;
    const int app_name_width = 36;
    Scrollbox scroll = Scrollbox(app_name_width, 10, SCREEN_WIDTH - app_name_width, SCREEN_HEIGHT - 10, _buffer_ptr);
    int currentSelection = 0;
    int notification_line_count = 0;
};

/****************************************************
 *                    Button
 ****************************************************/

class Button : public Drawable
{
public:
    Button(int x, int y, int width, int height, String text, GFXcanvas16 *buffer_ptr)
        : Drawable(x, y, width, height, buffer_ptr, "Button:" + text)
    {
        setTouchable(true);
        _text = text;
    }

    Button()
        : Drawable(-1, -1, 0, 0, nullptr, "uninitalized Button")
    {
    }

    Button(int x, int y, int width, int height, const uint16_t *image, int imageWidth, int imageHeight, GFXcanvas16 *buffer_ptr)
        : Drawable(x, y, width, height, buffer_ptr, "ImageButton")
    {
        setTouchable(true);
        _imageWidth = imageWidth;
        _imageHeight = imageHeight;
        _img = image;
    }

    void setBackgroundColor(uint16_t color)
    {
        _backgroundColor = color;
    }
    void setForegroundColor(uint16_t color)
    {
        _foregroundColor = color;
    }

    void setBorder(boolean border)
    {
        _border = border;
    }

    String getText()
    {
        return _text;
    }

    void draw()
    {
        // border is optional, may not want this in cases such as image drawing
        if (_border)
        {
            _buffer_ptr->fillRect(_x, _y, _width, _height, _backgroundColor);
            _buffer_ptr->drawRect(_x, _y, _width, _height, _foregroundColor);
        }

        if (!_text.equals(""))
        {
            // calculate width of the text itself.
            int text_width = _text.length() * 6;

            // get center of box
            int center_x = _x + _width / 2;
            int center_y = _y + _height / 2;

            // start cursor at
            _buffer_ptr->setCursor(
                center_x - text_width / 2,
                center_y - 3);

            _buffer_ptr->setTextColor(_foregroundColor);
            _buffer_ptr->print(_text);
        }
        else if (_img != nullptr)
        {
            int start_x = _x + (_width / 2) - _imageWidth / 2;
            int start_y = _y + (_height / 2) - _imageHeight / 2;

            // now loop through each pixel, consider black to be transparent.
            for (int y = 0; y < _imageHeight; y++)
                for (int x = 0; x < _imageWidth; x++)
                    if (_img[x + y * _imageWidth] != 0x0000)
                        _buffer_ptr->drawPixel(x + start_x, y + start_y, _img[x + y * _imageWidth]);
        }
    }

private:
    // support either images or text
    String _text = "";
    boolean _border = true;
    int _imageWidth = -1;
    int _imageHeight = -1;
    const uint16_t *_img;
    uint16_t _backgroundColor = BACKGROUND_COLOR;
    uint16_t _foregroundColor = INTERFACE_COLOR;
};

/****************************************************
 *                    Calculator
 ****************************************************/
class Calculator : public Drawable
{
public:
    Calculator(GFXcanvas16 *buffer_ptr)
        : Drawable(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, buffer_ptr, "Calculator")
    {

        for (int a = 0; a < CALCULATOR_COLUMNS * CALCULATOR_ROWS; a++)
        {
            calculatorButtons[a] = Button(
                xButtonSpacing * (a % CALCULATOR_COLUMNS) + PADDING / 2,
                yButtonSpacing * ((a) / CALCULATOR_COLUMNS) + PADDING / 2 + yButtonSpacing,
                xButtonSpacing - PADDING,
                yButtonSpacing - PADDING,
                calculatorButtonLabels[a],
                _buffer_ptr);
        }
        setTouchable(true);
    }

    void draw()
    {

        // draw the
        _buffer_ptr->fillRect(_x + PADDING / 2, _y + PADDING / 2, _width - PADDING, yButtonSpacing - PADDING, BACKGROUND_COLOR);
        _buffer_ptr->drawRect(_x + PADDING / 2, _y + PADDING / 2, _width - PADDING, yButtonSpacing - PADDING, INTERFACE_COLOR);
        _buffer_ptr->setCursor(_x + _width - PADDING / 2 - 6 * (dispStr.length() + 1), _y + (yButtonSpacing - 6) / 2);
        _buffer_ptr->print(dispStr);

        for (int a = 0; a < CALCULATOR_COLUMNS * CALCULATOR_ROWS; a++)
            calculatorButtons[a].draw();

        // request to keep screen on for the next 250 milliseconds (this way the calculator can be kept going indefinitely)
        requestWakeLock(250);
    }

    void addToText()
    {
        printDebug("addToText()");
    }

    void onTouch(int x, int y)
    {
        if (lastTouch + TOUCH_COOLDOWN < millis())
        {
            for (int a = 0; a < TOTAL_BUTTONS; a++)
            {
                boolean touched = calculatorButtons[a].isTouched(x, y);
                if (touched)
                {
                    // dispStr += calculatorButtons[a].getText();
                    char character = calculatorButtons[a].getText()[0]; // get the character
                    switch (character)
                    {
                    case '=':
                        printDebug("Calculating");
                        performCalculation();
                        break;
                    case 'C':
                        dispStr = "";
                        firstValue = "";
                        secondValue = "";
                        op = "";
                        break;
                    case '<':
                        if (op.length() > 0)
                            secondValue = secondValue.substring(0, secondValue.length() - 1);
                        else
                            firstValue = firstValue.substring(0, firstValue.length() - 1);
                        dispStr = firstValue + " " + op + " " + secondValue;
                        break;
                    default:
                        addCharacter(character);
                    }
                }
            }
            lastTouch = millis();
        }
    }

    void addCharacter(char c)
    {
        if (c == '+' || c == '-' || c == '*' || c == '/')
            op = String(c);
        else if (op.length() > 0)
            secondValue += c;
        else
            firstValue += c;

        dispStr = firstValue + " " + op + " " + secondValue;
    }

    void performCalculation()
    {
        double first = firstValue.toDouble();
        double second = secondValue.toDouble();
        double result = 0;
        boolean error = false;
        switch (op[0])
        {
        case '+':
            result = first + second;
            break;
        case '-':
            result = first - second;
            break;
        case '*':
            result = first * second;
            break;
        case '/':
            if (second != 0)
            {
                result = first / second;
            }
            else
            {
                error = true;
            }
            break;
        default:
            error = true;
            break;
        }

        if (error)
        {
            firstValue = "NaN";
        }
        else
        {
            firstValue = String(result, 4);
        }
        secondValue = "";
        op = "";
        dispStr = firstValue + " " + op + " " + secondValue;
    }

private:
    static const int CALCULATOR_COLUMNS = 5;
    static const int CALCULATOR_ROWS = 4;
    static const int TOTAL_BUTTONS = CALCULATOR_COLUMNS * CALCULATOR_ROWS;
    static const int PADDING = 4;
    static const int TOUCH_COOLDOWN = 100;

    unsigned long lastTouch = 0;
    int xButtonSpacing = SCREEN_WIDTH / CALCULATOR_COLUMNS;
    int yButtonSpacing = SCREEN_HEIGHT / (CALCULATOR_ROWS + 1); // the actual result will be the top row

    String dispStr = "";
    String firstValue = "";
    String secondValue = "";
    String op = "";

    Button calculatorButtons[CALCULATOR_COLUMNS * CALCULATOR_ROWS];
    String calculatorButtonLabels[CALCULATOR_COLUMNS * CALCULATOR_ROWS] = {
        "7", "8", "9", "/", "<",
        "4", "5", "6", "*", "",
        "1", "2", "3", "-", "C",
        "0", ".", "", "+", "="};
};

/****************************************************
 *              CALIBRATION SCREEN
 ****************************************************/
class CalibrationScreen : public Drawable
{
public:
    CalibrationScreen(GFXcanvas16 *buffer_ptr)
        : Drawable(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, buffer_ptr, "CalibrationScreen")
    {
        lastTouch = millis();
    }

    void draw()
    {
        _buffer_ptr->fillScreen(BACKGROUND_COLOR);
        _buffer_ptr->setCursor(0, 10);
        _buffer_ptr->setTextColor(INTERFACE_COLOR);
        _buffer_ptr->print("Touch Calibration");
        if (state < 4)
        {
            _buffer_ptr->drawCircle(CALIBRATION_POINTS_X[state], CALIBRATION_POINTS_Y[state], 5, RGB_TO_BGR565(255, 0, 0));
            _buffer_ptr->drawCircle(CALIBRATION_POINTS_X[state], CALIBRATION_POINTS_Y[state], 1, RGB_TO_BGR565(255, 0, 0));
        }
        else
        {
            _buffer_ptr->setCursor(0, 20);
            _buffer_ptr->setTextColor(INTERFACE_COLOR);
            _buffer_ptr->print("Swipe up to confirm calibration and exit");
            struct point p = readTouch();
            _buffer_ptr->fillCircle(p.x, p.y, 5, RGB_TO_BGR565(255, 0, 0));
        }
        readTouchState();
    }

    // this operates differently from the onTouch method since it needs to bypass some of the checks that
    // prevent invalid data
    void readTouchState()
    {
        if ((lastTouch + TOUCH_COOLDOWN) < millis())
        {
            printDebug("touchstate");
            // we're just going to ignore the parameter x y and get the raw values ourselves
            struct point p = readTouchRaw();
            if (state < 4)
                if (!digitalRead(TOUCH_IRQ))
                {
                    printDebug("Raw Reading from touchscreen " + String(p.x) + "," + String(p.y));
                    calx[state] = p.x;
                    caly[state] = p.y;

                    state++;
                }

            if (state == TOTAL_CALIBRATION_POINTS)
            {
                // load calibration data into the EEPROM
                for (int a = 0; a < TOTAL_CALIBRATION_POINTS; a++)
                {
                    setDataField(calx[a], ("calx" + String(a)).c_str());
                    setDataField(caly[a], ("caly" + String(a)).c_str());
                }

                loadEEPROMSettings();
                CLEAR_TOUCH_CALIBRATION = false;
                state++;
            }
            lastTouch = millis();
        }
    }

private:
    uint16_t calx[4];
    uint16_t caly[4];
    static const int TOUCH_COOLDOWN = 1000;
    unsigned long lastTouch = 0;
    int state = 0;
};

/****************************************************
 *              Application Icons
 ****************************************************/
class AppIconItem : public Drawable
{
public:
    AppIconItem(int x, int y, String appname, GFXcanvas16 *buffer_ptr)
        : Drawable(x, y, 32, 32, buffer_ptr)
    {
        _appname = appname;
    }

    void draw()
    {
        // if we want to init this without drawing anything, we can give it a blank app name.
        if (!_appname.equals(""))
            if (iconLoaded && _icon != nullptr)
            {
                for (int x = 0; x < 32; x++)
                {
                    for (int y = 0; y < 32; y++)
                    {
                        uint16_t color = ((uint16_t *)_icon)[x + y * 32];
                        if (color > 0)
                            _buffer_ptr->drawPixel(x + _x, y + _y, color );
                    }
                }
                // _buffer_ptr->drawRGBBitmap(_x, _y, (uint16_t *)_icon, 32, 32);
            }
            else
                iconLoaded = loadIconFromFile(_appname, _icon);
    }

    void setAppName(String appname)
    {
        _appname = appname;
        iconLoaded = false;
    }

private:
    boolean iconLoaded = false;
    String _appname;
    uint16_t _icon[32 * 32];
};

/****************************************************
 *           Application Notification
 ****************************************************/
class ApplicationNotification : public Drawable
{
public:
    ApplicationNotification(int x, int y, String appname, String *notificationData, GFXcanvas16 *buffer_ptr)
        : Drawable(x, y, 32, 32, buffer_ptr)
    {
        _appname = appname;
        _data = notificationData;
        appicon = AppIconItem(x, y, _appname, _buffer_ptr);
        setTouchable(true);
    }

    ApplicationNotification()
        : Drawable(-1, -1, 0, 0, nullptr, "uninitalized ApplicationNotification")
    {
    }

    void draw()
    {
        if (!_appname.equals("") && _buffer_ptr != nullptr)
        {
            appicon.draw();
            _buffer_ptr->fillCircle(_x + 32, _y + 32, 5, RGB_TO_BGR565(255, 0, 0));
            _buffer_ptr->setCursor(_x + 30, _y + 29);
            _buffer_ptr->print(String(notificationCount));

            if (last_update + update_rate < millis())
                updateNotificationData();
        }
    }

    void setAppName(String new_name)
    {
        appicon.setAppName(new_name);
        _appname = new_name;
    }

    String getAppName()
    {
        return _appname;
    }

    void updateNotificationData()
    {
        notificationCount = getNotificationCount(); // update the notification count
        last_update = millis();
    }

    String getNotificationContent()
    {
        int lines = getLineCount(*_data);
        String ret = "";

        for (int a = 0; a < lines; a++)
        {
            String line = parseField(*_data, '\n', a);
            String app_and_des = parseField(line, ';', 0);
            String name = parseField(app_and_des, ',', 0);
            String subtitle = parseField(app_and_des, ',', 1);
            name.toLowerCase();
            if (name.equals(_appname))
                ret += subtitle + "\n";
        }
        return ret;
    }

    // returns the number of notifications for this app
    int getNotificationCount()
    {
        int lines = getLineCount(*_data);
        int count = 0;

        // printDebug("Updating notification count for app " + _appname);

        for (int a = 0; a < lines; a++)
        {
            String line = parseField(*_data, '\n', a);
            String name = parseField(line, ',', 0);
            name.toLowerCase();
            if (name.equals(_appname))
                count++;
        }
        return count;
    }

private:
    const static int update_rate = 1000;
    unsigned long last_update = 0;
    String _appname = "";
    String *_data = nullptr;
    AppIconItem appicon = AppIconItem(0, 0, "", _buffer_ptr);
    int notificationCount = 0;
    String content = "";
};

/****************************************************
 *             Notification Floater
 * A floating window for displaying notification data
 * over the app notification icon grid.
 ****************************************************/
class TextFloaterWindow : public Drawable
{
public:
    TextFloaterWindow(int x, int y, int width, int height, GFXcanvas16 *buffer_ptr)
        : Drawable(x, y, width, height, buffer_ptr, "TextFloaterWindow")
    {
        expansion_rate = height / 8; // expand in 8 frames.
        scroll = Scrollbox(x, y + SCROLLBOX_VERTICAL_PADDING, width, height, buffer_ptr);
        scroll.setString("test text\n with some extra lines \n that can only bee seen \n when expanded");
        setTouchable(true);
    }

    void draw()
    {
        if (activated)
        {
            if (current_height < _height)
                current_height += expansion_rate;
        }
        else
        {
            if (current_height > 0)
                current_height -= expansion_rate;
        }
        if (current_height != 0)
        {
            _buffer_ptr->fillRect(_x, _y + (_height - current_height) / 2, _width, current_height, BACKGROUND_COLOR);
            _buffer_ptr->drawRect(_x, _y + (_height - current_height) / 2, _width, current_height, INTERFACE_COLOR);
            scroll.setDims(_x, _y + (_height - current_height) / 2 + SCROLLBOX_VERTICAL_PADDING, _width, current_height - SCROLLBOX_VERTICAL_PADDING * 2);
            scroll.draw();
        }
    }

    void activate(String content)
    {
        scroll.setString(content);
        activated = true;
    }

    void setString(String content)
    {
        scroll.setString(content);
    }

    void deactivate()
    {
        activated = false;
    }

    boolean isActive()
    {
        return activated;
    }

private:
    boolean activated = false;
    static const int SCROLLBOX_VERTICAL_PADDING = 2;
    int current_height = 0;
    int expansion_rate;
    Scrollbox scroll = Scrollbox(-1, -1, 0, 0, _buffer_ptr);
};

/***************************************************
 *                Spotify Overlay
 * Allows the user to control spotify playback and displays
 * current song.
 * ************************************************/

class SpotifyOverlay : public Drawable
{
public:
    SpotifyOverlay(String *song_string_ptr, GFXcanvas16 *buffer_ptr)
        : Drawable(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, buffer_ptr, "SpotifyOverlay")
    {

        // data is provided by a song string that is somewhere else in the code.
        // this makes it easier to perform async updating of the string
        current_song_string_ptr = song_string_ptr;

        // reusing the floater window because it's really nice.
        floater = TextFloaterWindow(
            FLOATER_X,
            FLOATER_Y,
            FLOATER_WIDTH,
            FLOATER_HEIGHT,
            buffer_ptr);

        // button spacing and padding, 4 buttons.
        BUTTON_PADDING = (_width - BUTTON_WIDTH * 4) / 5;
        BUTTON_ACTIVE_Y_POS = SCREEN_HEIGHT - (BUTTON_WIDTH + BUTTON_PADDING);

        // create buttons, the icons are declared in Declarations.h
        last_button = Button(BUTTON_PADDING,
                             BUTTON_ACTIVE_Y_POS,
                             BUTTON_WIDTH,
                             BUTTON_WIDTH,
                             (uint16_t *)&LAST_MEDIA_ICON_16X16,
                             16,
                             16,
                             _buffer_ptr);

        play_button = Button(1 * BUTTON_WIDTH + 2 * BUTTON_PADDING,
                             BUTTON_ACTIVE_Y_POS,
                             BUTTON_WIDTH,
                             BUTTON_WIDTH,
                             (uint16_t *)&PLAY_MEDIA_ICON_16X16,
                             16,
                             16,
                             _buffer_ptr);

        pause_button = Button(2 * BUTTON_WIDTH + 3 * BUTTON_PADDING,
                              BUTTON_ACTIVE_Y_POS,
                              BUTTON_WIDTH,
                              BUTTON_WIDTH,
                              (uint16_t *)&PAUSE_MEDIA_ICON_16X16,
                              16,
                              16,
                              _buffer_ptr);

        next_button = Button(3 * BUTTON_WIDTH + 4 * BUTTON_PADDING,
                             BUTTON_ACTIVE_Y_POS,
                             BUTTON_WIDTH,
                             BUTTON_WIDTH,
                             (uint16_t *)&NEXT_MEDIA_ICON_16X16,
                             16,
                             16,
                             _buffer_ptr);
    }

    void draw()
    {
        // floater will display nothing when inactive, so always draw
        floater.draw();

        // buttons on the other hand should only be drawn when active.
        if (isActive())
        {
            last_button.draw();
            play_button.draw();
            pause_button.draw();
            next_button.draw();

            // additionally only update the song when this window is active to avoid
            // excessive polling of the phone.
            if (last_update + update_rate < millis())
            {
                updateCurrentSong();
                last_update = millis();
            }

            // songs are formatted as "<song name>-<artist>" we have the space so replace the dash with a new line
            //(in the near future change the way songs are transmitted from the phone.)
            (*current_song_string_ptr).replace("-", "\n");
            floater.setString(*current_song_string_ptr);
        }
    }

    void onTouch(int x, int y)
    {
        // if the overlay is active pass the touch event to the buttons and perform their
        // respective actions, if no button is pressed assume that it was an intentional exit

        if (isActive())
        {
            if (last_button.isTouched(x, y))
                lastSong();
            else if (next_button.isTouched(x, y))
                nextSong();
            else if (play_button.isTouched(x, y))
                playMusic();
            else if (pause_button.isTouched(x, y))
                pauseMusic();
            else
                deactivate();
        }
    }

    boolean isActive()
    {
        return active;
    }

    void activate()
    {
        active = true;
        floater.activate("Reading song....");
    }

    void deactivate()
    {
        floater.deactivate();
        active = false;
    }

private:
    // Button(int x, int y, int width, int height, const uint16_t *image, int imageWidth, int imageHeight, GFXcanvas16 *buffer_ptr)
    Button play_button, pause_button, next_button, last_button;
    boolean active = false;
    String *current_song_string_ptr;
    static const int update_rate = 800;
    unsigned long last_update = 0;

    int BUTTON_WIDTH = 32;
    int BUTTON_ACTIVE_Y_POS, BUTTON_PADDING;

    const static int FLOATER_WIDTH = (int)(SCREEN_WIDTH * 0.9);
    const static int FLOATER_HEIGHT = (int)(SCREEN_HEIGHT * 0.4);
    const static int FLOATER_X = (SCREEN_WIDTH - FLOATER_WIDTH) / 2;
    const static int FLOATER_Y = 10;

    TextFloaterWindow floater = TextFloaterWindow(0, 0, 0, 0, _buffer_ptr);
};

/****************************************************
 *               Notification Grid
 *
 *     Displays the notification icons and manages
 * their touch actions along with displaying notification previews.
 * Also handles the spotify player overlay which is openable by
 * tapping on the spotify app icon
 ****************************************************/
class NotificationGrid : public Drawable
{
public:
    NotificationGrid(int x, int y, int width, int height, String *notificationData, GFXcanvas16 *buffer_ptr)
        : Drawable(x, y, width, height, buffer_ptr, "NotificationGrid")
    {
        _data = notificationData;

        int x_spacing = width / gridsize_x;
        int y_spacing = height / gridsize_y;

        for (int b = 0; b < gridsize_y; b++)
            for (int a = 0; a < gridsize_x; a++)
                app_notifications[gridsize_x * b + a] = ApplicationNotification(x + a * x_spacing, y + b * y_spacing, "", notificationData, buffer_ptr);

        // floater for displaying notification data
        floater = TextFloaterWindow(FLOATER_X, FLOATER_Y, FLOATER_WIDTH, FLOATER_HEIGHT, buffer_ptr);

        // spotify overlay for displaying spotify information and controls
        spotifyOverlay = SpotifyOverlay(&currentSong, buffer_ptr);
        setTouchable(true);
    }

    void draw()
    {
        for (int a = 0; a < gridsize_x * gridsize_y; a++)
            app_notifications[a].draw();

        // check for whether any apps were added to the grid
        if (last_update + update_rate < millis())
            updateGrid();

        // both the floater and spotify overlay will draw nothing when
        // they are not active.
        floater.draw();
        spotifyOverlay.draw();
    }

    void onTouch(int x, int y)
    {

        // when the spotify overlay is active it takes over the touch control of the notification grid.
        // this is a special case since spotify is the only app for which we have special actions that can be performed.
        if (spotifyOverlay.isActive())
        {
            // touch handling for spotify overlay
            spotifyOverlay.onTouch(x, y);
        }

        // if the floater is active, tapping anywhere other than the floater should result in the floater
        // collapsing and returning control to the notification grid.
        else if (floater.isActive())
        {
            if (!floater.isTouched(x, y))
                floater.deactivate();
        }
        // normal touch handling for the notification grid.
        else
        {
            // normal grid operation.
            int touched_index = -1;
            // check the touch of each notification grid item, if it's touched then we want to expand it to see the notification.
            for (int a = 0; a < (gridsize_x * gridsize_y); a++)
                if (!app_notifications[a].getAppName().equals(""))
                    if (app_notifications[a].isTouched(x, y))
                    {
                        // if the notification tapped happens to be spotify then we're going to yield control
                        // over to the spotify overlay.
                        if (app_notifications[a].getAppName().equals("spotify"))
                        {
                            printDebug("Spotify Selected, opening media player.");
                            spotifyOverlay.activate();
                        }
                        // otherwise, if it's just a standard application, perform the normal tap action and display information
                        // in the floater for the user.
                        else
                        {
                            printDebug("Notificaton icon " + app_notifications[a].getAppName() + " is touched");
                            floater.activate(app_notifications[a].getNotificationContent());
                        }
                        // if a part of the grid actually was touched then we want to save that index for later
                        touched_index = a;
                    }

            // if the touch was never determined to be on an app icon then do nothing and deactivate the floater.
            if (touched_index == -1)
            {
                floater.deactivate();
            }
        }
    }

    // handles creating new icons if they don't already exist, and populating the grid.
    void updateGrid()
    {
        int lines = getLineCount(*_data);

        // for each line
        for (int a = 0; a < lines; a++)
        {
            String line = parseField(*_data, '\n', a);
            String name = parseField(line, ',', 0);
            name.toLowerCase();

            boolean app_is_on_grid = false;

            // check whether app exists on the grid,
            for (int b = 0; b < (gridsize_x * gridsize_y); b++)
            {
                if (app_notifications[b].getAppName().equals(name))
                    app_is_on_grid = true;
            }
            // if not on grid then add to grid
            if (!app_is_on_grid)
                addAppToGrid(name);
        }

        // check for apps with count of 0, remove them from the grid
        for (int b = 0; b < (gridsize_x * gridsize_y); b++)
        {
            if (app_notifications[b].getNotificationCount() == 0)
                app_notifications[b].setAppName(""); // blank app name to remove from grid
        }

        // if there are any empty spots, move icons to the left to fill those gaps.
        shiftApps();
        last_update = millis();
    }

    boolean addAppToGrid(String appname)
    {
        for (int a = 0; a < gridsize_x * gridsize_y; a++)
            if (app_notifications[a].getAppName().equals(""))
            {
                app_notifications[a].setAppName(appname);
                return true;
            }
        return false;
    }

    void shiftApps()
    {
        // let the apps shift to the left and fill empty spots.
        for (int a = 1; a < gridsize_x * gridsize_y; a++)

            // if there is an empty spot to the left of the app icon
            if (!app_notifications[a].getAppName().equals("") && app_notifications[a - 1].getAppName().equals(""))
            {
                // move move the icon to the left and clear it's current spot.
                app_notifications[a - 1].setAppName(app_notifications[a].getAppName());
                app_notifications[a].setAppName("");
            }
    }

private:
    const static int update_rate = 1000;
    const static int gridsize_x = 4;
    const static int gridsize_y = 2;

    unsigned long last_update = 0;
    String _appname = "";
    String *_data;
    ApplicationNotification app_notifications[gridsize_x * gridsize_y];

    const static int FLOATER_WIDTH = (int)(SCREEN_WIDTH * 0.9);
    const static int FLOATER_HEIGHT = (int)(SCREEN_HEIGHT * 0.7);
    const static int FLOATER_X = (SCREEN_WIDTH - FLOATER_WIDTH) / 2;
    const static int FLOATER_Y = (SCREEN_HEIGHT - FLOATER_HEIGHT) / 2;

    SpotifyOverlay spotifyOverlay = SpotifyOverlay(&currentSong, _buffer_ptr);
    TextFloaterWindow floater = TextFloaterWindow(FLOATER_X, FLOATER_Y, FLOATER_WIDTH, FLOATER_HEIGHT, _buffer_ptr);
};
