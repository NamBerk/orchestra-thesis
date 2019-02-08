#include "contiki.h"
#include "net/seemoo-lab/flooding/flooding.h"
#include <stdio.h>


/*---------------------------------------------------------------------------*/
PROCESS(test_flooding_process, "Test Flooding process");
AUTOSTART_PROCESSES(&test_flooding_process);

PROCESS_THREAD(test_flooding_process, ev, data)
{
  static struct etimer et;
  PROCESS_BEGIN();

  init_flooding();

  // wait for 1 second to let the non-initiator nodes be prepared for reception.
  etimer_set(&et, CLOCK_SECOND);

  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  start_flooding();

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

