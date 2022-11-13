#include <pompeius/TUServer.h>



TUServer::TUServer() :
	parsing_pool(2)
{
	index = std::make_shared<Index>(false, false);
	parsing_pool.start();
}


TUServer::~TUServer(){
	parsing_pool.close();
	parsing_pool.cancel();
	parsing_pool.join();
}


void TUServer::register_observer(TUObserver *obs, bool send_digest){
	{
		std::lock_guard<std::mutex> lock(mutex_observers);
		observers.insert(obs);
	}
	if(send_digest){
		std::lock_guard<std::mutex> lock(mutex);
		for(const auto &entry_1 : translation_units){
			for(const auto &entry_2 : entry_1.second){
				obs->notify_new_tu(entry_2.second);
			}
		}
	}
}


void TUServer::unregister_observer(TUObserver *obs){
	std::lock_guard<std::mutex> lock(mutex_observers);
	observers.erase(obs);
}


void TUServer::update(const CompilationDatabase &database){
	std::vector<std::shared_ptr<TranslationUnit>> new_units;
	std::vector<std::shared_ptr<TranslationUnit>> deleted_units;
	size_t deleted_uninit_units = 0;

	{
		std::lock_guard<std::mutex> lock(mutex);

		const CompilationDatabase new_entries = current_database.diffplus(database);
		const CompilationDatabase disappeared_entries = database.diffplus(current_database);

		for(const auto &entry : new_entries){
			const std::string &file = entry.first;
			for(const auto &args : entry.second){
				auto unit = std::make_shared<TranslationUnit>(index, file, args);
				unit->set_parser(this);
				unit->set_parent(this);
				translation_units[file][args] = unit;
				translation_units_lookup[unit.get()] = unit;
				uninitialized_translation_units.insert(unit.get());
				new_units.push_back(unit);
			}
		}

		for(const auto &entry : disappeared_entries){
			const std::string &file = entry.first;
			auto find_file = translation_units.find(file);
			if(find_file != translation_units.end()){
				auto &sub_map = find_file->second;
				for(const auto &args : entry.second){
					auto find_args = sub_map.find(args);
					if(find_args != sub_map.end()){
						std::shared_ptr<TranslationUnit> unit = find_args->second;
						sub_map.erase(find_args);
						translation_units_lookup.erase(unit.get());
						{
							const auto iter = uninitialized_translation_units.find(unit.get());
							if(iter != uninitialized_translation_units.end()){
								deleted_uninit_units++;
								uninitialized_translation_units.erase(iter);
							}
						}
						deleted_units.push_back(unit);
					}
				}
			}
		}

		current_database = database;
	}

	{
		std::lock_guard<std::mutex> lock(mutex_counters);
		num_units += new_units.size();
		num_uninit_units += new_units.size();
		num_units -= deleted_units.size();
		num_uninit_units -= deleted_uninit_units;
		for(auto unit : deleted_units){
			if(unit->is_ready()){
				num_ready_units--;
			}
		}
	}
	{
		std::set<TUObserver *> to_notify;
		{
			std::lock_guard<std::mutex> lock(mutex_observers);
			to_notify = observers;
		}
		for(auto *obs : to_notify){
			for(auto unit : new_units){
				obs->notify_new_tu(unit);
			}
			for(auto unit : deleted_units){
				obs->notify_deleted_tu(unit);
			}
		}
	}
}


std::set<std::shared_ptr<TranslationUnit>> TUServer::get_translation_units() const{
	std::lock_guard<std::mutex> lock(mutex);
	std::set<std::shared_ptr<TranslationUnit>> result;
	for(const auto &entry_1 : translation_units){
		for(const auto &entry_2 : entry_1.second){
			result.insert(entry_2.second);
		}
	}
	return result;
}


std::set<std::shared_ptr<TranslationUnit>> TUServer::get_translation_units(const std::string &file) const{
	std::lock_guard<std::mutex> lock(mutex);
	std::set<std::shared_ptr<TranslationUnit>> result;
	auto find_file = translation_units.find(file);
	if(find_file != translation_units.end()){
		for(const auto &entry : find_file->second){
			result.insert(entry.second);
		}
	}
	return result;
}


size_t TUServer::count_units() const{
	std::lock_guard<std::mutex> lock(mutex_counters);
	return num_units;
}


size_t TUServer::count_init_units() const{
	std::lock_guard<std::mutex> lock(mutex_counters);
	return num_units - num_uninit_units;
}


size_t TUServer::count_ready_units() const{
	std::lock_guard<std::mutex> lock(mutex_counters);
	return num_ready_units;
}


void TUServer::parse_this(TranslationUnit *unit_ptr){
	std::shared_ptr<TranslationUnit> unit;
	{
		std::lock_guard<std::mutex> lock(mutex);
		auto find = translation_units_lookup.find(unit_ptr);
		if(find != translation_units_lookup.end()){
			unit = find->second;
		}
	}
	if(unit){
		std::lock_guard<std::mutex> lock(mutex_parsing_requests);
		auto find = parsing_requests.find(unit.get());
		if(find == parsing_requests.end()){
			parsing_requests[unit.get()] = false;
			parsing_pool.add_job(std::bind(&TUServer::parsing_loop, this, unit));
		}else{
			find->second = true;
		}
	}
}


void TUServer::auto_parse(){
	if(parsing_pool.get_load() >= auto_parse_threshold){
		return;
	}

	TranslationUnit *unit = NULL;
	{
		std::lock_guard<std::mutex> lock(mutex);
		if(!uninitialized_translation_units.empty()){
			std::lock_guard<std::mutex> lock(mutex_parsing_requests);
			for(auto iter=uninitialized_translation_units.begin(); iter!= uninitialized_translation_units.end(); iter++){
				auto *candidate = *iter;
				if(parsing_requests.find(candidate) == parsing_requests.end()){
					unit = candidate;
					break;
				}
			}
		}
	}
	if(unit){
		parse_this(unit);
	}
}


void TUServer::state_changed(TranslationUnit *unit, bool was_init, bool was_ready, bool is_init, bool is_ready){
	if(was_ready != is_ready){
		std::lock_guard<std::mutex> lock(mutex_counters);
		if(was_ready && !is_ready){
			num_ready_units--;
		}else if(!was_ready && is_ready){
			num_ready_units++;
		}
	}
	if(!was_init && is_init){
		std::lock_guard<std::mutex> lock(mutex);
		const auto find = uninitialized_translation_units.find(unit);
		if(find != uninitialized_translation_units.end()){
			uninitialized_translation_units.erase(find);
			std::lock_guard<std::mutex> lock(mutex_counters);
			num_uninit_units--;
		}
	}
}


void TUServer::parsing_loop(std::shared_ptr<TranslationUnit> unit){
	const std::string file = unit->get_main_file();
	bool done = false;
	while(!done){
		parse_translation_unit(unit);
		{
			std::lock_guard<std::mutex> lock(mutex_parsing_requests);
			auto find = parsing_requests.find(unit.get());
			if(find == parsing_requests.end()){
				// should not happen, but if it does, I guess we are done
				done = true;
			}else{
				done = !find->second;
				find->second = false;
				if(done){
					parsing_requests.erase(find);
				}
			}
		}
	}
	{
		std::set<TUObserver *> to_notify;
		{
			std::lock_guard<std::mutex> lock(mutex_observers);
			to_notify = observers;
		}
		for(auto *obs : to_notify){
			obs->notify_updated_tu(unit);
		}
	}
}

