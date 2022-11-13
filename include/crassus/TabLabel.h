#pragma once


#include <gtkmm.h>


class TabLabel : public Gtk::HBox{
public:
	TabLabel();
	TabLabel(const std::string &text);
	void set_text(const std::string &text);
	void set_text();
	std::string get_text() const;
	void set_modified(bool modified_);
	Glib::SignalProxy<void> signal_close_clicked();

private:
	std::string text;
	bool modified = false;
	Gtk::Label label;
	Gtk::Button button;
};


