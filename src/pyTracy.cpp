#include <Python.h>
#include <frameobject.h>

#define TRACY_ENABLE
#include <TracyC.h>

#include <unordered_map>

static struct ___tracy_source_location_data source_location_list [1000000] = {};
static size_t source_location_index = 0;

struct ThreadData
{
	TracyCZoneCtx tracy_stack [1000000] = {};
	size_t tracy_stack_index = 0;
	char thread_name[256] = {};
};

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

	printf("T: %d PY %ld: Tracy %ld ", thread_id, get_frame_stack_size(frame), thread_data->tracy_stack_index);
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
			const char* fileName = PyUnicode_AsUTF8(code->co_filename);
			const char* funcname = PyUnicode_AsUTF8(code->co_name);
			const unsigned int line = PyFrame_GetLineNumber(frame);

			struct ___tracy_source_location_data* source_location = NULL;

			for (size_t i = 0; i < source_location_index; i++)
			{
				if (source_location_list[i].line == line && strcmp(source_location_list[i].file, fileName) == 0)
				{
					source_location = &source_location_list[i];
					break;
				}
			}

			if (source_location == NULL)
			{
				source_location = &source_location_list[source_location_index]; 
				source_location_index++;
			}
			source_location->name = funcname;
			source_location->function = funcname;
			source_location->file = fileName;
			source_location->line = line;
			source_location->color = 0;

			ThreadData* thread_data = get_current_thread_data();

			thread_data->tracy_stack[thread_data->tracy_stack_index] = ___tracy_emit_zone_begin(source_location, 1);
			thread_data->tracy_stack_index++;

			Py_DECREF(code);

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

			if (thread_data->tracy_stack_index == 0)
			{
				printf("ERROR: tracy_stack_index == 0\n");
				break;
			}


			thread_data->tracy_stack_index--;
			___tracy_emit_zone_end(thread_data->tracy_stack[thread_data->tracy_stack_index]);

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