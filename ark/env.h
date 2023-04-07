#ifndef ARK_ENV_H_
#define ARK_ENV_H_

#include <string>

namespace ark {

// Environment variables.
struct Env
{
    Env();
    // Log level.
    const char *log_level;
    // Root directory where ARK is installed.
    std::string path_root_dir;
    // Temporary directory.
    std::string path_tmp_dir;
    // If true, we do not remove temporal files in `path_tmp_dir`.
    bool keep_tmp;
    // Hostfile.
    std::string hostfile;
    // PCIe name (domain:bus:slot.function) of the FPGA.
    std::string fpga_dbsf;
    // Base value of listen socket ports.
    int ipc_listen_port_base;
    // Number of ranks per host.
    int num_ranks_per_host;
    // Disable IB.
    bool disable_ib;
    // Disable P2P CUDA memcpy.
    bool disable_p2p_memcpy;
    // The scheduler to use.
    std::string scheduler;
};

// Get the global Env.
const Env &get_env();

} // namespace ark

#endif // ARK_ENV_H_