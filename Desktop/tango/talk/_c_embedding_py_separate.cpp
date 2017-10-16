#include <stdio.h>
#include <python2.7/Python.h>

int main(int argc, char *argv[])
{
    PyObject *pName, *pModule, *pDict, *pFunc;
    PyObject *pArgs, *pValue;
    int i;

    Py_SetProgramName(argv[0]);
    Py_Initialize();
    PySys_SetArgv(argc, argv);
//    pName = PyString_FromString(argv[1]);
    pName = PyString_FromString("_postag");
    /* Error checking of pName left out */

    pModule = PyImport_Import(pName);
    Py_DECREF(pName);

    if (pModule != NULL) {
//        pFunc = PyObject_GetAttrString(pModule, argv[2]);
        pFunc = PyObject_GetAttrString(pModule, "mySum");
        /* pFunc is a new reference */

        if (pFunc && PyCallable_Check(pFunc)) 
	{
	    int input[2] = {1, 3};
            pArgs = PyTuple_New(2);
            for (i = 0; i < 2; i++) 
	    {
                pValue = PyInt_FromLong(input[i]);
//                pValue = PyString_FromString(argv[i + 3]);
                if (!pValue) 
		{
                    Py_DECREF(pArgs);
                    Py_DECREF(pModule);
                    fprintf(stderr, "Cannot convert argument\n");
                    return 1;
                }
                /* pValue reference stolen here: */
                PyTuple_SetItem(pArgs, i, pValue);
            }
            pValue = PyObject_CallObject(pFunc, pArgs);
            Py_DECREF(pArgs);
            if (pValue != NULL) 
	    {
                printf("Result of call: %ld\n", PyInt_AsLong(pValue));
                Py_DECREF(pValue);
            }
            else {
                Py_DECREF(pFunc);
                Py_DECREF(pModule);
                PyErr_Print();
                fprintf(stderr,"Call failed\n");
                return 1;
            }
        }
        else {
            if (PyErr_Occurred())
                PyErr_Print();
            fprintf(stderr, "Cannot find function \"%s\"\n", argv[2]);
        }
        Py_XDECREF(pFunc);
        Py_DECREF(pModule);
    }
    else {
        PyErr_Print();
        fprintf(stderr, "Failed to load \"%s\"\n", argv[1]);
        return 1;
    }
    Py_Finalize();
    return 0;
}