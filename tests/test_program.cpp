#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>

unsigned char* load_binary(const char *filename, size_t *size) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) return nullptr;

    fseek(fp, 0, SEEK_END);
    *size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    unsigned char *buffer = (unsigned char*)malloc(*size);
    if (!buffer) { fclose(fp); return nullptr; }

    fread(buffer, sizeof(char), *size, fp);
    fclose(fp);
    return buffer;
}

void load_and_run_kernel() {
    cl_int err;

    const int VECTOR_SIZE = 1024;
    cl_uint num_platforms;
    cl_platform_id platform;
    clGetPlatformIDs(1, &platform, &num_platforms);

    cl_device_id device;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, &num_platforms);

    cl_context context;
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);


    cl_command_queue queue;
    cl_queue_properties properties[] = {0};
    queue = clCreateCommandQueueWithProperties(context, device, properties, &err);

    size_t binarySize;
    unsigned char* binaryData = load_binary("./kernel_add_f32_no_vpu.dfcafb", &binarySize);
    cl_int binaryStatus;
    cl_program program = clCreateProgramWithBinary(context, 1, &device, &binarySize, 
                                            (const unsigned char**)&binaryData, 
                                            &binaryStatus, &err);
    if (binaryStatus != CL_SUCCESS) {
        printf("Error: Failed to load kernel binary: %d\n", err);
        free(binaryData);
        return;
    }
    printf("Program created from binary successfully!\n");

    clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);

    cl_kernel kernel = clCreateKernel(program, "vector_add", &err);
    if (err != CL_SUCCESS) {
        printf("Error creating kernel: %d\n", err);
        printf("Kernel 'vector_add' not found in binary\n");
    } else {
        printf("Kernel created successfully! Binary is valid.\n");
    }
    
    float *a = (float *)malloc(VECTOR_SIZE * sizeof(float));
    float *b = (float *)malloc(VECTOR_SIZE * sizeof(float));
    float *c = (float *)malloc(VECTOR_SIZE * sizeof(float));
    int numElemsPerDie = VECTOR_SIZE;
    
    cl_mem bufA = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
                                VECTOR_SIZE * sizeof(float), a, &err);
    cl_mem bufB = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
                                VECTOR_SIZE * sizeof(float), b, &err);
    cl_mem bufC = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
                                VECTOR_SIZE * sizeof(float), c, &err);
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufA);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufB);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &bufC);
    clSetKernelArg(kernel, 3, sizeof(int), &numElemsPerDie);
    
    size_t global_work_size = VECTOR_SIZE; // 全局工作项总数
    err = clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global_work_size, nullptr, 0, NULL, NULL);
    if (err != CL_SUCCESS) {
        printf("Error launch kernel: %d\n", err);
    } else {
        printf("Kernel launch successfully!\n");
    }

    clFinish(queue);

    clEnqueueReadBuffer(
        queue,
        bufC,
        CL_TRUE,                // 阻塞读
        0,
        1024 * sizeof(float),
        c,              // 目标缓冲区
        0, NULL, NULL
    );
    
    // 清理资源
    free(binaryData);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
}

int main() {
    printf("=== load kernel from binary file ===\n");
    load_and_run_kernel();
    return 0;
}