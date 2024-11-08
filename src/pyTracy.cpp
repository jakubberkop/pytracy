#define PYBIND11_DETAILED_ERROR_MESSAGES 1
#include <pybind11/pybind11.h>

namespace py = pybind11;

#define TRACY_ENABLE
#include <TracyC.h>

#include <mutex>
#include <shared_mutex>
#include <deque>
#include <string_view>

#include "robin_hood.h"

struct PyTracyStackFrame
{
	TracyCZoneCtx tracyCtx;
	bool was_filtered_out;
};

struct ThreadData
{
	std::deque<PyTracyStackFrame> tracy_stack = {};
};

robin_hood::unordered_map<uint64_t, ThreadData*> thread_data_map;
std::shared_mutex thread_data_mutex;

const uint64_t INVALID_SOURCE_INDEX = 0;

// Compare the performance impact for threaded workloads
// Non threaded gets 5% speedup
class SharedLock
{
public:
	SharedLock(std::shared_mutex& mutex) : m_mutex(mutex)
	{
		m_mutex.lock_shared();
	}

	~SharedLock()
	{
		m_mutex.unlock_shared();
	}
private:
	std::shared_mutex& m_mutex;
};

class ExclusiveLock
{
public:
	ExclusiveLock(std::shared_mutex& mutex) : m_mutex(mutex)
	{
		m_mutex.lock();
	}

	~ExclusiveLock()
	{
		m_mutex.unlock();
	}
private:
	std::shared_mutex& m_mutex;
};

std::unordered_set<std::string> black_list;
static bool filtering_enabled = false;
static bool during_black_list_initialization = false;

py::list get_stdlib_paths()
{
	py::module os_module = py::module::import("os");
	py::module sys_module = py::module::import("sys");
	py::module inspect_module = py::module::import("inspect");

	py::function dirname_func = os_module.attr("path").attr("dirname");
	py::function getsourcefile_func = inspect_module.attr("getsourcefile");

	py::str os_module_dir = dirname_func(getsourcefile_func(os_module));

	py::list result;
	result.append(os_module_dir);

	return result;
}

py::list get_libraries_paths()
{
	py::module os_module = py::module::import("os");
	py::module sys_module = py::module::import("sys");
	py::module inspect_module = py::module::import("inspect");

	py::list paths = sys_module.attr("path");

	py::list stdlib_paths = get_stdlib_paths();
	py::list result;

	for (int i = 1; i < paths.size(); i++)
	{
		if (stdlib_paths.contains(paths[i]))
			continue;

		result.append(paths[i]);
	}

	return result;
}

// def set_filtering_mode(stdlib: bool, third_party: bool, user: bool) -> None: ...
py::none set_filtering_mode(bool stdlib, bool third_party, bool user)
{
	black_list.clear();

	if (stdlib)
	{
		for (const auto& path : get_stdlib_paths())
		{
			black_list.insert(path.cast<std::string>());
		}
	}

	if (third_party)
	{
		for (const auto& path : get_libraries_paths())
		{
			black_list.insert(path.cast<std::string>());
		}
	}

	return py::none();
}


// def get_filtering_mode() -> Tuple[bool, bool, bool]: ...
// TODO: Implement
py::tuple get_filtering_mode()
{
	bool stdlib = false;
	bool third_party = false;
	bool user = false;

	return py::make_tuple(stdlib, third_party, user);
}

static void initialize_filtering()
{
	if (filtering_enabled)
		return;

	during_black_list_initialization = true;

	set_filtering_mode(true, true, false);

	py::module sys_module = py::module::import("sys");
	py::list paths = sys_module.attr("path");

	for (int i = 1; i < paths.size(); i++)
	{
		std::string path_string = paths[i].cast<std::string>();
		black_list.insert(std::move(path_string));
	}

	filtering_enabled = true;
	during_black_list_initialization = false;
}

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

bool path_in_excluded_folder(const std::string_view& path)
{
	for (const auto& filter_path : black_list)
	{
		if (starts_with(path, filter_path))
		{
			return true;
		}
	}

	return false;
}

static bool is_path_acceptable(const std::string_view& path)
{
	if (path[0] == '<')
		return false;

	return !path_in_excluded_folder(path);
}

static ThreadData* get_current_thread_data()
{
	uint64_t thread_id = PyThread_get_thread_ident();

	{
		SharedLock lock(thread_data_mutex);

		auto it = thread_data_map.find(thread_id);
		if (it != thread_data_map.end())
		{
			return it->second;
		}
	}

	ExclusiveLock lock(thread_data_mutex);

	auto new_it = thread_data_map.emplace(thread_id, new ThreadData{});
	return new_it.first->second;
}

static uint64_t get_source_index_from_frame(PyFrameObject* frame)
{
	PyCodeObject* code = PyFrame_GetCode(frame);
	assert(code != 0);

	Py_ssize_t file_name_len;
	Py_ssize_t func_name_len;

	const char* file_name = PyUnicode_AsUTF8AndSize(code->co_filename, &file_name_len);
	const char* func_name = PyUnicode_AsUTF8AndSize(code->co_name, &func_name_len);
	int64_t line = code->co_firstlineno;

	if (during_black_list_initialization)
	{
		return INVALID_SOURCE_INDEX;
	}

	if (filtering_enabled)
	{
		std::string_view file_name_str(file_name, file_name_len);

		if (!is_path_acceptable(file_name_str))
		{
			return INVALID_SOURCE_INDEX;
		}
	}

	assert(file_name != 0);
	assert(func_name != 0);

	uint64_t source_index = ___tracy_alloc_srcloc(line, file_name, file_name_len, func_name, func_name_len, 0);

	Py_DECREF(code);

	return source_index;
}

int on_trace_event(PyObject* obj, PyFrameObject* frame, int what, PyObject *arg)
{
	switch (what)
	{
		case PyTrace_CALL:
		{
			uint64_t source_index = get_source_index_from_frame(frame);
			ThreadData* thread_data = get_current_thread_data();

			if (source_index == INVALID_SOURCE_INDEX)
			{
				thread_data->tracy_stack.push_back({ {}, true });
				break;
			}

			auto ctx = ___tracy_emit_zone_begin_alloc(source_index, 1);
			thread_data->tracy_stack.push_back({ctx, false});
			break;
		}
		case PyTrace_EXCEPTION:
			break;
		case PyTrace_LINE:
			break;
		case PyTrace_RETURN:
		{
			ThreadData* thread_data = get_current_thread_data();

			if (thread_data->tracy_stack.size() == 0)
			{
				printf("pytracy internal error: tracy_stack_index == 0\n");
				assert(false);
				break;
			}

			const auto stack_data = thread_data->tracy_stack.back();
			thread_data->tracy_stack.pop_back();

			if (!stack_data.was_filtered_out)
			{
				___tracy_emit_zone_end(stack_data.tracyCtx);
			}

			break;
		}
		case PyTrace_C_CALL:
			break;
		case PyTrace_C_EXCEPTION:
			break;
		case PyTrace_C_RETURN:
			break;
		case PyTrace_OPCODE:
			break;
	}

	return 0;
}

enum class TracingMode
{
	Disabled = 0,
	MarkedFunctions = 1,
	All = 2
};

static TracingMode tracing_mode = TracingMode::Disabled;

void initialize_call_stack(PyFrameObject* frame)
{
	PyFrameObject* back = PyFrame_GetBack(frame);

	if (back)
	{
		initialize_call_stack(back);
		Py_DECREF(back);
	}

	uint64_t source_index = get_source_index_from_frame(frame);
	ThreadData* thread_data = get_current_thread_data();

	if (source_index == INVALID_SOURCE_INDEX)
	{
		thread_data->tracy_stack.push_back({ {}, true });
		return;
	}

	TracyCZoneCtx ctx = ___tracy_emit_zone_begin_alloc(source_index, 1);
	thread_data->tracy_stack.push_back({ctx, false});
}

static void initialize_thread()
{
	// TODO: Consider mode changes during runtime
	if (tracing_mode == TracingMode::Disabled)
	{
		PyEval_SetTrace(NULL, NULL);
	}
	else if (tracing_mode == TracingMode::MarkedFunctions)
	{
		PyThreadState* ts = PyThreadState_Get();
		PyFrameObject* frame = PyThreadState_GetFrame(ts);
		initialize_call_stack(frame);

		PyEval_SetTrace(NULL, NULL);
	}
	else if (tracing_mode == TracingMode::All)
	{
		initialize_filtering();

		PyThreadState* ts = PyThreadState_Get();
		PyFrameObject* frame = PyThreadState_GetFrame(ts);

		initialize_call_stack(frame);

		PyEval_SetTrace(on_trace_event, NULL);
	}
}

static py::none set_tracing_mode(int _mode)
{
	if (_mode < 0 || _mode > 2)
	{
		throw std::invalid_argument("Invalid tracing mode");
	}

	TracingMode mode = static_cast<TracingMode>(_mode);

	if (tracing_mode == mode)
	{
		return py::none();
	}

	tracing_mode = mode;

	initialize_thread();

	return py::none();
}

static ___tracy_source_location_data native_function_source_location = { NULL, "Native function", "<unknown>", 0, 0 };

void mark_function_enter(py::function func)
{
	if (tracing_mode != TracingMode::MarkedFunctions)
	{
		return;
	}

	py::object code_object = func.attr("__code__");
	auto func_name = code_object.attr("co_name").cast<std::string>();
	auto file_name = code_object.attr("co_filename").cast<std::string>();
	auto line = code_object.attr("co_firstlineno").cast<int64_t>();

	if (func.is_cpp_function())
	{
		TracyCZoneCtx ctx = ___tracy_emit_zone_begin(&native_function_source_location, 1);
		ThreadData* thread_data = get_current_thread_data();

		// TODO: For now we are not filtering out native functions
		thread_data->tracy_stack.push_back({ctx, false});
		return;
	}

	uint64_t source_index = ___tracy_alloc_srcloc(line, file_name.c_str(), file_name.size(), func_name.c_str(), func_name.size(), 0);
	ThreadData* thread_data = get_current_thread_data();

	if (source_index == INVALID_SOURCE_INDEX)
	{
		thread_data->tracy_stack.push_back({ {}, true });
		return;
	}

	TracyCZoneCtx ctx = ___tracy_emit_zone_begin_alloc(source_index, 1);
	thread_data->tracy_stack.push_back({ctx, false});
}

void mark_function_exit()
{
	if (tracing_mode != TracingMode::MarkedFunctions)
	{
		return;
	}

	ThreadData* thread_data = get_current_thread_data();

	if (thread_data->tracy_stack.size() == 0)
	{
		printf("pytracy internal error: tracy_stack_index == 0\n");
		return;
	}

	const auto stack_data = thread_data->tracy_stack.back();
	thread_data->tracy_stack.pop_back();
	___tracy_emit_zone_end(stack_data.tracyCtx);
}

py::none initialize_tracing_on_thread_start(py::args args, py::kwargs kwargs)
{
	initialize_thread();
	return py::none();
}

static void patch_threading_module()
{
	py::module threading_module = py::module::import("threading");
	py::function settrace = threading_module.attr("settrace");

	settrace(py::cpp_function(initialize_tracing_on_thread_start));
}

// filtered_out_folders() -> List[str]: ...
py::list get_filtered_out_folders()
{
	py::list result;

	for (const auto& path : black_list)
	{
		result.append(path);
	}

	return result;
}

// set_filtered_out_folders(files: List[str]) -> None: ...
py::none set_filtered_out_folders(py::list files)
{
	// Ensure that files are all strings
	for (const auto& path : files)
	{
		if (!py::isinstance<py::str>(path))
		{
			throw std::invalid_argument("All elements of the list must be strings");
		}
	}

	black_list.clear();

	for (const auto& path : files)
	{
		black_list.insert(path.cast<std::string>());
	}

	return py::none();
}

PYBIND11_MODULE(pytracy, m) {
	m.doc() = "Tracy Profiler bindings for Python";
	m.def("set_tracing_mode", &set_tracing_mode, "Sets Tracy Profiler tracing mode");

	m.def("get_filtered_out_folders", &get_filtered_out_folders, "Returns a list of filtered out folders");

	m.def("set_filtering_mode", &set_filtering_mode, "Sets the filtering mode for the profiler");
	m.def("get_filtering_mode", &get_filtering_mode, "Returns the filtering mode for the profiler");

	py::enum_<TracingMode>(m, "TracingMode")
		.value("Disabled", TracingMode::Disabled)
		.value("MarkedFunctions", TracingMode::MarkedFunctions)
		.value("All", TracingMode::All)
		.export_values();

	patch_threading_module();
};
