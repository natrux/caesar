#pragma once

#include <brutus/CompilationDatabase.h>
#include <brutus/Index.h>

#include <pompeius/TranslationUnit.h>
#include <pompeius/TUObservable.h>
#include <pompeius/TUParser.h>
#include <pompeius/TUParent.h>

#include <util/ThreadPool.h>

#include <mutex>


class TUServer : public TUParser, public TUParent, public TUObservable{
public:
	TUServer();
	~TUServer();
	void register_observer(TUObserver *obs, bool send_digest) override;
	void unregister_observer(TUObserver *obs) override;
	void update(const CompilationDatabase &database);
	std::set<std::shared_ptr<TranslationUnit>> get_translation_units() const;
	std::set<std::shared_ptr<TranslationUnit>> get_translation_units(const std::string &file) const;
	size_t count_units() const;
	size_t count_init_units() const;
	size_t count_ready_units() const;
	void parse_this(TranslationUnit *unit) override;
	void auto_parse();

	void state_changed(TranslationUnit *unit, bool was_init, bool was_ready, bool is_init, bool is_ready) override;

private:
	mutable std::mutex mutex;
	std::shared_ptr<Index> index;
	const size_t num_threads = 2;
	ThreadPool parsing_pool;
	double auto_parse_threshold = 1.0;
	CompilationDatabase current_database;
	std::map<std::string, std::map<std::vector<std::string>, std::shared_ptr<TranslationUnit>>> translation_units;
	std::map<TranslationUnit *, std::shared_ptr<TranslationUnit>> translation_units_lookup;
	std::set<TranslationUnit *> uninitialized_translation_units;
	std::mutex mutex_parsing_requests;
	std::map<TranslationUnit *, bool> parsing_requests;
	std::mutex mutex_observers;
	std::set<TUObserver *> observers;
	mutable std::mutex mutex_counters;
	size_t num_units = 0;
	size_t num_uninit_units = 0;
	size_t num_ready_units = 0;

	void parsing_loop(std::shared_ptr<TranslationUnit> unit);
};

