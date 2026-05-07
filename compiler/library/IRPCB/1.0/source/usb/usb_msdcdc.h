#ifndef USB_MSD_CDC_H__
#define USB_MSD_CDC_H__

#include "shell.h"

#ifndef MSD_READ_ONLY
#define MSD_READ_ONLY   FALSE
#endif

#define MSD_TIMEOUT_MS   2700
#define MSD_DATABUF_SZ   4096

class UsbMsdCdc : public Shell {
private:
    void IStartTransmissionIfNotYet(); // Required for printf implementation
    retv IPutChar(char c);             // Required for printf implementation
public:
    void Init();
    void Reset();
    void Connect();
    void Disconnect();
    bool IsActive();
    retv TryParseRxBuff(); // Call this when something ip received
    retv ReceiveBinaryToBuf(uint8_t *ptr, uint32_t Len, uint32_t timeout_ms);
    retv TransmitBinaryFromBuf(uint8_t *ptr, uint32_t Len, uint32_t timeout_ms);
};

extern UsbMsdCdc usb_msd_cdc;

#endif // USB_MSD_CDC_H__
