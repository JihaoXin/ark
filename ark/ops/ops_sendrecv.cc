#include "ark/logging.h"
#include "ark/model_io.h"

using namespace std;

namespace ark {

//
Tensor *Model::send(Tensor *input, int id, int gpu_dst, size_t bytes,
                    Tensor *output, const string &name)
{
    size_t max_bytes = input->ldims_bytes();
    if (max_bytes < bytes) {
        LOGERR("invalid bytes: ", bytes, ", max: ", max_bytes);
    }
    if (bytes == 0) {
        bytes = max_bytes;
    }
    LOG(DEBUG, "send ", input->shape, " ", id, " ", gpu_dst, " ", bytes);
    input->exported = true;
    if (output == nullptr) {
        output = this->tensor({1, 1, 1, 1}, INT32);
    }
    this->create_op(OP_SEND, OP_PREC_NONE, {input}, {output},
                    {id, gpu_dst, bytes}, name);
    return output;
}

//
Tensor *Model::send_done(Tensor *input, int id, Tensor *output,
                         const string &name)
{
    LOG(DEBUG, "send_done ", input->shape, " ", id);
    if (output == nullptr) {
        output = this->tensor(input->shape, input->type, input->buf);
    }
    this->create_op(OP_SEND_DONE, OP_PREC_NONE, {input}, {output}, {id}, name);
    return output;
}

//
Tensor *Model::recv(Tensor *input, int id, int gpu_src, size_t bytes,
                    Tensor *output, const string &name)
{
    assert(input != nullptr);
    size_t max_bytes = input->ldims_bytes();
    if (max_bytes < bytes) {
        LOGERR("invalid bytes: ", bytes, ", max: ", max_bytes);
    }
    if (bytes == 0) {
        bytes = max_bytes;
    }
    LOG(DEBUG, "recv ", input->shape, " ", id, " ", gpu_src, " ", bytes);
    input->exported = true;
    if (output == nullptr) {
        output = this->tensor({1, 1, 1, 1}, INT32);
    }
    this->create_op(OP_RECV, OP_PREC_NONE, {input}, {output},
                    {id, gpu_src, bytes}, name);
    return output;
}

} // namespace ark