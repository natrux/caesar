#include <crassus/MainWindow.h>



int main(int /*argc*/, char **/*argv*/){
	auto app = Gtk::Application::create("caesar.gui", Gio::APPLICATION_NON_UNIQUE);
	Gsv::init();
	int ret = 0;
	{
		MainWindow main_window;
		ret = app->run(main_window);
	}
	return ret;
}

