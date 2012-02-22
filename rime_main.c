// ibus-rime program entry

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <glib.h>
#include <glib-object.h>
#include <ibus.h>
#include <rime_api.h>
#include "rime_engine.h"

// TODO:
#define _(x) (x)

#define DISTRIBUTION_NAME _("Rime")
#define DISTRIBUTION_CODE_NAME "ibus-rime"
#define DISTRIBUTION_VERSION "0.9"

#define IBUS_RIME_INSTALL_PREFIX "/usr"
#define IBUS_RIME_SHARED_DATA_DIR IBUS_RIME_INSTALL_PREFIX "/share/ibus-rime"

static const char* get_ibus_rime_user_data_dir(char *path) {
  const char* home = getenv("HOME");
  strcpy(path, home);
  strcat(path, "/.ibus/rime");
  return path;
}

static void ibus_disconnect_cb(IBusBus *bus, gpointer user_data) {
  g_debug("bus disconnected");
  ibus_quit();
}

static void rime_with_ibus() {
  ibus_init();
  IBusBus *bus = ibus_bus_new();
  g_object_ref_sink(bus);

  if (!ibus_bus_is_connected(bus)) {
    g_warning("not connected to ibus");
    exit(0);
  }

  char user_data_dir[1024] = "";
  get_ibus_rime_user_data_dir(user_data_dir);
  g_mkdir_with_parents(user_data_dir, 0700);

  RimeTraits ibus_rime_traits;
  ibus_rime_traits.shared_data_dir = IBUS_RIME_SHARED_DATA_DIR;
  ibus_rime_traits.user_data_dir = user_data_dir;
  ibus_rime_traits.distribution_name = DISTRIBUTION_NAME;
  ibus_rime_traits.distribution_code_name = DISTRIBUTION_CODE_NAME;
  ibus_rime_traits.distribution_version = DISTRIBUTION_VERSION;
  RimeInitialize(&ibus_rime_traits);
  if (RimeStartMaintenanceOnWorkspaceChange()) {
    // TODO: notification...
  }

  g_signal_connect(bus, "disconnected", G_CALLBACK(ibus_disconnect_cb), NULL);

  IBusFactory *factory = ibus_factory_new(ibus_bus_get_connection(bus));
  g_object_ref_sink(factory);
  
  ibus_factory_add_engine(factory, "rime", IBUS_TYPE_RIME_ENGINE);
  if (!ibus_bus_request_name(bus, "com.googlecode.rimeime.Rime", 0)) {
    g_error("error requesting bus name");
    exit(1);
  }
  
  ibus_main();
  
  g_object_unref(factory);
  g_object_unref(bus);
}

static void sigterm_cb(int sig) {
  RimeFinalize();
  exit(EXIT_FAILURE);
}

static void atexit_cb() {
  RimeFinalize();
}

int main(gint argc, gchar** argv) {
  signal(SIGTERM, sigterm_cb);
  signal(SIGINT, sigterm_cb);
  g_atexit(atexit_cb);

  rime_with_ibus();
  return 0;
}
