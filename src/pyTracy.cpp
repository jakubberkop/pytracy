#include <Python.h>
#include <frameobject.h>

#define TRACY_ENABLE
#include <TracyC.h>

#include <unordered_map>
#include <deque>

#include "robin_hood.h"

struct ThreadData
{
	std::deque<TracyCZoneCtx> tracy_stack = {};
	char thread_name[256] = {};
};

struct SourceLocationKey
{
	PyCodeObject* code;
	int64_t line;
};

bool operator==(const SourceLocationKey& lhs, const SourceLocationKey& rhs)
{
	return lhs.code == rhs.code && lhs.line == rhs.line;
}

namespace robin_hood
{
	template<>
	struct hash<SourceLocationKey>
	{
		size_t operator()(const SourceLocationKey& key) const
		{
			return hash<PyCodeObject*>()(key.code) ^ hash<uint64_t>()(key.line);
		}
	};
}

// TODO: Make this map thread safe
robin_hood::unordered_map<SourceLocationKey, uint64_t> source_location_map;
robin_hood::unordered_map<uint64_t, ThreadData> thread_data_map;

static ThreadData* get_current_thread_data()
{
	unsigned long thread_id = PyThread_get_thread_ident();

	auto it = thread_data_map.find(thread_id);

	if (it != thread_data_map.end())
	{
		return &it->second;
	}

	auto new_it = thread_data_map.emplace(thread_id, ThreadData{});
	ThreadData* thread_data = &new_it.first->second;
	
	snprintf(thread_data->thread_name, 256, "Python Thread %lu", thread_id);

	return thread_data;
}

static uint64_t get_source_index(PyFrameObject* frame)
{
	PyCodeObject* code = PyFrame_GetCode(frame);
	int64_t line = PyFrame_GetLineNumber(frame);

	uint64_t source_index;

	SourceLocationKey key = {code, line};
	const auto it = source_location_map.find(key);

	if (it == source_location_map.end())
	{
		Py_ssize_t file_name_len;
		Py_ssize_t func_name_len;

		const char* file_name = PyUnicode_AsUTF8AndSize(code->co_filename, &file_name_len);
		const char* func_name = PyUnicode_AsUTF8AndSize(code->co_name, &func_name_len);

		source_index = ___tracy_alloc_srcloc_name(line, file_name, file_name_len + 1, func_name, func_name_len + 1, func_name, func_name_len + 1);
		source_location_map.insert({key, source_index});
	}
	else
	{
		source_index = it->second;
	}

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

	printf("T: %d PY %ld: Tracy %ld ", thread_id, get_frame_stack_size(frame), thread_data->tracy_stack.size());
	printf("%s %s %d", fileName, funcname, line);
	printf("\n");

	Py_DECREF(code);
}

int trace_function(PyObject* obj, PyFrameObject* frame, int what, PyObject *arg)
{
	switch (what)
	{
		case PyTrace_CALL:
		{
			print_current_function_name(frame, what);

			uint64_t source_index = get_source_index(frame);
			ThreadData* thread_data = get_current_thread_data();

			thread_data->tracy_stack.push_back(___tracy_emit_zone_begin_alloc(source_index, 1));

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
				printf("ERROR: tracy_stack_index == 0\n");
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

static bool is_tracing_enabled = false;

void initialize_call_stack(PyFrameObject* frame)
{
	PyFrameObject* back = PyFrame_GetBack(frame);

	if (back)
	{
		initialize_call_stack(back);
		Py_DECREF(back);
	}

	uint64_t source_index = get_source_index(frame);
	ThreadData* thread_data = get_current_thread_data();

	TracyCZoneCtx ctx = ___tracy_emit_zone_begin_alloc(source_index, 1);
	thread_data->tracy_stack.push_back(ctx);
}

static PyObject* enable_tracing(PyObject*, PyObject*)
{
	if (is_tracing_enabled)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}

	PyThreadState* ts = PyThreadState_Get();
	PyFrameObject* frame = PyThreadState_GetFrame(ts);

	initialize_call_stack(frame);

	PyEval_SetTrace(trace_function, NULL);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyMethodDef moduleMethods[] = {
	{"enableTracing", enable_tracing, METH_VARARGS, NULL},
	{NULL, NULL, 0, NULL}
};

static struct PyModuleDef pyTracyModule = {
	PyModuleDef_HEAD_INIT,
	"pytracy",
	"Tracy Profiler bindings for Python",
	-1,
	moduleMethods
};

PyMODINIT_FUNC PyInit_pytracy(void) {
	// PyEval_SetProfile(trace_function, NULL);
	return PyModule_Create(&pyTracyModule);
};