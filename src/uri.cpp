#include <util/uri.h>
#include <util/strings.h>

#include <set>
#include <map>


std::vector<std::string> uri_short(const std::vector<std::string> &input, char separator, const std::string &ellipsis){
	std::vector<std::vector<std::string>> gap_set;
	std::vector<std::set<size_t>> clusters(1);
	size_t max_position = 0;
	for(size_t i=0; i<input.size(); i++){
		const auto &entry = input[i];
		gap_set.push_back(string_split(entry, separator));
		clusters[0].insert(i);
		max_position = std::max(max_position, gap_set.back().size());
	}

	size_t position = 0;
	while(position < max_position){
		std::vector<std::set<size_t>> new_clusters;
		for(const auto &cluster : clusters){
			if(cluster.size() == 1){
				auto &entry = gap_set[*cluster.begin()];
				if(position > 0 && position < entry.size()){
					entry[entry.size()-position-1].clear();
				}
				new_clusters.push_back(cluster);
				continue;
			}
			// See if position makes a difference
			std::map<std::string, std::set<size_t>> collection;
			for(size_t index : cluster){
				const auto &entry = gap_set[index];
				std::string key = entry[entry.size()-position-1];
				collection[key].insert(index);
			}

			if(position != 0 && collection.size() <= 1){
				// position does not make difference, punch a few holes
				for(size_t index : cluster){
					auto &entry = gap_set[index];
					entry[entry.size()-position-1].clear();
				}
			}
			for(const auto &entry : collection){
				new_clusters.push_back(entry.second);
			}
		}

		clusters = new_clusters;
		position++;
	}


	std::vector<std::string> result;
	for(const auto &entry : gap_set){
		std::string path;
		size_t i = 0;
		while(i < entry.size()){
			if(entry[i].empty()){
				if(!path.empty()){
					path += ellipsis;
					path.push_back(separator);
				}
				// by construction, the last crumb (the filename) is never empty.
				while(entry[i].empty()){
					i++;
				}
			}

			path += entry[i];
			if(i < entry.size()-1){
				path.push_back(separator);
			}
			i++;
		}
		result.push_back(path);
	}

	return result;
}


std::vector<std::string> file_path_short(const std::vector<std::string> &input){
#ifdef _WIN32
	const char path_sep = '\\';
#else
	const char path_sep = '/';
#endif
	return uri_short(input, path_sep, "...");
}


