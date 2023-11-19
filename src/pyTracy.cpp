// #include <Python.h>
// #include <frameobject.h>

#define PYBIND11_DETAILED_ERROR_MESSAGES 1

#include <pybind11/pybind11.h>

namespace py = pybind11;

#define TRACY_ENABLE
#include <TracyC.h>

#include <mutex>

#include <unordered_map>
#include <deque>

#include "robin_hood.h"

struct ThreadData
{
	std::deque<TracyCZoneCtx> tracy_stack = {};
};

robin_hood::unordered_map<uint64_t, ThreadData*> thread_data_map;
std::mutex thread_data_mutex;

static ThreadData* get_current_thread_data()
{
	uint64_t thread_id = PyThread_get_thread_ident();

	std::lock_guard <std::mutex> lock(thread_data_mutex);

	auto it = thread_data_map.find(thread_id);
	if (it != thread_data_map.end())
	{
		return it->second;
	}

	auto new_it = thread_data_map.emplace(thread_id, new ThreadData{});
	return new_it.first->second;
}

static uint64_t get_source_index_from_frame(PyFrameObject* frame)
{
	PyCodeObject* code = PyFrame_GetCode(frame);
	int64_t line = PyFrame_GetLineNumber(frame);

	assert(code != 0);

	Py_ssize_t file_name_len;
	Py_ssize_t func_name_len;

	const char* file_name = PyUnicode_AsUTF8AndSize(code->co_filename, &file_name_len);
	const char* func_name = PyUnicode_AsUTF8AndSize(code->co_name, &func_name_len);

	assert(file_name != 0);
	assert(func_name != 0);

	uint64_t source_index = ___tracy_alloc_srcloc(line, file_name, file_name_len, func_name, func_name_len);

	Py_DECREF(code);

	return source_index;
}

uint64_t get_frame_stack_size(PyFrameObject* frame)
{
	PyFrameObject * back = PyFrame_GetBack(frame);

	if (back == NULL)
	{
		return 0;
	}

	uint64_t size = get_frame_stack_size(back) + 1;

	Py_DECREF(back);

	return size;
}

void print_current_function_name(PyFrameObject* frame, int what)
{
	return;
	PyCodeObject* code = PyFrame_GetCode(frame);
	const char* fileName = PyUnicode_AsUTF8(code->co_filename);

	const char* funcname = PyUnicode_AsUTF8(code->co_name);
	const int line = PyFrame_GetLineNumber(frame);

	const char* message = nullptr;

	switch (what)
	{
	case PyTrace_CALL:
		message = "Call     ";
	 	break;
	case PyTrace_EXCEPTION:
		message = "Exception";
		break;
	case PyTrace_RETURN:
		message = "Return   ";
	 	break;
	case PyTrace_C_CALL:
		message = "C_Call   ";
		break;
	case PyTrace_C_EXCEPTION:
		message = "C_Exc    ";
		break;
	case PyTrace_C_RETURN:
		message = "C_Ret    ";
		break;
	default:
		message = "Unknown  ";
		break;
	}

	// print indent
	// for (int i = 0; i < tracy_stack_index; i++)
	// {
	// 	printf("  ");
	// }

	if (message != nullptr)
	{
		printf("%s", message);
	}

	ThreadData* thread_data = get_current_thread_data();
	unsigned int thread_id = PyThread_get_thread_ident();

	// printf("T: %d PY %ld: Tracy %ld ", thread_id, get_frame_stack_size(frame), thread_data->tracy_stack.size());
	// printf("%s %s %d", fileName, funcname, line);
	// printf("\n");

	Py_DECREF(code);
}

int on_trace_event(PyObject* obj, PyFrameObject* frame, int what, PyObject *arg)
{
	switch (what)
	{
		case PyTrace_CALL:
		{
			print_current_function_name(frame, what);

			uint64_t source_index = get_source_index_from_frame(frame);
			ThreadData* thread_data = get_current_thread_data();

			auto ctx = ___tracy_emit_zone_begin_alloc(source_index, 1);
			thread_data->tracy_stack.push_back(ctx);
			break;
		}
		case PyTrace_EXCEPTION:
			print_current_function_name(frame, what);
			break;
		case PyTrace_LINE:
			break;
		case PyTrace_RETURN:
		{
			print_current_function_name(frame, what);

			ThreadData* thread_data = get_current_thread_data();

			if (thread_data->tracy_stack.size() == 0)
			{
				printf("pytracy internal error: tracy_stack_index == 0\n");
				assert(false);
				break;
			}

			const auto ctx = thread_data->tracy_stack.back();
			thread_data->tracy_stack.pop_back();
			___tracy_emit_zone_end(ctx);
			break;
		}
		case PyTrace_C_CALL:
			print_current_function_name(frame, what);
			break;
		case PyTrace_C_EXCEPTION:
			print_current_function_name(frame, what);
			break;
		case PyTrace_C_RETURN:
			print_current_function_name(frame, what);
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

	TracyCZoneCtx ctx = ___tracy_emit_zone_begin_alloc(source_index, 1);
	thread_data->tracy_stack.push_back(ctx);
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
		return {};
	}

	tracing_mode = mode;

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
		PyThreadState* ts = PyThreadState_Get();
		PyFrameObject* frame = PyThreadState_GetFrame(ts);
		initialize_call_stack(frame);

		PyEval_SetTrace(on_trace_event, NULL);
	}

	printf("set_tracing_mode %d done\n", _mode);

	return {};
}

static ___tracy_source_location_data native_function_source_location = { NULL, "Native function", "<unknown>", 0, 0 };
class TracingFunctionWrapper
{
public:

	TracingFunctionWrapper(py::function func)
		: func(func) {}

	void mark_function_enter() const
	{
		if (tracing_mode == TracingMode::MarkedFunctions)
		{
			return;
		}

		if (func.is_cpp_function())
		{
			TracyCZoneCtx ctx = ___tracy_emit_zone_begin(&native_function_source_location, 1);
			ThreadData* thread_data = get_current_thread_data();
			thread_data->tracy_stack.push_back(ctx);
			return;
		}

		PyThreadState* ts = PyThreadState_Get();
		PyFrameObject* frame = PyThreadState_GetFrame(ts);
		
		int64_t line = PyFrame_GetLineNumber(frame);

		std::string func_name = func.attr("__code__").attr("co_name").cast<std::string>();
		std::string file_name = func.attr("__code__").attr("co_filename").cast<std::string>();

		uint64_t source_index = ___tracy_alloc_srcloc(line, file_name.c_str(), file_name.size(), func_name.c_str(), func_name.size());

		ThreadData* thread_data = get_current_thread_data();

		TracyCZoneCtx ctx = ___tracy_emit_zone_begin_alloc(source_index, 1);
		thread_data->tracy_stack.push_back(ctx);
	}

	void mark_function_exit() const
	{
		if (tracing_mode == TracingMode::MarkedFunctions)
		{
			return;
		}

		ThreadData* thread_data = get_current_thread_data();

		if (thread_data->tracy_stack.size() == 0)
		{
			printf("pytracy internal error: tracy_stack_index == 0\n");
			return;
		}

		const auto ctx = thread_data->tracy_stack.back();
		thread_data->tracy_stack.pop_back();
		___tracy_emit_zone_end(ctx);
	}

	py::object call(py::args args) const
	{
		mark_function_enter();

		py::object result;

		if (args.size() == 0)
		{
			result = func();
		}
		else
		{
			result = func(args);
		}

		mark_function_exit();

		return result;
	}

private:
	py::function func;
};


PYBIND11_MODULE(pytracy, m) {
	m.doc() = "Tracy Profiler bindings for Python";

	// m.def("enable_tracing", &enable_tracing, "Enables Tracy Profiler tracing");
	// m.def("disable_tracing", &disable_tracing, "Disables Tracy Profiler tracing");
	m.def("set_tracing_mode", &set_tracing_mode, "Sets Tracy Profiler tracing mode");

	py::class_<TracingFunctionWrapper>(m, "trace_function")
		.def(py::init<py::function>())
		.def("__call__", &TracingFunctionWrapper::call);
};
