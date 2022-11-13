#pragma once

#include <util/option_t.h>

#include <string>

#include <gtkmm.h>


struct tag_properties_t{
	option_t<std::string> foreground_color;
	option_t<std::string> background_color;
	option_t<bool> background_full_height;
	option_t<bool> invisible;
	option_t<int> weight;
	option_t<Pango::Style> style;
	option_t<Pango::Underline> underline;
	option_t<Gdk::RGBA> underline_color;
	option_t<bool> strike_through;
	option_t<Gdk::RGBA> strike_through_color;

	void clear();
	void apply(Glib::RefPtr<Gtk::TextTag> tag) const;
};



