#pragma once


#include <crassus/tag_properties_t.h>
#include <crassus/TabLabel.h>

#include <pompeius/File.h>
#include <pompeius/TranslationUnit.h>
#include <pompeius/TUObserver.h>
#include <pompeius/TUObservable.h>

#include <util/SafeQueue.h>

#include <set>
#include <memory>

#include <gtkmm.h>
#include <gtksourceviewmm.h>



class FileTab : public TUObserver, public Gtk::Grid{
public:
	FileTab(const std::string &path_file);
	Gtk::Widget &get_tab_label();
	Gtk::Widget &get_menu_label();
	std::string get_file_path() const;
	void set_display_name();
	void set_display_name(const std::string &display_name);
	void set_location(const source_location_t &location, bool scroll_to);
	void set_callback_open_location(const std::function<void(const source_location_t &)> &callback);
	void on_background() const;
	void on_foreground();
	Glib::SignalProxy<void> signal_close_requested();
	void register_tu_observers(TUObservable *server);
	void unregister_tu_observers(TUObservable *server);

	void notify_new_tu(std::shared_ptr<TranslationUnit> unit) override;
	void notify_deleted_tu(std::shared_ptr<TranslationUnit> unit) override;
	void notify_updated_tu(std::shared_ptr<TranslationUnit> unit) override;

private:
	enum class update_status_e{
		UP_TO_DATE,
		OUTDATED,
		PENDING,
	};
	unsigned int update_interval_ms = 100;
	unsigned int update_counter = 0;

	std::shared_ptr<File> file;
	std::string current_cursor_highlight_usr;
	update_status_e status_translation_unit = update_status_e::OUTDATED;
	unsigned int update_translation_unit_divider = 5;
	update_status_e status_highlight_cursor = update_status_e::OUTDATED;
	update_status_e status_highlight_semantics = update_status_e::OUTDATED;
	update_status_e status_highlight_diagnostics = update_status_e::OUTDATED;

	SafeQueue<std::shared_ptr<TranslationUnit>> new_translation_units;
	SafeQueue<std::shared_ptr<TranslationUnit>> deleted_translation_units;
	SafeQueue<std::shared_ptr<TranslationUnit>> updated_translation_units;

	Glib::RefPtr<Gsv::Language> language_cpp;
	Glib::RefPtr<Gsv::Language> language_none;

	TabLabel tab_label;
	Gtk::Label menu_label;
	Gtk::Grid grid_choose_tu;
	Gtk::Label label_choose_tu;
	Gtk::ComboBoxText combo_choose_tu;
	Gtk::CheckButton checkbox_auto_choose_tu;
	std::vector<std::shared_ptr<TranslationUnit>> listed_translation_units;
	tu_file_match_e current_tu_match = tu_file_match_e::NONE;
	Gtk::ScrolledWindow source_view_scroller;
	Gsv::View source_view;
	std::unique_ptr<Gtk::Widget> source_map;
	struct{
		Gtk::Grid grid;
		Gtk::Button completion;
		Gtk::Button jump_to_declaration;
		Gtk::Button jump_to_definition;
	} shortcuts;
	struct{
		Gtk::Grid bar;
		Gtk::Label something;
		Gtk::Separator sep_1;
		Gtk::Label position;
	} status;
	std::function<void(const source_location_t &)> callback_open_location;

	Glib::RefPtr<Gtk::TextTag> tag_variable_def;
	tag_properties_t tag_variable_def_properties;
	Glib::RefPtr<Gtk::TextTag> tag_variable_ref;
	tag_properties_t tag_variable_ref_properties;
	Glib::RefPtr<Gtk::TextTag> tag_class_def;
	tag_properties_t tag_class_def_properties;
	Glib::RefPtr<Gtk::TextTag> tag_class_ref;
	tag_properties_t tag_class_ref_properties;
	Glib::RefPtr<Gtk::TextTag> tag_enum_const_def;
	tag_properties_t tag_enum_const_def_properties;
	Glib::RefPtr<Gtk::TextTag> tag_enum_const_ref;
	tag_properties_t tag_enum_const_ref_properties;
	Glib::RefPtr<Gtk::TextTag> tag_member_variable_def;
	tag_properties_t tag_member_variable_def_properties;
	Glib::RefPtr<Gtk::TextTag> tag_member_variable_ref;
	tag_properties_t tag_member_variable_ref_properties;
	Glib::RefPtr<Gtk::TextTag> tag_function_def;
	tag_properties_t tag_function_def_properties;
	Glib::RefPtr<Gtk::TextTag> tag_function_ref;
	tag_properties_t tag_function_ref_properties;
	Glib::RefPtr<Gtk::TextTag> tag_method_def;
	tag_properties_t tag_method_def_properties;
	Glib::RefPtr<Gtk::TextTag> tag_method_ref;
	tag_properties_t tag_method_ref_properties;
	Glib::RefPtr<Gtk::TextTag> tag_parameter_def;
	tag_properties_t tag_parameter_def_properties;
	Glib::RefPtr<Gtk::TextTag> tag_parameter_ref;
	tag_properties_t tag_parameter_ref_properties;
	Glib::RefPtr<Gtk::TextTag> tag_namespace_def;
	tag_properties_t tag_namespace_def_properties;
	Glib::RefPtr<Gtk::TextTag> tag_namespace_ref;
	tag_properties_t tag_namespace_ref_properties;

	Glib::RefPtr<Gtk::TextTag> tag_highlight_cursor;
	tag_properties_t tag_highlight_cursor_properties;
	Glib::RefPtr<Gtk::TextTag> tag_diagnostics_note_range;
	tag_properties_t tag_diagnostics_note_range_properties;
	Glib::RefPtr<Gtk::TextTag> tag_diagnostics_warn_range;
	tag_properties_t tag_diagnostics_warn_range_properties;
	Glib::RefPtr<Gtk::TextTag> tag_diagnostics_error_range;
	tag_properties_t tag_diagnostics_error_range_properties;

	std::string mark_error_category = "libclang-error";
	int mark_error_priority = 0;
	Glib::RefPtr<Gsv::MarkAttributes> mark_error_attributes;

	std::string mark_warning_category = "libclang-warning";
	int mark_warning_priority = 0;
	Glib::RefPtr<Gsv::MarkAttributes> mark_warning_attributes;

	std::string mark_note_category = "libclang-note";
	int mark_note_priority = 0;
	Glib::RefPtr<Gsv::MarkAttributes> mark_note_attributes;

	std::map<GtkSourceMark *, diagnostic_t> source_mark_diagnostics;

	static gchar *on_query_source_mark_tooltip_text(GtkSourceMarkAttributes *attributes, GtkSourceMark *mark, void *user_data);
	source_location_t get_current_location();
	source_location_t get_location(const Gtk::TextIter &iter) const;
	source_location_t get_location(int x, int y) const;
	cursor_t get_cursor_at(const source_location_t &location) const;
	bool check_scroll_to_insert();
	void update_highlighting();
	bool update_cursor_highlighting();
	bool update_cursor_highlighting(const source_location_t &location);
	bool update_semantic_highlighting();
	bool update_diagnostics_highlighting();
	void update_translation_unit();
	void update_translation_units();
	void show_diagnostic(const diagnostic_t &diagnostic, const Glib::RefPtr<Gsv::Buffer> &buffer);
	void jump_to_declaration(const source_location_t &location);
	void jump_to_definition(const source_location_t &location);
	bool on_update_timeout();
	bool on_scroll(GdkEventScroll *event);
	bool on_key_press_event(GdkEventKey *event) override;
	bool on_source_view_mouse_moved(GdkEventMotion *event);
	void on_text_changed();
	bool on_button_press(GdkEventButton *event);
	void on_cursor_moved();
	void on_line_mark_activated(Gtk::TextIter &position, GdkEvent *event);
	void on_button_choose_tu();
	void on_button_save();
	void on_button_completion();
	void on_jump_to_declaration();
	void on_jump_to_definition();
};

