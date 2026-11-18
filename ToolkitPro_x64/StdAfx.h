// StdAfx.h : 为编译 Codejock Xtreme ToolkitPro v18.5.0 x64 静态库而重建的预编译头
// 说明：官方安装未附带库主体工程与 StdAfx，本文件依据源码包含关系重建（离线、合法）。
// 库工程统一包含 Common/XTPDLLExports.h：定义 _LIB 时 _XTP_EXT_CLASS 展开为空，
// 且不会引入无 include guard 的各模块 Resource.h（那些由各模块 Includes.h 自行受控包含一次）。
#pragma once

#ifndef __STDAFX_H__
#define __STDAFX_H__

#define _XTP_STATICLINK   // 静态链接形态
#define _LIB              // 编译静态库：_XTP_EXT_CLASS 展开为空
#define _XTP_USE_GDIPLUS  // 启用 GDI+（Codejock 默认）
#define VC_EXTRALEAN

#include "Common/XTPDLLExports.h"  // MFC 头 + 导出/类宏 + XTPVersion/XTPMacros
#include "Common/Includes.h"      // Common 基础：Framework(CXTPCmdTarget)/Application/SystemHelpers/ColorManager...

#endif // __STDAFX_H__
