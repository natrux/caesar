#pragma once

#include <brutus/fixit_t.h>

#include <gtkmm.h>

void apply_tag(const Glib::RefPtr<Gtk::TextBuffer> &text_buffer, const Glib::RefPtr<Gtk::TextTag> &tag, const source_range_t &range);
void apply_fixit(const Glib::RefPtr<Gtk::TextBuffer> &text_buffer, const fixit_t &fixit);
void apply_fixits(const Glib::RefPtr<Gtk::TextBuffer> &text_buffer, const std::vector<fixit_t> &fixits);

