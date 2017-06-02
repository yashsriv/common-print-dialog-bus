#include <gio/gio.h>

#include <fstream>
#include <iostream>
#include <list>
#include <string>

#include "../common/common.hpp"

struct backend_location {
  const gchar *dbus_name;
  const gchar *dbus_object;
};

static GDBusNodeInfo *introspection_data = NULL;

static void
handle_method_call (GDBusConnection       *connection,
                    const gchar           *sender,
                    const gchar           *object_path,
                    const gchar           *interface_name,
                    const gchar           *method_name,
                    GVariant              *parameters,
                    GDBusMethodInvocation *invocation,
                    gpointer               user_data)
{
  std::cout << "Method Called: " <<  method_name << std::endl;
  if (g_strcmp0 (method_name, "Ping") == 0)
    {
      GError *local_error;
      local_error = NULL;
      g_dbus_connection_emit_signal (connection,
                                     NULL,
                                     "/org/openprinting/frontend/Dummy",
                                     "org.openprinting.PrintFrontend",
                                     "GetBackends",
                                     g_variant_new ("(s)",
                                                    "Ping"),
                                     &local_error);
      g_assert_no_error (local_error);
      std::cout << "Emitted Signal GetBackends" << std::endl;
    }
}

static const GDBusInterfaceVTable interface_vtable =
  {
    handle_method_call,
    NULL,
    NULL
  };

static std::list<backend_location>
backends;

static void
got_signal (GDBusConnection          *connection,
                     const gchar     *sender_name,
                     const gchar     *object_path,
                     const gchar     *interface_name,
                     const gchar     *signal_name,
                     GVariant        *parameters,
                     gpointer         user_data)
{
  std::cout << "Received " << signal_name << " from " << object_path << " " << sender_name << std::endl;
  if (g_strcmp0 (signal_name, "RegisterBackend") == 0)
  {
    backend_location b = { sender_name, object_path };
    backends.push_front(b);
  } else if (g_strcmp0 (signal_name, "UpdatePrinter") == 0)
  {

  } else if (g_strcmp0 (signal_name, "DeletePrinter") == 0)
  {
  }
}

static void
on_name_acquired (GDBusConnection *connection,
                  const gchar     *name,
                  gpointer         user_data)
{
  GError *local_error;

  std::cout << "Name Acquired: " <<  name << std::endl;

  guint registration_id;

  registration_id = g_dbus_connection_register_object (connection,
                                                       "/org/openprinting/frontend/Dummy",
                                                       introspection_data->interfaces[0],
                                                       &interface_vtable,
                                                       NULL,  /* user_data */
                                                       NULL,  /* user_data_free_func */
                                                       NULL); /* GError** */

  g_assert (registration_id > 0);

  local_error = NULL;
  g_dbus_connection_emit_signal (connection,
                                 NULL,
                                 "/org/openprinting/frontend/Dummy",
                                 "org.openprinting.PrintFrontend",
                                 "GetBackends",
                                 g_variant_new ("(s)",
                                                "Ping"),
                                 &local_error);
  g_assert_no_error (local_error);
  std::cout << "Emitted Signal GetBackends" << std::endl;

}

static void
on_name_lost (GDBusConnection *connection,
              const gchar     *name,
              gpointer         user_data)
{
  std::cout << "Name Lost: " <<  name << std::endl;
  exit (1);
}

int
main (int argc, char **argv)
{
  GDBusConnection *connection;
  GError *error;
  GMainLoop *loop;
  guint owner_id;
  guint subscription_id;

  error = NULL;
  connection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &error);
  g_assert_no_error (error);
  g_assert (connection != NULL);

  introspection_data = g_dbus_node_info_new_for_xml (read_xml("./org.openprinting.PrintFrontend.xml").c_str(), NULL);
  g_assert (introspection_data != NULL);

  owner_id = g_bus_own_name_on_connection (connection,
                                           "org.openprinting.frontend.dummy",
                                           G_BUS_NAME_OWNER_FLAGS_NONE,
                                           on_name_acquired,
                                           on_name_lost,
                                           NULL,
                                           NULL);

  loop = g_main_loop_new (NULL, FALSE);
  subscription_id = g_dbus_connection_signal_subscribe (connection,                       // DBus Connection
                                                        NULL,                             // Sender Name
                                                        "org.openprinting.PrintBackend",  // Sender Interface
                                                        NULL,                             // Signal Name
                                                        NULL,                             // Object Path
                                                        NULL,                             // arg0 behaviour
                                                        G_DBUS_SIGNAL_FLAGS_NONE,         // Signal Flags
                                                        got_signal,              // Callback Function
                                                        NULL,
                                                        NULL);

  g_assert (subscription_id > 0);
  std::cout << "Subscribed: " <<  subscription_id << std::endl;
  g_main_loop_run (loop);

  g_bus_unown_name (owner_id);

  g_dbus_node_info_unref (introspection_data);

  return 0;

}
