/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         Testing the broadcast layer in Rime
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "net/rime.h"
#include "random.h"

#include "dev/button-sensor.h"

#include "dev/leds.h"
#include "dev/light-sensor.h"

#include "dev/cc2420.h"

#include <stdio.h>
#include <string.h>

struct message {
  uint8_t seqno;
};
/*---------------------------------------------------------------------------*/
PROCESS(example_broadcast_process, "Broadcast example");
AUTOSTART_PROCESSES(&example_broadcast_process);
/*---------------------------------------------------------------------------*/

struct message msg;

static void
broadcast_recv(struct broadcast_conn *c, const rimeaddr_t *from)
{
  memcpy(&msg, packetbuf_dataptr(), sizeof(struct message));

  printf("broadcast message received from %d.%d: count ( %u )\n", from->u8[0], from->u8[1], msg.seqno);
	
	//increase global sequence number
	msg.seqno++;
	
	// Continue broadcast sending
	process_post(&example_broadcast_process, PROCESS_EVENT_CONTINUE, NULL);
}
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_broadcast_process, ev, data)
{
  static struct etimer et_resend, et_delay;

  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();

  broadcast_open(&broadcast, 129, &broadcast_call);

  while(1) {		
		// delay for 5 seconds
		etimer_set(&et_delay, CLOCK_SECOND * 5);
		PROCESS_WAIT_EVENT_UNTIL(ev==PROCESS_EVENT_TIMER);

		// send broadcast packet
		packetbuf_copyfrom(&msg, sizeof(struct message));	
		broadcast_send(&broadcast);
		printf("broadcast message sent\n");

		// Resend packet until callback continue me
		etimer_set(&et_delay, CLOCK_SECOND * 10);
		// Timer expired or callback function wake up me!
		PROCESS_WAIT_EVENT();
		// while callback function send me event, resned packet
		while (!(ev==PROCESS_EVENT_CONTINUE)) {
			if (ev == PROCESS_EVENT_TIMER) {
				packetbuf_copyfrom(&msg, sizeof(struct message));	
				broadcast_send(&broadcast);
				printf("broadcast message resent\n");

				// reset timer for resend
				etimer_reset(&et_delay);
			}
			PROCESS_WAIT_EVENT();
		}
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
