// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "ark/gpu/gpu_kernel.h"
#include "ark/include/ark.h"
#include "ark/include/ark_utils.h"
#include "ark/logging.h"
#include "ark/unittest/unittest_utils.h"

using namespace std;

void test_sendrecv_internal()
{
    for (int gpu_id = 0; gpu_id < 2; ++gpu_id) {
        ark::unittest::spawn_process([gpu_id]() {
            //
            ark::Model model;
            ark::Tensor *tns_x = model.tensor({1024}, ark::FP16);
            if (gpu_id == 0) {
                model.send(tns_x, 0, 1, 1024);
                model.send_done(tns_x, 0);
            }
            if (gpu_id == 1) {
                model.recv(tns_x, 0, 0);
            }

            ark::Executor exe{gpu_id, gpu_id, 2, model, "test_sendrecv"};
            exe.compile();

            exe.launch();
            exe.run(1);
            exe.stop();
            return ark::unittest::SUCCESS;
        });
    }

    ark::unittest::wait_all_processes();
}

ark::unittest::State test_sendrecv()
{
    test_sendrecv_internal();
    return ark::unittest::SUCCESS;
}

int main()
{
    ark::init();
    UNITTEST(test_sendrecv);
    return ark::unittest::SUCCESS;
}