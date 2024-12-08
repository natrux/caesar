#include <crassus/FileTab.h>
#include <crassus/CompletionProvider.h>

#include <brutus/completion_t.h>

#include <util/unique_ptr.h>
#include <util/textbuffer.h>
#include <util/ordering_e.h>
#include <util/strings.h>

#include <gtksourceview/gtksource.h>


FileTab::FileTab(const std::string &path_file)
{
	file = std::make_shared<File>(path_file);
	language_cpp = Gsv::LanguageManager::get_default()->get_language("cpp");

	// Gsv::View
	source_view.set_highlight_current_line(true);
	source_view.set_show_right_margin(true);
	source_view.set_right_margin_position(120);
	source_view.set_show_line_numbers(true);
	source_view.set_show_line_marks(true);
	source_view.set_tab_width(8);
	source_view.set_insert_spaces_instead_of_tabs(false);
	source_view.set_indent_on_tab(true);
	source_view.set_smart_home_end(Gsv::SMART_HOME_END_BEFORE);
#ifdef GSV3_COMPAT
	source_view.set_draw_spaces(Gsv::DRAW_SPACES_SPACE | Gsv::DRAW_SPACES_TAB | Gsv::DRAW_SPACES_NBSP | Gsv::DRAW_SPACES_TRAILING);
#else
	{
		// Not exported?
		auto *space_drawer = gtk_source_view_get_space_drawer(source_view.gobj());
		gtk_source_space_drawer_set_types_for_locations(space_drawer, GTK_SOURCE_SPACE_LOCATION_LEADING, GTK_SOURCE_SPACE_TYPE_NONE);
		gtk_source_space_drawer_set_types_for_locations(space_drawer, GTK_SOURCE_SPACE_LOCATION_INSIDE_TEXT, GTK_SOURCE_SPACE_TYPE_NONE);
		gtk_source_space_drawer_set_types_for_locations(space_drawer, GTK_SOURCE_SPACE_LOCATION_TRAILING, static_cast<GtkSourceSpaceTypeFlags>(GTK_SOURCE_SPACE_TYPE_TAB | GTK_SOURCE_SPACE_TYPE_SPACE | GTK_SOURCE_SPACE_TYPE_NBSP));
		gtk_source_space_drawer_set_enable_matrix(space_drawer, true);
	}
#endif
	source_view.signal_line_mark_activated().connect(std::bind(&FileTab::on_line_mark_activated, this, std::placeholders::_1, std::placeholders::_2));
	{
		const auto completion = source_view.get_completion();
		completion->property_show_headers().set_value(false);
		completion->property_show_icons().set_value(true);
		completion->property_accelerators().set_value(0);
		completion->property_remember_info_visibility().set_value(true);
		completion->property_select_on_show().set_value(true);
		completion->property_proposal_page_size().set_value(7);

		const auto completion_provider = CompletionProvider::create(file, source_view.get_source_buffer());
		if(!completion->add_provider(completion_provider)){
			throw std::runtime_error("Adding completion provider failed");
		}
	}

	// Gtk::TextView
	source_view.set_wrap_mode(Gtk::WRAP_NONE);
	source_view.set_pixels_above_lines(0);
	source_view.set_pixels_below_lines(0);
	source_view.set_pixels_inside_wrap(0);
	source_view.set_justification(Gtk::JUSTIFY_LEFT);
	source_view.set_monospace(true);
	source_view.set_bottom_margin(200);

	// Gtk::Widget
	source_view.set_hexpand(true);
	source_view.set_vexpand(true);
	source_view.signal_motion_notify_event().connect(sigc::mem_fun(*this, &FileTab::on_source_view_mouse_moved));
	source_view.signal_button_press_event().connect(sigc::mem_fun(*this, &FileTab::on_button_press));

	const auto buffer = source_view.get_source_buffer();
	// Gsv::Buffer
	buffer->set_highlight_matching_brackets(true);
	buffer->set_highlight_syntax(true);
	buffer->set_style_scheme(Gsv::StyleSchemeManager::get_default()->get_scheme("cobalt"));

	// Gtk::TextBuffer
	buffer->begin_not_undoable_action();
	buffer->set_text(file->get_content());
	buffer->end_not_undoable_action();
	buffer->signal_changed().connect(std::bind(&FileTab::on_text_changed, this));
	buffer->signal_insert().connect(std::bind(&FileTab::on_insert, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	buffer->property_cursor_position().signal_changed().connect(std::bind(&FileTab::on_cursor_moved, this));

	source_map = std::unique_ptr<Gtk::Widget>(Glib::wrap(gtk_source_map_new()));
	{
		auto *map_view = GTK_SOURCE_VIEW(source_map->gobj());
		gtk_source_view_set_highlight_current_line(map_view, true);
	}

	tag_highlight_cursor_properties.background_color = "#009281"; //"#00dbc2";
	tag_highlight_cursor_properties.background_full_height = true;
	tag_highlight_cursor = buffer->create_tag();
	tag_highlight_cursor_properties.apply(tag_highlight_cursor);

	tag_variable_def_properties.foreground_color = "#ffbb00";
	tag_variable_def = buffer->create_tag();
	tag_variable_def_properties.apply(tag_variable_def);

	tag_variable_ref_properties.foreground_color = "#ffdd00";
	tag_variable_ref = buffer->create_tag();
	tag_variable_ref_properties.apply(tag_variable_ref);

	tag_class_def_properties.foreground_color = "#7777ff";
	tag_class_def_properties.weight = Pango::WEIGHT_BOLD;
	tag_class_def = buffer->create_tag();
	tag_class_def_properties.apply(tag_class_def);

	tag_class_ref_properties.foreground_color = "#7777ff";
	tag_class_ref_properties.weight = Pango::WEIGHT_BOLD;
	tag_class_ref = buffer->create_tag();
	tag_class_ref_properties.apply(tag_class_ref);

	tag_enum_const_def_properties.foreground_color = "#33ddff";
	tag_enum_const_def_properties.weight = Pango::WEIGHT_BOLD;
	tag_enum_const_def_properties.style = Pango::STYLE_ITALIC;
	tag_enum_const_def = buffer->create_tag();
	tag_enum_const_def_properties.apply(tag_enum_const_def);

	tag_enum_const_ref_properties.foreground_color = "#33ddff";
	tag_enum_const_ref_properties.weight = Pango::WEIGHT_BOLD;
	tag_enum_const_ref_properties.style = Pango::STYLE_ITALIC;
	tag_enum_const_ref = buffer->create_tag();
	tag_enum_const_ref_properties.apply(tag_enum_const_ref);

	tag_member_variable_def_properties.foreground_color = "#33ddff";
	tag_member_variable_def = buffer->create_tag();
	tag_member_variable_def_properties.apply(tag_member_variable_def);

	tag_member_variable_ref_properties.foreground_color = "#33ddff";
	tag_member_variable_ref = buffer->create_tag();
	tag_member_variable_ref_properties.apply(tag_member_variable_ref);

	tag_function_def_properties.foreground_color = "#00ff00";
	tag_function_def_properties.weight = Pango::WEIGHT_BOLD;
	tag_function_def = buffer->create_tag();
	tag_function_def_properties.apply(tag_function_def);

	tag_function_ref_properties.foreground_color = "#00ff00";
	tag_function_ref_properties.weight = Pango::WEIGHT_BOLD;
	tag_function_ref = buffer->create_tag();
	tag_function_ref_properties.apply(tag_function_ref);

	tag_method_def_properties.foreground_color = "#00ff00";
	tag_method_def_properties.weight = Pango::WEIGHT_BOLD;
	tag_method_def = buffer->create_tag();
	tag_method_def_properties.apply(tag_method_def);

	tag_method_ref_properties.foreground_color = "#00ff00";
	tag_method_ref = buffer->create_tag();
	tag_method_ref_properties.apply(tag_method_ref);

	tag_parameter_def_properties.foreground_color = "#ff7700";
	tag_parameter_def = buffer->create_tag();
	tag_parameter_def_properties.apply(tag_parameter_def);

	tag_parameter_ref_properties.foreground_color = "#ff9900";
	tag_parameter_ref = buffer->create_tag();
	tag_parameter_ref_properties.apply(tag_parameter_ref);

	tag_namespace_def_properties.foreground_color = "#ff0000";
	tag_namespace_def = buffer->create_tag();
	tag_namespace_def_properties.apply(tag_namespace_def);

	tag_namespace_ref_properties.foreground_color = "#ff0000";
	tag_namespace_ref = buffer->create_tag();
	tag_namespace_ref_properties.apply(tag_namespace_ref);

	tag_diagnostics_note_range_properties.underline = Pango::UNDERLINE_SINGLE;
	tag_diagnostics_note_range_properties.underline_color = Gdk::RGBA("#ffffff");
	tag_diagnostics_note_range = buffer->create_tag();
	tag_diagnostics_note_range_properties.apply(tag_diagnostics_note_range);

	tag_diagnostics_warn_range_properties.underline = Pango::UNDERLINE_SINGLE;
	tag_diagnostics_warn_range_properties.underline_color = Gdk::RGBA("#ffff00");
	tag_diagnostics_warn_range = buffer->create_tag();
	tag_diagnostics_warn_range_properties.apply(tag_diagnostics_warn_range);

	tag_diagnostics_error_range_properties.underline = Pango::UNDERLINE_ERROR;
	tag_diagnostics_error_range = buffer->create_tag();
	tag_diagnostics_error_range_properties.apply(tag_diagnostics_error_range);

	mark_error_attributes = Gsv::MarkAttributes::create();
	mark_error_attributes->set_icon_name("dialog-error");
	g_signal_connect(G_OBJECT(mark_error_attributes->gobj()), "query-tooltip-text", G_CALLBACK(on_query_source_mark_tooltip_text), this);
	source_view.set_mark_attributes(mark_error_category, mark_error_attributes, mark_error_priority);

	mark_warning_attributes = Gsv::MarkAttributes::create();
	mark_warning_attributes->set_icon_name("dialog-warning");
	g_signal_connect(G_OBJECT(mark_warning_attributes->gobj()), "query-tooltip-text", G_CALLBACK(on_query_source_mark_tooltip_text), this);
	source_view.set_mark_attributes(mark_warning_category, mark_warning_attributes, mark_warning_priority);

	mark_note_attributes = Gsv::MarkAttributes::create();
	mark_note_attributes->set_icon_name("dialog-information");
	g_signal_connect(G_OBJECT(mark_note_attributes->gobj()), "query-tooltip-text", G_CALLBACK(on_query_source_mark_tooltip_text), this);
	source_view.set_mark_attributes(mark_note_category, mark_note_attributes, mark_note_priority);

	label_choose_tu.set_text("Translation Unit:");
	label_choose_tu.set_padding(10, 0);
	combo_choose_tu.set_hexpand(true);
	for(auto *renderer : combo_choose_tu.get_cells()){
		auto *text_renderer = dynamic_cast<Gtk::CellRendererText *>(renderer);
		if(text_renderer){
			text_renderer->property_ellipsize().set_value(Pango::ELLIPSIZE_START);
		}
	}
	combo_choose_tu.signal_changed().connect(std::bind(&FileTab::on_button_choose_tu, this));
	combo_choose_tu.set_focus_on_click(false);
	combo_choose_tu.set_can_focus(false);
	combo_choose_tu.append("<None>");
	listed_translation_units.push_back(nullptr);
	combo_choose_tu.set_active(0);
	checkbox_auto_choose_tu.set_label("Switch to best");
	checkbox_auto_choose_tu.set_active(true);
	grid_choose_tu.attach(label_choose_tu, 0, 0, 1, 1);
	grid_choose_tu.attach(combo_choose_tu, 1, 0, 1, 1);
	grid_choose_tu.attach(checkbox_auto_choose_tu, 2, 0, 1, 1);

	source_view_scroller.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	source_view_scroller.set_kinetic_scrolling(true);
	source_view_scroller.set_overlay_scrolling(true);

	status.bar.set_border_width(3);
	status.bar.set_column_spacing(3);
	status.something.set_text("Something");
	status.something.set_hexpand(true);
	status.something.set_ellipsize(Pango::ELLIPSIZE_END);
	status.something.set_halign(Gtk::ALIGN_START);
	status.position.set_text("?, ?");
	{
		int col = 0;
		status.bar.attach(status.something, col++, 0, 1, 1);
		status.bar.attach(status.sep_1, col++, 0, 1, 1);
		status.bar.attach(status.position, col++, 0, 1, 1);
	}

	shortcuts.completion.set_label("Completion (Ctrl+,)");
	shortcuts.completion.set_hexpand(true);
	shortcuts.completion.signal_clicked().connect(std::bind(&FileTab::on_button_completion, this));
	shortcuts.jump_to_declaration.set_label("Jump to declaration (Shift+Click)");
	shortcuts.jump_to_declaration.set_hexpand(true);
	shortcuts.jump_to_declaration.signal_clicked().connect(std::bind(&FileTab::on_jump_to_declaration, this));
	shortcuts.jump_to_definition.set_label("Jump to definition (Ctrl+Click)");
	shortcuts.jump_to_definition.set_hexpand(true);
	shortcuts.jump_to_definition.signal_clicked().connect(std::bind(&FileTab::on_jump_to_definition, this));
	shortcuts.grid.attach(shortcuts.completion, 0, 0, 1, 1);
	shortcuts.grid.attach(shortcuts.jump_to_declaration, 1, 0, 1, 1);
	shortcuts.grid.attach(shortcuts.jump_to_definition, 2, 0, 1, 1);

	this->signal_scroll_event().connect(sigc::mem_fun(*this, &FileTab::on_scroll));
	Glib::signal_timeout().connect(sigc::mem_fun(*this, &FileTab::on_update_timeout), update_interval_ms, Glib::PRIORITY_LOW);

	add_events(Gdk::KEY_PRESS_MASK);


	source_view_scroller.add(source_view);
	attach(grid_choose_tu, 0, 0, 2, 1);
	attach(source_view_scroller, 0, 1, 1, 1);
	attach(*source_map, 1, 1, 1, 1);
	attach(shortcuts.grid, 0, 2, 2, 1);
	attach(status.bar, 0, 3, 2, 1);
	// This call must happen AFTER the source view has been added to its parent!
	gtk_source_map_set_view(GTK_SOURCE_MAP(source_map->gobj()), source_view.gobj());
}


Gtk::Widget &FileTab::get_tab_label(){
	return tab_label;
}


Gtk::Widget &FileTab::get_menu_label(){
	return menu_label;
}


std::string FileTab::get_file_path() const{
	return file->get_path();
}


void FileTab::set_display_name(){
	tab_label.set_modified(file->is_dirty());
	tab_label.set_text();
}


void FileTab::set_display_name(const std::string &display_name){
	tab_label.set_modified(file->is_dirty());
	tab_label.set_text(display_name);
	menu_label.set_text(display_name);
}


void FileTab::set_location(const source_location_t &location, bool scroll_to){
	if(location.file != file->get_path()){
		throw std::logic_error("Wrong file");
	}
	const auto buffer = source_view.get_buffer();
	auto iter = buffer->get_iter_at_line_index(location.row-1, location.column-1);
	buffer->place_cursor(iter);
	if(scroll_to){
		/*
		To scroll to the position, the text view must have calculated its line height and stuff. This is done in
		an idle handler. So if we try to scroll "too soon" after setting the text, it will not work.
		The docs says we should use scrolling to a mark but that has the same problem.
		Solution: Install our own idle handler, that checks if the position has been reached and scrolls
		again otherwise.
		*/
		source_view.scroll_to(iter, 0.2);
		Glib::signal_idle().connect(sigc::mem_fun(*this, &FileTab::check_scroll_to_insert));
	}
}


void FileTab::set_callback_open_location(const std::function<void(const source_location_t &)> &callback){
	callback_open_location = callback;
}


void FileTab::on_background() const{
}


void FileTab::on_foreground(){
	source_view.grab_focus();
}


Glib::SignalProxy<void> FileTab::signal_close_requested(){
	return tab_label.signal_close_clicked();
}


void FileTab::register_tu_observers(TUObservable *server){
	server->register_observer(this, true);
	server->register_observer(file.get(), false);
}


void FileTab::unregister_tu_observers(TUObservable *server){
	server->unregister_observer(this);
	server->unregister_observer(file.get());
}


void FileTab::notify_new_tu(std::shared_ptr<TranslationUnit> unit){
	new_translation_units.push(unit);
}


void FileTab::notify_deleted_tu(std::shared_ptr<TranslationUnit> unit){
	deleted_translation_units.push(unit);
}


void FileTab::notify_updated_tu(std::shared_ptr<TranslationUnit> unit){
	updated_translation_units.push(unit);
}


gchar *FileTab::on_query_source_mark_tooltip_text(GtkSourceMarkAttributes */*attributes*/, GtkSourceMark *mark, void *user_data){
	// We have to use the C API because the C++ binding only allows us
	// to return a Glib::ustring which leads to a double free
	// because the gutter frees the gchar pointer.

	std::string result;
	const auto *self = reinterpret_cast<FileTab *>(user_data);
	const auto find = self->source_mark_diagnostics.find(mark);
	if(find != self->source_mark_diagnostics.end()){
		const diagnostic_t &dig = find->second;
		size_t num_fixits = 0;
		if(!dig.category_name.empty()){
			result += dig.category_name;
			if(!dig.command_line_option.empty()){
				result += " (" + dig.command_line_option + ")";
			}
			result += ":\n";
		}
		result += dig.message;
		num_fixits += dig.fixits.size();

		for(const auto &child : dig.children){
			result += "\n\n";
			if(!child.location.equals(dig.location)){
				result += "On " + child.location.file + ":" + std::to_string(child.location.row) + ":" + std::to_string(child.location.column) + ": ";
			}
			result += child.message;
			num_fixits += child.fixits.size();
		}

		if(num_fixits > 0){
			result += "\n\nClick to fixit.";
		}
	}
	return strdup(result.c_str());
}


source_location_t FileTab::get_current_location(){
	const auto buffer = source_view.get_source_buffer();
	// get_insert() is not const, wtf?
	const auto insert = buffer->get_insert();
	const auto iter = buffer->get_iter_at_mark(insert);

	return get_location(iter);
}


source_location_t FileTab::get_location(const Gtk::TextIter &iter) const{
	source_location_t result;
	result.file = file->get_path();
	result.row = iter.get_line() + 1;
	result.column = iter.get_line_index() + 1;
	result.offset = -1;
	return result;
}


source_location_t FileTab::get_location(int x, int y) const{
	int buffer_x, buffer_y;
	source_view.window_to_buffer_coords(Gtk::TEXT_WINDOW_TEXT, x, y, buffer_x, buffer_y);
	Gtk::TextIter iter;
	source_view.get_iter_at_location(iter, buffer_x, buffer_y);
	return get_location(iter);
}


Gtk::TextIter FileTab::get_location(const source_location_t &location){
	if(location.file != file->get_path()){
		throw std::logic_error("Wrong file");
	}
	if(location.row == 0 || location.column == 0){
		throw std::logic_error("Invalid location L" + std::to_string(location.row) + "C" + std::to_string(location.column));
	}
	const auto buffer = source_view.get_source_buffer();
	return buffer->get_iter_at_line_index(location.row-1, location.column-1);
}


cursor_t FileTab::get_cursor_at(const source_location_t &location) const{
	if(auto lock = file->try_lock_tu()){
		if(file->has_translation_unit_ready(*lock)){
			return file->get_cursor_at(*lock, location);
		}
	}
	throw std::runtime_error("Translation unit not ready");
}


bool FileTab::check_scroll_to_insert(){
	const auto buffer = source_view.get_buffer();
	const auto insert_mark = buffer->get_insert();
	auto insert_iter = buffer->get_iter_at_mark(insert_mark);
	Gdk::Rectangle insert_pos;
	source_view.get_iter_location(insert_iter, insert_pos);

	Gdk::Rectangle visible_rectangle;
	source_view.get_visible_rect(visible_rectangle);

	const bool success = visible_rectangle.intersects(insert_pos);
	if(!success){
		source_view.scroll_to(insert_iter, 0.2);
	}

	// Signal handler gets removed when it returns false.
	return !success;
}


void FileTab::update_highlighting(){
	if(status_highlight_cursor == update_status_e::OUTDATED){
		status_highlight_cursor = update_status_e::PENDING;
	}else if(status_highlight_cursor == update_status_e::PENDING){
		if(update_cursor_highlighting()){
			status_highlight_cursor = update_status_e::UP_TO_DATE;
		}
	}

	if(status_highlight_semantics == update_status_e::OUTDATED){
		status_highlight_semantics = update_status_e::PENDING;
	}else if(status_highlight_semantics == update_status_e::PENDING){
		if(update_semantic_highlighting()){
			status_highlight_semantics = update_status_e::UP_TO_DATE;
		}
	}

	if(status_highlight_diagnostics == update_status_e::OUTDATED){
		status_highlight_diagnostics = update_status_e::PENDING;
	}else if(status_highlight_diagnostics == update_status_e::PENDING){
		if(update_diagnostics_highlighting()){
			status_highlight_diagnostics = update_status_e::UP_TO_DATE;
		}
	}
}


bool FileTab::update_cursor_highlighting(){
	return update_cursor_highlighting(get_current_location());
}


bool FileTab::update_cursor_highlighting(const source_location_t &location){
	cursor_t cursor;
	bool valid_cursor = false;
	bool spelling_cursor = false;
	std::string usr = cursor.get_usr_ref();
	bool usr_changed = (current_cursor_highlight_usr != usr);
	ASTReferences references;
	const auto buffer = source_view.get_source_buffer();
	bool result = true;

	if(file->has_translation_unit()){
		if(auto lock = file->try_lock_tu()){
			if(file->has_translation_unit_ready(*lock)){
				try{
					cursor = file->get_cursor_at(*lock, location);
					valid_cursor = true;
				}catch(const std::runtime_error &/*err*/){
				}
			}else{
				result = false;
			}

			if(valid_cursor){
				// lookup finds the "closest" match, so might actually be the surrounding stuff.
				spelling_cursor = cursor.spelling_range.contains(location);
				usr = cursor.get_usr_ref();
				usr_changed = (current_cursor_highlight_usr != usr);
				if(spelling_cursor && usr_changed){
					references = file->get_references(*lock, cursor);
				}
			}
		}else{
			result = false;
		}
	}

	if(!spelling_cursor || usr_changed){
		buffer->remove_tag(tag_highlight_cursor, buffer->begin(), buffer->end());
		current_cursor_highlight_usr.clear();
	}

	if(spelling_cursor && usr_changed){
		const auto ref_cursors = references.get_all();
		for(const auto &ref_cursor : ref_cursors){
			if(ref_cursor.spelling_range.begin.file != file->get_path() || ref_cursor.spelling_range.end.file != file->get_path()){
				continue;
			}
			const Gtk::TextIter start = buffer->get_iter_at_line_index(ref_cursor.spelling_range.begin.row-1, ref_cursor.spelling_range.begin.column-1);
			const Gtk::TextIter end = buffer->get_iter_at_line_index(ref_cursor.spelling_range.end.row-1, ref_cursor.spelling_range.end.column-1);
			buffer->apply_tag(tag_highlight_cursor, start, end);
		}
		current_cursor_highlight_usr = usr;
	}

	std::string context_string = "";
	if(valid_cursor){
		context_string = cursor.spelling;
		if(context_string.empty()){
			context_string = "[" + cursor.kind_spelling() + "]";
		}
		if(!cursor.type_spelling.empty()){
			context_string += " : " + cursor.type_spelling;
		}
		if(cursor.semantic_parent.exists){
			auto parent = cursor.semantic_parent.get();
			while(parent->kind != CXCursor_TranslationUnit){
				context_string = parent->spelling + " -> " + context_string;
				if(parent->semantic_parent.exists){
					parent = parent->semantic_parent.get();
				}else{
					break;
				}
			}
		}
	}
	status.something.set_text(context_string);

	return result;
}


bool FileTab::update_semantic_highlighting(){
	const bool has_tu = file->has_translation_unit();
	bool tu_ready = false;
	std::vector<cursor_t> cursors;
	const auto buffer = source_view.get_source_buffer();

	if(has_tu){
		if(auto lock = file->try_lock_tu()){
			if(file->has_translation_unit_ready(*lock)){
				tu_ready = true;
				cursors = file->get_cursors(*lock);
			}
		}
	}

	if(!has_tu || tu_ready){
		const auto begin = buffer->begin();
		const auto end = buffer->end();
		buffer->remove_tag(tag_variable_def, begin, end);
		buffer->remove_tag(tag_variable_ref, begin, end);
		buffer->remove_tag(tag_class_def, begin, end);
		buffer->remove_tag(tag_class_ref, begin, end);
		buffer->remove_tag(tag_enum_const_def, begin, end);
		buffer->remove_tag(tag_enum_const_ref, begin, end);
		buffer->remove_tag(tag_member_variable_def, begin, end);
		buffer->remove_tag(tag_member_variable_ref, begin, end);
		buffer->remove_tag(tag_function_def, begin, end);
		buffer->remove_tag(tag_function_ref, begin, end);
		buffer->remove_tag(tag_method_def, begin, end);
		buffer->remove_tag(tag_method_ref, begin, end);
		buffer->remove_tag(tag_parameter_def, begin, end);
		buffer->remove_tag(tag_parameter_ref, begin, end);
		buffer->remove_tag(tag_namespace_def, begin, end);
		buffer->remove_tag(tag_namespace_ref, begin, end);
	}

	for(const auto &cursor : cursors){
		if(cursor.location.file != file->get_path()){
			continue;
		}

		switch(cursor.kind){
		case CXCursor_VarDecl:
			apply_tag(buffer, tag_variable_def, cursor.spelling_range);
			break;
		case CXCursor_ClassDecl:
		case CXCursor_ClassTemplate:
		case CXCursor_EnumDecl:
		case CXCursor_StructDecl:
		case CXCursor_UnionDecl:
		case CXCursor_TypedefDecl:
		case CXCursor_TypeAliasDecl:
		case CXCursor_TemplateTypeParameter:
			apply_tag(buffer, tag_class_def, cursor.spelling_range);
			break;
		case CXCursor_FieldDecl:
			apply_tag(buffer, tag_member_variable_def, cursor.spelling_range);
			break;
		case CXCursor_EnumConstantDecl:
			apply_tag(buffer, tag_enum_const_def, cursor.spelling_range);
			break;
		case CXCursor_FunctionDecl:
		case CXCursor_FunctionTemplate:
			apply_tag(buffer, tag_function_def, cursor.spelling_range);
			break;
		case CXCursor_CXXMethod:
		case CXCursor_Constructor:
		case CXCursor_Destructor:
			apply_tag(buffer, tag_method_def, cursor.spelling_range);
			break;
		case CXCursor_ParmDecl:
			// If parameter is commented out, spelling_range points at the next (?) character (?):
			// void something(int /*blabla*/, int blubblub){
			//                              ^
			// So only apply if spelling_range is contained inside extent.
			if(cursor.extent.contains(cursor.spelling_range)){
				apply_tag(buffer, tag_parameter_def, cursor.spelling_range);
			}
			break;
		case CXCursor_Namespace:
		case CXCursor_NamespaceAlias:
			apply_tag(buffer, tag_namespace_def, cursor.spelling_range);
			break;
		case CXCursor_DeclRefExpr:
			if(cursor.reference.exists){
				switch(cursor.reference.get()->kind){
				case CXCursor_VarDecl:
					apply_tag(buffer, tag_variable_ref, cursor.spelling_range);
					break;
				case CXCursor_FunctionDecl:
				case CXCursor_CXXMethod:
					// In this case the "method" is a static function.
					// Actual method references are found under MemberRefExpr
					apply_tag(buffer, tag_function_ref, cursor.spelling_range);
					break;
				case CXCursor_ParmDecl:
					apply_tag(buffer, tag_parameter_ref, cursor.spelling_range);
					break;
				case CXCursor_EnumConstantDecl:
					apply_tag(buffer, tag_enum_const_ref, cursor.spelling_range);
					break;
				default:
					break;
				}
			}
			break;
		case CXCursor_TypeRef:
			if(cursor.reference.exists){
				switch(cursor.reference.get()->kind){
				case CXCursor_ClassDecl:
				case CXCursor_EnumDecl:
				case CXCursor_StructDecl:
				case CXCursor_UnionDecl:
				case CXCursor_TypedefDecl:
				case CXCursor_TypeAliasDecl:
				case CXCursor_TemplateTypeParameter:
					apply_tag(buffer, tag_class_ref, cursor.spelling_range);
					break;
				default:
					break;
				}
			}
			break;
		case CXCursor_MemberRef:
		case CXCursor_MemberRefExpr:
			if(cursor.reference.exists){
				switch(cursor.reference.get()->kind){
				case CXCursor_FieldDecl:
					apply_tag(buffer, tag_member_variable_ref, cursor.spelling_range);
					break;
				case CXCursor_CXXMethod:
				case CXCursor_Constructor:
				case CXCursor_Destructor:
					apply_tag(buffer, tag_method_ref, cursor.spelling_range);
					break;
				default:
					break;
				}
			}
			break;
		case CXCursor_CallExpr:
			// I still don't entirely get what this cursor represents.
			break;
		case CXCursor_NamespaceRef:
			apply_tag(buffer, tag_namespace_ref, cursor.spelling_range);
			break;
		case CXCursor_TemplateRef:
			if(cursor.reference.exists){
				switch(cursor.reference.get()->kind){
				case CXCursor_ClassTemplate:
					apply_tag(buffer, tag_class_ref, cursor.spelling_range);
					break;
				default:
					break;
				}
			}
			break;
		default:
			break;
		}
	}

	return tu_ready || !has_tu;
}


bool FileTab::update_diagnostics_highlighting(){
	const bool has_tu = file->has_translation_unit();
	std::vector<diagnostic_t> diagnostics;
	bool tu_ready = false;

	if(has_tu){
		if(auto lock = file->try_lock_tu()){
			if(file->has_translation_unit_ready(*lock)){
				tu_ready = true;
				// get_diagnostics() does not need the full lock, only the parsed_info lock.
				// But if I can get the full lock anyway, might as well use it.
				diagnostics = file->get_diagnostics();
			}
		}
	}

	const auto buffer = source_view.get_source_buffer();
	if(!has_tu || tu_ready){
		const auto begin = buffer->begin();
		const auto end = buffer->end();
		buffer->remove_tag(tag_diagnostics_note_range, begin, end);
		buffer->remove_tag(tag_diagnostics_warn_range, begin, end);
		buffer->remove_tag(tag_diagnostics_error_range, begin, end);
		buffer->remove_source_marks(begin, end, mark_error_category);
		buffer->remove_source_marks(begin, end, mark_warning_category);
		buffer->remove_source_marks(begin, end, mark_note_category);
		source_mark_diagnostics.clear();
	}

	for(const auto &diagnostic : diagnostics){
		if(diagnostic.severity != diagnostic_severity_e::NOTE){
			show_diagnostic(diagnostic, buffer);
		}
	}

	return tu_ready || !has_tu;
}


void FileTab::update_translation_unit(){
	if(file->has_translation_unit()){
		if(status_translation_unit == update_status_e::OUTDATED){
			status_translation_unit = update_status_e::PENDING;
		}else if(status_translation_unit == update_status_e::PENDING){
			if(file->parse_translation_unit()){
				status_translation_unit = update_status_e::UP_TO_DATE;
			}
		}
	}
}


void FileTab::update_translation_units(){
	std::shared_ptr<TranslationUnit> unit;

	while((new_translation_units.pop(unit) || updated_translation_units.pop(unit))){
		if(file->has_translation_unit(unit)){
			status_highlight_semantics = update_status_e::OUTDATED;
			status_highlight_diagnostics = update_status_e::OUTDATED;
		}else{
			int found_index = -1;
			for(size_t i=0; i<listed_translation_units.size(); i++){
				if(listed_translation_units[i] == unit){
					found_index = i;
					break;
				}
			}

			const bool auto_switch = checkbox_auto_choose_tu.get_active();
			if(found_index == -1 || (auto_switch && current_tu_match != tu_file_match_e::MAIN_FILE)){
				const tu_file_match_e match = file->match_translation_unit(unit);
				if(found_index == -1 && match != tu_file_match_e::NONE){
					combo_choose_tu.append(unit->get_main_file());
					listed_translation_units.push_back(unit);
					found_index = listed_translation_units.size() - 1;
				}
				if(found_index != -1 && auto_switch){
					const bool is_better = (list_rank(tu_file_match_rank, current_tu_match, match) == ordering_e::GREATER);
					if(is_better){
						// This will call the signal handler which does the actual switch
						combo_choose_tu.set_active(found_index);
					}
				}
			}
		}
	}

	while(deleted_translation_units.pop(unit)){
		auto found = listed_translation_units.begin();
		int found_index = 0;
		for(/* nothing */; found!=listed_translation_units.end(); found++){
			if(unit == *found){
				break;
			}
			found_index++;
		}
		if(found != listed_translation_units.end()){
			const bool is_active = (file->has_translation_unit(*found));
			combo_choose_tu.remove_text(found_index);
			listed_translation_units.erase(found);
			if(is_active){
				// Choose <None>
				// This will call the signal handler which does the actual switch
				combo_choose_tu.set_active(0);
			}
		}
	}
}


void FileTab::show_diagnostic(const diagnostic_t &diagnostic, const Glib::RefPtr<Gsv::Buffer> &buffer){
	if(diagnostic.location.file != file->get_path()){
		return;
	}
	Glib::RefPtr<Gtk::TextTag> tag_range;
	std::string mark_category;

	if(diagnostic.severity == diagnostic_severity_e::NOTE){
		tag_range = tag_diagnostics_note_range;
		mark_category = mark_note_category;
	}else if(diagnostic.severity == diagnostic_severity_e::WARNING){
		tag_range = tag_diagnostics_warn_range;
		mark_category = mark_warning_category;
	}else if(diagnostic.severity == diagnostic_severity_e::ERROR || diagnostic.severity == diagnostic_severity_e::FATAL){
		tag_range = tag_diagnostics_error_range;
		mark_category = mark_error_category;
	}

	if(!mark_category.empty()){
		if(diagnostic.location.row > 0){
			Gtk::TextIter iter = buffer->get_iter_at_line_index(diagnostic.location.row-1, 0);
			auto mark = buffer->create_source_mark(mark_category, iter);
			// This seems to be necessary due to:
			// https://gitlab.gnome.org/GNOME/gtksourceviewmm/-/issues/2
			mark->reference();
			source_mark_diagnostics[mark->gobj()] = diagnostic;
		}
	}

	if(tag_range){
		for(const auto &range : diagnostic.ranges){
			if(range.begin.row > 0 && range.end.row > 0 && range.begin.column > 0 && range.end.column > 0){
				const Gtk::TextIter start = buffer->get_iter_at_line_index(range.begin.row-1, range.begin.column-1);
				const Gtk::TextIter end = buffer->get_iter_at_line_index(range.end.row-1, range.end.column-1);
				buffer->apply_tag(tag_range, start, end);
			}
		}
	}
}


void FileTab::jump_to_declaration(const source_location_t &location){
	if(!callback_open_location || !file->has_translation_unit()){
		return;
	}

	std::shared_ptr<cursor_t> cursor;
	try{
		cursor = std::make_shared<cursor_t>(get_cursor_at(location));
	}catch(const std::runtime_error &/*err*/){
	}

	if(cursor){
		std::shared_ptr<const cursor_t> target;
		if(cursor->is_declaration){
			target = cursor;
		}else if(cursor->reference.exists){
			target = cursor->reference.get();
		}
		if(target && target->canonical_declaration.exists){
			target = target->canonical_declaration.get();
		}
		if(target){
			callback_open_location(target->location);
		}
	}
}


void FileTab::jump_to_definition(const source_location_t &location){
	if(!callback_open_location || !file->has_translation_unit()){
		return;
	}

	std::shared_ptr<cursor_t> cursor;
	try{
		cursor = std::make_shared<cursor_t>(get_cursor_at(location));
	}catch(const std::runtime_error &/*err*/){
	}

	if(cursor){
		std::shared_ptr<const cursor_t> target;
		if(cursor->is_definition){
			target = cursor;
		}else if(cursor->definition.exists){
			target = cursor->definition.get();
		}
		if(target){
			callback_open_location(target->location);
		}
	}
}


bool FileTab::on_update_timeout(){
	update_counter++;

	if(update_counter % update_translation_unit_divider == 0){
		update_translation_unit();
	}
	update_translation_units();
	update_highlighting();

	return true;
}


bool FileTab::on_scroll(GdkEventScroll */*event*/){
	// Sometimes the source view scroller doesn't process the event,
	// e.g. when the scroll area is big enough to hold the whole source view.
	// Then we swallow it here, so it doesn't get propagated to the file tabs.
	return true;
}


bool FileTab::on_key_press_event(GdkEventKey* event){
	if(event->state & GDK_CONTROL_MASK){
		if(event->keyval == GDK_KEY_s || event->keyval == GDK_KEY_S){
			on_button_save();
			return true;
		}else if(event->keyval == GDK_KEY_comma){
			on_button_completion();
			return true;
		}
	}
	return false;
}


bool FileTab::on_source_view_mouse_moved(GdkEventMotion */*event*/){
	/*const source_location_t location = get_location(event->x, event->y);
	try{
		const cursor_t cursor = file->get_cursor_at(location);
	}catch(const std::runtime_error &err){
	}
	*/

	return true;
}


void FileTab::on_text_changed(){
	const auto buffer = source_view.get_source_buffer();
	file->set_content(buffer->get_text());
	set_display_name();
	status_translation_unit = update_status_e::OUTDATED;
}


void FileTab::on_insert(const Gtk::TextIter &pos, const std::string &text, int bytes){
	// not cool, but no other way
	Gtk::TextIter &pos_mut = const_cast<Gtk::TextIter &>(pos);
	auto location = get_location(pos);

	const bool is_line_break = (string_ends_with(text, "\n") || string_ends_with(text, "\r\n"));
	const auto buffer = source_view.get_source_buffer();
	if(is_line_break && location.row > 1){
		// auto indent
		auto iter_previous_line = pos;
		iter_previous_line.backward_line();
		iter_previous_line.set_line_index(0);
		size_t num_indents = 0;
		while(!iter_previous_line.is_end() && iter_previous_line.get_char() == '\t'){
			num_indents++;
			iter_previous_line.forward_char();
		}

		iter_previous_line.forward_to_line_end();
		if(iter_previous_line.get_line() == pos.get_line()){
			// We jumped to the next line, which means we were already at the end of the previous line.
			// So the line consists of only tabs and should be cleared.
			iter_previous_line.backward_line();
			auto line_start = iter_previous_line;
			line_start.set_line_index(0);
			iter_previous_line.forward_to_line_end();
			if(line_start.get_line() == iter_previous_line.get_line()){
				buffer->erase(line_start, iter_previous_line);
				pos_mut = get_location(location);
			}
		}else if(!iter_previous_line.is_start()){
			const std::vector<gunichar> add_indent = {'{', '[', '('};
			// check last character of previous line, add one level of indent
			iter_previous_line.backward_char();
			const auto last_char = iter_previous_line.get_char();
			for(const auto &chr : add_indent){
				if(last_char == chr){
					num_indents++;
					break;
				}
			}
		}

		buffer->insert(pos, std::string(num_indents, '\t'));
		location.column += num_indents;
		pos_mut = get_location(location);
	}else if(bytes == 1 && text.length() == 1){
		const auto chr = text.front();
		if(chr == '}'){
			// reduce indent level by one
			auto iter = pos;
			iter.backward_char();
			if(!iter.is_start()){
				iter.backward_char();
				if(iter.get_char() == '\t'){
					auto after = iter;
					after.forward_char();
					buffer->erase(iter, after);
					location.column--;
					pos_mut = get_location(location);
				}
			}
		}
	}
}


bool FileTab::on_button_press(GdkEventButton *event){
	const bool is_shift = ((event->state & GDK_SHIFT_MASK) != 0);
	const bool is_ctrl = ((event->state & GDK_CONTROL_MASK) != 0);

	if(event->button == GDK_BUTTON_PRIMARY && (is_shift || is_ctrl)){
		const auto location = get_location(event->x, event->y);
		if(is_shift){
			jump_to_declaration(location);
		}else if(is_ctrl){
			jump_to_definition(location);
		}
	}
	return true;
}


void FileTab::on_cursor_moved(){
	const source_location_t location = get_current_location();
	status.position.set_text("L" + std::to_string(location.row) + ", C" + std::to_string(location.column) + "; B" + std::to_string(location.offset));
	status_highlight_cursor = update_status_e::OUTDATED;
}


void FileTab::on_line_mark_activated(Gtk::TextIter &position, GdkEvent *event){
	if(event->type == GdkEventType::GDK_BUTTON_PRESS && event->button.button == GDK_BUTTON_PRIMARY){
		auto buffer = source_view.get_source_buffer();
		const auto marks = buffer->get_source_marks_at_iter(position);
		for(const auto &mark : marks){
			const auto find = source_mark_diagnostics.find(mark->gobj());
			if(find != source_mark_diagnostics.end()){
				const diagnostic_t &diagnostic = find->second;
				std::vector<fixit_t> fixits = diagnostic.fixits;
				for(const auto &child : diagnostic.children){
					fixits.insert(fixits.end(), child.fixits.begin(), child.fixits.end());
				}
				apply_fixits(buffer, fixits);
			}
		}
	}
}


void FileTab::on_button_choose_tu(){
	const bool had_unit = file->has_translation_unit();

	std::shared_ptr<TranslationUnit> new_unit = nullptr;
	const int row = combo_choose_tu.get_active_row_number();
	if(row >= 0){
		size_t index = static_cast<size_t>(row);
		if(index < listed_translation_units.size()){
			new_unit = listed_translation_units[index];
		}
	}

	file->set_translation_unit(new_unit);
	current_tu_match = file->match_translation_unit(new_unit);
	if(!had_unit && new_unit){
		source_view.get_source_buffer()->set_language(language_cpp);
	}else if(had_unit && !new_unit){
		source_view.get_source_buffer()->set_language(language_none);
	}
	if(new_unit && !new_unit->is_initialized()){
		status_translation_unit = update_status_e::OUTDATED;
	}
	status_highlight_cursor = update_status_e::OUTDATED;
	status_highlight_semantics = update_status_e::OUTDATED;
	status_highlight_diagnostics = update_status_e::OUTDATED;
}


void FileTab::on_button_save(){
	file->save();
	set_display_name();
	status_translation_unit = update_status_e::OUTDATED;
}


void FileTab::on_button_completion(){
	auto completion = source_view.get_completion();
	auto context = completion->create_context();

	// Without this, the program crashes as soon as context (RefPtr) goes out of scope.
	// From the C documentation: "The reference being returned is a 'floating' reference, so if you invoke gtk_source_completion_show with this context you don't need to unref it."
	// Assuming this means we are not allowed to free the memory, increase the reference count instead, so that it doesn't do that here.
	// (If that assumption is wrong, this is a memory leak. If it is correct, this is a bug in gtksourceviewmm.)
	context->reference();

#ifdef GSV3_COMPAT
	completion->show(completion->get_providers(), context);
#else
	completion->start(completion->get_providers(), context);
#endif
}


void FileTab::on_jump_to_declaration(){
	jump_to_declaration(get_current_location());
}


void FileTab::on_jump_to_definition(){
	jump_to_definition(get_current_location());
}


