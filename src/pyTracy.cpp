#include <Python.h>
#include <frameobject.h>

#define TRACY_ENABLE
#include <TracyC.h>

#include <unordered_map>
#include <deque>

struct ThreadData
{
	std::deque<TracyCZoneCtx> tracy_stack = {};
	char thread_name[256] = {};
};

// TODO: Make this map thread safe
static std::unordered_map<PyCodeObject*, uint64_t> source_location_map;
static std::unordered_map<uint64_t, ThreadData*> thread_data_map;

static ThreadData* get_current_thread_data()
{
	unsigned long thread_id = PyThread_get_thread_ident();

	auto it = thread_data_map.find(thread_id);

	if (it != thread_data_map.end())
	{
		return it->second;
	}

	auto new_it = thread_data_map.insert({thread_id, new ThreadData()});
	ThreadData* thread_data = new_it.first->second;
	
	snprintf(thread_data->thread_name, 256, "Python Thread %lu", thread_id);

	return thread_data_map[thread_id];
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

			PyCodeObject* code = PyFrame_GetCode(frame);

			uint64_t source_index;
			const auto it = source_location_map.find(code);
	
			if (it == source_location_map.end())
			{
				static char file_name[1024];
				static char func_name[1024];

				const char* temp_file_name = PyUnicode_AsUTF8(code->co_filename);
				const char* temp_func_name = PyUnicode_AsUTF8(code->co_name);

				strncpy(file_name, temp_file_name, 1024);
				strncpy(func_name, temp_func_name, 1024);

				uint64_t file_name_len = strlen(file_name);
				uint64_t func_name_len = strlen(func_name);

				unsigned int line = PyFrame_GetLineNumber(frame);

				source_index = ___tracy_alloc_srcloc_name(line, file_name, file_name_len, func_name, func_name_len, func_name, func_name_len);
				source_location_map.insert({code, source_index});
			}
			else
			{
				source_index = it->second;
			}

			Py_DECREF(code);

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

static PyObject* enable_tracing(PyObject*, PyObject*)
{
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