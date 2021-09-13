#ifndef _SMARTWINDOW_H_
#define _SMARTWINDOW_H_

#include <SinricProDevice.h>
#include <Capabilities/ModeController.h>

class SmartWindow 
: public SinricProDevice
, public ModeController<SmartWindow> {
  friend class ModeController<SmartWindow>;
public:
  SmartWindow(const String &deviceId) : SinricProDevice(deviceId, "SmartWindow") {};
};

#endif
