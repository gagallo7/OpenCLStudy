#ifdef __APPLE__
    #include <OpenCL/cl.h>  
#else
    #include <CL/cl.h>
#endif

#if !defined(CL_CALLBACK)
    #define CL_CALLBACK
#endif

#include <stdio.h>
#include <stdlib.h>
#define INF 1 << 30


void infoPlataforma (cl_platform_id * listaPlataformaID, cl_uint i);

// Checagem de erros
inline void checkErr(cl_int err, const char *name);
void CL_CALLBACK contextCallback ();

void prepareAllDataForDevice (  cl_int errNum,
                                cl_uint nPlataformas,
                                cl_uint nDispositivos,
    //                            cl_platform_id *listaPlataformaID,
      //                          cl_device_id *listaDispositivoID,
                                cl_context* contexto,
                                cl_command_queue fila,
                                cl_program programa, cl_program programa2,
                                cl_kernel *, cl_kernel *
                                ) ;
