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

std::string replace_all(const std::string& str, const std::string& from, const std::string& to)
{
	std::string result;
	result.reserve(str.size());

	size_t start = 0;
	size_t end = str.find(from);

	while (end != std::string::npos)
	{
		result.append(str, start, end - start);
		result.append(to);

		start = end + from.size();
		end = str.find(from, start);
	}

	result.append(str, start, end);

	return result;
}
