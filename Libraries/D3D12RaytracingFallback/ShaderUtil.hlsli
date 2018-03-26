//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
#define GLUE(a,b) a##b
#define REGISTER_SPACE(spaceNum) GLUE(space,spaceNum)
#define DECLARE_REGISTER(prefix, regNum, spaceNum) register(GLUE(prefix,regNum), REGISTER_SPACE(spaceNum))

#define CONSTANT_REGISTER_SPACE(regNum, spaceNum) DECLARE_REGISTER(b, regNum, spaceNum)
#define CONSTANT_REGISTER(regNum) CONSTANT_REGISTER_SPACE(regNum, 0)
#define SRV_REGISTER_SPACE(regNum, spaceNum) DECLARE_REGISTER(t, regNum, spaceNum)
#define SRV_REGISTER(regNum) SRV_REGISTER_SPACE(regNum, 0)
#define UAV_REGISTER_SPACE(regNum, spaceNum) DECLARE_REGISTER(u, regNum, spaceNum)
#define UAV_REGISTER(regNum) UAV_REGISTER_SPACE(regNum, 0)

#define FLT_MAX asfloat(0x7F7FFFFF)
