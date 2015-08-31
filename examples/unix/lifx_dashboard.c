/**
 * Please do not remove the following notices.
 *
 * \file       lifx_dashboard.c
 * \author     Andy Gelme <andyg@geekscape.org>
 * \copyright  (c) 2015 by Geekscape Pty. Ltd.
 * \license    AGPLv3 http://geekscape.org/static/aiko_license.html
 *
 * Description
 * ~~~~~~~~~~~
 * LIFX dashboard
 *
 * To Do
 * ~~~~~
 * - None, yet.
 */

#include <stdio.h>

#include "aiko_engine.h"

#include "lifx.h"

void lifx_discovery_handler(void) {
}

int main(
  int   argc,
  char *argv[]) {

  printf("[%s %s]\n", argv[0], LIFX_VERSION);

  lifx_initialize(lifx_discovery_handler);

  aiko_loop(AIKO_LOOP_FOREVER);

  return(0);
}
