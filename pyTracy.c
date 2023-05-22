#include <Python.h>
#include <frameobject.h>

#define TRACY_ENABLE
#include <TracyC.h>

static PyMethodDef CountingMethods[] = {
	//   {"primecounter", py_primecounter, METH_VARARGS, "Function for counting primes in a range in c"},
	{NULL, NULL, 0, NULL}
};

static TracyCZoneCtx tracy_stack [1000000] = {};
static size_t tracy_stack_index = 0;
static struct ___tracy_source_location_data source_location_list [1000000] = {};

int trace_function(PyObject* obj, PyFrameObject *frame, int what, PyObject *arg)
{
	switch (what)
	{
		case PyTrace_CALL:
		{

			PyCodeObject* code = PyFrame_GetCode(frame);
			const char* fileName = PyUnicode_AsUTF8(code->co_filename);

			PyThreadState* state = PyThreadState_GET();
			PyFrameObject* frame = state->frame;

			const char* funcname = PyUnicode_AsUTF8(frame->f_code->co_name);
			// const int line = PyCode_Addr2Line(frame->f_code, frame->f_lasti);
			const int line = PyFrame_GetLineNumber(frame);

			// struct ___tracy_source_location_data source_location;
			struct ___tracy_source_location_data* source_location = &source_location_list[tracy_stack_index];

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
			break;
		case PyTrace_LINE:
			break;
		case PyTrace_RETURN:
		{
			// PyCodeObject* code = PyFrame_GetCode(frame);
			// char* fileName = PyUnicode_AsUTF8(code->co_filename);

			tracy_stack_index--;
			___tracy_emit_zone_end(tracy_stack[tracy_stack_index]);

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


static struct PyModuleDef pyTracyModule = {
	PyModuleDef_HEAD_INIT,
  	"pytracy", // module name
	"C library for counting fast",
	-1,
	CountingMethods
};

PyMODINIT_FUNC PyInit_PyTracy(void) {
	// ___tracy_startup_profiler();

	PyEval_SetProfile(trace_function, NULL);

	// Log using python logging module
	// PyObject *logging = PyImport_ImportModule("logging");
	// PyObject *log = PyObject_GetAttrString(logging, "info");
	// PyObject *args = Py_BuildValue("(s)", "Hello from C");
	// PyObject_CallObject(log, args);

	return PyModule_Create(&pyTracyModule);
};