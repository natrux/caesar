#include <crassus/MainWindow.h>

#include <util/unique_ptr.h>
#include <util/uri.h>


MainWindow::MainWindow(){
	set_title("Caesar");
	set_default_size(800, 400);
	tabs_projects.set_scrollable(true);
	tabs_projects.popup_enable();
	tabs_projects.append_page(grid_open_project, "[*]");

	label_open_project.set_text("Open Project");
	label_path_project.set_text("Project Directory:");
	entry_path_project.set_hexpand(true);
	button_path_project.set_label("...");
	label_path_build.set_text("CMake Build Directory:");
	entry_path_build.set_hexpand(true);
	button_path_build.set_label("...");
	button_open_project.set_label("Open");

	button_path_project.signal_clicked().connect(std::bind(&MainWindow::on_button_path_project, this));
	button_path_build.signal_clicked().connect(std::bind(&MainWindow::on_button_path_build, this));
	button_open_project.signal_clicked().connect(std::bind(&MainWindow::on_button_open_project, this));

	grid_open_project.attach(label_open_project, 0, 0, 3, 1);
	grid_open_project.attach(label_path_project, 0, 1, 1, 1);
	grid_open_project.attach(entry_path_project, 1, 1, 1, 1);
	grid_open_project.attach(button_path_project, 2, 1, 1, 1);
	grid_open_project.attach(label_path_build, 0, 2, 1, 1);
	grid_open_project.attach(entry_path_build, 1, 2, 1, 1);
	grid_open_project.attach(button_path_build, 2, 2, 1, 1);
	grid_open_project.attach(button_open_project, 0, 3, 2, 1);

	add(tabs_projects);

	show_all_children();
}


void MainWindow::open_project(const std::string &path_project_directory, const std::string &path_build_directory, bool switch_to){
	bool already_open = false;

	for(const auto &p : projects){
		if(p->get_build_directory() == path_build_directory){
			tabs_projects.set_current_page(tabs_projects.page_num(*p));
			already_open = true;
			break;
		}
	}

	if(!already_open){
		auto ins = projects.insert(std::make_unique<ProjectTab>(path_project_directory, path_build_directory));
		auto &new_tab = **ins.first;
		new_tab.signal_close_requested().connect(std::bind(&MainWindow::on_tab_close_request, this, &new_tab));

		const int num = tabs_projects.append_page(new_tab, new_tab.get_tab_label(), new_tab.get_menu_label());
		tabs_projects.set_tab_reorderable(new_tab);
		tabs_projects.show_all();
		update_display_names();
		if(switch_to){
			tabs_projects.set_current_page(num);
		}
	}
}


void MainWindow::update_display_names() const{
	std::vector<std::string> project_paths;
	for(const auto &p : projects){
		project_paths.push_back(p->get_build_directory());
	}
	const std::vector<std::string> names = file_path_short(project_paths);

	size_t i = 0;
	for(const auto &p : projects){
		p->set_display_name(names[i]);
		i++;
	}
}


void MainWindow::on_button_path_project(){
	Gtk::FileChooserDialog dialog(*this, "Select project directory ...", Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
	dialog.add_button("Select", Gtk::RESPONSE_ACCEPT);
	dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
	if(dialog.run() == Gtk::RESPONSE_ACCEPT) {
		const std::string path = dialog.get_filename();
		entry_path_project.set_text(path);
		entry_path_build.set_text(path + "/build");
	}
}


void MainWindow::on_button_path_build(){
	Gtk::FileChooserDialog dialog(*this, "Select build directory ...", Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
	dialog.add_button("Select", Gtk::RESPONSE_ACCEPT);
	dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
	{
		const std::string project_dir = entry_path_project.get_text();
		if(!project_dir.empty()){
			dialog.set_current_folder(project_dir);
		}
	}
	if(dialog.run() == Gtk::RESPONSE_ACCEPT) {
		entry_path_build.set_text(dialog.get_filename());
	}
}


void MainWindow::on_button_open_project(){
	open_project(
		entry_path_project.get_text(),
		entry_path_build.get_text(),
		true
	);
}


void MainWindow::on_tab_close_request(ProjectTab *tab){
	bool tab_closed = false;
	// TODO: Use projects.find() somehow
	for(auto iter=projects.begin(); iter!=projects.end(); iter++){
		if(iter->get() == tab){
			tabs_projects.remove_page(*tab);
			projects.erase(iter);
			tab_closed = true;
			break;
		}
	}
	if(tab_closed){
		update_display_names();
	}
}


