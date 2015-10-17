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
 * List current set of LIFX device targets ...
 *
 *   ./configure_targets -l
 *
 * Assign a new set of LIFX device targets ...
 *
 *   ./configure_targets [LIFX_TARGET1 LIFX_TARGET2 ...]
 *
 * To Do
 * ~~~~~
 * - No command-line arguments, then display current target list from device.
 *   ... or discover switches and targets and provide a means to select them.
 *
 * - https://en.wikipedia.org/wiki/Datagram_Transport_Layer_Security ?
 */

#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>

#include "aiko_engine.h"
#include "aiko_network.h"
#include "aiko_state.h"

#include "lifx.h"

lifx_targets_t *lifx_targets = NULL;

typedef enum {
  OPTION_ASSIGN_TARGETS,
  OPTION_LIST_TARGETS
}
  option_type;

typedef enum {
  STATE_ASSIGN_RECEIVE_RESPONSE_1 = AIKO_STATE_USER_DEFINED,
  STATE_ASSIGN_RECEIVE_RESPONSE_2,
  STATE_LIST_RECEIVE_RESPONSE
}
  state_type;

/* ------------------------------------------------------------------------- */

void show_help(
  char *name) {

  fprintf(stderr, "\nUsage: %s [target1 ...]\n", name);
  fprintf(stderr,   "       %s -l\n\n", name);
}

/* ------------------------------------------------------------------------- */

option_type initialize(
  int   argc,
  char *argv[]) {

  extern char *optarg;
  extern int   optind;
  int          option;

  lifx_targets = & lifx_targets_all;

  tExpression *lisp_environment = lisp_initialize(LISP_DEBUG);

  if (lispError) {
    printf("lisp_initialize(): %s\n", lispErrorMessage);
    exit(-1);
  }

  while ((option = getopt(argc, argv, "hl")) != -1) {
    switch(option) {
      case 'h':
        show_help(argv[0]);
        exit(-1);

      case 'l':
        return(OPTION_LIST_TARGETS);
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

  return(OPTION_ASSIGN_TARGETS);
}

/* ------------------------------------------------------------------------- */

uint8_t action_error(
  aiko_stream_t *aiko_stream,
  tExpression   *expression) {

  printf("Error: Retry message maximum limit\n");
  exit(0);
}

/* ------------------------------------------------------------------------- */

uint8_t action_assign_device_targets(
  aiko_stream_t *aiko_stream,
  tExpression   *expression) {

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

  aiko_socket_send_broadcast(aiko_stream, (uint8_t *) message, strlen(message));

  return(STATE_ASSIGN_RECEIVE_RESPONSE_1);
}

uint8_t action_assign_receive_response_1(
  aiko_stream_t *aiko_stream,
  tExpression   *expression) {

  char *message = "(4:save)\n";

  aiko_socket_send_broadcast(aiko_stream, (uint8_t *) message, strlen(message));

  return(STATE_ASSIGN_RECEIVE_RESPONSE_2);
}

uint8_t action_assign_receive_response_2(
  aiko_stream_t *aiko_stream,
  tExpression   *expression) {

  printf("Success\n");
  exit(0);
}

/* ------------------------------------------------------------------------- */

uint8_t action_list_device_targets(
  aiko_stream_t *aiko_stream,
  tExpression   *expression) {

  char *message = "(7:targets)\n";

  aiko_socket_send_broadcast(aiko_stream, (uint8_t *) message, strlen(message));

  return(STATE_LIST_RECEIVE_RESPONSE);
}

uint8_t action_list_receive_response(
  aiko_stream_t *aiko_stream,
  tExpression   *expression) {

  printf("LIFX targets: ");

  while (lispIsList(expression)) {
    tExpression *target = expression->list.car;

    printf("%.*s ", target->atom.name.size, (char *) target->atom.name.ptr);

    expression = expression->list.cdr;
  }

  printf("\n");
  exit(0);
}

/* ------------------------------------------------------------------------- */

aiko_state_t states[] = {
  { AIKO_STATE_ERROR,
      AIKO_MATCH_ANY, NULL, action_error
  },

  { STATE_ASSIGN_RECEIVE_RESPONSE_1,
      AIKO_MATCH_TRUE, NULL, action_assign_receive_response_1
  },
  { STATE_ASSIGN_RECEIVE_RESPONSE_2,
      AIKO_MATCH_TRUE, NULL, action_assign_receive_response_2
  },

  { STATE_LIST_RECEIVE_RESPONSE,
      AIKO_MATCH_LIST_NUMBER, "x", action_list_receive_response
  }
};

uint8_t states_count = sizeof(states) / sizeof(aiko_state_t);

/* ------------------------------------------------------------------------- */

int main(
  int   argc,
  char *argv[]) {

  printf("[%s %s]\n", argv[0], LIFX_VERSION);

  option_type option = initialize(argc, argv);

  aiko_stream_t *aiko_stream = aiko_create_socket_stream(
    AIKO_STREAM_SOCKET_UDP4, FALSE, 0, AIKO_PORT
  );

  switch (option) {
    case OPTION_ASSIGN_TARGETS:
      aiko_state_machine(
        states, states_count, aiko_stream, action_assign_device_targets
      );
      break;

    case OPTION_LIST_TARGETS:
      aiko_state_machine(
        states, states_count, aiko_stream, action_list_device_targets
      );
      break;
  }

  aiko_loop(AIKO_LOOP_FOREVER);

  return(0);
}

/* ------------------------------------------------------------------------- */
