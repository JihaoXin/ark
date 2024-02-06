#include "gpu/gpu_loop_kernel.h"

#include <sstream>

#include "env.h"
#include "file_io.h"
#include "gpu/gpu.h"
#include "gpu/gpu_event.h"
#include "gpu/gpu_logging.h"

#define MAX_LOOP_COUNTER 10000000

#if defined(ARK_CUDA)
#include <cuda/atomic>
static int atomicLoadRelaxed(int* ptr) {
    return cuda::atomic_ref<int, cuda::thread_scope_system>{*ptr}.load(
        cuda::memory_order_relaxed);
}
static void atomicStoreRelaxed(int* ptr, int val) {
    cuda::atomic_ref<int, cuda::thread_scope_system>{*ptr}.store(
        val, cuda::memory_order_relaxed);
}
#elif defined(ARK_ROCM)
static int atomicLoadRelaxed(int* ptr) {
    return __atomic_load_n(ptr, __ATOMIC_RELAXED);
}
static void atomicStoreRelaxed(int* ptr, int val) {
    __atomic_store_n(ptr, val, __ATOMIC_RELAXED);
}
#endif  // defined(ARK_ROCM)

namespace ark {

GpuLoopKernel::GpuLoopKernel(std::shared_ptr<GpuContext> ctx,
                             const std::string& name,
                             const std::vector<std::string>& codes_body,
                             int num_sm, int num_warp, unsigned int smem_bytes)
    : GpuKernel(
          ctx, {},
          {num_warp * ctx->get_gpu_manager()->info().threads_per_warp, 1, 1},
          {num_sm, 1, 1}, (smem_bytes < 4) ? 4 : smem_bytes, name,
          {{nullptr, sizeof(GpuPtr)}}),
      timer_begin_(ctx->get_gpu_manager()->create_event()),
      timer_end_(ctx->get_gpu_manager()->create_event()) {
    flag_ = ctx->get_gpu_manager()->malloc_host(
        sizeof(int), gpuHostAllocMapped | gpuHostAllocWriteCombined);
    *(int**)params_ptr_[0] = (int*)flag_->ref<int>();

    auto& code_path = get_env().enforce_kernel_code_path;
    if (!code_path.empty()) {
        LOG(INFO, "Enforce kernel code path: ", code_path);
        codes_ = std::move(read_file(code_path));
    } else if (codes_body.size() > 0) {
        const std::string* ark_loop_body_code = nullptr;
        for (auto& code : codes_body) {
            if (code.find("ark_loop_body") != std::string::npos) {
                ark_loop_body_code = &code;
                break;
            }
        }
        assert(ark_loop_body_code != nullptr);

        std::stringstream ss;
        // clang-format off
        ss <<
        "// THIS KERNEL IS MACHINE-GENERATED BY ARK.\n"
        "#define ARK_THREADS_PER_BLOCK " << block_dim_[0] << "\n"
        "__device__ int _ITER = 0;\n"
        "#include \"ark_kernels.h\"\n"
        "__device__ ark::sync::State " ARK_LSS_NAME ";\n"
        "__device__ char *" ARK_BUF_NAME ";\n"
        << *ark_loop_body_code <<
        "extern \"C\" __global__ __launch_bounds__(" << block_dim_[0] << ", 1)\n"
        "void " << kernel_name_ << "(int *_it)\n"
        "{\n"
        "  char *_buf = " ARK_BUF_NAME ";\n"
        "  int *shared_mem = (int *)_ARK_SMEM;\n"
        "  for (int i = threadIdx.x; i < ARK_SMEM_RESERVED_BYTES / sizeof(int); i += blockDim.x) {\n"
        "    shared_mem[i] = 0;\n"
        "  }\n"
        "  for (;;) {\n"
        "    if (threadIdx.x == 0 && blockIdx.x == 0) {\n"
        "      int iter;\n"
        "      while ((iter = ark::atomicLoadRelaxed(_it)) == 0) {}\n"
        "      _ITER = iter;\n"
        "    }\n"
        "    ark::sync_gpu<" << num_sm << ">(" ARK_LSS_NAME ");\n"
        "    if (_ITER < 0) {\n"
        "      return;\n"
        "    }\n"
        "    for (int _i = 0; _i < _ITER; ++_i) {\n"
        "      ark_loop_body(_buf, _i);\n"
        "      ark::sync_gpu<" << num_sm << ">(" ARK_LSS_NAME ");\n"
        "    }\n"
        "    if (threadIdx.x == 0 && blockIdx.x == 0) {\n"
        "      ark::atomicStoreRelaxed(_it, 0);\n"
        "    }\n"
        "    ark::sync_gpu<" << num_sm << ">(" ARK_LSS_NAME ");\n"
        "  }\n"
        "}\n";
        // clang-format on
        codes_ = std::move(ss.str());
    }
}

void GpuLoopKernel::load() {
    if (!is_compiled()) {
        ERR(InvalidUsageError, "Need to compile first before initialization.");
    }
    if (stream_ != nullptr) {
        // Wait until previous works finish.
        wait();
        return;
    }
    // Initialize global variables in the loop kernel.
    std::shared_ptr<GpuManager> manager = ctx_->get_gpu_manager();
    void* buf_ptr_val = ctx_->get_data_memory()->ref();
    GpuPtr lss_ptr_addr;
    GpuPtr buf_ptr_addr;
    size_t tmp = 0;
    GLOG_DRV(gpuModuleGetGlobal(&lss_ptr_addr, &tmp, module_, ARK_LSS_NAME));
    GLOG_DRV(gpuModuleGetGlobal(&buf_ptr_addr, &tmp, module_, ARK_BUF_NAME));
    std::array<int, 4> data = {0, 0, 0, 0};
    manager->memcpy_htod((void*)lss_ptr_addr, 0, data.data(), 0,
                         sizeof(int) * data.size());
    manager->memcpy_htod((void*)buf_ptr_addr, 0, &buf_ptr_val, 0,
                         sizeof(GpuPtr));
    // TODO: remove this hack
    GpuPtr lss_0_ptr_addr;
    GpuPtr lss_1_ptr_addr;
    gpuDrvError ret =
        gpuModuleGetGlobal(&lss_0_ptr_addr, &tmp, module_, ARK_LSS_NAME "_0");
    if (ret == gpuDrvSuccess) {
        manager->memcpy_htod((void*)lss_0_ptr_addr, 0, data.data(), 0,
                             sizeof(int) * data.size());
    } else if (ret != gpuErrorNotFound) {
        GLOG_DRV(ret);
    }
    ret = gpuModuleGetGlobal(&lss_1_ptr_addr, &tmp, module_, ARK_LSS_NAME "_1");
    if (ret == gpuDrvSuccess) {
        manager->memcpy_htod((void*)lss_1_ptr_addr, 0, data.data(), 0,
                             sizeof(int) * data.size());
    } else if (ret != gpuErrorNotFound) {
        GLOG_DRV(ret);
    }
    // set the data buffer pointers of remote gpus
    int nrph = get_env().num_ranks_per_host;
    int nodes_id = ctx_->gpu_id() / nrph;
    // only set the GPU remote data buf pointers of the GPUs on the same node
    for (int i = nodes_id * nrph;
         i < (nodes_id + 1) * nrph && i < ctx_->world_size(); i++) {
        void* data_buf_value = ctx_->get_data_memory(i)->ref();
        if (data_buf_value == 0) {
            continue;
        }
        GpuPtr data_buf_ptr;
        std::string data_buf_name = ARK_BUF_NAME + std::to_string(i);
        gpuDrvError _e = gpuModuleGetGlobal(&data_buf_ptr, &tmp, module_,
                                            data_buf_name.c_str());
        if (_e == gpuErrorNotFound) {
            LOG(DEBUG, "global variable ", data_buf_name, " not found");
            continue;
        }
        LOG(DEBUG, data_buf_name, " data_buf_ptr=", std::hex, data_buf_ptr,
            " data_buf_value=", data_buf_value);
        manager->memcpy_htod((void*)data_buf_ptr, 0, &data_buf_value, 0,
                             sizeof(GpuPtr));
    }

    std::shared_ptr<GpuCommSw> comm = ctx_->get_comm_sw();
    if (comm->get_proxy_channels_num() > 0) {
        GpuPtr channel_addr;
        GLOG_DRV(gpuModuleGetGlobal(&channel_addr, &tmp, module_,
                                    "_ARK_PROXY_CHANS"));
        const void* chans_ref = comm->get_proxy_channels_ref();
        size_t chans_bytes = comm->get_proxy_channels_bytes();
        manager->memcpy_htod((void*)channel_addr, 0,
                             const_cast<void*>(chans_ref), 0, chans_bytes);
    }
    if (comm->get_sm_channels_num() > 0) {
        GpuPtr channel_addr;
        GLOG_DRV(
            gpuModuleGetGlobal(&channel_addr, &tmp, module_, "_ARK_SM_CHANS"));
        const void* chans_ref = comm->get_sm_channels_ref();
        size_t chans_bytes = comm->get_sm_channels_bytes();
        manager->memcpy_htod((void*)channel_addr, 0,
                             const_cast<void*>(chans_ref), 0, chans_bytes);
    }
}

void GpuLoopKernel::launch(std::shared_ptr<GpuStream> stream,
                           bool disable_timing) {
    elapsed_msec_ = -1;
    if (!is_compiled()) {
        ERR(InvalidUsageError, "Need to compile first before initialization.");
    } else if (stream == nullptr) {
        ERR(InvalidUsageError, "Given an invalid stream.");
    } else if (stream_ != nullptr) {
        if (stream_ == stream) {
            LOG(WARN, "Ignore launching twice.");
            return;
        } else {
            ERR(InvalidUsageError, "This loop kernel is already running.");
        }
    }
    if (!disable_timing) {
        timer_begin_->record(stream);
    }

    ctx_->get_comm_sw()->launch_request_loop();

    // Initialize loop flags.
    atomicStoreRelaxed(flag_->ref<int>(), 0);
    GpuKernel::launch(stream);
    stream_ = stream;
    if (!disable_timing) {
        timer_end_->record(stream);
        is_recording_ = true;
    }
}

void GpuLoopKernel::run(int iter) {
    if (iter > 0) {
        while (atomicLoadRelaxed(flag_->ref<int>()) > 0) {
        }
        atomicStoreRelaxed(flag_->ref<int>(), iter);
    }
}

bool GpuLoopKernel::poll() { return atomicLoadRelaxed(flag_->ref<int>()) <= 0; }

void GpuLoopKernel::wait() {
    int cnt = MAX_LOOP_COUNTER;
    while (atomicLoadRelaxed(flag_->ref<int>()) > 0) {
        if (--cnt > 0) {
            continue;
        }
        // Check if the kernel encountered an error.
        gpuError res = stream_->query();
        if (res == gpuSuccess) {
            if (atomicLoadRelaxed(flag_->ref<int>()) > 0) {
                LOG(WARN, "Stream is finished but the loop flag is still set.");
                break;
            } else {
                LOG(WARN,
                    "wait() is delayed by a stream query. Regarding "
                    "timing measurements may be inaccurate.");
                break;
            }
        } else if (res == gpuErrorNotReady) {
            cnt = MAX_LOOP_COUNTER;
        } else {
            GLOG(res);
        }
    }
}

void GpuLoopKernel::stop() {
    wait();
    atomicStoreRelaxed(flag_->ref<int>(), -1);
    stream_->sync();
    if (is_recording_) {
        elapsed_msec_ = timer_end_->elapsed_msec(*timer_begin_);
        is_recording_ = false;
    }
    stream_ = nullptr;
    ctx_->get_comm_sw()->stop_request_loop();
}

float GpuLoopKernel::get_elapsed_msec() const {
    if (is_recording_) {
        ERR(InvalidUsageError, "Need to stop the kernel first.");
    }
    return elapsed_msec_;
}

}  // namespace ark