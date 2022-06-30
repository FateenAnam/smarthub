#ifndef VIRTUAL_STRIP_CPP
#define VIRTUAL_STRIP_CPP

#include "VirtualStrip.h"

VirtualStrip::VirtualStrip()
    : stripPtr(nullptr),
    numLeds (0),
    reversed(false) {
}

VirtualStrip::VirtualStrip(CRGB* strip, uint16_t begin, uint16_t numLeds, bool reversed = false) 
    : stripPtr(new CRGB*[numLeds]), // Dynamically allocate array
    numLeds(numLeds),
    reversed(reversed)
    {
    if (!reversed) {
        for (int i = 0; i < numLeds; ++i) {
        stripPtr[i] = &(strip[begin + i]);
        }
    }
    else {
        for (int i = 0; i < numLeds; ++i) {
        stripPtr[i] = &(strip[begin + numLeds - i]);
        }
    }
}

VirtualStrip::~VirtualStrip() {
    if (!stripPtr)
        delete stripPtr;
}

void VirtualStrip::appendStrip(VirtualStrip& newStrip) {
    uint16_t newLength = numLeds + newStrip.getLength();
    CRGB** newStripPtr = new CRGB*[newLength];

    // Transfer original array
    for (int i = 0; i < numLeds; ++i) {
        newStripPtr[i] = stripPtr[i];
    }
    // Add new strip
    for (int i = numLeds; i < newLength; ++i) {
        newStripPtr[i] = &(newStrip[i - numLeds]);
}

numLeds = newLength;

CRGB** tmp = stripPtr;
stripPtr = newStripPtr;
delete tmp;
}

CRGB& VirtualStrip::operator[](int index) {
    if (index >= numLeds) {
      return *(stripPtr[numLeds - 1]);
    }
    else if (index < 0) {
        return *(stripPtr[0]);
    }
    
    return *(stripPtr[index]);
}

uint16_t VirtualStrip::getLength() {
    return numLeds;
}

#endif