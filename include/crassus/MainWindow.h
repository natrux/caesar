#pragma once


#include <crassus/ProjectTab.h>

#include <memory>
#include <gtkmm.h>



class MainWindow : public Gtk::Window{
public:
	MainWindow();
	void open_project(const std::string &path_project_directory, const std::string &path_build_directory, bool switch_to);

private:
	std::set<std::unique_ptr<ProjectTab>> projects;
	Gtk::Notebook tabs_projects;
	Gtk::Grid grid_open_project;
	Gtk::Label label_open_project;
	Gtk::Label label_path_project;
	Gtk::Entry entry_path_project;
	Gtk::Button button_path_project;
	Gtk::Label label_path_build;
	Gtk::Entry entry_path_build;
	Gtk::Button button_path_build;
	Gtk::Button button_open_project;

	void update_display_names() const;
	void on_button_path_project();
	void on_button_path_build();
	void on_button_open_project();
	void on_tab_close_request(ProjectTab *tab);
};

