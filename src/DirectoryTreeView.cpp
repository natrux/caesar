#include <crassus/DirectoryTreeView.h>

#include <util/strings.h>
#include <util/ls.h>


DirectoryTreeView::DirectoryTreeView(const std::string &path_, const std::function<void(const std::string &)> open_action):
	Gtk::ListViewText(1),
	path(path_),
	on_open(open_action)
{
	column_record.add(column_name);
	tree_model = Gtk::TreeStore::create(column_record);
	set_model(tree_model);
	set_headers_visible(false);
	set_enable_tree_lines(true);
	set_show_expanders(true);

	update();
}


void DirectoryTreeView::select_entry(const std::string &path){
	const auto find = files_paths.find(path);
	if(find != files_paths.end()){
		const auto &tree_path = find->second;

		expand_to_path(tree_path);
		scroll_to_row(tree_path);
		const auto selection = get_selection();
		selection->unselect_all();
		selection->select(tree_path);
	}
}


void DirectoryTreeView::update(){
	tree_model->clear();
	files_paths.clear();
	paths_files.clear();
	const auto root = tree_model->append(tree_model->children());
	root->set_value<Glib::ustring>(0, path);
	read_directory(path, root->children());
	expand_row(tree_model->get_path(root), false);
}


void DirectoryTreeView::read_directory(const std::string &full_path, const Gtk::TreeNodeChildren& position){
	std::vector<std::string> files;
	std::vector<std::string> directories;
	ls(full_path, files, directories);

	std::sort(files.begin(), files.end(), compare_case_insensitive);
	std::sort(directories.begin(), directories.end(), compare_case_insensitive);

	for(const auto &name : directories){
		const std::string sub_path = full_path + path_sep + name;
		const auto entry = append(name, position);
		const auto tree_path = tree_model->get_path(entry);
		files_paths[sub_path] = tree_path;
		paths_files[tree_path] = sub_path;
		read_directory(sub_path, entry->children());
	}
	for(const auto &name : files){
		const std::string sub_path = full_path + path_sep + name;
		const auto entry = append(name, position);
		const auto tree_path = tree_model->get_path(entry);
		files_paths[sub_path] = tree_path;
		paths_files[tree_path] = sub_path;
	}
}


Gtk::TreeIter DirectoryTreeView::append(const std::string &name, const Gtk::TreeNodeChildren& position) const{
	const auto entry = tree_model->append(position);
	entry->set_value<Glib::ustring>(0, name);
	return entry;
}


void DirectoryTreeView::on_row_activated(const Gtk::TreeModel::Path &path, Gtk::TreeViewColumn */*column*/){
	const auto iter = tree_model->get_iter(path);
	if(!iter){
		return;
	}else if(!iter->children().empty()){
		// toggle the node
		if(row_expanded(path)){
			collapse_row(path);
		}else{
			expand_row(path, false);
		}
	}else{
		const auto find = paths_files.find(path);
		if(find != paths_files.end()){
			on_open(find->second);
		}
	}
}


