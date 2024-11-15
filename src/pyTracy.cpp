#define TRACY_ENABLE
#include <TracyC.h>
#include <Tracy.hpp>

#include <mutex>
#include <shared_mutex>
#include <deque>
#include <string_view>
#include <fstream>
#include <iostream>

#include "robin_hood.h"

// #define PYTRACY_DEBUG
// #define PYTRACY_PROFILE
// #define PYTRACY_DEBUG_ALLOW_ALL


#ifdef PYTRACY_PROFILE

TracyCZoneCtx pytracy_zone_start(uint64_t srcloc, int active)
{
	return{};
}

void pytracy_zone_end(TracyCZoneCtx ctx)
{
}

#else

#undef ZoneScoped
#undef ZoneScopedN
#define ZoneScoped do { } while (0);
#define ZoneScopedN(name) do { } while (0);

#undef TracySharedLockable
#define TracySharedLockable(type, varname) type varname
#undef SharedLockableBase
#define SharedLockableBase(type) type


TracyCZoneCtx pytracy_zone_start(uint64_t srcloc, int active)
{
	return ___tracy_emit_zone_begin_alloc(srcloc, active);
}

void pytracy_zone_end(TracyCZoneCtx ctx)
{
	___tracy_emit_zone_end(ctx);
}

#endif

#define PYBIND11_DETAILED_ERROR_MESSAGES 0
#include <pybind11/pybind11.h>

namespace py = pybind11;

#include "utility.h"

struct ProcessedFunctionData
{
	int64_t line;

	std::string file_name;
	std::string func_name;
	std::string full_qual_name;

	// Caches the results of filtering, so that it doesn't have to be checked on every call
	// It needs to be recalculated when the filtering mode changes (is_filtered_out_dirty == true)
	bool is_filtered_out_internal;
	bool is_filtered_out_dirty;
};

struct PyTracyStackFrame
{
	TracyCZoneCtx tracyCtx;
	bool is_active;
	ProcessedFunctionData* func_data;
};

struct ThreadData
{
	std::deque<PyTracyStackFrame> tracy_stack = {};
	uint64_t thread_id;

#ifdef PYTRACY_DEBUG
	bool is_on_event = false;
	std::ofstream file;
#endif
};

py::object on_trace_event_wrapper(py::args args, py::kwargs kwargs);

enum class TracingMode
{
	Disabled = 0,
	All = 2
};

struct PyTracyState;
static void initialize_filtering(PyTracyState& state);

struct PyTracyState
{
	TracingMode tracing_mode = TracingMode::Disabled;

	// Filtering
	std::unordered_set<std::string> filter_list;

	robin_hood::unordered_map<uint64_t, ThreadData*> thread_data_map;
	TracySharedLockable(std::shared_mutex, thread_data_mutex);

	// Function data
	robin_hood::unordered_map<PyCodeObject*, ProcessedFunctionData*> function_data;
	TracySharedLockable(std::shared_mutex, function_data_mutex);

	// Python interface
	// TraceFunction: TypeAlias = Callable[[FrameType, str, Any], TraceFunction | None]
	// def settrace(function: TraceFunction | None, /) -> None: ...
	py::cpp_function on_trace_event_wrapped;

	py::module os_module;
	py::module sys_module;
	py::module inspect_module;
	py::module threading_module;
	py::object inspect_currentframe;
	py::object inspect_getmodule;

#ifdef PYTRACY_DEBUG
	std::ofstream global_stack_file{"threads/global_stack.txt"};
#endif

	// This is intentionally leaked, as it has to be kept alive for the entire program lifetime
	static PyTracyState* instance;
	static bool during_initialization;

	static PyTracyState& the()
	{
		assert(!during_initialization);

		if (!instance)
		{
			during_initialization = true;
			instance = new PyTracyState{};
			during_initialization = false;
		}
		return *instance;
	}

	PyTracyState()
	{
		// Initialization needs to happen after the Python interpreter has been initialized,
		// and we hold the GIL
		assert(PyGILState_Check());

		os_module = py::module::import("os");
		sys_module = py::module::import("sys");
		inspect_module = py::module::import("inspect");
		threading_module = py::module::import("threading");

		inspect_currentframe = inspect_module.attr("currentframe");
		inspect_getmodule =  inspect_module.attr("getmodule");

		on_trace_event_wrapped = py::cpp_function(on_trace_event_wrapper);

		initialize_filtering(*this);
	}

};

PyTracyState* PyTracyState::instance = nullptr;
bool PyTracyState::during_initialization = false;

#ifdef PYTRACY_DEBUG

void _print_stack(ThreadData& thread, std::string operation, std::ostream& file)
{
	file << "T:" << thread.thread_id << " Q: " << thread.tracy_stack.size() << " " << operation << std::endl;
	for (auto& frame : thread.tracy_stack)
	{
		file << frame.func_data->full_qual_name << std::endl;
	}
}

void print_stack(ThreadData& thread, std::string operation)
{
	PyTracyState& state = PyTracyState::the();

	_print_stack(thread, operation, thread.file);
	_print_stack(thread, operation, state.global_stack_file);
	_print_stack(thread, operation, std::cout);
}
#endif

py::list internal_get_stdlib_paths(PyTracyState& state)
{
	py::module os_module = state.os_module;
	py::module sys_module = state.sys_module;
	py::module inspect_module = state.inspect_module;

	py::function dirname_func = os_module.attr("path").attr("dirname");
	py::function getsourcefile_func = inspect_module.attr("getsourcefile");

	py::str os_module_dir = dirname_func(getsourcefile_func(os_module));

	py::list result;
	result.append(os_module_dir);

	return result;
}

py::list get_stdlib_paths()
{
	PyTracyState& state = PyTracyState::the();
	return internal_get_stdlib_paths(state);
}

py::list internal_get_libraries_paths(PyTracyState& state)
{
	py::module sys_module = state.sys_module;
	py::list paths = sys_module.attr("path");

	py::list stdlib_paths = internal_get_stdlib_paths(state);
	py::list result;

	for (int i = 1; i < paths.size(); i++)
	{
		if (stdlib_paths.contains(paths[i]))
			continue;

		result.append(paths[i]);
	}

	return result;
}

py::list get_libraries_paths()
{
	PyTracyState& state = PyTracyState::the();
	return internal_get_libraries_paths(state);
}

void make_function_is_filtered_out_dirty(PyTracyState& state)
{
	ZoneScoped;

	py::gil_scoped_release release;

	ExclusiveLock lock(state.function_data_mutex);
	// TODO: Performance, we don't have to capture whole function_data_mutex

	for (auto& [_, data] : state.function_data)
	{
		data->is_filtered_out_dirty = true;
	}
}

void internal_set_filtering_mode(bool stdlib, bool third_party, bool user, PyTracyState& state)
{
	state.filter_list.clear();

	if (stdlib)
	{
		for (const auto& path : internal_get_stdlib_paths(state))
		{
			state.filter_list.insert(path.cast<std::string>());
		}
	}

	if (third_party)
	{
		for (const auto& path : internal_get_libraries_paths(state))
		{
			state.filter_list.insert(path.cast<std::string>());
		}
	}

	make_function_is_filtered_out_dirty(state);
}

// def set_filtering_mode(stdlib: bool, third_party: bool, user: bool) -> None: ...
py::none set_filtering_mode(bool stdlib, bool third_party, bool user)
{
	PyTracyState& state = PyTracyState::the();
	internal_set_filtering_mode(stdlib, third_party, user, state);
	return py::none();
}

static void initialize_filtering(PyTracyState& state)
{
	internal_set_filtering_mode(true, true, false, state);
}

static void initialize_call_stack(PyFrameObject* frame, ThreadData* thread_data);

static thread_local ThreadData* current_thread_data = nullptr;
static ThreadData* get_current_thread_data(PyFrameObject* frame, bool& just_initialized);
static ThreadData* get_current_thread_data_impl(PyFrameObject* frame, bool& just_initialized);

static ThreadData* get_current_thread_data(PyFrameObject* frame, bool& just_initialized)
{
	ZoneScoped;

	if(current_thread_data)
	{
		just_initialized = false;
		assert(current_thread_data->thread_id == PyThread_get_thread_ident());
		return current_thread_data;
	}

	current_thread_data = get_current_thread_data_impl(frame, just_initialized);
	return current_thread_data;
}

static ThreadData* get_current_thread_data_impl(PyFrameObject* frame, bool& just_initialized)
{
	ZoneScoped;

	assert(!PyGILState_Check());

	uint64_t thread_id;
	
	{
		py::gil_scoped_acquire acquire;
		thread_id = PyThread_get_thread_ident();
	}

	PyTracyState& state = PyTracyState::the();

	{
		SharedLock lock(state.thread_data_mutex);

		auto it = state.thread_data_map.find(thread_id);
		if (it != state.thread_data_map.end())
		{
			just_initialized = false;
			return it->second;
		}
	}

	ExclusiveLock lock(state.thread_data_mutex);

	auto new_it = state.thread_data_map.emplace(thread_id, new ThreadData{});

	ThreadData* thread_data = new_it.first->second;
	thread_data->thread_id = thread_id;

#ifdef PYTRACY_DEBUG
	std::string file_name = "threads/" + std::to_string(thread_id) + ".txt";
	thread_data->file = std::ofstream{file_name};
#endif

	assert(thread_data);
	{
		py::gil_scoped_acquire acquire;
		initialize_call_stack(frame, thread_data);
	}

	just_initialized = true;

	return new_it.first->second;
}

namespace std
{
	template<>
	struct hash<std::pair<PyCodeObject*, PyFrameObject*>>
	{
		size_t operator()(const std::pair<PyCodeObject*, PyFrameObject*>& pair) const
		{
			return std::hash<void*>{}(pair.first) ^ std::hash<void*>{}(pair.second);
		}
	};
}

ProcessedFunctionData* get_function_data(PyCodeObject* code, PyFrameObject* frame)
{
	ZoneScoped;

	assert(!PyGILState_Check());

	PyTracyState& state = PyTracyState::the();

	{
		SharedLock lock(state.function_data_mutex);

		auto it = state.function_data.find(code);
		if (it != state.function_data.end())
		{
			return it->second;
		}
	}

	ExclusiveLock lock(state.function_data_mutex);
	py::gil_scoped_acquire acquire;

	assert(PyGILState_Check());

	// This is done on purpose, as we need to keep code alive for the entire program lifetime
	// This way we can be sure that the pointer is always valid
	Py_INCREF(code);

	Py_ssize_t file_name_len;
	Py_ssize_t func_name_len;
	Py_ssize_t qual_name_len;

	const char* file_name = PyUnicode_AsUTF8AndSize(code->co_filename, &file_name_len);
	const char* func_name = PyUnicode_AsUTF8AndSize(code->co_name, &func_name_len);
	const char* qual_name = PyUnicode_AsUTF8AndSize(code->co_qualname, &qual_name_len);

	int64_t line = code->co_firstlineno;

	assert(file_name != 0);
	assert(func_name != 0);
	assert(qual_name != 0);

	std::string full_qual_name;

	py::handle f_back = py::handle((PyObject*)frame);
	py::object module = py::none{};

	try
	{
		module = state.inspect_getmodule(f_back);
	}
	catch(...)
	{
		// At shutdown, getmodule can throw an exception
		// Best course of action is to ignore it, since module name is not critical
	}

	if (module.is_none())
	{
		full_qual_name = std::string(qual_name, (size_t)qual_name_len);
	}
	else
	{
		py::str module_name = module.attr("__name__");
		full_qual_name = module_name.cast<std::string>() + "." + std::string(qual_name, (size_t)qual_name_len);
	}

	full_qual_name = replace_all(full_qual_name, ".", "::");

	py::gil_scoped_release release2;

	std::string_view file_name_str(file_name, file_name_len);
	bool is_filtered_out = !is_path_acceptable(file_name_str, state.filter_list);

	// This purposefully leaks memory, as it has to be kept alive for the entire program lifetime
	auto data = new ProcessedFunctionData{ line, file_name, func_name, full_qual_name, is_filtered_out, false};
	state.function_data[code] = data;

	return data;
}

bool update_should_be_filtered_out(ProcessedFunctionData* data)
{
	ZoneScoped;

#ifdef PYTRACY_DEBUG_ALLOW_ALL
	return false;
#endif

	PyTracyState& state = PyTracyState::the();

	if (data->is_filtered_out_dirty)
	{
		// TODO: Performance. We don't have to capture whole function_data_mutex
		ExclusiveLock lock(state.function_data_mutex);

		data->is_filtered_out_internal = !is_path_acceptable(data->file_name, state.filter_list);
		data->is_filtered_out_dirty = false;
	}

	return data->is_filtered_out_internal;
}

static uint64_t get_source_index_from_frame(PyFrameObject* frame, PyCodeObject* code, bool& is_active)
{
	ZoneScoped;
	assert(!PyGILState_Check());

	ProcessedFunctionData* data = get_function_data(code, frame);

	is_active = !update_should_be_filtered_out(data);

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
	ZoneScoped;
	assert(PyGILState_Check());

	switch (what)
	{
		case PyTrace_CALL:
		{
			ZoneScopedN("PyTrace_CALL");
			// Return value: New reference.
			// Return a strong reference.
			PyCodeObject* code = PyFrame_GetCode(frame);
			assert(code);

			py::gil_scoped_release release;

			bool just_initialized;
			ThreadData* thread_data = get_current_thread_data(frame, just_initialized);

			bool is_active = false;
			uint64_t source_index = get_source_index_from_frame(frame, code, is_active);

			ProcessedFunctionData* data = nullptr;

#ifdef PYTRACY_DEBUG
			assert(!thread_data->is_on_event);
			thread_data->is_on_event = true;
			data = get_function_data(code, frame);
#endif

			// If we are just initializing the stack, we don't want to push the current frame.
			// This leads to a duplicate entry in the stack
			if (!just_initialized)
			{
				TracyCZoneCtx ctx = pytracy_zone_start(source_index, is_active);
				thread_data->tracy_stack.push_back({ctx, is_active, data});
			}


#ifdef PYTRACY_DEBUG
			thread_data->is_on_event = false;
			print_stack(*thread_data, "After push");
#endif

			Py_DECREF(code);
			break;
		}
		case PyTrace_EXCEPTION:
			break;
		case PyTrace_LINE:
			break;
		case PyTrace_RETURN:
		{
			ZoneScopedN("PyTrace_RETURN");

			bool _;
			ThreadData* thread_data = get_current_thread_data(frame, _);

#ifdef PYTRACY_DEBUG
			assert(!thread_data->is_on_event);
			thread_data->is_on_event = true;
			print_stack(*thread_data, "Befor pop");
#endif

			assert(thread_data->tracy_stack.size());

			const auto stack_data = thread_data->tracy_stack.back();
			thread_data->tracy_stack.pop_back();
			pytracy_zone_end(stack_data.tracyCtx);

#ifdef PYTRACY_DEBUG
			print_stack(*thread_data, "After pop");
			thread_data->is_on_event = false;
#endif

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


static void initialize_call_stack(PyFrameObject* frame, ThreadData* thread_data)
{
	ZoneScoped;
	assert(PyGILState_Check());
	assert(frame);

	// PyFrameObject *PyFrame_GetBack(PyFrameObject *frame)
    // Return value: New reference.
    // Return a strong reference, or NULL if frame has no outer frame.
	PyFrameObject* back = PyFrame_GetBack(frame);

	if (back)
	{
		initialize_call_stack(back, thread_data);
		Py_DECREF(back);
	}

	PyCodeObject* code = PyFrame_GetCode(frame);

	bool is_active = false;
	uint64_t source_index;
	ProcessedFunctionData* data; 

	{
		py::gil_scoped_release release;
		source_index = get_source_index_from_frame(frame, code, is_active);
		data = get_function_data(code, frame);
	}

#ifdef PYTRACY_DEBUG
	print_stack(*thread_data, "Befor push");
#endif

	TracyCZoneCtx ctx = pytracy_zone_start(source_index, is_active);
	thread_data->tracy_stack.push_back({ctx, is_active, data});

#ifdef PYTRACY_DEBUG
	print_stack(*thread_data, "After push");
#endif
}

py::object on_trace_event_wrapper(py::args args, py::kwargs kwargs)
{
	ZoneScoped;

	py::object frame_object = args[0].cast<py::object>();
	PyFrameObject* frame = reinterpret_cast<PyFrameObject*>(frame_object.ptr());

	std::string what_str = args[1].cast<std::string>();

	std::optional<int> what = std::nullopt;

	if (what_str == "call") {
		what = PyTrace_CALL;
	} else if (what_str == "return") {
		what = PyTrace_RETURN;
	}
	// We are purposefully not checking other events, as we are not interested in them

	if (what.has_value())
	{
		on_trace_event(nullptr, frame, what.value(), nullptr);
	}

	PyTracyState& state = PyTracyState::the();
	if (state.tracing_mode == TracingMode::All)
	{
		// on_trace_event_wrapper needs to return the next trace event function,
		// which is on_trace_event_wrapper itself
		return state.on_trace_event_wrapped;
	}
	else if (state.tracing_mode == TracingMode::Disabled)
	{
		// unless we are about to change the mode, which means that we need to return None
		return py::none();
	}
	else
	{
		// Should not be reached
		assert(false);
		return py::none();
	}
}

static void set_tracing_internal(PyTracyState& state)
{
	ZoneScoped;

	// TODO: Consider mode changes during runtime
	if (state.tracing_mode == TracingMode::Disabled)
	{
		py::module threading_module = state.threading_module;
		py::function setprofile = threading_module.attr("setprofile");

		setprofile(py::none());
		PyEval_SetProfile(NULL, NULL);
	}
	else if (state.tracing_mode == TracingMode::All)
	{
		assert(PyGILState_Check());

		py::module threading_module = state.threading_module;
		py::function setprofile = threading_module.attr("setprofile");

		setprofile(state.on_trace_event_wrapped);
		PyEval_SetProfile(on_trace_event, NULL);
	}
	else
	{
		// Should not be reached
		assert(false);
	}
}

static py::none set_tracing_mode(int _mode)
{
	ZoneScoped;

	if (_mode < 0 || _mode > 2)
	{
		throw std::invalid_argument("Invalid tracing mode");
	}

	TracingMode mode = static_cast<TracingMode>(_mode);

	PyTracyState& state = PyTracyState::the();

	if (state.tracing_mode == mode)
	{
		return py::none();
	}

	state.tracing_mode = mode;

	set_tracing_internal(state);

	return py::none();
}

// filtered_out_folders() -> List[str]: ...
py::list get_filtered_out_folders()
{
	ZoneScoped;

	py::list result;

	PyTracyState& state = PyTracyState::the();

	for (const auto& path : state.filter_list)
	{
		result.append(path);
	}

	return result;
}

// set_filtered_out_folders(files: List[str]) -> None: ...
py::none set_filtered_out_folders(py::list files)
{
	ZoneScoped;

	PyTracyState& state = PyTracyState::the();

	// Ensure that files are all strings
	for (const auto& path : files)
	{
		if (!py::isinstance<py::str>(path))
		{
			throw std::invalid_argument("All elements of the list must be strings");
		}
	}

	state.filter_list.clear();

	for (const auto& path : files)
	{
		state.filter_list.insert(path.cast<std::string>());
	}

	make_function_is_filtered_out_dirty(state);

	return py::none();
}

py::none enable_tracing(bool enable)
{
	ZoneScoped;

	TracingMode mode = enable ? TracingMode::All : TracingMode::Disabled;
	set_tracing_mode(static_cast<int>(mode));

	return py::none();
}

PYBIND11_MODULE(pytracy, m) {
	m.doc() = "Tracy Profiler bindings for Python";
	m.def("enable_tracing", &enable_tracing, "Sets Tracy Profiler tracing mode");

	m.def("set_filtered_out_folders", &set_filtered_out_folders, "Sets which folders should be ignored while profiling");
	m.def("get_filtered_out_folders", &get_filtered_out_folders, "Returns a list of filtered out folders");

	m.def("set_filtering_mode", &set_filtering_mode, "Sets the filtering mode for the profiler");

	// Initialize the state
	PyTracyState& state = PyTracyState::the();
};
