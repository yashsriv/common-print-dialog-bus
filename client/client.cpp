#include <gio/gio.h>

#include <fstream>
#include <iostream>
#include <string>

#include "../common/common.hpp"

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
  if (g_strcmp0 (method_name, "SendPing") == 0)
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
      g_dbus_method_invocation_return_value (invocation,
                                             NULL);
      std::cout << "Emitted Signal GetBackends" << std::endl;
    }
}


static const GDBusInterfaceVTable interface_vtable =
  {
    handle_method_call,
    NULL,
    NULL
  };

static void
send_message_signal (GDBusConnection *connection,
                     const gchar *sender_name,
                     const gchar *object_path,
                     const gchar *interface_name,
                     const gchar *signal_name,
                     GVariant *parameters,
                     gpointer user_data)
{
  const gchar *greeting;

  g_variant_get (parameters, "(&s)", &greeting);

  std::cout << "Received " << greeting << " from " << object_path;
}

static void
on_bus_acquired (GDBusConnection *connection,
                 const gchar     *name,
                 gpointer         user_data)
{
  guint registration_id;

  registration_id = g_dbus_connection_register_object (connection,
                                                       "/org/openprinting/frontend/Dummy",
                                                       introspection_data->interfaces[0],
                                                       &interface_vtable,
                                                       NULL,  /* user_data */
                                                       NULL,  /* user_data_free_func */
                                                       NULL); /* GError** */

  g_assert (registration_id > 0);

  std::cout << "Bus Acquired & subscribed: " <<  name << std::endl;
}

static void
on_name_acquired (GDBusConnection *connection,
                  const gchar     *name,
                  gpointer         user_data)
{
  guint subscription_id;
  std::cout << "Name Acquired: " <<  name << std::endl;
  subscription_id = g_dbus_connection_signal_subscribe (connection, // DBus Connection
                                                        NULL,       // Sender Name
                                                        "org.openprinting.PrintBackend", // Sender Interface
                                                        "RegisterBackend",  // Signal Name
                                                        NULL,  // Object Path
                                                        NULL,  // arg0 behaviour
                                                        G_DBUS_SIGNAL_FLAGS_NONE,  // Signal Flags
                                                        send_message_signal,  // Callback Function
                                                        NULL,
                                                        NULL);

  g_assert (subscription_id > 0);
  std::cout << "Subscribed: " <<  subscription_id << std::endl;
  while(1){}

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
  guint owner_id;
  GMainLoop *loop;

  introspection_data = g_dbus_node_info_new_for_xml (read_xml("./org.openprinting.PrintFrontend.xml").c_str(), NULL);
  g_assert (introspection_data != NULL);

  owner_id = g_bus_own_name (G_BUS_TYPE_SESSION,
                             "org.openprinting.frontend.dummy",
                             G_BUS_NAME_OWNER_FLAGS_NONE,
                             on_bus_acquired,
                             on_name_acquired,
                             on_name_lost,
                             NULL,
                             NULL);

  loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (loop);

  g_bus_unown_name (owner_id);

  g_dbus_node_info_unref (introspection_data);

  return 0;

}
