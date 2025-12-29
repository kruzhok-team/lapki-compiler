/*
 * EvtMsg.h
 *
 *  Created on: 21 ���. 2017 �.
 *      Author: Kreyl
 */

#ifndef MSGQ_H_
#define MSGQ_H_
#include <inttypes.h>
//#include "gd_lib.h"
#include "EvtMsgIDs.h"
#include "yartos.h"
//#include "board.h"
#if BUTTONS_ENABLED
#include "buttons.h"
#endif

/*
 * Example of msg:
 * union RMsg_t {
    uint32_t DWord[3];
    CmdUniversal_t cmd;
    RMsg_t& operator = (const RMsg_t &right) {
        DWord[0] = right.DWord[0];
        DWord[1] = right.DWord[1];
        DWord[2] = right.DWord[2];
        return *this;
    }
    RMsg_t() {
        DWord[0] = 0;
        DWord[1] = 0;
        DWord[2] = 0;
    }
    RMsg_t(CmdUniversal_t *PCmd) {
        cmd.CmdID = PCmd->CmdID;
        cmd.SnsID = PCmd->SnsID;
        cmd.w16[0] = PCmd->w16[0];
        cmd.w16[1] = PCmd->w16[1];
        cmd.w16[2] = PCmd->w16[2];
    }
} __attribute__((__packed__));
[...]
EvtMsgQ_t<RMsg_t, RMSG_Q_LEN> MsgQ;
*/

#define MAIN_EVT_Q_LEN      11  // Messages in queue
#define EMSG_DATA8_CNT      7   // ID + 7 bytes = 8 = 2x DWord32
#define EMSG_DATA16_CNT     3   // ID + 3x2bytes = 3

union EvtMsg {
    uint32_t dw32[2];
    struct {
        union {
            void* ptr;
            struct {
                int32_t value;
                uint8_t value_id;
            } __attribute__((__packed__));
//            uint8_t b[EMSG_DATA8_CNT];
//            uint16_t w16[EMSG_DATA16_CNT];
#if BUTTONS_ENABLED
            BtnEvtInfo_t BtnEvtInfo;
#endif
        } __attribute__((__packed__));
        EvtId id;
    } __attribute__((__packed__));

    EvtMsg& operator = (const EvtMsg &right) {
        dw32[0] = right.dw32[0];
        dw32[1] = right.dw32[1];
        return *this;
    }
    EvtMsg() : ptr(nullptr), id(EvtId::None) {}
    EvtMsg(EvtId aid) : ptr(nullptr), id(aid) {}
    EvtMsg(EvtId aid, void *aptr) : ptr(aptr), id(aid) {}
    EvtMsg(EvtId aid, int32_t avalue) : value(avalue), id(aid) {}
    EvtMsg(EvtId aid, uint8_t avalue_id, int32_t avalue) : value(avalue), value_id(avalue_id), id(aid) {}
} __attribute__((__packed__));


template<typename T, uint32_t sz>
class EvtMsgQ {
private:
    union {
        uint64_t __align;
        T ibuf[sz];
    };
    T *read_ptr, *write_ptr;
    Semaphore full_sem;    // Full slots counter
    Semaphore empty_sem;   // Empty slots counter
public:
    EvtMsgQ() : __align(0), read_ptr(ibuf), write_ptr(ibuf) {}
    void Init() {
        read_ptr = ibuf;
        write_ptr = ibuf;
        empty_sem.Init(sz);
        full_sem.Init(0);
    }

    /* Retrieves a message from a mailbox, returns empty msg if failed.
     * The invoking thread waits until a message is posted in the mailbox
     * for a timeout (may be TIME_INFINITE or TIME_IMMEDIATE) */
    T Fetch(systime_t timeout) {
        T msg;
        Sys::Lock();
        if(full_sem.WaitTimeoutS(timeout) == retv::Ok) {
            // There is something in the queue, get it
            msg = *read_ptr++;
            if(read_ptr >= &ibuf[sz]) read_ptr = ibuf;  // Circulate pointer
            empty_sem.SignalI();
            Sys::RescheduleS();
        }
        Sys::Unlock();
        return msg;
    }

    /* Post a message into a mailbox.
     * The function returns a timeout condition if the queue is full */
    retv SendNowOrExitI(const T &msg) {
        if(empty_sem.GetCntI() <= 0L) return retv::Timeout; // Q is full
        empty_sem.FastWaitI();
        *write_ptr++ = msg;
        if(write_ptr >= &ibuf[sz]) write_ptr = ibuf;  // Circulate pointer
        full_sem.SignalI();
        return retv::Ok;
    }

    retv SendNowOrExit(const T &msg) {
        Sys::Lock();
        retv rslt = SendNowOrExitI(msg);
        Sys::RescheduleS();
        Sys::Unlock();
        return rslt;
    }

    /* Posts a message into a mailbox.
     * The invoking thread waits until a empty slot in the mailbox becomes available
     * or the specified time runs out. */
    retv SendWaitingAbility(const T &msg, systime_t timeout) {
        Sys::Lock();
        retv rslt = empty_sem.WaitTimeoutS(timeout);
        if(rslt == retv::Ok) {
            *write_ptr++ = msg;
            if(write_ptr >= &ibuf[sz]) write_ptr = ibuf;  // Circulate pointer
            full_sem.SignalI();
            Sys::RescheduleS();
        }
        Sys::Unlock();
        return rslt;
    }

    uint32_t GetFullCnt() {
        Sys::Lock();
        uint32_t rslt = full_sem.GetCntI();
        Sys::Unlock();
        return rslt;
    }
};

/* Always presents in main:
 * EvtMsgQ_t<EvtMsg, MAIN_EVT_Q_LEN> evt_q_main;
 * ...
 * evt_q_main.Init();
 * ...
 * EvtMsg msg = evt_q_main.Fetch(TIME_INFINITE);
 * switch(msg.id) {
        case evtIdButtons:
        break;
        ...
        default: Printf("Unhandled msg %u\r", msg.id); break;
    } // Switch
 */
extern EvtMsgQ<EvtMsg, MAIN_EVT_Q_LEN> evt_q_main;

#endif // MSGQ_H_
