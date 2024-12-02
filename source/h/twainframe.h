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
#ifndef TWAINFRAME_H
#define TWAINFRAME_H

#include <array>
#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#endif
#include <twain.h>

namespace dynarithmic
{
    struct TwainFrameInternal
    {
        static constexpr int FRAMELEFT = 0;
        static constexpr int FRAMETOP = 1;
        static constexpr int FRAMERIGHT = 2;
        static constexpr int FRAMEBOTTOM = 3;

        std::array<double, 4> m_FrameComponent;

        static bool IsValidComponent(int nWhich);

        TwainFrameInternal(double left = 0, double top = 0, double right = 0, double bottom = 0);
        TwainFrameInternal(TW_FRAME frame);
        TwainFrameInternal& operator=(TW_FRAME frame);
        TW_FRAME To_TWFRAME() const;
        void From_TWFRAME(TW_FRAME frame);
        double Left() const;
        double Top() const;
        double Right() const;
        double Bottom() const;
        void SetFrame(double left, double top, double right, double bottom);
    };
}
#endif // TWAINFRAME_H
