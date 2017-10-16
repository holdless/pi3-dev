#include <stdio.h>
#include <python2.7/Python.h>

int main(int argc, char* argv[])
{
    FILE* file;
    char filePath[] = "/home/pi/Desktop/python_test/_stt_tts_postag.py";
    file = fopen(filePath, "r");
    Py_SetProgramName(argv[0]);
    Py_Initialize();
    PyRun_SimpleFile(file, filePath);
    
    Py_Finalize();
    return 0;
}