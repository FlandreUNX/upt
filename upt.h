/*
 * Copyright (C) 2020 Flandreunx@outlook.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

////
//// Author: Adam Dunkels <adam@sics.se>
////

#ifndef _UPT_H_
#define _UPT_H_

#include <stdint.h>

/**
 * @addtogroup Define
 * @note none
 */
 
/*@{*/

#define _UPT_MAJOR_VERSION        "A"
#define _UPT_SECONDARY_VERSION    "03"
#define UPT_VERSION                _UPT_MAJOR_VERSION "." _UPT_SECONDARY_VERSION

#define _UPT_DELAY_COUNT_TYPE         uint32_t
#define _UPT_SEMAPHORE_COUNT_TYPE     uint8_t
#define _UPT_FLAG_INDEX_TYPE          uint32_t

#define _ISWITCH_INIT(s) \
    s = 0;
    
#define _ISWITCH_RESUME(s) \
    switch (s) { \
        case 0: \
      
#define _ISWITCH_SET(s) \
    s = __LINE__; case __LINE__:

#define _ISWITCH_END(s) \
    }

/*@}*/

/**
 * @addtogroup Typdef
 * @note none
 */
 
/*@{*/
    
typedef enum {
    UPT_HANDLER_RETURN_WAITING,
    UPT_HANDLER_RETURN_YIELDED,
    UPT_HANDLER_RETURN_SUSPEND,
    UPT_HANDLER_RETURN_EXITED,
} UPT_HandlerReturn_t;
    
typedef enum {
    UPT_HANDLER_STATE_NOINIT,
    UPT_HANDLER_STATE_READY,
    UPT_HANDLER_STATE_WAITING,
    UPT_HANDLER_STATE_SUSPEND,
    UPT_HANDLER_STATE_EXITED,
} UPT_HandlerState_t;

typedef struct UPT {
    uint16_t ix;
    UPT_HandlerState_t state;
    _UPT_DELAY_COUNT_TYPE timeout;
} UPT_Handler_t;

typedef struct {
    _UPT_SEMAPHORE_COUNT_TYPE count;
} UPT_Semaphore_t;

typedef struct {
    uint8_t count;
} UPT_MessageBox_t;

typedef struct {
    volatile _UPT_FLAG_INDEX_TYPE flag;
} UPT_EventFlag_t;

/*@}*/

/**
 * @addtogroup Protect IX functions
 * @note none
 */
 
/*@{*/

////
//// Declare the start of a protothread inside the C function implementing the protothread
////
#define _UPT_BEGIN() { \
    _ISWITCH_RESUME((upt)->ix)
    
    
////
//// Declare the end of a protothread
////
#define _UPT_END() \
    _ISWITCH_END((upt)->ix); \
    UPT_EXIT(); \
}

/*@}*/

/**
 * @addtogroup Initialize functions
 * @note none
 */
 
/*@{*/

////
//// Extern handler
////
#define UPT_EXTERN(name) \
    extern UPT_Handler_t gUPTHandler_##name; \
    extern UPT_HandlerReturn_t _UPTThreadFunc_##name(UPT_Handler_t *upt);


////
//// Declaration and definition
////
#define UPT_THREAD(name) \
    UPT_Handler_t gUPTHandler_##name = { \
        .state = UPT_HANDLER_STATE_NOINIT, \
    }; \
    UPT_HandlerReturn_t _UPTThreadFunc_##name(UPT_Handler_t *upt)

    
#define UPT_THREAD_SAMPLE(name, onPoll, onSchedule, onExit) \
    UPT_Handler_t gUPTHandler_##name = { \
        .state = UPT_HANDLER_STATE_NOINIT, \
    }; \
    UPT_HandlerReturn_t _UPTThreadFunc_##name(UPT_Handler_t *upt) { \
        onPoll \
        _UPT_BEGIN(); \
        onSchedule \
        onExit \
        _UPT_END(); \
    }
    
/*@}*/
    
/**
 * @addtogroup Local functions
 * @note none
 */
 
/*@{*/

////
//// Lock and wait until condition is true.
////
#define UPT_WAIT_UNTIL(condition) do { \
    _ISWITCH_SET(upt->ix); \
    if (!(condition)) { \
        return UPT_HANDLER_RETURN_WAITING; \
    } \
} while (0)


////
//// Lock and wait timeout
////
#define UPT_DELAY(_timeout) do { \
    _ISWITCH_SET(upt->ix); \
    if (upt->state == UPT_HANDLER_STATE_WAITING) { \
        if (--upt->timeout > 0) { \
            return UPT_HANDLER_RETURN_WAITING; \
        } else { \
            upt->timeout = 0; \
            upt->state = UPT_HANDLER_STATE_READY; \
        } \
    } else { \
        if (_timeout != 0) { \
            upt->timeout = _timeout; \
            upt->state = UPT_HANDLER_STATE_WAITING; \
            return UPT_HANDLER_RETURN_WAITING; \
        } \
    } \
} while (0)


//// 
//// Yield from the current protothread.
////
#define UPT_YIELD() do { \
    _ISWITCH_SET(upt->ix); \
    return UPT_HANDLER_RETURN_YIELDED; \
} while (0)


//// 
//// Suspend from the current protothread.
////
#define UPT_SUSPEND() do { \
    _ISWITCH_SET(upt->ix); \
    upt->state = UPT_HANDLER_STATE_SUSPEND; \
    return UPT_HANDLER_RETURN_SUSPEND; \
} while (0)


////
//// Exit the protothread.
////
#define UPT_EXIT() do { \
    upt->state = UPT_HANDLER_STATE_EXITED; \
    return UPT_HANDLER_RETURN_EXITED; \
} while (0)

/*@}*/

/**
 * @addtogroup Hierarchical protothreads funtions
 * @note none
 */
 
/*@{*/

////
//// Start a protothread
////
#define UPT_START(threadName) do { \
    extern UPT_Handler_t gUPTHandler_##threadName; \
    if (gUPTHandler_##threadName.state == UPT_HANDLER_STATE_NOINIT) { \
        _ISWITCH_INIT((&(gUPTHandler_##threadName))->ix); \
        gUPTHandler_##threadName.timeout = 0; \
        gUPTHandler_##threadName.state = UPT_HANDLER_STATE_READY; \
    } \
} while (0)


////
//// Restart the protothread.
////
#define UPT_RESTART(threadName) do { \
    extern UPT_Handler_t gUPTHandler_##threadName; \
    if (gUPTHandler_##threadName.state == UPT_HANDLER_STATE_EXITED) { \
        _ISWITCH_INIT((&(gUPTHandler_##threadName))->ix); \
        gUPTHandler_##threadName.timeout = 0; \
        gUPTHandler_##threadName.state = UPT_HANDLER_STATE_READY; \
    } \
} while (0)


////
//// Resume the protothread.
////
#define UPT_RESUME(threadName) do { \
    extern UPT_Handler_t gUPTHandler_##threadName; \
    if (gUPTHandler_##threadName.state == UPT_HANDLER_STATE_SUSPEND) { \
        gUPTHandler_##threadName.state = UPT_HANDLER_STATE_READY; \
    } \
} while (0)

/*@}*/

/**
 * @addtogroup Callable a protothread functions
 * @note none
 */
 
/*@{*/

////
//// Schedule a protothread.
////
#define UPT_POLL(x, delayOperate) \
    while (x) delayOperate \
    
#define UPT_SCHEDULE(threadName) \
    ((gUPTHandler_##threadName.state <= UPT_HANDLER_STATE_WAITING \
            ? _UPTThreadFunc_##threadName(&gUPTHandler_##threadName) \
            : UPT_HANDLER_RETURN_EXITED) \
        != UPT_HANDLER_RETURN_EXITED)

/*@}*/

/**
 * @addtogroup Semaphore functions
 * @note none
 */
 
/*@{*/

////
//// Initialize a semaphore
////
#define UPT_SEM_INIT(s, c) do { \
    (s)->count = 0; \
} while (0)


////
//// Wait for a semaphore
////
#define UPT_SEM_WAIT(s, delay, result) do { \
    _ISWITCH_SET(upt->ix); \
    if (upt->state == _UPT_DELAY_STATE_RUNNING) { \
        if ((--upt->timeout > 0) && ((s)->count > 0)) { \
            return _UPT_RETURN_WAITING; \
        } else { \
            if (((s)->count == 0) { \
                *result = 0; \
            } else if (upt->timeout <= 0) { \
                *result = -1; \
            } \
            upt->state = _UPT_DELAY_STATE_READY; \
            upt->timeout = 0; \
        } \
    } else { \
        if (((s)->count == 0) { \
            *result = 0; \
        } else { \
            if (delay != 0) { \
                upt->state = _UPT_DELAY_STATE_RUNNING; \
                upt->timeout = delay; \
                return _UPT_RETURN_WAITING; \
            } \
        } \
    } \
} while (0)


////
//// Signal a semaphore
////
#define UPT_SEM_SIGNAL(s) do { \
    ++(s)->count; \
} while (0)
    

////
//// Clear a semaphore
////
#define UPT_SEM_RELEASE(s) do { \
    if ((s)->count == 0) { \
        break; \
    } \
    --(s)->count; \
} while (0)

/*@}*/

/**
 * @addtogroup Event flag functions
 * @note none
 */
 
/*@{*/

////
//// Initialize a flag
////
#define UPT_FLAG_INIT(f) do { \
    (f)->flag = 0; \
} while (0)


////
//// Wait for a flag with any
////
#define UPT_FLAG_WAIT_ANY(f, mark, noClear, _timeout, result) do { \
    _ISWITCH_SET(upt->ix); \
    if (upt->state == UPT_HANDLER_STATE_WAITING) { \
        if ((--upt->timeout > 0) && (((f)->flag & mark) == 0)) { \
            return UPT_HANDLER_RETURN_WAITING; \
        } else { \
            if (((f)->flag & mark) != 0) { \
                *result = (f)->flag & mark; \
                if (!noClear) { \
                    UPT_FLAG_CLEAR(f, mark); \
                } \
            } else if (upt->timeout <= 0) { \
                *result = -1; \
            } \
            upt->timeout = 0; \
            upt->state = UPT_HANDLER_STATE_READY; \
        } \
    } else { \
        if (((f)->flag & mark) != 0) { \
            *result = (f)->flag & mark; \
            if (!noClear) { \
                UPT_FLAG_CLEAR(f, mark); \
            } \
        } else { \
            if (_timeout != 0) { \
                upt->timeout = _timeout; \
                upt->state = UPT_HANDLER_STATE_WAITING; \
                return UPT_HANDLER_RETURN_WAITING; \
            } \
        } \
    } \
} while (0)


////
//// Wait for a flag with all
////
#define UPT_FLAG_WAIT_ALL(f, mark, noClear, _timeout, result) do { \
    _ISWITCH_SET(upt->ix); \
    if (upt->state == UPT_HANDLER_STATE_WAITING) { \
        if ((--upt->timeout > 0) && (((f)->flag & mark) != mark)) { \
            return UPT_HANDLER_RETURN_WAITING; \
        } else { \
            *result = 0; \
            if (((f)->flag & mark) == mark) { \
                *result = mark; \
                if (!noClear) { \
                    UPT_FLAG_CLEAR(f, mark); \
                } \
            } else if (upt->timeout <= 0) { \
                *result = -1; \
            } \
            upt->timeout = 0; \
            upt->state = UPT_HANDLER_STATE_READY; \
        } \
    } else { \
        if (((f)->flag & mark) == mark) { \
            *result = mark; \
            if (!noClear) { \
                UPT_FLAG_CLEAR(f, mark); \
            } \
        } else { \
            if (_timeout != 0) { \
                upt->timeout = _timeout; \
                upt->state = UPT_HANDLER_STATE_WAITING; \
                return UPT_HANDLER_RETURN_WAITING; \
            } \
        } \
    } \
} while (0)

////
//// Set a flag
////
#define UPT_FLAG_SET(f, mark) do { \
    (f)->flag |= mark; \
} while (0)
    

////
//// Clear a flag
////
#define UPT_FLAG_CLEAR(f, mark) do { \
    (f)->flag &= ~mark; \
} while (0)

/*@}*/

/**
 * @addtogroup Message functions
 * @note none
 */
 
/*@{*/

/*@}*/

#endif
