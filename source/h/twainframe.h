/*
This file is part of the Dynarithmic TWAIN Library (DTWAIN).
Copyright (c) 2002-2026 Dynarithmic Software.

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
#include "ctlconstexprutils.h"

namespace dynarithmic
{
    struct TwainFrameInternal
    {
        static constexpr int FRAMELEFT = 0;
        static constexpr int FRAMETOP = 1;
        static constexpr int FRAMERIGHT = 2;
        static constexpr int FRAMEBOTTOM = 3;

        std::array<double, 4> m_FrameComponent;

        static constexpr bool IsValidComponent(int nWhich)
        {
			return nWhich == FRAMETOP ||
				nWhich == FRAMELEFT ||
				nWhich == FRAMERIGHT ||
				nWhich == FRAMEBOTTOM;
        }

        constexpr TwainFrameInternal(double left = 0, double top = 0, double right = 0, double bottom = 0) : m_FrameComponent{}
        {
			SetFrame(left, top, right, bottom);
        }

        constexpr TwainFrameInternal(TW_FRAME frame)
        {
			SetFrame(static_cast<double>(Fix32ToFloat(frame.Left)),
				static_cast<double>(Fix32ToFloat(frame.Top)),
				static_cast<double>(Fix32ToFloat(frame.Right)),
				static_cast<double>(Fix32ToFloat(frame.Bottom)));
        }

		constexpr TwainFrameInternal& operator=(TW_FRAME frame) { From_TWFRAME(frame); return *this; }

        constexpr TW_FRAME To_TWFRAME() const
        {
			TW_FRAME ret = {};
			ret.Top = FloatToFix32(static_cast<float>(m_FrameComponent[FRAMETOP]));
			ret.Left = FloatToFix32(static_cast<float>(m_FrameComponent[FRAMELEFT]));
			ret.Right = FloatToFix32(static_cast<float>(m_FrameComponent[FRAMERIGHT]));
			ret.Bottom = FloatToFix32(static_cast<float>(m_FrameComponent[FRAMEBOTTOM]));
			return ret;
        }

        constexpr void From_TWFRAME(TW_FRAME frame)
        {
			SetFrame(static_cast<double>(Fix32ToFloat(frame.Left)),
				static_cast<double>(Fix32ToFloat(frame.Top)),
				static_cast<double>(Fix32ToFloat(frame.Right)),
				static_cast<double>(Fix32ToFloat(frame.Bottom)));
        }

		constexpr double Left() const
		{
			return m_FrameComponent[FRAMELEFT];
		}

		constexpr double Top() const
		{
			return m_FrameComponent[FRAMETOP];
		}

		constexpr double Right() const
		{
			return m_FrameComponent[FRAMERIGHT];
		}

		constexpr double Bottom() const
		{
			return m_FrameComponent[FRAMEBOTTOM];
		}

        constexpr void SetFrame(double left, double top, double right, double bottom)
        {
			m_FrameComponent[FRAMELEFT] = left;
			m_FrameComponent[FRAMETOP] = top;
			m_FrameComponent[FRAMERIGHT] = right;
			m_FrameComponent[FRAMEBOTTOM] = bottom;
        }
    };
}
#endif // TWAINFRAME_H
