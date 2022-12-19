#pragma once

#include <pompeius/File.h>

#include <gtksourceviewmm.h>


class CompletionProvider : public Gsv::CompletionProvider, public Glib::Object{
public:
	static Glib::RefPtr<CompletionProvider> create(std::shared_ptr<const File> file, Glib::RefPtr<Gsv::Buffer> source_buffer);

	CompletionProvider(std::shared_ptr<const File> file, Glib::RefPtr<Gsv::Buffer> source_buffer);

private:
	const Glib::ustring name = "libclang";
	const size_t longest_prefix_allowed = 24;
	Glib::RefPtr<Gdk::Pixbuf> m_pixbuf;
	std::map<availability_e, Glib::RefPtr<Gdk::Pixbuf>> availability_icons;
	Gtk::Label info_widget;

	std::shared_ptr<const File> file;
	Glib::RefPtr<Gsv::Buffer> source_buffer;

	std::map<Glib::RefPtr<const Gsv::CompletionProposal>, completion_t> proposal_source;
	std::vector<completion_t> last_completions;
	Glib::RefPtr<const Gsv::CompletionContext> last_completion_context;
	source_location_t last_start_pos;
	source_location_t last_end_pos;

	source_location_t get_location(const Gtk::TextIter &iter) const;

	Glib::ustring get_name_vfunc() const override;
	//Glib::RefPtr<Gdk::Pixbuf> get_icon_vfunc() override;
	void populate_vfunc(const Glib::RefPtr<Gsv::CompletionContext> &context) override;
	bool match_vfunc(const Glib::RefPtr<const Gsv::CompletionContext> &context) const override;
	Gsv::CompletionActivation get_activation_vfunc() const override;
	Gtk::Widget *get_info_widget_vfunc(const Glib::RefPtr<const Gsv::CompletionProposal> &proposal) const override;
	void update_info_vfunc(const Glib::RefPtr<const Gsv::CompletionProposal> &proposal, const Gsv::CompletionInfo &info) override;
	//bool get_start_iter_vfunc(const Glib::RefPtr<const Gsv::CompletionContext> &context, const Glib::RefPtr<const Gsv::CompletionProposal> &proposal, Gtk::TextIter &iter) override;
	bool activate_proposal_vfunc(const Glib::RefPtr<Gsv::CompletionProposal> &proposal, const Gtk::TextIter &iter) override;
	//int get_interactive_delay_vfunc() const override;
	//int get_priority_vfunc() const override;
};

