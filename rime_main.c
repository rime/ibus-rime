// ibus-rime program entry

#include "rime_config.h"
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
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
#define DISTRIBUTION_VERSION RIME_VERSION

static const char* get_ibus_rime_user_data_dir(char *path) {
  const char* home = getenv("HOME");
  strcpy(path, home);
  strcat(path, "/.config/ibus/rime");
  return path;
}

static const char* get_ibus_rime_old_user_data_dir(char *path) {
  const char* home = getenv("HOME");
  strcpy(path, home);
  strcat(path, "/.ibus/rime");
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

void ibus_rime_start(gboolean full_check) {
  char user_data_dir[512] = {0};
  char old_user_data_dir[512] = {0};
  get_ibus_rime_user_data_dir(user_data_dir);
  if (!g_file_test(user_data_dir, G_FILE_TEST_IS_DIR)) {
    get_ibus_rime_old_user_data_dir(old_user_data_dir);
    if (g_file_test(old_user_data_dir, G_FILE_TEST_IS_DIR)) {
      g_rename(old_user_data_dir, user_data_dir);
    }
    else {
      g_mkdir_with_parents(user_data_dir, 0700);
    }
  }
  RimeSetNotificationHandler(notification_handler, NULL);
  RIME_STRUCT(RimeTraits, ibus_rime_traits);
  ibus_rime_traits.shared_data_dir = IBUS_RIME_SHARED_DATA_DIR;
  ibus_rime_traits.user_data_dir = user_data_dir;
  ibus_rime_traits.distribution_name = DISTRIBUTION_NAME;
  ibus_rime_traits.distribution_code_name = DISTRIBUTION_CODE_NAME;
  ibus_rime_traits.distribution_version = DISTRIBUTION_VERSION;
  static RIME_MODULE_LIST(ibus_rime_modules, "default", "legacy");
  ibus_rime_traits.modules = ibus_rime_modules;
  RimeInitialize(&ibus_rime_traits);
  RimeStartMaintenance((Bool)full_check);
}

static void* legacy_module_handle = NULL;

static void load_plugin_modules() {
  legacy_module_handle = dlopen("librime-legacy.so", RTLD_LAZY);
}

static void unload_plugin_modules() {
  dlclose(legacy_module_handle);
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
  if (!ibus_bus_request_name(bus, "com.googlecode.rimeime.Rime", 0)) {
    g_error("error requesting bus name");
    exit(1);
  }

  if (!notify_init("ibus-rime")) {
    g_error("notify_init failed");
    exit(1);
  }

  load_plugin_modules();
  RimeSetupLogging("rime.ibus");

  gboolean full_check = FALSE;
  ibus_rime_start(full_check);
  ibus_rime_load_settings();

  ibus_main();

  RimeFinalize();
  unload_plugin_modules();
  notify_uninit();

  g_object_unref(factory);
  g_object_unref(bus);
}

static void sigterm_cb(int sig) {
  RimeFinalize();
  notify_uninit();
  exit(EXIT_FAILURE);
}

int main(gint argc, gchar** argv) {
  signal(SIGTERM, sigterm_cb);
  signal(SIGINT, sigterm_cb);

  rime_with_ibus();
  return 0;
}
