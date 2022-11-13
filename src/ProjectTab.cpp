#include <crassus/ProjectTab.h>

#include <util/unique_ptr.h>
#include <util/uri.h>


// fix for old GTK versions
#ifndef GTK_LEVEL_BAR_OFFSET_LOW
#define GTK_LEVEL_BAR_OFFSET_LOW  "low"
#endif
#ifndef GTK_LEVEL_BAR_OFFSET_HIGH
#define GTK_LEVEL_BAR_OFFSET_HIGH "high"
#endif
#ifndef GTK_LEVEL_BAR_OFFSET_FULL
#define GTK_LEVEL_BAR_OFFSET_FULL "full"
#endif



ProjectTab::ProjectTab(const std::string &path_project_directory, const std::string &path_build_directory):
	project(path_build_directory),
	directory_tree(path_project_directory, std::bind(&ProjectTab::open_file, this, std::placeholders::_1, true))
{
	parse_status_label.set_padding(5, 5);
	parse_status_label.set_text("Units initialized:");
	parse_status_value.set_padding(5, 5);
	parse_status_value.set_text("0 / 0");
	parse_status_level.set_hexpand(true);
	parse_status_level.remove_offset_value(GTK_LEVEL_BAR_OFFSET_LOW);
	parse_status_level.remove_offset_value(GTK_LEVEL_BAR_OFFSET_HIGH);
	parse_status_level.remove_offset_value(GTK_LEVEL_BAR_OFFSET_FULL);
	parse_status_level.set_min_value(0);
	parse_status_level.set_max_value(0);
	checkbox_auto_parse.set_label("Auto parse");
	checkbox_auto_parse.set_active(auto_parse);
	checkbox_auto_parse.signal_toggled().connect([this](){ auto_parse = checkbox_auto_parse.get_active(); });
	button_update_build.set_label("Update build");
	button_update_build.signal_clicked().connect(std::bind(&ProjectTab::on_build_update_click, this));
	button_update_files.set_label("Update files");
	button_update_files.signal_clicked().connect(std::bind(&ProjectTab::on_files_update_click, this));
	checkbox_sync_tab_directory_view.set_label("Sync to tabs");
	checkbox_sync_tab_directory_view.set_active(sync_tab_directory_view);
	checkbox_sync_tab_directory_view.signal_toggled().connect([this](){ sync_tab_directory_view = checkbox_sync_tab_directory_view.get_active(); });

	tabs_files.set_tab_pos(Gtk::POS_LEFT);
	tabs_files.set_scrollable(true);
	tabs_files.popup_enable();
	tabs_files.signal_switch_page().connect(std::bind(&ProjectTab::on_page_switch, this, std::placeholders::_1, std::placeholders::_2));
	tabs_files.add_events(Gdk::SCROLL_MASK | Gdk::SMOOTH_SCROLL_MASK);
	tabs_files.signal_scroll_event().connect(sigc::mem_fun(*this, &ProjectTab::on_filetabs_scroll));
	directory_tree_scroller.set_shadow_type(Gtk::SHADOW_IN);
	directory_tree_scroller.add(directory_tree);
	directory_tree.set_hexpand(true);
	directory_tree.set_vexpand(true);

	add_events(Gdk::KEY_PRESS_MASK);
	Glib::signal_timeout().connect(sigc::mem_fun(*this, &ProjectTab::on_update_timeout), 100, Glib::PRIORITY_LOW);

	grid_directory.attach(button_update_files, 0, 0, 1, 1);
	grid_directory.attach(checkbox_sync_tab_directory_view, 0, 1, 1, 1);
	grid_directory.attach(directory_tree_scroller, 0, 2, 1, 1);

	pane.pack1(tabs_files);
	pane.pack2(grid_directory);

	attach(parse_status_label, 0, 0, 1, 1);
	attach(parse_status_value, 1, 0, 1, 1);
	attach(parse_status_level, 2, 0, 1, 1);
	attach(checkbox_auto_parse, 3, 0, 1, 1);
	attach(button_update_build, 4, 0, 1, 1);
	attach(pane, 0, 1, 5, 1);
}


void ProjectTab::open_location(const source_location_t &location, bool switch_to){
	auto *tab = open_file_internal(location.file, switch_to);
	tab->set_location(location, true);
}


void ProjectTab::open_file(const std::string &file, bool switch_to){
	open_file_internal(file, switch_to);
}


Gtk::Widget &ProjectTab::get_tab_label(){
	return tab_label;
}


Gtk::Widget &ProjectTab::get_menu_label(){
	return menu_label;
}


std::string ProjectTab::get_build_directory() const{
	return project.get_build_directory();
}


void ProjectTab::set_display_name(const std::string &display_name){
	tab_label.set_text(display_name);
	menu_label.set_text(display_name);
}


Glib::SignalProxy<void> ProjectTab::signal_close_requested(){
	return tab_label.signal_close_clicked();
}


FileTab *ProjectTab::open_file_internal(const std::string &file, bool switch_to){
	FileTab *result;
	bool already_open = false;

	for(const auto &f : file_tabs){
		if(f->get_file_path() == file){
			already_open = true;
			if(switch_to){
				tabs_files.set_current_page(tabs_files.page_num(*f));
			}
			result = f.get();
			break;
		}
	}

	if(!already_open){
		auto ins = file_tabs.insert(std::make_unique<FileTab>(file));
		auto &new_tab = **ins.first;
		new_tab.register_tu_observers(project.get_tu_server());
		new_tab.signal_close_requested().connect(std::bind(&ProjectTab::on_tab_close_request, this, &new_tab));
		new_tab.set_callback_open_location(std::bind(&ProjectTab::open_location, this, std::placeholders::_1, true));

		const int num = tabs_files.append_page(new_tab, new_tab.get_tab_label(), new_tab.get_menu_label());
		tabs_files.set_tab_reorderable(new_tab);
		tabs_files.show_all();
		update_display_names();
		if(switch_to){
			tabs_files.set_current_page(num);
		}
		result = &new_tab;
	}

	return result;
}


void ProjectTab::update_display_names() const{
	std::vector<std::string> file_paths;
	for(const auto &f : file_tabs){
		file_paths.push_back(f->get_file_path());
	}
	std::vector<std::string> names = file_path_short(file_paths);
	//std::vector<std::string> names = file_paths;

	size_t i = 0;
	for(const auto &f : file_tabs){
		f->set_display_name(names[i]);
		i++;
	}
}


void ProjectTab::update_unit_numbers(){
	const size_t num_units = project.count_units();
	const size_t num_init_units = project.count_init_units();
	if(num_units != current_num_units || num_init_units != current_num_init_units){
		parse_status_value.set_text(std::to_string(num_init_units) + " / " + std::to_string(num_units));
		parse_status_level.set_max_value(num_units);
		parse_status_level.set_value(num_init_units);

		current_num_units = num_units;
		current_num_init_units = num_init_units;
	}
}


bool ProjectTab::on_update_timeout(){
	update_unit_numbers();

	if(auto_parse){
		project.auto_parse();
	}

	return true;
}


void ProjectTab::on_build_update_click(){
	project.update();
}


void ProjectTab::on_files_update_click(){
	directory_tree.update();
}


void ProjectTab::on_tab_close_request(FileTab *tab){
	bool tab_closed = false;
	// TODO use files.find() somehow
	for(auto iter=file_tabs.begin(); iter!=file_tabs.end(); iter++){
		if(iter->get() == tab){
			tab->unregister_tu_observers(project.get_tu_server());
			tabs_files.remove_page(*tab);
			if(current_file_tab == iter->get()){
				current_file_tab = NULL;
			}
			file_tabs.erase(iter);
			tab_closed = true;
			break;
		}
	}
	if(tab_closed){
		update_display_names();
	}
}


void ProjectTab::on_page_switch(Gtk::Widget *widget, unsigned int /*page_number*/){
	if(current_file_tab == widget){
		return;
	}

	auto *file_tab = dynamic_cast<FileTab *>(widget);
	if(file_tab && file_tab != current_file_tab){
		if(current_file_tab){
			current_file_tab->on_background();
		}
		current_file_tab = file_tab;
		current_file_tab->on_foreground();
		if(sync_tab_directory_view){
			directory_tree.select_entry(current_file_tab->get_file_path());
		}
	}
}


bool ProjectTab::on_filetabs_scroll(GdkEventScroll *event){
	if(event->delta_y > 0){
		tabs_files.next_page();
	}else if(event->delta_y < 0){
		tabs_files.prev_page();
	}

	return true;
}

