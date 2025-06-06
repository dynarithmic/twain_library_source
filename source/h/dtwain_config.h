/*
This file is part of the Dynarithmic TWAIN Library (DTWAIN).
Copyright (c) 2002-2025 Dynarithmic Software.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

FOR ANY PART OF THE COVERED WORK IN WHICH THE COPYRIGHT IS OWNED BY
DYNARITHMIC SOFTWARE. DYNARITHMIC SOFTWARE DISCLAIMS THE WARRANTY OF NON INFRINGEMENT
OF THIRD PARTY RIGHTS.
*/
#ifndef DTWAIN_CONFIG_H
#define DTWAIN_CONFIG_H

/* Build with DTWAIN logging of call stack */
#define DTWAIN_BUILD_LOGCALLSTACK 1 

/* Build without DTWAIN logging pointer values on return of functions. 
   Note that DTWAIN_BUILD_LOGCALLSTACK must be 1 for this to take effect */
#define DTWAIN_BUILD_LOGPOINTERS 1

/* This indicates that the build requires Visual C++ runtime if set to 1*/
/* Since there is no way to set the runtime to use at compile time, this */
/* setting must match the link-time options */
#define DTWAIN_BUILD_NEEDVCRUNTIME 0

#endif
