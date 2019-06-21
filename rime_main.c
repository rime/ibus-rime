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

RimeApi *rime_api = NULL;

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

static void **plugin_handles;
static const char **plugin_modules;

static int load_plugins(const char *config_file)
{
  RimeConfig config = {0};

  if (!rime_api->config_open(config_file, &config)) {
    g_error("error loading settings for %s\n", config_file);
    return 1;
  }

  RimeConfigIterator iter;
  int n = 0;
  if (rime_api->config_begin_list(&iter, &config, "plugins")) {
    while(rime_api->config_next(&iter)) n++;
    rime_api->config_end(&iter);
  }

  int m = 0;
  if (rime_api->config_begin_list(&iter, &config, "modules")) {
    while(rime_api->config_next(&iter)) m++;
    rime_api->config_end(&iter);
  }

  plugin_handles = malloc(sizeof(void *) * (n + 1));
  if (!plugin_handles) {
    return 2;
  }

  plugin_modules = malloc(sizeof(const char *) * (m + 2));
  if (!plugin_modules) {
    free(plugin_handles);
    plugin_handles = NULL;
    return 3;
  }

  n = 0;
  if (rime_api->config_begin_list(&iter, &config, "plugins")) {
    while(rime_api->config_next(&iter)) {
      const char *file = rime_api->config_get_cstring(&config, iter.path);
      if (file) {
        plugin_handles[n] = dlopen(file, RTLD_LAZY | RTLD_GLOBAL);
        n++;
      }
    }
    rime_api->config_end(&iter);
  }
  plugin_handles[n] = NULL;

  m = 1;
  plugin_modules[0] = "default";
  if (rime_api->config_begin_list(&iter, &config, "modules")) {
    while(rime_api->config_next(&iter)) {
      const char *mod = rime_api->config_get_cstring(&config, iter.path);
      if (mod) {
        plugin_modules[m] = strdup(mod);
        m++;
      }
    }
    rime_api->config_end(&iter);
  }
  plugin_modules[m] = NULL;

  return 0;
}

static void unload_plugins() {
  if (plugin_handles) {
    for (int i = 0; plugin_handles[i]; i++) {
      dlclose(plugin_handles[i]);
    }
    free(plugin_handles);
    plugin_handles = NULL;
  }

  if (plugin_modules) {
    for (int i = 1; plugin_modules[i]; i++) {
      free((void *) plugin_modules[i]);
    }
    free(plugin_modules);
    plugin_modules = NULL;
  }
}

static void fill_traits(RimeTraits *traits) {
  traits->shared_data_dir = IBUS_RIME_SHARED_DATA_DIR;
  traits->distribution_name = DISTRIBUTION_NAME;
  traits->distribution_code_name = DISTRIBUTION_CODE_NAME;
  traits->distribution_version = DISTRIBUTION_VERSION;
  traits->app_name = "ibus";
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
  rime_api->set_notification_handler(notification_handler, NULL);
  RIME_STRUCT(RimeTraits, ibus_rime_traits);
  fill_traits(&ibus_rime_traits);
  ibus_rime_traits.user_data_dir = user_data_dir;
  if (plugin_modules)
    ibus_rime_traits.modules = plugin_modules;

  rime_api->initialize(&ibus_rime_traits);
  if (rime_api->start_maintenance((Bool)full_check)) {
    // update frontend config
    rime_api->deploy_config_file("ibus_rime.yaml", "config_version");
  }
}

void ibus_rime_stop() {
  rime_api->finalize();
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

  RIME_STRUCT(RimeTraits, ibus_rime_traits);
  fill_traits(&ibus_rime_traits);
  rime_api->setup(&ibus_rime_traits);

  gboolean full_check = FALSE;
  ibus_rime_start(full_check);

  // parse & load plugin modules
  if (load_plugins("ibus_rime") == 0) {
    // second initialization
    ibus_rime_stop();
    ibus_rime_start(full_check);
  }

  ibus_rime_load_settings();

  ibus_main();

  ibus_rime_stop();
  unload_plugins();
  notify_uninit();

  g_object_unref(factory);
  g_object_unref(bus);
}

static void sigterm_cb(int sig) {
  if (rime_api) {
    ibus_rime_stop();
  }
  notify_uninit();
  exit(EXIT_FAILURE);
}

int main(gint argc, gchar** argv) {
  signal(SIGTERM, sigterm_cb);
  signal(SIGINT, sigterm_cb);

  rime_api = rime_get_api();
  rime_with_ibus();
  return 0;
}
