#include <Python.h>
#include <frameobject.h>

#define TRACY_ENABLE
#include <TracyC.h>

static PyMethodDef CountingMethods[] = {
	{NULL, NULL, 0, NULL}
};

static TracyCZoneCtx tracy_stack [1000000] = {};
static size_t tracy_stack_index = 0;
static struct ___tracy_source_location_data source_location_list [1000000] = {};
static size_t source_location_index = 0;

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

void print_current_function_name(PyFrameObject* frame, const char* message = nullptr)
{
	PyCodeObject* code = PyFrame_GetCode(frame);
	const char* fileName = PyUnicode_AsUTF8(code->co_filename);

	const char* funcname = PyUnicode_AsUTF8(frame->f_code->co_name);
	const int line = PyFrame_GetLineNumber(frame);

	// print indent
	// for (int i = 0; i < tracy_stack_index; i++)
	// {
	// 	printf("  ");
	// }

	if (message != nullptr)
	{
		printf("%s", message);
	}

// 
	printf(" PY %d - %d: Tracy %d ", frame->f_code->co_stacksize, get_frame_stack_size(frame) , tracy_stack_index);
	printf("%s %s %d", fileName, funcname, line);
	printf("\n");
}

int trace_function(PyObject* obj, PyFrameObject* frame, int what, PyObject *arg)
{
	switch (what)
	{
		case PyTrace_CALL:
		{
			print_current_function_name(frame, "Call     ");

			PyCodeObject* code = PyFrame_GetCode(frame);
			const char* fileName = PyUnicode_AsUTF8(code->co_filename);

			const char* funcname = PyUnicode_AsUTF8(frame->f_code->co_name);
			const int line = PyFrame_GetLineNumber(frame);

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

			tracy_stack[tracy_stack_index] = ___tracy_emit_zone_begin(source_location, 1);
			tracy_stack_index++;

			Py_DECREF(code);

			break;
		}
		case PyTrace_EXCEPTION:
			print_current_function_name(frame, "Exception");
			break;
		case PyTrace_LINE:
			break;
		case PyTrace_RETURN:
		{
			print_current_function_name(frame, "Return   ");

			if (tracy_stack_index == 0)
			{
				printf("ERROR: tracy_stack_index == 0\n");
				break;
			}


			tracy_stack_index--;
			___tracy_emit_zone_end(tracy_stack[tracy_stack_index]);

			break;
		}
		case PyTrace_C_CALL:
			print_current_function_name(frame, "C_Call   ");
			break;
		case PyTrace_C_EXCEPTION:
			print_current_function_name(frame, "C_Exc    ");
			break;
		case PyTrace_C_RETURN:
			print_current_function_name(frame, "C_Ret    ");
			break;
		case PyTrace_OPCODE:
			break;
	}
	return 0;
}


static struct PyModuleDef pyTracyModule = {
	PyModuleDef_HEAD_INIT,
  	"pytracy",
	"Tracy Profiler bindings for Python",
	-1,
	CountingMethods
};

PyMODINIT_FUNC PyInit_pytracy(void) {
	PyEval_SetProfile(trace_function, NULL);
	return PyModule_Create(&pyTracyModule);
};