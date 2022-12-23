#include <util/textbuffer.h>


void apply_tag(const Glib::RefPtr<Gtk::TextBuffer> &text_buffer, const Glib::RefPtr<Gtk::TextTag> &tag, const source_range_t &range){
	if(range.begin.row == 0 || range.begin.column == 0 || range.end.row == 0 || range.end.column == 0){
		return;
	}
	const Gtk::TextIter start = text_buffer->get_iter_at_line_index(range.begin.row-1, range.begin.column-1);
	const Gtk::TextIter end = text_buffer->get_iter_at_line_index(range.end.row-1, range.end.column-1);
	text_buffer->apply_tag(tag, start, end);
}


static void replace(const Glib::RefPtr<Gtk::TextBuffer> &text_buffer, const Gtk::TextBuffer::iterator &begin, const Gtk::TextBuffer::iterator &end, const std::string &text){
	const auto iter_insert = text_buffer->erase(begin, end);
	text_buffer->insert(iter_insert, text);
}


void apply_fixit(const Glib::RefPtr<Gtk::TextBuffer> &text_buffer, const fixit_t &fixit){
	const auto iter_start = text_buffer->get_iter_at_line_index(fixit.range.begin.row-1, fixit.range.begin.column-1);
	const auto iter_end = text_buffer->get_iter_at_line_index(fixit.range.end.row-1, fixit.range.end.column-1);
	text_buffer->begin_user_action();
	replace(text_buffer, iter_start, iter_end, fixit.replace);
	text_buffer->end_user_action();
}


struct gtk_fixit_t : fixit_t{
	Glib::RefPtr<Gtk::TextMark> begin;
	Glib::RefPtr<Gtk::TextMark> end;
};


// The fixits contain the initial positions. So in case of several fixits, we have to keep track of the changing text.
// Luckily, Gtk::TextMark can do that for us.
void apply_fixits(const Glib::RefPtr<Gtk::TextBuffer> &text_buffer, const std::vector<fixit_t> &fixits){
	const size_t num_fixits = fixits.size();
	if(num_fixits == 0){
		return;
	}else if(num_fixits == 1){
		// no need for complicated stuff
		apply_fixit(text_buffer, fixits.front());
		return;
	}

	std::vector<gtk_fixit_t> inserts;
	for(const fixit_t &fixit : fixits){
		gtk_fixit_t gtk_fixit;
		gtk_fixit.fixit_t::operator=(fixit);
		const auto iter_start = text_buffer->get_iter_at_line_index(fixit.range.begin.row-1, fixit.range.begin.column-1);
		const auto iter_end = text_buffer->get_iter_at_line_index(fixit.range.end.row-1, fixit.range.end.column-1);
		gtk_fixit.begin = text_buffer->create_mark(iter_start);
		gtk_fixit.end = text_buffer->create_mark(iter_end);
		inserts.push_back(gtk_fixit);
	}
	text_buffer->begin_user_action();
	for(const auto &insert : inserts){
		const auto iter_start = text_buffer->get_iter_at_mark(insert.begin);
		const auto iter_end = text_buffer->get_iter_at_mark(insert.end);
		replace(text_buffer, iter_start, iter_end, insert.replace);
		text_buffer->delete_mark(insert.begin);
		text_buffer->delete_mark(insert.end);
	}
	text_buffer->end_user_action();
}

