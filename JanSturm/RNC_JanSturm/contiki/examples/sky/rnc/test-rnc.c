#include "contiki.h"
#include "net/seemoo-lab/rnc/params.h"
#include "net/seemoo-lab/rnc/rnc.h"
#ifdef POWERTRACE
#include "powertrace.h"
#endif
#include <stdio.h>

/*---------------------------------------------------------------------------*/
PROCESS(test_rnc_process, "Test RNC process");
AUTOSTART_PROCESSES(&test_rnc_process);

PROCESS_THREAD(test_rnc_process, ev, data) {
  static struct etimer et;
  PROCESS_BEGIN();

#if defined(DEBUG) || defined(DEMO)
	printf("\n@@@@ console output affects behaviour and timings dramatically! @@@@\n");
#endif
  init_rnc();
  // wait for 1 second to initialize nodes
  etimer_set(&et, CLOCK_SECOND);

  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
#ifdef POWERTRACE
  powertrace_start(CLOCK_SECOND * 2);
#endif
  start_rnc();

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
