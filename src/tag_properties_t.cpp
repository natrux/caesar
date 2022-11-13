#include <crassus/tag_properties_t.h>


void tag_properties_t::clear(){
	foreground_color.clear();
	background_color.clear();
	background_full_height.clear();
	invisible.clear();
	weight.clear();
	style.clear();
	underline.clear();
	underline_color.clear();
	strike_through.clear();
	strike_through_color.clear();
}


void tag_properties_t::apply(Glib::RefPtr<Gtk::TextTag> tag) const{
	if(foreground_color){
		tag->property_foreground() = foreground_color();
	}
	if(background_color){
		tag->property_background() = background_color();
	}
	if(background_full_height){
		tag->property_background_full_height() = background_full_height();
	}
	if(invisible){
		tag->property_invisible() = invisible();
	}
	if(weight){
		tag->property_weight() = weight();
	}
	if(style){
		tag->property_style() = style();
	}
	if(underline){
		tag->property_underline() = underline();
	}
	if(underline_color){
		tag->property_underline_rgba() = underline_color();
	}
	if(strike_through){
		tag->property_strikethrough() = strike_through();
	}
	if(strike_through_color){
		tag->property_strikethrough_rgba() = strike_through_color();
	}
}


