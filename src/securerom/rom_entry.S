# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/securerom/rom_entry.S $
#
# OpenPOWER HostBoot Project
#
# Contributors Listed Below - COPYRIGHT 2013,2017
# [+] International Business Machines Corp.
#
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied. See the License for the specific language governing
# permissions and limitations under the License.
#
# IBM_PROLOG_END_TAG
.include "kernel/ppcconsts.S"

.section .text

.global call_rom_verify
.global call_rom_SHA512

call_rom_verify:
call_rom_SHA512:

call_rom_entry:
    std r2, 40(r1)
    mflr r0
    std r0, 16(r1)
    stdu r1, -128(r1)
    li r2, 0
    mtctr r3
    mr r3, r4
    mr r4, r5
    mr r5, r6
    mr r6, r7
    mr r7, r8
    bctrl
    addi r1, r1, 128
    ld r2, 40(r1)
    ld r0, 16(r1)
    mtlr r0
    blr

