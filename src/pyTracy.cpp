#define PYBIND11_DETAILED_ERROR_MESSAGES 0
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

class SharedLock
{
public:
	SharedLock(std::shared_mutex& mutex) : mutex(mutex)
	{
		mutex.lock_shared();
	}

	~SharedLock()
	{
		mutex.unlock_shared();
	}
private:
	std::shared_mutex& mutex;
};

class ExclusiveLock
{
public:
	ExclusiveLock(std::shared_mutex& mutex) : mutex(mutex)
	{
		mutex.lock();
	}

	~ExclusiveLock()
	{
		mutex.unlock();
	}
private:
	std::shared_mutex& mutex;
};

std::unordered_set<std::string> filter_list;
static bool filtering_enabled = false;
static bool during_filter_list_initialization = false;

struct PyTracyGlobalImports
{
	PyTracyGlobalImports()
	{
		pybind11::gil_scoped_acquire gil;

		os_module = py::module::import("os");
		sys_module = py::module::import("sys");
		inspect_module = py::module::import("inspect");
		threading_module = py::module::import("threading");

		inspect_currentframe = inspect_module.attr("currentframe");
		inspect_getmodule =  inspect_module.attr("getmodule");
	}

	PyTracyGlobalImports &operator=(const PyTracyGlobalImports&) = delete;
	PyTracyGlobalImports(const PyTracyGlobalImports&) = delete;

	PyTracyGlobalImports &operator=(PyTracyGlobalImports&& other)
	{
		pybind11::gil_scoped_acquire gil;

		py::gil_scoped_acquire acquire;
		os_module = std::move(other.os_module);
		sys_module = std::move(other.sys_module);
		inspect_module = std::move(other.inspect_module);
		threading_module = std::move(other.threading_module);
		inspect_currentframe = std::move(other.inspect_currentframe);
		inspect_getmodule = std::move(other.inspect_getmodule);

		return *this;
	}

	PyTracyGlobalImports(PyTracyGlobalImports&& other)
	{
		pybind11::gil_scoped_acquire gil;

		os_module = std::move(other.os_module);
		sys_module = std::move(other.sys_module);
		inspect_module = std::move(other.inspect_module);
		threading_module = std::move(other.threading_module);
		inspect_currentframe = std::move(other.inspect_currentframe);
		inspect_getmodule = std::move(other.inspect_getmodule);
	}

	py::module os_module;
	py::module sys_module;
	py::module inspect_module;
	py::module threading_module;
	py::object inspect_currentframe;
	py::object inspect_getmodule;
};

std::optional<PyTracyGlobalImports> global_imports = std::nullopt;

PyTracyGlobalImports& get_global_imports()
{
	if (!global_imports.has_value())
	{
		global_imports = PyTracyGlobalImports{};
	}

	return global_imports.value();
}

py::list get_stdlib_paths()
{
	py::module os_module = get_global_imports().os_module;
	py::module sys_module = get_global_imports().sys_module;
	py::module inspect_module = get_global_imports().inspect_module;

	py::function dirname_func = os_module.attr("path").attr("dirname");
	py::function getsourcefile_func = inspect_module.attr("getsourcefile");

	py::str os_module_dir = dirname_func(getsourcefile_func(os_module));

	py::list result;
	result.append(os_module_dir);

	return result;
}

py::list get_libraries_paths()
{
	py::module os_module = get_global_imports().os_module;
	py::module sys_module = get_global_imports().sys_module;
	py::module inspect_module = get_global_imports().inspect_module;

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
	filter_list.clear();

	if (stdlib)
	{
		for (const auto& path : get_stdlib_paths())
		{
			filter_list.insert(path.cast<std::string>());
		}
	}

	if (third_party)
	{
		for (const auto& path : get_libraries_paths())
		{
			filter_list.insert(path.cast<std::string>());
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

	during_filter_list_initialization = true;

	set_filtering_mode(true, true, false);

	py::module sys_module = py::module::import("sys");
	py::list paths = sys_module.attr("path");

	for (int i = 1; i < paths.size(); i++)
	{
		std::string path_string = paths[i].cast<std::string>();
		filter_list.insert(std::move(path_string));
	}

	filtering_enabled = true;
	during_filter_list_initialization = false;
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
	for (const auto& filter_path : filter_list)
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

struct ProcessedFunctionData
{
	std::string file_name;
	std::string func_name;
	std::string full_qual_name;
	int64_t line;
};

// TODO: Make multi-threaded
robin_hood::unordered_map<std::pair<PyCodeObject*, PyFrameObject*>, ProcessedFunctionData*> function_data;

ProcessedFunctionData* get_function_data(PyCodeObject* code, PyFrameObject* frame)
{
	const auto pair = std::make_pair(code, frame);
	auto it = function_data.find(pair);
	if (it != function_data.end())
	{
		return it->second;
	}

	Py_ssize_t file_name_len;
	Py_ssize_t func_name_len;
	Py_ssize_t qual_name_len;

	const char* file_name = PyUnicode_AsUTF8AndSize(code->co_filename, &file_name_len);
	const char* func_name = PyUnicode_AsUTF8AndSize(code->co_name, &func_name_len);
	const char* qual_name = PyUnicode_AsUTF8AndSize(code->co_qualname, &qual_name_len);

	int64_t line = code->co_firstlineno;

	if (filtering_enabled)
	{
		std::string_view file_name_str(file_name, file_name_len);

		if (!is_path_acceptable(file_name_str))
		{
			return nullptr;
		}
	}

	assert(file_name != 0);
	assert(func_name != 0);
	assert(qual_name != 0);

	std::string full_qual_name;

	py::handle f_back = py::handle((PyObject*)frame);
	py::object module = get_global_imports().inspect_getmodule(f_back);

	if (module.is_none())
	{
		full_qual_name = std::string(qual_name, (size_t)qual_name_len);
	}
	else
	{
		py::str module_name = module.attr("__name__");
		full_qual_name = module_name.cast<std::string>() + "." + std::string(qual_name, (size_t)qual_name_len);
	}

	// This purposefully leaks memory, as it has to be kept alive for the entire program lifetime
	auto data = new ProcessedFunctionData{ file_name, func_name, full_qual_name, line };

	function_data[pair] = data;

	return data;
}

static uint64_t get_source_index_from_frame(PyFrameObject* frame)
{
	if (during_filter_list_initialization)
	{
		return INVALID_SOURCE_INDEX;
	}

	PyCodeObject* code = PyFrame_GetCode(frame);
	// We purposefully dont Py_DECREF(code), as we need to keep it alive for the entire program lifetime
	// This way we can know if we have already processed this code object
	assert(code != 0);

	ProcessedFunctionData* data = get_function_data(code, frame);
	if (data == nullptr)
	{
		return INVALID_SOURCE_INDEX;
	}

	uint64_t source_index = ___tracy_alloc_srcloc(
		data->line,
		data->file_name.c_str(), data->file_name.size(),
		data->full_qual_name.c_str(), data->full_qual_name.size(),
		0
	);

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
	py::module threading_module = get_global_imports().threading_module;
	py::function settrace = threading_module.attr("settrace");

	settrace(py::cpp_function(initialize_tracing_on_thread_start));
}

// filtered_out_folders() -> List[str]: ...
py::list get_filtered_out_folders()
{
	py::list result;

	for (const auto& path : filter_list)
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

	filter_list.clear();

	for (const auto& path : files)
	{
		filter_list.insert(path.cast<std::string>());
	}

	return py::none();
}

PYBIND11_MODULE(pytracy, m) {
	m.doc() = "Tracy Profiler bindings for Python";
	m.def("set_tracing_mode", &set_tracing_mode, "Sets Tracy Profiler tracing mode");

	m.def("set_filtered_out_folders", &set_filtered_out_folders, "Sets which folders should be ignored while profiling");
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
