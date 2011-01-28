#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "json.h"
#include "tracer.h"
#include "tramp.h"
#include "util.h"

struct memprof_resources_stats {
  long nsignals;

  long inblock;
  long oublock;

  double utime;
  double stime;
};

static struct tracer tracer;
static struct memprof_resources_stats stats;

#define TVAL_TO_DBL(tv) ((double)tv.tv_sec + (double)tv.tv_usec * 1e-6)

static void
resources_trace_start() {
  struct rusage usage;
  getrusage(RUSAGE_SELF, &usage);

  stats.nsignals = -usage.ru_nsignals;

  stats.inblock = -usage.ru_inblock;
  stats.oublock = -usage.ru_oublock;

  stats.stime = -TVAL_TO_DBL(usage.ru_stime);
  stats.utime = -TVAL_TO_DBL(usage.ru_utime);
}

static void
resources_trace_dump(json_gen gen) {
  { // calculate diff before dump, since stop is called after dump
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);

    stats.nsignals += usage.ru_nsignals;

    stats.inblock += usage.ru_inblock;
    stats.oublock += usage.ru_oublock;

    stats.stime += TVAL_TO_DBL(usage.ru_stime);
    stats.utime += TVAL_TO_DBL(usage.ru_utime);
  }

  json_gen_cstr(gen, "signals");
  json_gen_integer(gen, stats.nsignals);

  json_gen_cstr(gen, "inputs");
  json_gen_integer(gen, stats.inblock);

  json_gen_cstr(gen, "outputs");
  json_gen_integer(gen, stats.oublock);

  json_gen_cstr(gen, "stime");
  json_gen_double(gen, stats.stime);

  json_gen_cstr(gen, "utime");
  json_gen_double(gen, stats.utime);
}

static void
resources_trace_stop() {
}

static void
resources_trace_reset() {
}

void install_resources_tracer()
{
  tracer.start = resources_trace_start;
  tracer.stop = resources_trace_stop;
  tracer.reset = resources_trace_reset;
  tracer.dump = resources_trace_dump;
  tracer.id = "resource";

  trace_insert(&tracer);
}
