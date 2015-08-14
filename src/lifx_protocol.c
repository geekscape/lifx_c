/**
 * Please do not remove the following notices.
 *
 * \file       lifx_protocol.c
 * \author     Andy Gelme <andyg@geekscape.org>
 * \copyright  (c) 2015 by Geekscape Pty. Ltd.
 * \license    AGPLv3 http://geekscape.org/static/aiko_license.html
 *
 * Description
 * ~~~~~~~~~~~
 * LIFX LAN protocol implementation
 *
 * To Do
 * ~~~~~
 * - None, yet.
 */

#include "aiko_engine.h"
#include "aiko_network.h"

#include "lifx.h"

static int lifx_socket_fd;

static lifx_discovery_handler_t *lifx_discovery_callback = NULL;

uint8_t lifx_discovery_timer_handler(
  void *timer_self) {

  lifx_message_t *message = lifx_create_device_get_service();
  lifx_message_send(lifx_socket_fd, & lifx_targets_all, message, LIFX_RETRIES);
  free(message);

  return(AIKO_HANDLED);
}

uint8_t ATTRIBUTES
lifx_initialize(
  lifx_discovery_handler_t *lifx_discovery_handler) {

  lifx_socket_fd = udp_create_socket(1, LIFX_UDP_PORT);

  aiko_add_handler(
    aiko_create_source(AIKO_SOURCE_SOCKET_UDP4, lifx_socket_fd),
    lifx_message_handler
  );

  if (lifx_discovery_handler != NULL) {
    lifx_discovery_callback = lifx_discovery_handler;

    aiko_time_t   period = { LIFX_DISCOVERY_PERIOD, 0 };
    aiko_timer_t *timer = aiko_add_timer(
      & period, lifx_discovery_timer_handler
    );

    lifx_discovery_timer_handler(timer);
  }

  return(0);
}
