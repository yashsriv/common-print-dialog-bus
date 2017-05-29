#include <gio/gio.h>

#include <fstream>
#include <iostream>
#include <string>

static GDBusNodeInfo *introspection_data = NULL;

std::string read_xml(std::string xml) {

  std::ifstream xml_file(xml);
  std::string file_contents;

  xml_file.seekg(0, std::ios::end);
  file_contents.reserve(xml_file.tellg());
  xml_file.seekg(0, std::ios::beg);

  file_contents.assign((std::istreambuf_iterator<char>(xml_file)),
                       std::istreambuf_iterator<char>());

  return file_contents;

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
  if (g_strcmp0 (method_name, "Call") == 0)
    {
      gchar *response;
      response = g_strdup_printf ("You greeted me. Thanks!");
      g_dbus_method_invocation_return_value (invocation,
                                             g_variant_new ("(s)", response));
      g_free (response);
    }
}


static const GDBusInterfaceVTable interface_vtable =
  {
    handle_method_call
  };

static void
on_bus_acquired (GDBusConnection *connection,
                 const gchar     *name,
                 gpointer         user_data)
{
  guint registration_id;

  registration_id = g_dbus_connection_register_object (connection,
                                                       "/org/openprinting/backend/GooogleCloudPrint",
                                                       introspection_data->interfaces[0],
                                                       &interface_vtable,
                                                       NULL,  /* user_data */
                                                       NULL,  /* user_data_free_func */
                                                       NULL); /* GError** */
  g_assert (registration_id > 0);

  std::cout << "Bus Acquired: " <<  name << std::endl;
}

static void
on_name_acquired (GDBusConnection *connection,
                  const gchar     *name,
                  gpointer         user_data)
{
  std::cout << "Name Acquired: " <<  name << std::endl;
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

  introspection_data = g_dbus_node_info_new_for_xml (read_xml("./org.openprinting.PrintBackend.xml").c_str(), NULL);
  g_assert (introspection_data != NULL);

  owner_id = g_bus_own_name (G_BUS_TYPE_SESSION,
                             "org.openprinting.backend.google-cloud-print",
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
