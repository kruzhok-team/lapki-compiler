#include "qhsm.h"

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
    State* source = me->current_;
    State* effective = me->effective_;
    State* target = me->target_;

    while (source != effective) {
        source->process_state_func(me, &standard_events[Q_EXIT_SIG]);
        source->process_state_func(me, &standard_events[QEP_EMPTY_SIG_]);
        source = me->effective_;
    }

    if (source == target) {
        source->process_state_func(me, &standard_events[Q_EXIT_SIG]);
        target->process_state_func(me, &standard_events[Q_ENTRY_SIG]);

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
        target->process_state_func(me, &standard_events[QEP_EMPTY_SIG_]);
        target = me->effective_;
        path[++top] = target;

        if (target == source) {
            lca = top;
            break;
        }
    }

    while (lca == -1) {
        source->process_state_func(me, &standard_events[Q_EXIT_SIG]);
        source->process_state_func(me, &standard_events[QEP_EMPTY_SIG_]);
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
        target->process_state_func(me, &standard_events[Q_ENTRY_SIG]);
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
    me->current_->process_state_func(me, event);

    me->effective_ = &{ &QHsm_top, NULL };
    do_transition(me);
}

QState QMsm_dispatch(QHsm *me, const QEvt *const event)
{
    QState result = me->current_->process_state_func(me, event);

    while (result == Q_RET_SUPER) {
        result = me->effective_->process_state_func(me, event);
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
