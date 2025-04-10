#include <CL/cl.h>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <ctime>

const char* multiply_kernel_source = R"(
__kernel void matrix_multiply(__global const float* A, __global const float* B, __global float* C, int N) {
    int row = get_global_id(0);
    int col = get_global_id(1);
    float sum = 0.0f;
    for (int k = 0; k < N; k++) {
        sum += A[row * N + k] * B[k * N + col];
    }
    C[row * N + col] = sum;
}
)";

const char* add_kernel_source = R"(
__kernel void matrix_add(__global const float* A, __global const float* B, __global float* C, int N) {
    int row = get_global_id(0);
    int col = get_global_id(1);
    C[row * N + col] = A[row * N + col] + B[row * N + col];
}
)";

const char* scalar_multiply_kernel_source = R"(
__kernel void scalar_multiply(__global float* A, float scalar, int N) {
    int row = get_global_id(0);
    int col = get_global_id(1);
    A[row * N + col] *= scalar;
}
)";

int main() {
    cl_int err;
    int N;
    const int num_terms = 10; // Количество членов ряда Тейлора

    // Ввод размера матрицы
    std::cout << "Введите размер квадратной матрицы N: ";
    std::cin >> N;

    // Генерация случайной матрицы A
    std::srand(std::time(0));
    std::vector<float> A(N * N);
    for (int i = 0; i < N * N; ++i) {
        A[i] = static_cast<float>(std::rand() % 10); // Случайные числа от 0 до 9
    }

    // Вывод матрицы A
    std::cout << "Матрица A:\n";
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            std::cout << A[i * N + j] << " ";
        }
        std::cout << std::endl;
    }

    // Настройка OpenCL
    cl_platform_id platform;
    clGetPlatformIDs(1, &platform, NULL);
    cl_device_id device;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    cl_command_queue queue = clCreateCommandQueue(context, device, 0, &err);

    // Создание буферов
    cl_mem buffer_A = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * N * N, A.data(), &err);
    cl_mem buffer_expA = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(float) * N * N, NULL, &err);
    cl_mem buffer_term = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(float) * N * N, NULL, &err);
    cl_mem buffer_temp = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(float) * N * N, NULL, &err);

    // Инициализация expA как единичной матрицы
    std::vector<float> I(N * N, 0.0f);
    for (int i = 0; i < N; ++i) {
        I[i * N + i] = 1.0f;
    }
    clEnqueueWriteBuffer(queue, buffer_expA, CL_TRUE, 0, sizeof(float) * N * N, I.data(), 0, NULL, NULL);
    clEnqueueWriteBuffer(queue, buffer_term, CL_TRUE, 0, sizeof(float) * N * N, I.data(), 0, NULL, NULL);

    // Компиляция kernel'ов
    cl_program multiply_program = clCreateProgramWithSource(context, 1, &multiply_kernel_source, NULL, &err);
    clBuildProgram(multiply_program, 1, &device, NULL, NULL, NULL);
    cl_kernel multiply_kernel = clCreateKernel(multiply_program, "matrix_multiply", &err);

    cl_program add_program = clCreateProgramWithSource(context, 1, &add_kernel_source, NULL, &err);
    clBuildProgram(add_program, 1, &device, NULL, NULL, NULL);
    cl_kernel add_kernel = clCreateKernel(add_program, "matrix_add", &err);

    cl_program scalar_multiply_program = clCreateProgramWithSource(context, 1, &scalar_multiply_kernel_source, NULL, &err);
    clBuildProgram(scalar_multiply_program, 1, &device, NULL, NULL, NULL);
    cl_kernel scalar_multiply_kernel = clCreateKernel(scalar_multiply_program, "scalar_multiply", &err);

    // Вычисление exp(A)
    size_t global_work_size[2] = {static_cast<size_t>(N), static_cast<size_t>(N)};
    for (int k = 1; k <= num_terms; ++k) {
        // term = term * A
        clSetKernelArg(multiply_kernel, 0, sizeof(cl_mem), &buffer_term);
        clSetKernelArg(multiply_kernel, 1, sizeof(cl_mem), &buffer_A);
        clSetKernelArg(multiply_kernel, 2, sizeof(cl_mem), &buffer_temp);
        clSetKernelArg(multiply_kernel, 3, sizeof(int), &N);
        clEnqueueNDRangeKernel(queue, multiply_kernel, 2, NULL, global_work_size, NULL, 0, NULL, NULL);
        clEnqueueCopyBuffer(queue, buffer_temp, buffer_term, 0, 0, sizeof(float) * N * N, 0, NULL, NULL);

        // term = term / k
        float scalar = 1.0f / static_cast<float>(k);
        clSetKernelArg(scalar_multiply_kernel, 0, sizeof(cl_mem), &buffer_term);
        clSetKernelArg(scalar_multiply_kernel, 1, sizeof(float), &scalar);
        clSetKernelArg(scalar_multiply_kernel, 2, sizeof(int), &N);
        clEnqueueNDRangeKernel(queue, scalar_multiply_kernel, 2, NULL, global_work_size, NULL, 0, NULL, NULL);

        // expA += term
        clSetKernelArg(add_kernel, 0, sizeof(cl_mem), &buffer_expA);
        clSetKernelArg(add_kernel, 1, sizeof(cl_mem), &buffer_term);
        clSetKernelArg(add_kernel, 2, sizeof(cl_mem), &buffer_temp);
        clSetKernelArg(add_kernel, 3, sizeof(int), &N);
        clEnqueueNDRangeKernel(queue, add_kernel, 2, NULL, global_work_size, NULL, 0, NULL, NULL);
        clEnqueueCopyBuffer(queue, buffer_temp, buffer_expA, 0, 0, sizeof(float) * N * N, 0, NULL, NULL);
    }

    // Чтение результата
    std::vector<float> expA(N * N);
    clEnqueueReadBuffer(queue, buffer_expA, CL_TRUE, 0, sizeof(float) * N * N, expA.data(), 0, NULL, NULL);

    // Вывод результата
    std::cout << "exp(A):\n";
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            std::cout << expA[i * N + j] << " ";
        }
        std::cout << std::endl;
    }

    // Освобождение ресурсов
    clReleaseKernel(scalar_multiply_kernel);
    clReleaseProgram(scalar_multiply_program);
    clReleaseKernel(add_kernel);
    clReleaseProgram(add_program);
    clReleaseKernel(multiply_kernel);
    clReleaseProgram(multiply_program);
    clReleaseMemObject(buffer_temp);
    clReleaseMemObject(buffer_term);
    clReleaseMemObject(buffer_expA);
    clReleaseMemObject(buffer_A);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return 0;
}