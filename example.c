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

#include "global.h"



/**
 * @addtogroup Static func
 * @note none
 */
 
/*@{*/

#include "./upt.h"

UPT_EventFlag_t flag;

UPT_THREAD_SAMPLE(testThread0, {
    UPT_FLAG_INIT(&flag);

    LOG_I("THREAD-0-BEGIN");
    
    for (;;) {
        UPT_DELAY(1000);
        LOG_I("THREAD-0");
    }
}, {
});


UPT_THREAD_SAMPLE(testThread1, {
    UPT_DELAY(1);
    LOG_I("THREAD-1");
}, {
});


uint32_t mk = 0x00000001u;
UPT_THREAD_SAMPLE(testThread2, {
    int32_t flagResult = 0;
    
    UPT_FLAG_WAIT_ANY(&flag, 0x00000001u, 1, 2000, &flagResult);
    if (flagResult > 0 && flagResult & 0x00000001u) { 
        LOG_I("THREAD-3-RECV-ANY");
    }
    
    UPT_FLAG_WAIT_ALL(&flag, (0x00000001u | 0x00000002u), 0, 100, &flagResult);
    if (flagResult > 0 && flagResult == (0x00000001u | 0x00000002u)) { 
        LOG_I("THREAD-3-RECV-ALL");
    }
}, {
});

/*@}*/

/**
 * @addtogroup sys thread
 * @note none
 */

static void threadInit(void *arg) {
    UPT_START(testThread0);
    UPT_START(testThread1);
    UPT_START(testThread2);
    UPT_POLL(
        UPT_SCHEDULE(testThread0)
        | UPT_SCHEDULE(testThread1)
        | UPT_SCHEDULE(testThread2), {
            osDelay(1);
        });
}

/*@}*/

/**
 * @addtogroup Main
 * @note none
 */
 
/*@{*/

int main(void) {
    kpwr_sysInit(KPWR_RUN_HS);
    
    osKernelInitialize();
    osThreadNew(threadInit, NULL, NULL);
    osKernelStart();
}


/*@}*/