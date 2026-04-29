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
#ifndef IOHANDLERJPEG_H
#define IOHANDLERJPEG_H

#include "ctldib32.h"

namespace dynarithmic
{
	class CTL_JpegIOHandler : public CTL_ImageIOHandler
	{
		public:
			CTL_JpegIOHandler(int Quality = 75, BOOL bJpegProgressive = FALSE) :
				CTL_ImageIOHandler(),
				m_nJpegQuality(Quality),
				m_bJpegProgressive(bJpegProgressive) {
				SetSaveArgs();
			}

			CTL_JpegIOHandler(CTL_TwainDib* pDib, DTWAINImageInfoEx& ImageInfoEx)
				: CTL_ImageIOHandler(pDib),
				m_nJpegQuality(75),
				m_bJpegProgressive(false),
				m_ImageInfoEx(ImageInfoEx) {
				SetSaveArgs();
			}

			void SetSaveArgs()
			{
			}

			int WriteBitmap(LPCTSTR szFile, bool bOpenFile, int fh, DibMultiPageStruct* pDibStruct = nullptr) override;

		private:
			int     m_nJpegQuality;
			BOOL    m_bJpegProgressive;
			DTWAINImageInfoEx m_ImageInfoEx;
	};
}
#endif