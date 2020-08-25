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
#define _UPT_SECONDARY_VERSION    "02"
#define UPT_VERSION                _UPT_MAJOR_VERSION "." _UPT_SECONDARY_VERSION

#define _UPT_WAITING         0
#define _UPT_YIELDED         1
#define _UPT_EXITED          2
#define _UPT_ENDED           3

#define _UPT_DELAY_STATE_READY     0
#define _UPT_DELAY_STATE_RUNNING   1


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

typedef struct UPT {
    uint16_t ix;
    uint8_t delayState;
    int32_t timeout;
} UPT_t;

typedef struct {
    uint8_t count;
} UPT_Sem_t;

typedef volatile uint32_t UPT_Flag_t;

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
    extern UPT_t UPT_HANDLER_##name; \
    extern UPT_THREAD_##name(UPT_t *upt);

////
//// Initialize a protothread
////
#define UPT_INIT(name) do { \
    extern UPT_t UPT_HANDLER_##name; \
    _ISWITCH_INIT((&(UPT_HANDLER_##name))->ix); \
    (&(UPT_HANDLER_##name))->timeout = 0; \
    (&(UPT_HANDLER_##name))->delayState = _UPT_DELAY_STATE_READY; \
} while (0)


////
//// Declaration and definition
////
#define UPT_THREAD(name) \
    UPT_t UPT_HANDLER_##name; \
    int32_t UPT_THREAD_##name(UPT_t *upt)


////
//// Declare the start of a protothread inside the C function implementing the protothread
////
#define UPT_BEGIN() { \
    uint8_t _uptYieldFlag = 1; \
    _ISWITCH_RESUME((upt)->ix)
    
    
////
//// Declare the end of a protothread
////
#define UPT_END() \
    _ISWITCH_END((upt)->ix); \
    _uptYieldFlag = 0; \
    _ISWITCH_INIT((upt)->ix); \
    (upt)->timeout = 0; \
    (upt)->delayState = _UPT_DELAY_STATE_READY; \
    return _UPT_ENDED; \
}

/*@}*/
    
/**
 * @addtogroup Block functions
 * @note none
 */
 
/*@{*/

////
//// Lock and wait until condition is true.
////
#define UPT_WAIT_UNTIL(condition) do { \
    _ISWITCH_SET(upt->ix); \
    if (!(condition)) { \
        return _UPT_WAITING; \
    } \
} while (0)


////
//// Block and wait while condition is true.
////
#define UPT_WAIT_WHILE(cond) \
    UPT_WAIT_UNTIL(!(cond))

////
//// Lock and wait timeout
////
#define UPT_DELAY(_timeout) do { \
    _ISWITCH_SET(upt->ix); \
    if (upt->delayState == _UPT_DELAY_STATE_RUNNING) { \
        if (--upt->timeout > 0) { \
            return _UPT_WAITING; \
        } else { \
            upt->delayState = _UPT_DELAY_STATE_READY; \
            upt->timeout = 0; \
        } \
    } else { \
        upt->delayState = _UPT_DELAY_STATE_RUNNING; \
        upt->timeout = _timeout; \
        return _UPT_WAITING; \
    } \
} while (0)

/*@}*/

/**
 * @addtogroup Hierarchical protothreads funtions
 * @note none
 */
 
/*@{*/

////
//// Block and wait until a child protothread completes.
////
#define UPT_WAIT_THREAD(thread) \
    UPT_WAIT_WHILE(UPT_SCHEDULE(thread))


////
//// Spawn a child protothread and wait until it exits.
////
#define UPT_SPAWN(nameChild, thread) do { \
    extern UPT_t UPT_HANDLER_##nameChild; \
    _ISWITCH_INIT(UPT_HANDLER_##nameChild.ix); \
    UPT_WAIT_THREAD((thread)); \
} while (0)

/*@}*/

/**
 * @addtogroup Exiting and restarting functions
 * @note none
 */
 
/*@{*/

////
//// Restart the protothread.
////
#define UPT_RESTART(name) do { \
    extern UPT_t UPT_HANDLER_##name; \
    _ISWITCH_INIT(UPT_HANDLER_##name.ix); \
    return _UPT_WAITING; \
} while (0)


////
//// Exit the protothread.
////
#define UPT_EXIT() do { \
    _ISWITCH_INIT(upt->ix); \
    return _UPT_EXITED; \
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
#define UPT_SCHEDULE(name) \
    ((UPT_THREAD_##name(&UPT_HANDLER_##name)) < _UPT_EXITED)

/*@}*/

/**
 * @addtogroup Yielding from a protothread functions
 * @note none
 */
 
/*@{*/

//// 
//// Yield from the current protothread.
////
#define UPT_YIELD() do { \
    _uptYieldFlag = 0; \
    _ISWITCH_SET(upt->ix); \
    if (_uptYieldFlag == 0) { \
        return _UPT_YIELDED; \
    } \
} while (0)


//// 
//// Yield from the protothread until a condition occurs.
////
#define UPT_YIELD_UNTIL(cond) do { \
    _uptYieldFlag = 0; \
    _ISWITCH_SET(upt->ix); \
    if ((_uptYieldFlag == 0) || !(cond)) {	\
        return _UPT_YIELDED; \
    } \
} while (0)

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
    if (upt->delayState == _UPT_DELAY_STATE_RUNNING) { \
        if ((--upt->timeout > 0) && ((s)->count > 0)) { \
            return _UPT_WAITING; \
        } else { \
            if (((s)->count == 0) { \
                *result = 0; \
            } else if (upt->timeout <= 0) { \
                *result = -1; \
            } \
            upt->delayState = _UPT_DELAY_STATE_READY; \
            upt->timeout = 0; \
        } \
    } else { \
        if (((s)->count == 0) { \
            *result = 0; \
        } else { \
            upt->delayState = _UPT_DELAY_STATE_RUNNING; \
            upt->timeout = delay; \
            return _UPT_WAITING; \
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
 * @addtogroup Flag functions
 * @note none
 */
 
/*@{*/

////
//// Initialize a flag
////
#define UPT_FLAG_INIT(f) do { \
    (*f) = 0; \
} while (0)


////
//// Wait for a flag with any
////
#define UPT_FLAG_WAIT_ANY(f, mark, noClear, delay, result) do { \
    _ISWITCH_SET(upt->ix); \
    if (upt->delayState == _UPT_DELAY_STATE_RUNNING) { \
        if ((--upt->timeout > 0) && (((*f) & mark) == 0)) { \
            return _UPT_WAITING; \
        } else { \
            if (((*f) & mark) != 0) { \
                *result = (*f) & mark; \
                if (!noClear) { \
                    UPT_FLAG_CLEAR(f, mark); \
                } \
            } else if (upt->timeout <= 0) { \
                *result = -1; \
            } \
            upt->delayState = _UPT_DELAY_STATE_READY; \
            upt->timeout = 0; \
        } \
    } else { \
        if (((*f) & mark) != 0) { \
            *result = (*f) & mark; \
            if (!noClear) { \
                UPT_FLAG_CLEAR(f, mark); \
            } \
        } else { \
            upt->delayState = _UPT_DELAY_STATE_RUNNING; \
            upt->timeout = delay; \
            return _UPT_WAITING; \
        } \
    } \
} while (0)


////
//// Wait for a flag with all
////
#define UPT_FLAG_WAIT_ALL(f, mark, noClear, delay, result) do { \
    _ISWITCH_SET(upt->ix); \
    if (upt->delayState == _UPT_DELAY_STATE_RUNNING) { \
        if ((--upt->timeout > 0) && (((*f) & mark) != mark)) { \
            return _UPT_WAITING; \
        } else { \
            *result = 0; \
            if (((*f) & mark) == mark) { \
                *result = mark; \
                if (!noClear) { \
                    UPT_FLAG_CLEAR(f, mark); \
                } \
            } else if (upt->timeout <= 0) { \
                *result = -1; \
            } \
            upt->delayState = _UPT_DELAY_STATE_READY; \
            upt->timeout = 0; \
        } \
    } else { \
        if (((*f) & mark) == mark) { \
            *result = mark; \
            if (!noClear) { \
                UPT_FLAG_CLEAR(f, mark); \
            } \
        } else { \
            upt->delayState = _UPT_DELAY_STATE_RUNNING; \
            upt->timeout = delay; \
            return _UPT_WAITING; \
        } \
    } \
} while (0)

////
//// Set a flag
////
#define UPT_FLAG_SET(f, mark) do { \
    (*f) |= mark; \
} while (0)
    

////
//// Clear a flag
////
#define UPT_FLAG_CLEAR(f, mark) do { \
    (*f) &= ~mark; \
} while (0)

/*@}*/

#endif
