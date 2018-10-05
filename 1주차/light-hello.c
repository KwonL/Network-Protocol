/* This is a very simple hello_world program.
 * It aims to demonstrate the co-existence of two processes:
 * One of them prints a hello world message and the other blinks the LEDs
 *
 * It is largely based on hello_world in $(CONTIKI)/examples/sensinode
 *
 * Author: George Oikonomou <G.Oikonomou@lboro.ac.uk>
 */

#include "contiki.h"
#include "dev/leds.h"
#include "dev/light-sensor.h"

#include <stdio.h>
/*---------------------------------------------------------------------------*/
PROCESS(light_process, "light sensor process");

AUTOSTART_PROCESSES(&light_process);
/*---------------------------------------------------------------------------*/
/*
 * This function return calculated sensor value (lux)
 * maybe unit is incorrect..cannot find reference
 */
static int get_light(void) {
  // Return calculated value
  return 10 * light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC) / 7;
}
/*---------------------------------------------------------------------------*/
/*
 * This process get value from sensor every 1 second
 */
PROCESS_THREAD(light_process, ev, data)
{
  static struct etimer timer;
  static int count = 0;
  int light = 0;
  static char buf[100];

  PROCESS_BEGIN();

  // First, Activate sensor
  SENSORS_ACTIVATE(light_sensor);

  while(1) {
    // Set interval as 1 second
    etimer_set(&timer, CLOCK_CONF_SECOND);

    // Wait etimer 
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
		light = get_light();
		
    // print second and light
		printf("%d %d\n", count, light);
    count ++;
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
