#ifndef _JANELAPRO_H_
#define _JANELAPRO_H_

#include <SinricProDevice.h>
#include <Capabilities/ModeController.h>
#include <Capabilities/RangeController.h>

class JanelaPRO 
: public SinricProDevice
, public ModeController<JanelaPRO>
, public RangeController<JanelaPRO> {
  friend class ModeController<JanelaPRO>;
  friend class RangeController<JanelaPRO>;
public:
  JanelaPRO(const String &deviceId) : SinricProDevice(deviceId, "JanelaPRO") {};
};

#endif
