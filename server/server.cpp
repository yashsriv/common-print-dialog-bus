#include <gio/gio.h>
#include <gio/gunixfdlist.h>

#include <iostream>

#include "../common/common.hpp"

static GDBusNodeInfo *introspection_data = NULL;

static void
respond_to_signal (GDBusConnection *connection,
                   const gchar *sender_name,
                   const gchar *object_path,
                   const gchar *interface_name,
                   const gchar *signal_name,
                   GVariant *parameters,
                   gpointer user_data)
{
  std::cout << "Received Signal GetBackends" << std::endl;
  GError *local_error;
  local_error = NULL;
  g_dbus_connection_emit_signal (connection,
                                 sender_name,
                                 "/org/openprinting/backend/Dummy",
                                 "org.openprinting.PrintBackend",
                                 "RegisterBackend",
                                 g_variant_new ("(s)",
                                                "Pong"),
                                 &local_error);
  g_assert_no_error (local_error);
}

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
  if (g_strcmp0 (method_name, "GetPrinterOptions") == 0)
    {
      const gchar *uid;
      gchar *key;
      gint16 value;

      g_variant_get (parameters, "(&s)", &uid);
      std::cout << "GetPrinterOptions of Printer having uid: " << uid << std::endl;

      key = g_strdup_printf ("Answer");
      value = 42;
      g_dbus_method_invocation_return_value (invocation,
                                             g_variant_new ("(sn)", key, value));
      g_free (key);
    } else if (g_strcmp0 (method_name, "StopListing") == 0)
    {
      g_dbus_method_invocation_return_value (invocation, NULL);
      std::cout << "Stop Listing Printers" << std::endl;
    } else if (g_strcmp0 (method_name, "PrintFile") == 0)
    {
      const gchar *uid;
      GUnixFDList *fd_list;
      GDBusMessage *msg;
      gint fd;
      GError *error;
      error = NULL;
      g_variant_get (parameters, "(&s)", &uid);
      msg = g_dbus_method_invocation_get_message(invocation);
      fd_list = g_dbus_message_get_unix_fd_list (msg);
      fd = g_unix_fd_list_get (fd_list, 0, &error);
      g_assert_no_error (error);
      std::cout << "Print File having fd: " << fd << " to Printer " << uid << std::endl;
    }
}


static const GDBusInterfaceVTable interface_vtable =
  {
    handle_method_call,
    NULL,
    NULL
  };

static void
on_name_acquired (GDBusConnection *connection,
                  const gchar     *name,
                  gpointer         user_data)
{
  std::cout << "Name Acquired: " <<  name << std::endl;
  guint registration_id;

  registration_id = g_dbus_connection_register_object (connection,
                                                       "/org/openprinting/backend/Dummy",
                                                       introspection_data->interfaces[0],
                                                       &interface_vtable,
                                                       NULL,  /* user_data */
                                                       NULL,  /* user_data_free_func */
                                                       NULL); /* GError** */

  g_assert (registration_id > 0);

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

  introspection_data = g_dbus_node_info_new_for_xml (read_xml("./org.openprinting.PrintBackend.xml").c_str(), NULL);
  g_assert (introspection_data != NULL);

  owner_id = g_bus_own_name_on_connection (connection,
                             "org.openprinting.backend.dummy",
                             G_BUS_NAME_OWNER_FLAGS_NONE,
                             on_name_acquired,
                             on_name_lost,
                             NULL,
                             NULL);

  loop = g_main_loop_new (NULL, FALSE);

  subscription_id = g_dbus_connection_signal_subscribe (connection,                        // DBus Connection
                                                        NULL,                              // Sender Name
                                                        "org.openprinting.PrintFrontend",  // Sender Interface
                                                        "GetBackends",                 // Signal Name
                                                        NULL,                              // Object Path
                                                        NULL,                              // arg0 behaviour
                                                        G_DBUS_SIGNAL_FLAGS_NONE,          // Signal Flags
                                                        respond_to_signal,                 // Callback Function
                                                        NULL,
                                                        NULL);

  g_assert (subscription_id > 0);
  std::cout << "Subscribed: " <<  subscription_id << std::endl;
  g_main_loop_run (loop);

  g_bus_unown_name (owner_id);

  g_dbus_node_info_unref (introspection_data);

  return 0;

}
