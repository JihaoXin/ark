#ifndef ARK_SCHED_OPSEQ_H_
#define ARK_SCHED_OPSEQ_H_

#include <cassert>
#include <map>
#include <string>
#include <tuple>

#include "ark/gpu/gpu_mgr.h"
#include "ark/sched/sched_op.h"

namespace ark {
class SchedOpSeq
{
  public:
    SchedOpSeq() : id{-1}
    {
    }
    SchedOpSeq(int id);
    SchedOpSeq(int id_, const Op *op, const OpConfig *cfg);
    bool append(const Op *op, const OpConfig *cfg);

    bool is_virtual() const
    {
        return num_warps == 0;
    }
    bool is_send() const;
    bool is_recv() const;

    const int &get_id() const
    {
        return id;
    }
    const std::vector<SchedOp> &get_sched_ops() const
    {
        return seq;
    }
    const Op *get_last_op() const
    {
        return seq.back().get_op();
    }
    const std::vector<std::pair<int, int>> &get_fdims() const
    {
        return seq_fdims;
    }
    const unsigned int get_num_warps() const
    {
        return num_warps;
    }
    const unsigned int get_smem_bytes() const
    {
        return smem_bytes;
    }
    const std::array<int, 3> &get_tdims() const
    {
        return tdims;
    }
    const int get_tdims_size() const
    {
        return tdims[0] * tdims[1] * tdims[2];
    }
    const int get_tdim_x() const
    {
        return tdims[2];
    }
    const int get_tdim_y() const
    {
        return tdims[1];
    }
    const int get_tdim_z() const
    {
        return tdims[0];
    }
    const int get_tdim_xz() const
    {
        return tdims[0] * tdims[2];
    }

    friend bool operator<(const SchedOpSeq &ops1, const SchedOpSeq &ops2);
    friend bool operator==(const SchedOpSeq &ops1, const SchedOpSeq &ops2);

  private:
    const int id;
    std::vector<SchedOp> seq;
    std::vector<std::pair<int, int>> seq_fdims;
    unsigned int num_warps = 0;
    unsigned int smem_bytes = 0;
    std::array<int, 3> tdims = {{0, 0, 0}};
};

void to_json(nlohmann::json &j, const SchedOpSeq &opseq);
void from_json(const nlohmann::json &j, SchedOpSeq &opseq);

struct Sched
{
    // Indicates:
    // if (sm_b <= blockIdx.x < sm_e) and (th_b <= threadIdx.x < th_e);
    // do run opseq(alpha * (blockIdx.x - sm_b) + beta); fi.
    // opseq == nullptr implicitly indicates a global sync.
    Sched(SchedOpSeq *opseq_, int sm_b_, int sm_e_, int th_b_, int th_e_,
          int alpha_, int beta_)
        : opseq{opseq_}, sm_b{sm_b_}, sm_e{sm_e_}, th_b{th_b_}, th_e{th_e_},
          alpha{alpha_}, beta{beta_}
    {
    }
    SchedOpSeq *opseq;
    int sm_b;
    int sm_e;
    int th_b;
    int th_e;
    int alpha;
    int beta;
};

} // namespace ark
#endif // ARK_SCHED_OPSEQ_H_