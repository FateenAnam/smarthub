#ifndef VIRTUAL_STRIP_H
#define VIRTUAL_STRIP_H

#include <FastLED.h>

/**
 * @class VirtualStrip
 * @brief Allows the user to group the strips into their logical locations. For example,
 * the front-left bar can have its own strip
 */
class VirtualStrip {
  public:
  // Default constructor
    VirtualStrip();

  // Constructor
    VirtualStrip(CRGB* strip, uint16_t begin, uint16_t numLeds, bool reversed = false);

    // Destructor
  ~VirtualStrip();

    // Append newStrip to the end of the current strip
  void appendStrip(VirtualStrip& newStrip);

    // Returns the underlying CRGB object
    CRGB& operator[](int index);

    // Returns the length of the strip
  uint16_t getLength();

  private:
    CRGB** stripPtr;
    uint16_t numLeds;
    bool reversed;
};

#endif