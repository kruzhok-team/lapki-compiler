#include "qhsm.h"

#include <stddef.h>

// стандартные события
const QEvt standard_events[] = {
    // сигнал для кодирования пустых действий
    {(QSignal)(QEP_EMPTY_SIG_)},
    // сигнал для кодирования действий входа
    {(QSignal)(Q_ENTRY_SIG)},
    // сигнал для кодирования действий выхода
    {(QSignal)(Q_EXIT_SIG)},
    // сигнал для кодирования вложенных начальных переходоов
    {(QSignal)(Q_INIT_SIG)},
};

QState QHsm_top(void *const me, const QEvt *const event)
{
    (void)(me);
    (void)(event);

    return (QState)(Q_RET_IGNORED);
}

/*
Совершить преобразование (переход) внутри иерархической машины состояния.

После этого процесса, текущее (current) состояние становится равным родительскому (effective) состоянию, а целевое (target) состояние пропадает (NULL) 
*/ 
static void do_transition(QHsm *me)
{
    QStateHandler source = me->current_;
    QStateHandler effective = me->effective_;
    QStateHandler target = me->target_;

    /* 
    Текущее состояние (source) вызывается со стандартными событиями Q_EXIT_SIG и QEP_EMPTY_SIG_.
    Похоже, что эти вызовы могут повлиять на машину состояний me, таким образом, что его родительское состояние (effective) меняется
    (иначе не понятно, зачем идёт сравнение source с effective, если в конце source = me->effective_)
    */
    while (source != effective) {
        source(me, &standard_events[Q_EXIT_SIG]);
        source(me, &standard_events[QEP_EMPTY_SIG_]);
        source = me->effective_;
    }

    if (source == target) {
        /*
        Предыдущий блок кода гарантирует, что source == effective, следовательно 
        source == effective == target
        */
        source(me, &standard_events[Q_EXIT_SIG]);
        target(me, &standard_events[Q_ENTRY_SIG]);

        me->current_ = target;
        me->effective_ = target;
        me->target_ = NULL;
        return;
    }

    // Поиск наименьшего общего предка (LCA -  lowest common ancestor.)

    QStateHandler path[Q_MAX_DEPTH];
    // текущее расстояние от target
    ptrdiff_t top = 0;
    // наименьший общий предок (длина пути от target до source)
    ptrdiff_t lca = -1;

    path[0] = target;

    /*
    Обход и передача сигналов EP_EMPTY_SIG_ target и его предкам.

    Цикл выполняется до тех пор, пока не будет достигнута вершина машины состояния, 
    либо до тех пор пока не будет найден длина пути (top) от target до source.
    */ 
    while (target != &QHsm_top) {
        target(me, &standard_events[QEP_EMPTY_SIG_]);
        target = me->effective_;
        path[++top] = target;

        if (target == source) {
            lca = top;
            break;
        }
    }

    /*
    Обход и передача сигналов Q_EXIT_SIG и EP_EMPTY_SIG_ source и его предкам.

    Цикл выполняется только в том случае, если на предыдущем шаге не удалось определить
    расстояние от target до source.

    Значение top в этом случае должно равняться расстоянию от изначального значения target до QHsm_top.
    */
    while (lca == -1) {
        source(me, &standard_events[Q_EXIT_SIG]);
        source(me, &standard_events[QEP_EMPTY_SIG_]);
        source = me->effective_;
        /*
        Проверка наличия source на пути от target до QHsm_top.
        */
        for (ptrdiff_t i = 0; i <= top; ++i) {
            if (path[i] == source) {
                lca = i;
                break;
            }
        }
    }

    target = path[lca];
    // передача всем предшествующим до target состояниям сигнала Q_ENTRY_SIG
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
