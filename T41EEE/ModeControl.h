// Added October 2025.  Mode control struct.  Greg Raven KF5N

#pragma once

struct ModeControl {

public:

  void CWReceiveMode();
  void CWTransmitMode();
  void SSBReceiveMode();
  void SSBTransmitMode();
  void FT8ReceiveMode();
  void FT8TransmitMode();
  void AMReceiveMode();
  void SAMReceiveMode();
};