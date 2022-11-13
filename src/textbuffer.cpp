#include <util/textbuffer.h>


void apply_tag(const Glib::RefPtr<Gtk::TextBuffer> &text_buffer, const Glib::RefPtr<Gtk::TextTag> &tag, const source_range_t &range){
	if(range.begin.row == 0 || range.begin.column == 0 || range.end.row == 0 || range.end.column == 0){
		return;
	}
	const Gtk::TextIter start = text_buffer->get_iter_at_line_offset(range.begin.row-1, range.begin.column-1);
	const Gtk::TextIter end = text_buffer->get_iter_at_line_offset(range.end.row-1, range.end.column-1);
	text_buffer->apply_tag(tag, start, end);
}


void apply_fixit(const Glib::RefPtr<Gtk::TextBuffer> &text_buffer, const fixit_t &fixit){
	const auto iter_start = text_buffer->get_iter_at_line_offset(fixit.range.begin.row-1, fixit.range.begin.column-1);
	const auto iter_end = text_buffer->get_iter_at_line_offset(fixit.range.end.row-1, fixit.range.end.column-1);
	text_buffer->begin_user_action();
	const auto iter_insert = text_buffer->erase(iter_start, iter_end);
	text_buffer->insert(iter_insert, fixit.replace);
	text_buffer->end_user_action();
}


struct gtk_fixit_t{
	Glib::RefPtr<Gtk::TextMark> begin;
	Glib::RefPtr<Gtk::TextMark> end;
	std::string replace;
};


// The fixits contain the initial positions. So in case of several fixits, we have to keep track of the changing text.
// Luckily, Gtk::TextMark can do that for us.
void apply_fixits(const Glib::RefPtr<Gtk::TextBuffer> &text_buffer, const std::vector<fixit_t> &fixits){
	std::vector<gtk_fixit_t> inserts;
	for(const fixit_t &fixit : fixits){
		gtk_fixit_t gtk_fixit;
		const auto iter_start = text_buffer->get_iter_at_line_offset(fixit.range.begin.row-1, fixit.range.begin.column-1);
		const auto iter_end = text_buffer->get_iter_at_line_offset(fixit.range.end.row-1, fixit.range.end.column-1);
		gtk_fixit.begin = text_buffer->create_mark(iter_start);
		gtk_fixit.end = text_buffer->create_mark(iter_end);
		gtk_fixit.replace = fixit.replace;
		inserts.push_back(gtk_fixit);
	}
	text_buffer->begin_user_action();
	for(const auto &insert : inserts){
		const auto iter_start = text_buffer->get_iter_at_mark(insert.begin);
		const auto iter_end = text_buffer->get_iter_at_mark(insert.end);
		const auto iter_insert = text_buffer->erase(iter_start, iter_end);
		text_buffer->insert(iter_insert, insert.replace);
		text_buffer->delete_mark(insert.begin);
		text_buffer->delete_mark(insert.end);
	}
	text_buffer->end_user_action();
}

