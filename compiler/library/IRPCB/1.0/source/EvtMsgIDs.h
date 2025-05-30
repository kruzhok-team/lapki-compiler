/*
 * EvtMsgIDs.h
 *
 *  Created on: 21 ���. 2017 �.
 *      Author: Kreyl
 */

#ifndef EVTMSGIDS_H__
#define EVTMSGIDS_H__

enum class EvtId : uint8_t {
    None = 0, // Always

    UartCheckTime,

    TestingTime,

    // Usb
    UsbConnect,
    UsbDisconnect,
    UsbReady,
    UsbCdcDataRcvd,
};

#endif //EVTMSGIDS_H__
