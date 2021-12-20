#include <Adafruit_GFX.h>
#ifndef DRAWABLE
#define DRAWABLE

class Drawable
{
public:
    Drawable(int x, int y, int width, int height, GFXcanvas16 *buffer_ptr);
    Drawable(int x, int y, int width, int height, GFXcanvas16 *buffer_ptr, String type);
    virtual void draw();
    int getx();
    int gety();
    int getWidth();
    int getHeight();
    boolean isTouched(int x, int y);
    void setTouchable(boolean val);
    void registerCallback(void (*cb)());
    virtual void onTouch();
    String toString(); 

protected:
    String _type = "Drawable";
    int _x;
    int _y;
    int _width;
    int _height;
    void (*_callback)();
    boolean _touchable = false;
    GFXcanvas16 *_buffer_ptr;
};
#endif