#pragma once


#include <gtkmm.h>


class DirectoryTreeView : public Gtk::ListViewText{
public:
	DirectoryTreeView(const std::string &path_, const std::function<void(const std::string &)> open_action);
	void select_entry(const std::string &path);
	void update();

private:
	std::string path;
#ifdef _WIN32
	const char path_sep = '\\';
#else
	const char path_sep = '/';
#endif
	Gtk::TreeModel::ColumnRecord column_record;
	Gtk::TreeModelColumn<Glib::ustring> column_name;
	Glib::RefPtr<Gtk::TreeStore> tree_model;
	std::map<std::string, Gtk::TreeModel::Path> files_paths;
	std::map<Gtk::TreeModel::Path, std::string> paths_files;
	std::function<void(const std::string &)> on_open;

	void read_directory(const std::string &full_path, const Gtk::TreeNodeChildren& position);
	Gtk::TreeIter append(const std::string &name, const Gtk::TreeNodeChildren& position) const;
	void on_row_activated(const Gtk::TreeModel::Path &path, Gtk::TreeViewColumn *column) override;
};

