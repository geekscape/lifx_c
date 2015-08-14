/**
 * Please do not remove the following notices.
 *
 * \file       lifx.h
 * \author     Andy Gelme <andyg@geekscape.org>
 * \copyright  (c) 2015 by Geekscape Pty. Ltd.
 * \license    AGPLv3 http://geekscape.org/static/aiko_license.html
 *
 * To Do
 * ~~~~~
 * - None, yet.
 */

#pragma once

#include "aiko_compatibility.h"

#include "lifx_message.h"

#define LIFX_DISCOVERY_PERIOD  20  // seconds

typedef void (lifx_discovery_handler_t)(void);

typedef uint8_t lifx_target_t[12];

typedef struct {
  uint8_t       count;
  lifx_target_t targets[];
}
  lifx_targets_t;

static lifx_targets_t lifx_targets_all = { 1, {"000000000000"} };

uint8_t lifx_initialize(lifx_discovery_handler_t *handler);

uint8_t lifx_message_handler(uint8_t *message, uint16_t length);

lifx_message_t *lifx_create_device_get_service(void);

lifx_message_light_set_color_t *lifx_create_light_set_color(
  lifx_color_t *color, uint32_t duration);

lifx_message_light_set_power_t *lifx_create_light_set_power(
  uint16_t level, uint32_t duration);

void lifx_message_send(
  int fd, lifx_targets_t *targets, lifx_message_t *message, uint8_t retries);
