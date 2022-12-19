#include <crassus/CompletionProvider.h>

#include <util/strings.h>
#include <util/textbuffer.h>

#include <gtkmm.h>


static bool is_alphanumerical(const gunichar &chr){
	return
		(chr >= '0' && chr <= '9') ||
		(chr >= 'a' && chr <= 'z') ||
		(chr >= 'A' && chr <= 'Z') ||
		(chr == '_') ||
		false;
}


static void iter_backward_alphanumerical(Gtk::TextIter &iter){
	// The same algorithm as the default completion replacement uses
	// https://www.manpagez.com/html/gtksourceview/gtksourceview-3.12.0/GtkSourceCompletionProvider.php#gtk-source-completion-provider-activate-proposal
	while(!iter.starts_line()){
		iter.backward_char();
		if(!is_alphanumerical(iter.get_char())){
			iter.forward_char();
			break;
		}
	}
}


static Gtk::TextIter iter_backward_alphanumerical(const Gtk::TextIter &iter){
	Gtk::TextIter result = iter;
	iter_backward_alphanumerical(result);
	return result;
}


template<class T>
static T abs_diff(const T &a, const T &b){
	if(a > b){
		return a - b;
	}
	return b - a;
}


Glib::RefPtr<CompletionProvider> CompletionProvider::create(std::shared_ptr<const File> file, Glib::RefPtr<Gsv::Buffer> source_buffer){
	return Glib::RefPtr<CompletionProvider>(new CompletionProvider(file, source_buffer));
}


CompletionProvider::CompletionProvider(std::shared_ptr<const File> file, Glib::RefPtr<Gsv::Buffer> source_buffer):
	Glib::ObjectBase(typeid(CompletionProvider)),
	Gsv::CompletionProvider(),
	Glib::Object(),
	file(file),
	source_buffer(source_buffer)
{
	info_widget.show();
	info_widget.set_line_wrap(true);
	info_widget.set_max_width_chars(100);
	info_widget.set_lines(-1);

	const auto theme = Gtk::IconTheme::get_default();
	availability_icons[availability_e::AVAILABLE] = theme->load_icon("emblem-ok-symbolic", 10, Gtk::IconLookupFlags::ICON_LOOKUP_USE_BUILTIN);
	availability_icons[availability_e::DEPRECATED] = theme->load_icon("dialog-warning", 10, Gtk::IconLookupFlags::ICON_LOOKUP_USE_BUILTIN);
	availability_icons[availability_e::NOT_ACCESSIBLE] = theme->load_icon("emblem-readonly", 10, Gtk::IconLookupFlags::ICON_LOOKUP_USE_BUILTIN);
	availability_icons[availability_e::NOT_AVAILABLE] = theme->load_icon("emblem-unreadable", 10, Gtk::IconLookupFlags::ICON_LOOKUP_USE_BUILTIN);
}


source_location_t CompletionProvider::get_location(const Gtk::TextIter &iter) const{
	source_location_t result;
	result.file = file->get_path();
	result.row = iter.get_line() + 1;
	result.column = iter.get_line_offset() + 1;
	result.offset = -1;
	return result;
}


Glib::ustring CompletionProvider::get_name_vfunc() const{
	return name;
}


void CompletionProvider::populate_vfunc(const Glib::RefPtr<Gsv::CompletionContext> &context){
	if(!file->has_translation_unit()){
		return;
	}

	std::vector<completion_t> completions;
	{
		const auto iter_end = context->get_iter();
		const auto location_end = get_location(iter_end);
		const auto iter_start = iter_backward_alphanumerical(iter_end);
		const auto location_start = get_location(iter_start);

		if(last_completion_context == context && location_start.equals(last_start_pos)){
			// reuse last_completions
		}else{
			last_completions = file->code_complete(location_start);
		}
		last_completion_context = context;
		last_start_pos = location_start;
		last_end_pos = location_end;

		const std::string input = iter_start.get_text(iter_end);
		for(const auto &completion : last_completions){
			bool is_relevant = false;
			try{
				const std::string typed_text = completion.get_typed_text();
				is_relevant = is_relevant || string_starts_with(typed_text, input);
			}catch(const std::runtime_error &err){
				// The docs says a completion will always have exactly one TYPED_TEXT.
				// But for example
				//     my_func(
				//             ^
				// will have a completion with just
				//     <void> <my_func> <(> <)>
			}
			if(is_relevant){
				completions.push_back(completion);
			}
		}
	}

	size_t longest_prefix = 0;
	for(const auto &completion : completions){
		size_t prefix_length = 0;
		for(const auto &chunk : completion.chunks){
			if(chunk.type == completion_chunk_type_e::TYPED_TEXT){
				break;
			}else if(chunk.type != completion_chunk_type_e::RETURN_TYPE){
				prefix_length += chunk.text.length();
			}
		}
		longest_prefix = std::max(longest_prefix, std::min(prefix_length, longest_prefix_allowed));
	}

	std::vector<Glib::RefPtr<Gsv::CompletionProposal>> proposals;
	proposal_source.clear();
	for(const auto &completion : completions){
		std::string text;
		std::string prefix = "";
		std::string label_main = "";
		std::string return_type;
		std::string info = "";
		Glib::RefPtr<const Gdk::Pixbuf> icon;
		bool prefix_over = false;
		for(const auto &chunk : completion.chunks){
			info += chunk.text;
			if(chunk.type == completion_chunk_type_e::TYPED_TEXT){
				text = chunk.text;
				label_main += chunk.text;
				prefix_over = true;
			}else if(chunk.type == completion_chunk_type_e::RETURN_TYPE){
				return_type += chunk.text;
				info += " ";
			}else{
				if(prefix_over){
					label_main += chunk.text;
				}else{
					prefix += chunk.text;
				}
			}
		}
		info += "\n\n" + completion.brief_comment;
		const auto find_icon = availability_icons.find(completion.availability);
		if(find_icon != availability_icons.end()){
			icon = find_icon->second;
		}

		std::string label;
		if(prefix.size() <= longest_prefix){
			label = std::string(longest_prefix - prefix.size(), ' ') + prefix;
		}else{
			label = prefix.substr(0, longest_prefix-1) + "…";
		}
		label += label_main;
		if(!return_type.empty()){
			label += " : " + return_type;
		}

		// c_str() to avoid the markup (?) overload.
		auto item = Gsv::CompletionItem::create(label, text, icon, info.c_str());
		proposal_source[item] = completion;
		proposals.push_back(item);
	}

	auto self = Glib::RefPtr<Gsv::CompletionProvider>(this);
	self->reference();
	context->add_proposals(self, proposals, true);
}


bool CompletionProvider::match_vfunc(const Glib::RefPtr<const Gsv::CompletionContext> &context) const{
	if(!file->has_translation_unit()){
		return false;
	}

	const auto iter_end = context->get_iter();
	const auto location_end = get_location(iter_end);
	const auto iter_start = iter_backward_alphanumerical(iter_end);
	const auto location_start = get_location(iter_start);
	const auto activation_reason = context->get_activation();

	if(activation_reason == Gsv::COMPLETION_ACTIVATION_USER_REQUESTED){
		if(
			last_completion_context == context &&
			last_start_pos.file == location_start.file &&
			last_end_pos.file == location_end.file &&
			!last_start_pos.equals(location_start) &&
			(last_end_pos.row == location_end.row && abs_diff(last_end_pos.column, location_end.column) == 1)
		){
			// Prevent completion dragging
			return false;
		}
		// TODO: If user cancels with ESC key and then types a character, we also get a USER_REQUESTED.
		// We want to reject it, but it looks exactly like a _real_ user request.
		// Need out-of-band signaling?
		return true;
	}else if(activation_reason == Gsv::COMPLETION_ACTIVATION_INTERACTIVE){
		const std::vector<std::string> triggers = {
			".",
			"->",
			//"::",
		};
		for(const std::string &act : triggers){
			bool mismatch = false;
			auto current_pos = iter_start;
			for(size_t i=act.length(); i>0; i--){
				if(current_pos.starts_line()){
					mismatch = true;
					break;
				}
				current_pos.backward_char();
				const auto character = act.at(i-1);
				if(character < 0 || current_pos.get_char() != static_cast<unsigned char>(character)){
					mismatch = true;
					break;
				}
			}
			if(!mismatch){
				return true;
			}
		}
	}
	return false;
}


Gsv::CompletionActivation CompletionProvider::get_activation_vfunc() const{
	return Gsv::COMPLETION_ACTIVATION_INTERACTIVE | Gsv::COMPLETION_ACTIVATION_USER_REQUESTED;
}


Gtk::Widget *CompletionProvider::get_info_widget_vfunc(const Glib::RefPtr<const Gsv::CompletionProposal> &/*proposal*/) const{
	// Need to return non-const ptr from const method, wtf?
	return const_cast<Gtk::Label *>(&info_widget);
}


void CompletionProvider::update_info_vfunc(const Glib::RefPtr<const Gsv::CompletionProposal> &proposal, const Gsv::CompletionInfo &/*info*/){
	std::string info = proposal->get_info();
	info_widget.set_text(info);
}


bool CompletionProvider::activate_proposal_vfunc(const Glib::RefPtr<Gsv::CompletionProposal> &proposal, const Gtk::TextIter &/*iter*/){
	const auto find = proposal_source.find(proposal);
	if(find != proposal_source.end()){
		const completion_t &completion = find->second;
		apply_fixits(source_buffer, completion.fixits);
		// Let the source view do the rest
		return false;
	}
	return false;
}


