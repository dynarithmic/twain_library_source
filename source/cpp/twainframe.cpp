/*
This file is part of the Dynarithmic TWAIN Library (DTWAIN).
Copyright (c) 2002-2023 Dynarithmic Software.

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
#include <array>
#include "twainframe.h"
#include "twainfix32.h"

namespace dynarithmic
{
    bool TwainFrameInternal::IsValidComponent(int nWhich)
    {
        return nWhich == FRAMETOP || 
            nWhich == FRAMELEFT || 
            nWhich == FRAMERIGHT || 
            nWhich == FRAMEBOTTOM;
    }

    TwainFrameInternal::TwainFrameInternal(double left, double top, double right, double bottom) : m_FrameComponent{}
    {
        SetFrame(left, top, right, bottom);
    }

    TwainFrameInternal::TwainFrameInternal(const TW_FRAME& frame) : m_FrameComponent{}
    {
        SetFrame(static_cast<double>(Fix32ToFloat(frame.Left)),
                 static_cast<double>(Fix32ToFloat(frame.Top)),
                 static_cast<double>(Fix32ToFloat(frame.Right)),
                 static_cast<double>(Fix32ToFloat(frame.Bottom)));
    }

    TwainFrameInternal& TwainFrameInternal::operator=(const TW_FRAME& frame)
    {
        From_TWFRAME(frame);
        return *this;
    }

    void TwainFrameInternal::SetFrame(double left, double top, double right, double bottom)
    {
        m_FrameComponent[FRAMELEFT] = left;
        m_FrameComponent[FRAMETOP] = top;
        m_FrameComponent[FRAMERIGHT] = right;
        m_FrameComponent[FRAMEBOTTOM] = bottom;
    }

    TW_FRAME TwainFrameInternal::To_TWFRAME() const
    {
        TW_FRAME ret;
        ret.Top = FloatToFix32(static_cast<float>(m_FrameComponent[FRAMETOP]));
        ret.Left = FloatToFix32(static_cast<float>(m_FrameComponent[FRAMELEFT]));
        ret.Right = FloatToFix32(static_cast<float>(m_FrameComponent[FRAMERIGHT]));
        ret.Bottom = FloatToFix32(static_cast<float>(m_FrameComponent[FRAMEBOTTOM]));
        return ret;
    }

    void TwainFrameInternal::From_TWFRAME(const TW_FRAME& frame)
    {
        SetFrame(static_cast<double>(Fix32ToFloat(frame.Left)),
                 static_cast<double>(Fix32ToFloat(frame.Top)),
                 static_cast<double>(Fix32ToFloat(frame.Right)),
                 static_cast<double>(Fix32ToFloat(frame.Bottom)));
    }

    double TwainFrameInternal::Left() const
    {
        return m_FrameComponent[FRAMELEFT];
    }

    double TwainFrameInternal::Top() const
    {
        return m_FrameComponent[FRAMETOP];
    }

    double TwainFrameInternal::Right() const
    {
        return m_FrameComponent[FRAMERIGHT];
    }

    double TwainFrameInternal::Bottom() const
    {
        return m_FrameComponent[FRAMEBOTTOM];
    }
}
