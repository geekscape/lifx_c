/**
 * Please do not remove the following notices.
 *
 * \file       lifx_message.c
 * \author     Andy Gelme <andyg@geekscape.org>
 * \copyright  (c) 2015 by Geekscape Pty. Ltd.
 * \license    AGPLv3 http://geekscape.org/static/aiko_license.html
 *
 * Description
 * ~~~~~~~~~~~
 * LIFX LAN message implementation
 *
 * To Do
 * ~~~~~
 * - None, yet.
 */

#include "aiko_engine.h"
#include "aiko_network.h"

#include "lifx.h"

lifx_message_t ATTRIBUTES
*lifx_create_message(
  uint16_t type,
  uint8_t  size) {

  lifx_message_t *message = (lifx_message_t *) malloc(size);

  memset(message, 0x00, size);
  message->size         = size;
  message->origin       = 0;     // UDP
  message->tagged       = 0;
  message->addressable  = 1;
  message->protocol     = LIFX_PROTOCOL_NUMBER;
  message->source       = 0;

  memset(message->target, 0x00, 8);
  message->ack_required = 0;
  message->res_required = 0;
  message->sequence     = 0;

  message->type         = type;

  return(message);
}

uint8_t ATTRIBUTES
lifx_message_handler(
  void     *aiko_stream,
  uint8_t  *message,
  uint16_t  length) {

  uint8_t handled = AIKO_NOT_HANDLED;

  if (length >= sizeof(lifx_message_t)) {
    lifx_message_t *lifx_message = (lifx_message_t *) message;

printf("lifx_message_handler(): type %d\n", lifx_message->type);

// TODO: Per LIFX message type handler, using function table
  }

  return(handled);
}

void ATTRIBUTES
lifx_set_target(
  lifx_message_t *message,
  lifx_target_t  *target) {

  uint8_t hexadecimal[3], index;

  for (index = 0;  index < sizeof(lifx_target_t);  index += 2) {
    hexadecimal[0] = (*target)[index];
    hexadecimal[1] = (*target)[index + 1];
    hexadecimal[2] = 0x00;

    message->target[index >> 1] =
      (uint8_t) strtol((const char *) hexadecimal, NULL, 16);
  }

  message->target[6] = 0x00;
  message->target[7] = 0x00;

  message->tagged = (memcmp(target, "000000000000", 12) == 0)  ?  1  :  0;
}

lifx_message_t ATTRIBUTES
*lifx_create_device_get_service(void) {

  lifx_message_t *message = lifx_create_message(
    LIFX_DEVICE_GET_SERVICE, sizeof(lifx_message_t)
  );

  return(message);
}

lifx_message_light_set_color_t ATTRIBUTES
*lifx_create_light_set_color(
  lifx_color_t *color,
  uint32_t      duration) {

  lifx_message_light_set_color_t *message =
    (lifx_message_light_set_color_t *) lifx_create_message(
      LIFX_LIGHT_SET_COLOR, sizeof(lifx_message_light_set_color_t)
    );

  message->reserved = 0x00;
  memcpy(& message->color, color, sizeof(lifx_color_t));
  message->duration = duration;

  return(message);
}

lifx_message_light_set_power_t ATTRIBUTES
*lifx_create_light_set_power(
  uint16_t level,
  uint32_t duration) {

  lifx_message_light_set_power_t *message =
    (lifx_message_light_set_power_t *) lifx_create_message(
      LIFX_LIGHT_SET_POWER, sizeof(lifx_message_light_set_power_t)
    );

  message->level    = level  ?  65535  :  0;
  message->duration = duration;

  return(message);
}

void ATTRIBUTES
lifx_message_send(
  aiko_stream_t  *aiko_stream,
  lifx_targets_t *targets,
  lifx_message_t *message,
  uint8_t         retries) {

  int retry, target;

  for (retry = 0;  retry < retries;  retry ++) {
    for (target = 0;  target < targets->count;  target ++) {
      lifx_set_target(message, & targets->targets[target]);
      aiko_socket_send_broadcast(
        aiko_stream, (uint8_t *) message, message->size
      );
    }

    if (retries > 1) usleep(10000);  // microseconds
  }
}
