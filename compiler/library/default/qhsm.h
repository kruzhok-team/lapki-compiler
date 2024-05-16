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

// максимальная глубина состояний относительно вверха (top)
#define Q_MAX_DEPTH 8

// сигнал
typedef int QSignal;
// состояние
typedef int QState;

// событие
typedef struct
{
    QSignal sig;
} QEvt;

// состояние
typedef QState (*QStateHandler)(void *const me, const QEvt *const event);

// сигналы
enum
{
    // пустой сигнал
    QEP_EMPTY_SIG_ = 0,
    // сигнал входа
    Q_ENTRY_SIG,
    // сигнал выхода
    Q_EXIT_SIG,
    // начальный сигнал
    Q_INIT_SIG,
    // пользовательский сигнал
    Q_USER_SIG,
};

// возращаемые сигналы (RETurn)
enum
{
    // ?
    Q_RET_SUPER,
    // необработано
    Q_RET_UNHANDLED,
    // обработано
    Q_RET_HANDLED,
    // проигнорировано
    Q_RET_IGNORED,
    // трансформировано (переходное состояние)
    Q_RET_TRAN,
};

// структура, обозначающая иерархическую машину состояний
typedef struct
{
    // текущее состояние
    QStateHandler current_;
    // родительское состояние (?)
    QStateHandler effective_;
    // цель для перехода
    QStateHandler target_;
} QHsm;

// создание указателя из более конкретного типа к более общему QHsm (upcast)
// позволяет убедиться, что в коде не используются лишние, конкретные свойства, что будет мешать в будущем обобщить код
#define Q_MSM_UPCAST(me) ((QHsm *)(me))
// преобразование объекта handler в тип QStateHandler
#define Q_STATE_CAST(handler) ((QStateHandler)(handler))

// макрос, который приводит Q_RET_UNHANDLED к типу QState
#define Q_UNHANDLED() ((QState)(Q_RET_UNHANDLED))
// макрос, который приводит Q_RET_HANDLED к типу QState
#define Q_HANDLED() ((QState)(Q_RET_HANDLED))

// приведение target 
#define Q_TRAN(target) \
    ((Q_MSM_UPCAST(me))->target_ = Q_STATE_CAST(target), \
    (QState)(Q_RET_TRAN))
#define Q_SUPER(super) \
    ((Q_MSM_UPCAST(me))->effective_ = Q_STATE_CAST(super), \
    (QState)(Q_RET_SUPER))

// инициализация QMsm из QHsm (me) и события (event)
#define QMSM_INIT(me, event) (QMsm_init(me, event))
// отправка события (event) в QHsm (me)
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

// Глобальное состояние по умолчанию
QState QHsm_top(void *const me, const QEvt *const event);
// конструктор QHsm 
void QHsm_ctor(QHsm *const me, QStateHandler initial);

// инициализация QMsm из QHsm (me) и события (event)
void QMsm_init(QHsm *me, const QEvt *const event);
// отправка события (event) в QHsm (me)
QState QMsm_dispatch(QHsm *me, const QEvt *const event);

#ifdef __cplusplus
}
#endif

#endif
