
#include "DBusAppIF.h"

#include <iostream>

// this function is used to encapsulate any knowledge of the internal guts of GDBus
// from the top-level application
void DBusAppIF::InitEnv()
{
    Gio::init();
}


DBusAppIF::DBusAppIF()
    : loop(Glib::MainLoop::create())
    , vtable(sigc::mem_fun(*this, &DBusAppIF::on_method_call))
{
}

DBusAppIF::~DBusAppIF()
{
}

bool DBusAppIF::on_timeout()
{
    std::string result {"{data: \"fan/sensor data\"}"};

    //std::cout << "Timeout" << std::endl;

    if (timeOutTask)
    {
        result = timeOutTask();
    }

    auto x = Glib::VariantContainerBase::create_tuple(Glib::Variant<std::string>::create(result));
    connection->emit_signal("/com/pobetiger/macfanpp/Control",
                            "com.pobetiger.macfanpp",
                            "Timeout",
                            "", // to all listeners
                            x);



    return true; // recur
}

void DBusAppIF::on_method_call(const Glib::RefPtr<Gio::DBus::Connection>&,
                         const Glib::ustring& , // sender
                         const Glib::ustring& , // obj path
                         const Glib::ustring& , // interface
                         [[maybe_unused]]const Glib::ustring& method_name,
                         [[maybe_unused]]const Glib::VariantContainerBase& params,
                         const Glib::RefPtr<Gio::DBus::MethodInvocation>& invocation)
{
    std::cout << "Method called" << std::endl;

    auto resp = Glib::VariantContainerBase{};
    invocation->return_value(resp);

    loop->quit();
}

void DBusAppIF::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection>& conn, const Glib::ustring&)
{
    std::cout << "Name acquired" << std::endl;
    try
    {

        conn->register_object("/com/pobetiger/macfanpp/Control", introspectData->lookup_interface(), vtable);
        connection = conn; // making a ref copy
    }
    catch (const Glib::Error& ex)
    {
        std::cout << "Exception: " << ex.what() << std::endl;
    }
}

void DBusAppIF::RegisterTimeoutHandle(std::function<std::string()> &&timeOutTask_)
{
    timeOutTask = std::move(timeOutTask_);
}

void DBusAppIF::Run()
{
    try
    {
        introspectData = Gio::DBus::NodeInfo::create_for_xml(Glib::ustring{serviceXml.data()});
        const auto id = Gio::DBus::own_name(Gio::DBus::BusType::BUS_TYPE_SESSION,
                                            "com.pobetiger.macfanpp",
                                            {}, // bus acq
                                            sigc::mem_fun(*this, &DBusAppIF::on_name_acquired),
                                            //sigc::ptr_fun(&on_name_acquired), // name acq
                                            {}); // name lost

        Glib::signal_timeout().connect(sigc::mem_fun(*this, &DBusAppIF::on_timeout), 3000);
        // start the loop
        loop->run();
        Gio::DBus::unown_name(id);
    }
    catch(const Glib::Error& ex)
    {
        std::cout << "Exception: " << ex.what() << std::endl;
        throw; // propagate and quit
    }
}



