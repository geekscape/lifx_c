/**
 * Please do not remove the following notices.
 *
 * \file       lifx_extend.c
 * \author     Andy Gelme <andyg@geekscape.org>
 * \copyright  (c) 2015 by Geekscape Pty. Ltd.
 * \license    AGPLv3 http://geekscape.org/static/aiko_license.html
 *
 * Description
 * ~~~~~~~~~~~
 * LIFX extensions based on the Aiko Engine.
 *
 * Usage
 * ~~~~~
 * (7:targets)                                  // Provide current targets
 * (7:targets(12:000000000000))                 // All lightbulbs
 * (7:targets(12:d073d5000000))                 // One lightbulb
 * (7:targets(12:d073d500000012:d073d5000001))  // Two lightbulbs
 *
 * To Do
 * ~~~~~
 * - Improve "lifx_store_t" to be dynamically sized, not fixed.
 */

#include "aiko_engine.h"
#ifdef __ets__
#include "aiko_serial.h"
#endif

#include "lisp.h"
#include "lisp_extend.h"

#include "lifx.h"

uint16_t lifx_store_size =
  sizeof(lifx_store_t) + sizeof(lifx_target_t) * LIFX_STORE_TARGETS_MAXIMUM;

lifx_store_t *lifx_store;

/* ------------------------------------------------------------------------- */

tExpression ATTRIBUTES
*primitiveTargets(
  tExpression *expression,
  tExpression *environment) {

  tExpression *result = nil;

  if (expression == NULL) {
    int index;
    for (index = 0;  index < lifx_store->lifx_targets.count; index ++) {
      tExpression *target = lispCreateAtom(
        (const char *) lifx_store->lifx_targets.targets[index],
        sizeof(lifx_target_t)
      );

      if (result == nil) {
        result = lispCreateList(target, NULL);
      }
      else {
        lispAppend(result, target);
      }
    }
  }
  else {
    tExpression *targets = expression->list.car;

    lifx_store->lifx_targets.count = 0;

    while (lispIsList(targets)) {
      if (lifx_store->lifx_targets.count >= LIFX_STORE_TARGETS_MAXIMUM) break;

      tExpression *target = targets->list.car;

      uint8_t target_string[sizeof(lifx_target_t) + 1];
      memset(& target_string, 0x00, sizeof(target_string));
      lispToString(target, & target_string, sizeof(target_string));

      memcpy(
        & lifx_store->lifx_targets.targets[lifx_store->lifx_targets.count ++],
        & target_string,
          sizeof(lifx_target_t)
      );

      targets = targets->list.cdr;
    }

    result = truth;
  }

  return(result);
}

/* ------------------------------------------------------------------------- */

lifx_store_t *lifx_extend(
  uint8_t         debug_flag,
  lifx_targets_t *lifx_targets) {

  lifx_store = (lifx_store_t *) malloc(lifx_store_size);

  memset(lifx_store, 0x00, lifx_store_size);
  lifx_store->aiko_store.size    = lifx_store_size;
  lifx_store->aiko_store.magic   = LIFX_STORE_MAGIC;
  lifx_store->aiko_store.version = LIFX_STORE_VERSION;

  lifx_store->lifx_targets.count = lifx_targets->count;

  if (lifx_targets->count >= LIFX_STORE_TARGETS_MAXIMUM) {
    lifx_store->lifx_targets.count = LIFX_STORE_TARGETS_MAXIMUM;
  }

  memcpy(
    & lifx_store->lifx_targets.targets, & lifx_targets->targets,
      sizeof(lifx_target_t) * lifx_store->lifx_targets.count
  );

  tExpression *lisp_environment = lisp_initialize(debug_flag);

  if (lispError) {
    printf("Error: Aiko Lisp initialization: %d\n", lispError);
    return(NULL);
  }

  lisp_extend(lisp_environment, (aiko_store_t *) lifx_store);

  lispAppend(
    lisp_environment, lispCreatePrimitive("targets",  primitiveTargets)
  );

// TODO: Ultimately, shouldn't need to do this ...
  lispExpressionBookmark = lispExpressionCurrent;

#ifdef __ets__
  aiko_add_handler(
    aiko_create_serial_source(NULL, BAUD_NO_CHANGE, '\r'), lisp_message_handler
  );
#else
  aiko_add_handler(aiko_create_file_source(stdin), lisp_message_handler);
#endif

  aiko_add_handler(
    aiko_create_socket_source(AIKO_SOURCE_SOCKET_UDP4, 0, AIKO_PORT),
    lisp_message_handler
  );

  return(lifx_store);
}

/* ------------------------------------------------------------------------- */
