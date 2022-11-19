#include <crassus/TabLabel.h>



TabLabel::TabLabel(){
	button.set_image_from_icon_name("gtk-close");
	button.set_relief(Gtk::RELIEF_NONE);
	button.set_focus_on_click(false);
	pack_start(label, true, true, 0);
	pack_end(button, false, true, 0);
	show_all();
}


TabLabel::TabLabel(const std::string &text):
	TabLabel()
{
	set_text(text);
}


void TabLabel::set_text(const std::string &text_){
	text = text_;
	const std::string show = (modified ? "*"+text : " "+text);
	label.set_text(show);
}


void TabLabel::set_text(){
	set_text(text);
}


std::string TabLabel::get_text() const{
	return text;
}


void TabLabel::set_modified(bool modified_){
	modified = modified_;
}


Glib::SignalProxy<void> TabLabel::signal_close_clicked(){
	return button.signal_clicked();
}




