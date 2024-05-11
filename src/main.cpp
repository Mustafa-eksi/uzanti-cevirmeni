#include "giomm/cancellable.h"
#include "sigc++/functors/ptr_fun.h"
#include <gtkmm.h>

const std::string EXECUTABLE_NAME = "uzanti-cevirici";

struct MainWindow {
    Gtk::Window *window;
    Gtk::Button *select_button;
    Glib::RefPtr<Gtk::FileDialog> fdialog;
    Glib::RefPtr<Gio::Cancellable> cancel;
};

struct UzantiCevirici {
    Glib::RefPtr<Gtk::Application> app;
    MainWindow mw;
} uc;

void* selected_file(std::shared_ptr<Gio::AsyncResult>& result) {
    //auto files = uc.mw.fdialog->open_finish();
    //printf("Selected: %s\n", files->get_basename().c_str());
    return nullptr;
}

void select_file() {
    uc.mw.fdialog = Gtk::FileDialog::create();
    uc.mw.cancel = Gio::Cancellable::create();
    /*uc.mw.cancel->signal_cancelled().connect({
        printf("cancel\n")
    });*/
    uc.mw.fdialog->open(sigc::ptr_fun(&select_file), uc.mw.cancel);
}

void init_ui(std::string ui_path) {
    auto builder = Gtk::Builder::create_from_file(ui_path + "uc.ui");
    uc.mw.window = builder->get_widget<Gtk::Window>("main_window");
    uc.mw.window->set_application(uc.app);

    uc.mw.select_button = builder->get_widget<Gtk::Button>("select_button");
    uc.mw.select_button->signal_clicked().connect(sigc::ptr_fun(select_file));

    uc.mw.window->present();
}

int main(int argc, char* argv[]) {
    std::string first_arg = argv[0];
    std::string abs_path = first_arg.substr(0, first_arg.length()-EXECUTABLE_NAME.length());
    uc.app = Gtk::Application::create("org.mustafa.dizelge");
    uc.app->signal_activate().connect(std::bind(init_ui, abs_path));
    return uc.app->run(argc, argv);
}
