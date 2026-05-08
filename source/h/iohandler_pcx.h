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
#ifndef IOHANDLERPCX_H
#define IOHANDLERPCX_H

#include "ctldib32.h"

namespace dynarithmic
{
	class CTL_PcxIOHandler : public CTL_ImageIOHandler
	{
		public:
			CTL_PcxIOHandler() : CTL_ImageIOHandler(), m_nFormat{} {};
			CTL_PcxIOHandler(CTL_TwainDib* pDib, int nFormat, DTWAINImageInfoEx& ImageInfoEx) : CTL_ImageIOHandler(pDib),
				m_nFormat(nFormat), m_ImageInfoEx(ImageInfoEx) {}

			int WriteBitmap(LPCTSTR szFile, bool bOpenFile, int fh, DibMultiPageStruct* pDibStruct = nullptr) override;

		private:
			int m_nFormat;
			DTWAINImageInfoEx m_ImageInfoEx;
			DTWAINPcxDcxOutput output;
	};
}
#endif