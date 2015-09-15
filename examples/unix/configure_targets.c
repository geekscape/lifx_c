/**
 * Please do not remove the following notices.
 *
 * \file       configure_targets.c
 * \author     Andy Gelme <andyg@geekscape.org>
 * \copyright  (c) 2015 by Geekscape Pty. Ltd.
 * \license    AGPLv3 http://geekscape.org/static/aiko_license.html
 *
 * Description
 * ~~~~~~~~~~~
 * LIFX device targets configuration example: Unix implementation
 *
 * Usage
 * ~~~~~
 * ./configure_targets [LIFX_TARGET1 LIFX_TARGET2 ...]
 *
 * To Do
 * ~~~~~
 * - No command-line arguments, then display current target list from device.
 *
 * - https://en.wikipedia.org/wiki/Datagram_Transport_Layer_Security ?
 */

#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>

#include "aiko_engine.h"
#include "aiko_network.h"

#include "lifx.h"

/* ------------------------------------------------------------------------- */

void show_help(
  char *name) {

  fprintf(stderr, "\nUsage: %s [target1 ...]\n\n", name);
}

/* ------------------------------------------------------------------------- */

lifx_targets_t *initialize(
  int   argc,
  char *argv[]) {

  extern char *optarg;
  extern int   optind;
  int          option;

  lifx_targets_t *lifx_targets = & lifx_targets_all;

  while ((option = getopt(argc, argv, "h")) != -1) {
    switch(option) {
      case 'h':
      case '?':
        show_help(argv[0]);
        exit(-1);
    }
  }

  int index, index2;
  int argument_count = argc - optind;

  if (argument_count > 0) {
    lifx_targets = (lifx_targets_t *)
      malloc(sizeof(lifx_targets_t) + sizeof(lifx_target_t) * argument_count);

    lifx_targets->count = argument_count;

    for (index = 0;  index < argument_count;  index ++) {
      char *argument = argv[optind + index];
      int   argument_length = strlen(argument);

      if (argument_length != sizeof(lifx_target_t)) {
        fprintf(
          stderr, "Error: Target must be exactly %lu characters long: %s\n",
          sizeof(lifx_target_t), argument
        );

        exit(-1);
      }

      memset(lifx_targets->targets[index], 0x00, sizeof(lifx_target_t));

      for (index2 = 0;  index2 < argument_length;  index2 ++) {
        if (isxdigit(argument[index2]) == 0) {
          fprintf(
            stderr, "Error: Target must be 12 hexadecimal digits: %s\n",
            argument
          );

          exit(-1);
        }

        if (index2 == sizeof(lifx_target_t)) break;
        lifx_targets->targets[index][index2] = argument[index2];
      }
    }
  }

  return(lifx_targets);
}

/* ------------------------------------------------------------------------- */

int main(
  int   argc,
  char *argv[]) {

  printf("[%s %s]\n", argv[0], LIFX_VERSION);

  lifx_targets_t *lifx_targets = initialize(argc, argv);

  char message[15 + 15 * LIFX_STORE_TARGETS_MAXIMUM];

  char *message_end = stpcpy(message, "(7:targets(");

  int index;
  for (index = 0;  index < lifx_targets->count;  index ++) {
    message_end = stpcpy(message_end, "12:");

    message_end = stpncpy(
      message_end, (char *) lifx_targets->targets[index], 12
    );
  }

  stpcpy(message_end, "))\n");

  int socket_fd = aiko_create_socket_udp(0, AIKO_PORT);

  aiko_socket_send_broadcast(                           // aiko_socket_send() ?
    socket_fd, AIKO_PORT, (uint8_t *) message, strlen(message)
  );

  strcpy(message, "(4:save)\n");

  aiko_socket_send_broadcast(                           // aiko_socket_send() ?
    socket_fd, AIKO_PORT, (uint8_t *) message, strlen(message)
  );

  return(0);
}

/* ------------------------------------------------------------------------- */
