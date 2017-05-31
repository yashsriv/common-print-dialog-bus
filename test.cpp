#include <gio/gio.h>

#include <iostream>

static void
test_signal (GDBusConnection *connection,
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

int
main(int argc, char *argv[])
{
  GDBusConnection *connection;
  GError *error;
  GMainLoop *loop;
  guint signal_handler_id;

  error = NULL;
  connection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &error);
  g_assert_no_error (error);
  g_assert (connection != NULL);

  signal_handler_id = g_dbus_connection_signal_subscribe (connection,
                                                          NULL, /* sender */
                                                          "org.example.GDBus",
                                                          "SomeSignal",
                                                          NULL,
                                                          NULL,
                                                          G_DBUS_SIGNAL_FLAGS_NONE,
                                                          test_signal,
                                                          NULL,
                                                          NULL);
  g_assert_cmpint (signal_handler_id, !=, 0);

  loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (loop);
  g_dbus_connection_signal_unsubscribe (connection, signal_handler_id);

  return 0;

}
