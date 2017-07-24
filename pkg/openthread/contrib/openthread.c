/*
 * Copyright (C) 2017 Fundacion Inria Chile
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 * @ingroup     net
 * @file
 * @brief       Implementation of OpenThread main functions
 *
 * @author      Jose Ignacio Alamos <jialamos@uc.cl>
 * @}
 */

#include <assert.h>

#include "openthread/platform/alarm.h"
#include "openthread/platform/uart.h"
#include "ot.h"
#include "random.h"
#include "thread.h"
#include "xtimer.h"

#ifdef MODULE_AT86RF2XX
#include "at86rf2xx.h"
#include "at86rf2xx_params.h"
#endif

#ifdef MODULE_CC2538_RF
#include "cc2538_rf.h"
#endif

#define ENABLE_DEBUG (1)
#include "debug.h"

#ifdef MODULE_AT86RF2XX     /* is mutual exclusive with above ifdef */
#define OPENTHREAD_NETIF_NUMOF        (sizeof(at86rf2xx_params) / sizeof(at86rf2xx_params[0]))
#endif

#ifdef MODULE_AT86RF2XX
static at86rf2xx_t at86rf2xx_dev;
#endif

#ifdef MODULE_CC2538_RF
static cc2538_rf_t cc2538_rf_dev;
#endif

//#define OPENTHREAD_NETDEV_BUFLEN (ETHERNET_MAX_LEN)
#define OPENTHREAD_NETDEV_BUFLEN   127

static uint8_t rx_buf[OPENTHREAD_NETDEV_BUFLEN];
static uint8_t tx_buf[OPENTHREAD_NETDEV_BUFLEN];
static char ot_thread_stack[2 * THREAD_STACKSIZE_MAIN];

/* init and run OpeanThread's UART simulation (stdio) */
void openthread_uart_run(void)
{
    char buf[256];
    msg_t msg;

    msg.type = OPENTHREAD_SERIAL_MSG_TYPE_EVENT;
    msg.content.ptr = buf;

    buf[1] = 0;
    while (1) {
        char c = getchar();
        buf[0] = c;
        msg_send(&msg, openthread_get_pid());
    }
}

void openthread_bootstrap(void)
{
    /* init random */
    ot_random_init();

    /* setup netdev modules */
#ifdef MODULE_AT86RF2XX
    at86rf2xx_setup(&at86rf2xx_dev, &at86rf2xx_params[0]);
    netdev_t *netdev = (netdev_t *) &at86rf2xx_dev;
#endif


#ifdef MODULE_CC2538_RF
    cc2538_setup(&cc2538_rf_dev);
    DEBUG("cc2538_setup()...\n");
    netdev_t *netdev = (netdev_t *) &cc2538_rf_dev;
#endif

    openthread_radio_init(netdev, tx_buf, rx_buf);
    openthread_netdev_init(ot_thread_stack, sizeof(ot_thread_stack), THREAD_PRIORITY_MAIN - 2, "openthread", netdev);
    //openthread_netdev_init(ot_thread_stack, sizeof(ot_thread_stack), THREAD_PRIORITY_MAIN + 1, "openthread", netdev);

}
