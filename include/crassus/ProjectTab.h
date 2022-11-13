#pragma once


#include <crassus/FileTab.h>
#include <crassus/DirectoryTreeView.h>
#include <crassus/TabLabel.h>

#include <pompeius/Project.h>

#include <gtkmm.h>


class ProjectTab : public Gtk::Grid{
public:
	ProjectTab(const std::string &path_project_directory, const std::string &path_build_directory);
	void open_location(const source_location_t &location, bool switch_to);
	void open_file(const std::string &file, bool switch_to);
	Gtk::Widget &get_tab_label();
	Gtk::Widget &get_menu_label();
	std::string get_build_directory() const;
	void set_display_name(const std::string &display_name);
	Glib::SignalProxy<void> signal_close_requested();

private:
	Project project;
	std::set<std::unique_ptr<FileTab>> file_tabs;
	FileTab *current_file_tab = NULL;
	size_t current_num_units = -1;
	size_t current_num_init_units = -1;

	TabLabel tab_label;
	Gtk::Label menu_label;
	Gtk::Label parse_status_label;
	Gtk::Label parse_status_value;
	Gtk::LevelBar parse_status_level;
	Gtk::CheckButton checkbox_auto_parse;
	Gtk::Button button_update_build;
	Gtk::Button button_update_files;
	Gtk::CheckButton checkbox_sync_tab_directory_view;
	Gtk::HPaned pane;
	Gtk::Notebook tabs_files;
	Gtk::Grid grid_directory;
	Gtk::ScrolledWindow directory_tree_scroller;
	DirectoryTreeView directory_tree;

	bool sync_tab_directory_view = true;
	bool auto_parse = false;

	FileTab *open_file_internal(const std::string &file, bool switch_to);
	void update_display_names() const;
	void update_unit_numbers();
	bool on_update_timeout();
	void on_build_update_click();
	void on_files_update_click();
	void on_tab_close_request(FileTab *tab);
	void on_page_switch(Gtk::Widget *widget, unsigned int page_number);
	bool on_filetabs_scroll(GdkEventScroll *event);
};

