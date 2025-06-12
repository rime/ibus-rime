// ibus-rime program entry

#include "rime_config.h"
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <glib.h>
#include <glib-object.h>
#include <glib/gstdio.h>
#include <ibus.h>
#include <libnotify/notify.h>
#include <rime_api.h>
#include "rime_engine.h"
#include "rime_settings.h"

// TODO:
#define _(x) (x)

#define DISTRIBUTION_NAME _("Rime")
#define DISTRIBUTION_CODE_NAME "ibus-rime"
#define DISTRIBUTION_VERSION IBUS_RIME_VERSION

RimeApi *rime_api = NULL;

static const char* get_ibus_rime_user_data_dir(char *path) {
  const char* home = getenv("HOME");
  strcpy(path, home);
  strcat(path, "/.config/ibus/rime");
  return path;
}

static void show_message(const char* summary, const char* details) {
  NotifyNotification* notice = notify_notification_new(summary, details, NULL);
  notify_notification_show(notice, NULL);
  g_object_unref(notice);
}

static void notification_handler(void* context_object,
                                 RimeSessionId session_id,
                                 const char* message_type,
                                 const char* message_value) {
  if (!strcmp(message_type, "deploy")) {
    if (!strcmp(message_value, "start")) {
      show_message(_("Rime is under maintenance ..."), NULL);
    }
    else if (!strcmp(message_value, "success")) {
      show_message(_("Rime is ready."), NULL);
      ibus_rime_load_settings();
    }
    else if (!strcmp(message_value, "failure")) {
      show_message(_("Rime has encountered an error."),
                   _("See /tmp/rime.ibus.ERROR for details."));
    }
    return;
  }
}

static void fill_traits(RimeTraits *traits) {
  traits->shared_data_dir = IBUS_RIME_SHARED_DATA_DIR;
  traits->distribution_name = DISTRIBUTION_NAME;
  traits->distribution_code_name = DISTRIBUTION_CODE_NAME;
  traits->distribution_version = DISTRIBUTION_VERSION;
  traits->app_name = "rime.ibus";
}

void ibus_rime_start(gboolean full_check) {
  char user_data_dir[512] = {0};
  get_ibus_rime_user_data_dir(user_data_dir);
  if (!g_file_test(user_data_dir, G_FILE_TEST_IS_DIR)) {
    g_mkdir_with_parents(user_data_dir, 0700);
  }
  RIME_STRUCT(RimeTraits, ibus_rime_traits);
  fill_traits(&ibus_rime_traits);
  ibus_rime_traits.user_data_dir = user_data_dir;

  rime_api->initialize(&ibus_rime_traits);
  if (rime_api->start_maintenance((Bool)full_check)) {
    // update frontend config
    rime_api->deploy_config_file("ibus_rime.yaml", "config_version");
  }
}

void ibus_rime_stop() {
  if (rime_api) {
    rime_api->finalize();
  }
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

  g_signal_connect(bus, "disconnected", G_CALLBACK(ibus_disconnect_cb), NULL);

  IBusFactory *factory = ibus_factory_new(ibus_bus_get_connection(bus));
  g_object_ref_sink(factory);

  ibus_factory_add_engine(factory, "rime", IBUS_TYPE_RIME_ENGINE);
  if (!ibus_bus_request_name(bus, "im.rime.Rime", 0)) {
    g_error("error requesting bus name");
    exit(1);
  }

  if (!notify_init("ibus-rime")) {
    g_error("notify_init failed");
    exit(1);
  }
  rime_api->set_notification_handler(notification_handler, NULL);

  RIME_STRUCT(RimeTraits, ibus_rime_traits);
  fill_traits(&ibus_rime_traits);
  rime_api->setup(&ibus_rime_traits);

  gboolean full_check = FALSE;
  ibus_rime_start(full_check);
  ibus_rime_load_settings();

  ibus_main();

  ibus_rime_stop();
  notify_uninit();

  g_object_unref(factory);
  g_object_unref(bus);
}

static void sigterm_cb(int sig) {
  // Notify the main program to exit.
  ibus_quit();
}

int main(gint argc, gchar** argv) {
  signal(SIGTERM, sigterm_cb);
  signal(SIGINT, sigterm_cb);

  rime_api = rime_get_api();
  rime_with_ibus();
  return 0;
}
