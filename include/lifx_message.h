/**
 * Please do not remove the following notices.
 *
 * \file       lifx_message.h
 * \author     Andy Gelme <andyg@geekscape.org>
 * \copyright  (c) 2015 by Geekscape Pty. Ltd.
 * \license    AGPLv3 http://geekscape.org/static/aiko_license.html
 *
 * LIFX LAN protocol message definition.
 * See http://lan.developer.lifx.com/docs/header-description
 *
 * Note: Numeric values are little-endian.
 */

#pragma once

#include "aiko_compatibility.h"

#define LIFX_PROTOCOL_NUMBER   1024
#define LIFX_RETRIES              2
#define LIFX_UDP_PORT         56700

typedef struct {
  uint16_t hue;
  uint16_t saturation;
  uint16_t brightness;
  uint16_t kelvin;      // 2500 C (warm) to 9000 C (cool)
}
  lifx_color_t;

typedef uint8_t lifx_target_t[12];

typedef struct {
  uint8_t       count;
  lifx_target_t targets[];
}
  lifx_targets_t;

static const lifx_targets_t lifx_targets_all = { 1, {"000000000000"} };

typedef enum {
  LIFX_DEVICE_GET_SERVICE   =   2,
  LIFX_DEVICE_STATE_SERVICE =   3,
  LIFX_DEVICE_GET_LABEL     =  23,
  LIFX_DEVICE_SET_LABEL     =  24,
  LIFX_DEVICE_STATE_LABEL   =  25,
  LIFX_LIGHT_GET_COLOR      = 101,
  LIFX_LIGHT_SET_COLOR      = 102,
  LIFX_LIGHT_STATE_COLOR    = 107,
  LIFX_LIGHT_GET_POWER      = 116,
  LIFX_LIGHT_SET_POWER      = 117,
  LIFX_LIGHT_STATE_POWER    = 118
}
  lifx_message_type;

#pragma pack(push, 1)

typedef struct {
// LIFX Frame ...
  uint16_t size;             // Total message size (bytes) including this field

  uint16_t protocol:12;      // Protocol number: Must be 1024
  uint8_t  addressable:1;    // Message includes a target address: Must be 1
  uint8_t  tagged:1;         // Determines usage of the "target" field
  uint8_t  origin:2;         // Message origin indicator: Must be zero

  uint32_t source;           // Source identifier: Unique value set by client

// LIFX Frame address ...
  uint8_t  target[8];        // Device (MAC) address or zero means all devices

  uint8_t  reserved[6];      // Must be all zero

  uint8_t  res_required:1;   // Response message required flag
  uint8_t  ack_required:1;   // Acknowledgement message required flag
  uint8_t  :6;               // Reserved

  uint8_t  sequence;         // Message sequence number: Can wrap around

// LIFX Protocol header ...
  uint64_t :64;              // Reserved
  uint16_t type;             // Message type determines the payload being used
  uint16_t :16;              // Reserved

// LIFX Payload follows ...
}
  lifx_message_t;

typedef struct {
  lifx_message_t header;
  uint8_t        reserved;
  lifx_color_t   color;
  uint32_t       duration;
}
  lifx_message_light_set_color_t;

typedef struct {
  lifx_message_t header;
  uint16_t       level;
  uint32_t       duration;
}
  lifx_message_light_set_power_t;

#pragma pack(pop)

uint8_t lifx_message_handler(uint8_t *message, uint16_t  length);

lifx_message_light_set_color_t *lifx_create_light_set_color(
  lifx_color_t *color, uint32_t duration);

lifx_message_light_set_power_t *lifx_create_light_set_power(
  uint16_t level, uint32_t duration);

void lifx_message_send(
  int fd, lifx_targets_t *targets, lifx_message_t *message, uint8_t retries);
