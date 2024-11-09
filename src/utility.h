#pragma once

#include <shared_mutex>

class SharedLock
{
public:
	SharedLock(SharedLockableBase(std::shared_mutex)& mutex) : mutex(mutex)
	{
		mutex.lock_shared();
	}

	~SharedLock()
	{
		mutex.unlock_shared();
	}
private:
	SharedLockableBase(std::shared_mutex)& mutex;
};

class ExclusiveLock
{
public:
	ExclusiveLock(SharedLockableBase(std::shared_mutex)& mutex) : mutex(mutex)
	{
		mutex.lock();
	}

	~ExclusiveLock()
	{
		mutex.unlock();
	}
private:
	SharedLockableBase(std::shared_mutex)& mutex;
};


static std::vector<std::string> split_path(const std::string& path)
{
	std::vector<std::string> result;

	size_t start = 0;
	size_t end = path.find('\\');

	while (end != std::string::npos)
	{
		result.push_back(path.substr(start, end - start));
		start = end + 1;
		end = path.find('\\', start);
	}

	result.push_back(path.substr(start, end));

	return result;
}

inline bool starts_with(const std::string_view& str, const std::string_view& prefix)
{
	const size_t prefix_size = prefix.size();

	if (str.size() < prefix_size)
	{
		return false;
	}
	
	for (size_t i = 0; i < prefix_size; i++)
	{
		if (str[i] != prefix[i])
		{
			return false;
		}
	}

	return true;
}

bool path_in_excluded_folder(const std::string_view& path, const std::unordered_set<std::string>& filter_list)
{
	for (const auto& filter_path : filter_list)
	{
		if (starts_with(path, filter_path))
		{
			return true;
		}
	}

	return false;
}

static bool is_path_acceptable(const std::string_view& path, const std::unordered_set<std::string>& filter_list)
{
	if (path[0] == '<')
		return false;

	return !path_in_excluded_folder(path, filter_list);
}
