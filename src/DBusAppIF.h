
#pragma once

#include <string_view>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++98-compat"
#pragma clang diagnostic ignored "-Wc++98-compat-pedantic"
#pragma clang diagnostic ignored "-Wpadded"
#pragma clang diagnostic ignored "-Wdocumentation"
#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wextra-semi"
#pragma clang diagnostic ignored "-Wextra-semi-stmt"

#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wunused-exception-parameter"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wexit-time-destructors"

#include <glibmm.h>
#include <giomm.h>

#pragma clang diagnostic pop


class DBusAppIF
{
public:

    static void InitEnv();

    DBusAppIF();
    ~DBusAppIF();

    DBusAppIF(const DBusAppIF&) =delete;
    DBusAppIF(DBusAppIF&&) =delete;
    DBusAppIF& operator=(const DBusAppIF&) =delete;
    DBusAppIF& operator=(DBusAppIF&&) =delete;

    void Run();
    void RegisterTimeoutHandle(std::function<void()> &&timeOutTask_);

protected:

    void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection>& conn, const Glib::ustring& name);
    void on_method_call(const Glib::RefPtr<Gio::DBus::Connection>&,
                        const Glib::ustring& , // sender
                        const Glib::ustring& , // obj path
                        const Glib::ustring& , // interface
                        const Glib::ustring& method_name,
                        const Glib::VariantContainerBase& params,
                        const Glib::RefPtr<Gio::DBus::MethodInvocation>& invocation);
    bool on_timeout();

private:

    Glib::RefPtr<Glib::MainLoop> loop;

    Glib::RefPtr<Gio::DBus::NodeInfo> introspectData;
    Gio::DBus::InterfaceVTable vtable;
    Glib::RefPtr<Gio::DBus::Connection> connection;

    std::function<void()> timeOutTask;

    static constexpr std::string_view serviceXml = {
        "<node>"
        "   <interface name='com.pobetiger.macfanpp'>"
        "       <method name='quit'></method>"
        "   </interface>"
        "</node>"
    };
};



