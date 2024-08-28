#pragma once

#ifndef QHSM_H
#define QHSM_H
/****************************************************************************/
/*! The current QP version as a decimal constant XXYZ, where XX is a 2-digit
* major version number, Y is a 1-digit minor version number, and Z is
* a 1-digit release number.
*/
#define QP_VERSION      650U

/*! The current QP version number string of the form XX.Y.Z, where XX is
* a 2-digit major version number, Y is a 1-digit minor version number,
* and Z is a 1-digit release number.
*/
#define QP_VERSION_STR  "6.5.0"

/*! Tamperproof current QP release (6.5.0) and date (2019-03-31) */
#define QP_RELEASE      0x8E8DC8C5U


/****************************************************************************/


#include <stdint.h>

#define Q_MAX_DEPTH 8

typedef int QSignal;
typedef int QState;

typedef struct
{
    QSignal sig;
} QEvt;

typedef QState (*QStateHandler)(void *const me, const QEvt *const event);

enum
{
    QEP_EMPTY_SIG_ = 0,
    Q_ENTRY_SIG,
    Q_EXIT_SIG,
    Q_INIT_SIG,
    Q_VERTEX_SIG,
    Q_USER_SIG,
};

enum
{
    Q_RET_SUPER,
    Q_RET_UNHANDLED,
    Q_RET_HANDLED,
    Q_RET_IGNORED,
    Q_RET_TRAN,
};

typedef struct
{
    QStateHandler current_;
    QStateHandler effective_;
    QStateHandler target_;
} QHsm;

#define Q_MSM_UPCAST(me) ((QHsm *)(me))
#define Q_STATE_CAST(handler) ((QStateHandler)(handler))

#define Q_UNHANDLED() ((QState)(Q_RET_UNHANDLED))
#define Q_HANDLED() ((QState)(Q_RET_HANDLED))
#define Q_TRAN(target) \
    ((Q_MSM_UPCAST(me))->target_ = Q_STATE_CAST(target), \
    (QState)(Q_RET_TRAN))
#define Q_SUPER(super) \
    ((Q_MSM_UPCAST(me))->effective_ = Q_STATE_CAST(super), \
    (QState)(Q_RET_SUPER))

#define QMSM_INIT(me, event) (QMsm_init(me, event))
#define QMSM_DISPATCH(me, event) (QMsm_dispatch(me, event))

#define SIMPLE_DISPATCH(me_, sig_) \
        do { QEvt e_; e_.sig = sig_##_SIG; QMSM_DISPATCH(me_, &e_); } while (0)  // Macro to simple dispatch calls with signal only
#define SIGNAL_DISPATCH(me_, sig_) \
        do { QEvt e_; e_.sig = sig_; QMSM_DISPATCH(me_, &e_); } while (0)  // Macro to simple dispatch calls with signal only
#define PASS_EVENT_TO(obj_) \
        do { QMSM_DISPATCH(obj_, e);  } while (0)  // Macro with clear name

#ifdef __cplusplus
extern "C" {
#endif

QState QHsm_top(void *const me, const QEvt *const event);

void QHsm_ctor(QHsm *const me, QStateHandler initial);

void QMsm_init(QHsm *me, const QEvt *const event);
QState QMsm_dispatch(QHsm *me, const QEvt *const event);

#ifdef __cplusplus
}
#endif

#endif



#include <stddef.h>

const QEvt standard_events[] = {
    {(QSignal)(QEP_EMPTY_SIG_)},
    {(QSignal)(Q_ENTRY_SIG)},
    {(QSignal)(Q_EXIT_SIG)},
    {(QSignal)(Q_INIT_SIG)},
};

QState QHsm_top(void *const me, const QEvt *const event)
{
    (void)(me);
    (void)(event);

    return (QState)(Q_RET_IGNORED);
}

static void do_transition(QHsm *me)
{
    QStateHandler source = me->current_;
    QStateHandler effective = me->effective_;
    QStateHandler target = me->target_;

    while (source != effective) {
        source(me, &standard_events[Q_EXIT_SIG]);
        source(me, &standard_events[QEP_EMPTY_SIG_]);
        source = me->effective_;
    }

    if (source == target) {
        source(me, &standard_events[Q_EXIT_SIG]);
        target(me, &standard_events[Q_ENTRY_SIG]);

        me->current_ = target;
        me->effective_ = target;
        me->target_ = NULL;
        return;
    }

    QStateHandler path[Q_MAX_DEPTH];
    ptrdiff_t top = 0;
    ptrdiff_t lca = -1;

    path[0] = target;

    while (target != &QHsm_top) {
        target(me, &standard_events[QEP_EMPTY_SIG_]);
        target = me->effective_;
        path[++top] = target;

        if (target == source) {
            lca = top;
            break;
        }
    }

    while (lca == -1) {
        source(me, &standard_events[Q_EXIT_SIG]);
        source(me, &standard_events[QEP_EMPTY_SIG_]);
        source = me->effective_;

        for (ptrdiff_t i = 0; i <= top; ++i) {
            if (path[i] == source) {
                lca = i;
                break;
            }
        }
    }

    target = path[lca];

    for (ptrdiff_t i = lca - 1; i >= 0; --i) {
        target = path[i];
        target(me, &standard_events[Q_ENTRY_SIG]);
    }

    me->current_ = target;
    me->effective_ = target;
    me->target_ = NULL;
}

void QHsm_ctor(QHsm *const me, QStateHandler initial)
{
    me->current_ = initial;
    me->effective_ = initial;
    me->target_ = NULL;
}

void QMsm_init(QHsm *me, const QEvt *const event)
{
    me->current_(me, event);

    me->effective_ = &QHsm_top;
    do_transition(me);
}

QState QMsm_dispatch(QHsm *me, const QEvt *const event)
{
    QState result = me->current_(me, event);

    while (result == Q_RET_SUPER) {
        result = me->effective_(me, event);
    }

    switch (result) {
    case (QState)(Q_RET_TRAN):
        do_transition(me);
        break;
    case (QState)(Q_RET_HANDLED):
    case (QState)(Q_RET_UNHANDLED):
    case (QState)(Q_RET_IGNORED):
        me->effective_ = me->current_;
        break;
    default:
        break;
    }

    return result;
}

QState QMsm_simple_dispatch(QHsm *me, QSignal signal)
{
    const QEvt event = {signal};
    return QMsm_dispatch(me, &event);
}
