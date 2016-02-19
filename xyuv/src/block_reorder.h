/*
 * Copyright:
 * ----------------------------------------------------------------------------
 * This confidential and proprietary software may be used only as authorized
 * by a licensing agreement from ARM Limited.
 *      (C) COPYRIGHT 2016 ARM Limited and its affiliates, ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorized copies and
 * copies may only be made to the extent permitted by a licensing agreement
 * from ARM Limited.
 * ----------------------------------------------------------------------------
 */

#ifndef CROSSYUV_BLOCK_REORDER_H
#define CROSSYUV_BLOCK_REORDER_H

#include <cstdint>
namespace xyuv {

    struct format;

    bool needs_reorder(const xyuv::format & format );
    void reorder_transform(uint8_t *frame_base_ptr, const xyuv::plane &plane);
    void reorder_inverse(uint8_t *frame_base_ptr, const xyuv::plane &plane);

}
#endif //CROSSYUV_BLOCK_REORDER_H
