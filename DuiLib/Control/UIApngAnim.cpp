#include "StdAfx.h"

#ifdef SUPPORT_APNG

#include "UIApngAnim.h"

namespace DuiLib
{
	extern Color ARGB2Color(DWORD dwColor);

	CApngAnimUI::CApngAnimUI(void)
	{
		m_pgdipBmp		=	NULL;	
		m_nFramePosition	=	0;	
		m_nPlays = 0;
		m_bIsAutoPlay		=	true;
		m_bIsAutoSize		=	false;
		m_bIsPlaying		=	false;
		m_nPngWidth = 0;
		m_nPngHeight = 0;
		m_nRowSize = 0;
		m_nChannels = 0;

		m_uTextStyle = DT_SINGLELINE | DT_VCENTER | DT_CENTER;
		m_TextRenderingAlias = TextRenderingHintAntiAlias;
		m_dwTextColor = 0xFF000000;
		m_dwDisabledTextColor = 0xFF000000;
		m_iFont = -1;
		m_rcTextPadding.left = m_rcTextPadding.top = m_rcTextPadding.right = m_rcTextPadding.bottom = 0;
	}


	CApngAnimUI::~CApngAnimUI(void)
	{
		DeleteApng();
		if (m_pManager)
			m_pManager->KillTimer( this, EVENT_TIME_ID );

	}

	LPCTSTR CApngAnimUI::GetClass() const
	{
		return DUI_CTR_APNGANIM;
	}

	LPVOID CApngAnimUI::GetInterface( LPCTSTR pstrName )
	{
		if( _tcscmp(pstrName, DUI_CTR_APNGANIM) == 0 ) return static_cast<CApngAnimUI*>(this);
		return CControlUI::GetInterface(pstrName);
	}

	void CApngAnimUI::DoInit()
	{
		InitApngImage();
	}

	bool CApngAnimUI::DoPaint(HDC hDC, const RECT& rcPaint, CControlUI* pStopControl)
	{
		if ( m_ApngImage.IsEmpty() )
		{		
			InitApngImage();
		}
		DrawFrame( hDC );
		return true;
	}

	void CApngAnimUI::DoEvent( TEventUI& event )
	{
		if( event.Type == UIEVENT_TIMER )
			OnTimer( (UINT_PTR)event.wParam );
		else
			CControlUI::DoEvent(event);
	}

	void CApngAnimUI::SetVisible(bool bVisible /* = true */)
	{
		CControlUI::SetVisible(bVisible);
		if (bVisible)
			Play();
		else
			Stop();
	}

	void CApngAnimUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcscmp(pstrName, _T("bkimage")) == 0 ) SetBkImage(pstrValue);
		else if( _tcscmp(pstrName, _T("autoplay")) == 0 ) {
			SetAutoPlay(_tcscmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcscmp(pstrName, _T("autosize")) == 0 ) {
			SetAutoSize(_tcscmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcscmp(pstrName, _T("align")) == 0 ) 
		{
			if( _tcscmp(pstrValue, _T("left")) == 0 ) {
				m_uTextStyle &= ~(DT_CENTER | DT_RIGHT);
				m_uTextStyle |= DT_LEFT;
			}
			else if( _tcscmp(pstrValue, _T("center")) == 0 ) {
				m_uTextStyle &= ~(DT_LEFT | DT_RIGHT);
				m_uTextStyle |= DT_CENTER;
			}
			else if( _tcscmp(pstrValue, _T("right")) == 0 ) {
				m_uTextStyle &= ~(DT_LEFT | DT_CENTER);
				m_uTextStyle |= DT_RIGHT;
			}
		}
		else if (_tcscmp(pstrName, _T("valign")) == 0)
		{
			if (_tcscmp(pstrValue, _T("top")) == 0) {
				m_uTextStyle &= ~(DT_BOTTOM | DT_VCENTER);
				m_uTextStyle |= DT_TOP;
			}
			else if (_tcscmp(pstrValue, _T("vcenter")) == 0) {
				m_uTextStyle &= ~(DT_TOP | DT_BOTTOM);
				m_uTextStyle |= DT_VCENTER;
			}
			else if (_tcscmp(pstrValue, _T("bottom")) == 0) {
				m_uTextStyle &= ~(DT_TOP | DT_VCENTER);
				m_uTextStyle |= DT_BOTTOM;
			}
		}
		else if( _tcscmp(pstrName, _T("textcolor")) == 0 ) {
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetTextColor(clrColor);
		}
		else if( _tcscmp(pstrName, _T("disabledtextcolor")) == 0 ) {
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetDisabledTextColor(clrColor);
		}
		else if(_tcscmp(pstrName, _T("rhaa")) == 0 ) SetTextRenderingAlias(_ttoi(pstrValue));
		else if( _tcscmp(pstrName, _T("multiline")) == 0 ) SetMultiLine(_tcscmp(pstrValue, _T("true")) == 0);
		else if( _tcscmp(pstrName, _T("font")) == 0 ) SetFont(_ttoi(pstrValue));
		else if( _tcscmp(pstrName, _T("textpadding")) == 0 ) {
			RECT rcTextPadding = { 0 };
			LPTSTR pstr = NULL;
			rcTextPadding.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
			rcTextPadding.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
			rcTextPadding.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
			rcTextPadding.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
			SetTextPadding(rcTextPadding);
		}
		else
			CControlUI::SetAttribute(pstrName, pstrValue);
	}

	void CApngAnimUI::SetTextColor(DWORD dwTextColor)
	{
		m_dwTextColor = dwTextColor;
	}

	DWORD CApngAnimUI::GetTextColor() const
	{
		return m_dwTextColor;
	}

	bool CApngAnimUI::IsMultiLine()
	{
		return (m_uTextStyle & DT_SINGLELINE) == 0;
	}

	void CApngAnimUI::SetMultiLine(bool bMultiLine)
	{
		if (bMultiLine)	{
			m_uTextStyle  &= ~DT_SINGLELINE;
			m_uTextStyle |= DT_WORDBREAK;
		}
		else 
			m_uTextStyle |= DT_SINGLELINE;
	}

	void CApngAnimUI::SetDisabledTextColor(DWORD dwTextColor)
	{
		m_dwDisabledTextColor = dwTextColor;
		Invalidate();
	}

	DWORD CApngAnimUI::GetDisabledTextColor() const
	{
		return m_dwDisabledTextColor;
	}

	void CApngAnimUI::SetTextRenderingAlias(int nTextRenderingAlias)
	{
		m_TextRenderingAlias = (TextRenderingHint)nTextRenderingAlias;
		Invalidate();
	}

	TextRenderingHint CApngAnimUI::GetTextRenderingAlias()
	{
		return m_TextRenderingAlias;
	}

	void CApngAnimUI::SetFont(int index)
	{
		m_iFont = index;
	}

	int CApngAnimUI::GetFont() const
	{
		return m_iFont;
	}

	RECT CApngAnimUI::GetTextPadding() const
	{
		return m_rcTextPadding;
	}

	void CApngAnimUI::SetTextPadding(RECT rc)
	{
		m_rcTextPadding = rc;
	}

	void CApngAnimUI::SetBkImage(const CDuiString& strImage)
	{
		if( m_sBkImage == strImage || strImage.IsEmpty()) return;

		m_sBkImage = strImage;

		Stop();
		DeleteApng();

		Invalidate();
	}

	CDuiString CApngAnimUI::GetBkImage()
	{
		return m_sBkImage;
	}

	void CApngAnimUI::SetAutoPlay(bool bIsAuto)
	{
		m_bIsAutoPlay = bIsAuto;
	}

	bool CApngAnimUI::IsAutoPlay() const
	{
		return m_bIsAutoPlay;
	}

	void CApngAnimUI::SetAutoSize(bool bIsAuto)
	{
		m_bIsAutoSize = bIsAuto;
	}

	bool CApngAnimUI::IsAutoSize() const
	{
		return m_bIsAutoSize;
	}

	void CApngAnimUI::Play()
	{
		if (m_bIsPlaying || m_ApngImage.IsEmpty())
		{
			return;
		}

		long lPause = ((ApngFrame*)m_ApngImage[m_nFramePosition])->nDelayMS;
		if ( lPause == 0 ) lPause = 100;
		m_pManager->SetTimer( this, EVENT_TIME_ID, lPause );

		m_bIsPlaying = true;
	}

	void CApngAnimUI::Pause()
	{
		if (!m_bIsPlaying || m_ApngImage.IsEmpty())
		{
			return;
		}

		m_pManager->KillTimer(this, EVENT_TIME_ID);
		this->Invalidate();
		m_bIsPlaying = false;
	}

	void CApngAnimUI::Stop()
	{
		if (!m_bIsPlaying)
		{
			return;
		}

		m_pManager->KillTimer(this, EVENT_TIME_ID);
		m_nFramePosition = 0;
		this->Invalidate();
		m_bIsPlaying = false;
	}

	void CApngAnimUI::InitApngImage()
	{
		LoadApngFromFile(GetBkImage().GetData());
		if ( m_ApngImage.IsEmpty() ) return;

		if (m_bIsAutoSize)
		{
			SetFixedWidth(m_nPngWidth);
			SetFixedHeight(m_nPngHeight);
		}
		if (m_bIsAutoPlay && m_ApngImage.GetSize() > 0)
		{
			Play();
		}
	}

	void CApngAnimUI::DeleteApng()
	{
		if ( m_ApngImage.IsEmpty() == false )
		{
			for (int i=0;i<m_ApngImage.GetSize();i++)
			{
				ApngFrame* pFrame = (ApngFrame*)m_ApngImage[i];
				delete pFrame;
			}
			m_ApngImage.Empty();
		}

		m_nFramePosition	=	0;	
	}

	void CApngAnimUI::OnTimer( UINT_PTR idEvent )
	{
		if ( idEvent != EVENT_TIME_ID )
			return;
		m_pManager->KillTimer( this, EVENT_TIME_ID );
		this->Invalidate();

		if (m_ApngImage.GetSize() > 0)
			m_nFramePosition = (++m_nFramePosition) % m_ApngImage.GetSize();

		long lPause = ((ApngFrame*)m_ApngImage[m_nFramePosition])->nDelayMS;
		if ( lPause == 0 ) lPause = 100;
		m_pManager->SetTimer( this, EVENT_TIME_ID, lPause );
	}

	void CApngAnimUI::DrawFrame( HDC hDC )
	{
		if ( NULL == hDC || m_ApngImage.IsEmpty() ) return;
	
		int nItemWidth = m_rcItem.right-m_rcItem.left;
		int nItemHeight = m_rcItem.bottom-m_rcItem.top;

		//绘制背景色
		Gdiplus::Graphics graphics( hDC );
		graphics.SetSmoothingMode(SmoothingMode::SmoothingModeHighQuality);
		SolidBrush backBrush(ARGB2Color(m_dwBackColor));
		graphics.FillRectangle(&backBrush,m_rcItem.left, m_rcItem.top, nItemWidth, nItemHeight);

		//绘制当前帧
		ApngFrame* pFrame = (ApngFrame*)m_ApngImage[m_nFramePosition];
		if (m_pgdipBmp)
		{
			delete m_pgdipBmp;
			m_pgdipBmp = NULL;
		}
		m_pgdipBmp = new Bitmap(m_nPngWidth, m_nPngHeight);		
		Rect rc(0, 0, m_nPngWidth, m_nPngHeight);
		BitmapData bmd;
		LPBYTE pBuf = pFrame->pframeBits;
		m_pgdipBmp->LockBits(&rc, ImageLockModeWrite, PixelFormat32bppARGB, &bmd);
		LPBYTE pDst = (LPBYTE)bmd.Scan0;
		for (UINT32 i = 0; i < m_nPngHeight; ++i)
		{
			memcpy(pDst, pBuf, m_nRowSize);
			pDst += bmd.Stride;
			pBuf += m_nRowSize;
		}
		m_pgdipBmp->UnlockBits(&bmd);
		graphics.DrawImage( m_pgdipBmp, m_rcItem.left, m_rcItem.top,nItemWidth, nItemHeight );		
		
		//绘制文本
		if (m_sText.IsEmpty() == false)
		{
			graphics.SetTextRenderingHint(GetTextRenderingAlias());
			Gdiplus::Font	nFont(hDC,m_pManager->GetFont(GetFont()));

			RECT rcText = {m_rcItem.left+m_rcTextPadding.left,m_rcItem.top+m_rcTextPadding.top,m_rcItem.right-m_rcTextPadding.right,m_rcItem.bottom-m_rcTextPadding.bottom};
			RectF nRc((float)rcText.left,(float)rcText.top,(float)rcText.right-rcText.left,(float)rcText.bottom-rcText.top);

			StringFormat format;
			StringAlignment sa = StringAlignmentNear;
			if ((m_uTextStyle & DT_VCENTER) != 0) 
				sa = StringAlignmentCenter;
			else if( (m_uTextStyle & DT_BOTTOM) != 0) 
				sa = StringAlignmentFar;
			format.SetLineAlignment((StringAlignment)sa);
			sa = StringAlignmentNear;
			if ((m_uTextStyle & DT_CENTER) != 0) 
				sa = StringAlignmentCenter;
			else if( (m_uTextStyle & DT_RIGHT) != 0) 
				sa = StringAlignmentFar;
			format.SetAlignment((StringAlignment)sa);
			if ((m_uTextStyle & DT_SINGLELINE) != 0) 
				format.SetFormatFlags(StringFormatFlagsNoWrap);

			DWORD clrColor = IsEnabled()?m_dwTextColor:m_dwDisabledTextColor;
			SolidBrush nBrush( ARGB2Color(clrColor) );

			CDuiString sText1 = m_sText;
			CPaintManagerUI::ProcessMultiLanguageTokens(sText1);
#ifdef _UNICODE			
			LPCWSTR pstrText = sText1.GetData();
#else
			int iLen = _tcslen(sText1.GetData());
			LPWSTR pWideText = new WCHAR[iLen + 1];
			::ZeroMemory(pWideText, (iLen + 1) * sizeof(WCHAR));
			::MultiByteToWideChar(CP_ACP, 0, sText1.GetData(), -1, pWideText, iLen);
			LPCWSTR pstrText = pWideText;			
#endif
			graphics.DrawString(pstrText,wcslen(pstrText),&nFont,nRc,&format,&nBrush);
#ifndef _UNICODE
			delete[] pWideText;
#endif
		}
	}

	void CApngAnimUI::LoadApngFromFile(LPCTSTR pstrApngPath)
	{
		LPBYTE pData = NULL;
		DWORD dwSize = 0;
		do 
		{
			CDuiString sFile = CPaintManagerUI::GetResourcePath();
			if( CPaintManagerUI::GetResourceZip().IsEmpty() ) {
				sFile += pstrApngPath;
				HANDLE hFile = ::CreateFile(sFile.GetData(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, \
					FILE_ATTRIBUTE_NORMAL, NULL);
				if( hFile == INVALID_HANDLE_VALUE ) break;
				dwSize = ::GetFileSize(hFile, NULL);
				if( dwSize == 0 )
				{
					::CloseHandle( hFile );
					break;
				}

				DWORD dwRead = 0;
				pData = new BYTE[ dwSize ];
				::ReadFile( hFile, pData, dwSize, &dwRead, NULL );
				::CloseHandle( hFile );

				if( dwRead != dwSize ) {
					delete[] pData;
					pData = NULL;
					break;
				}
			}
			else {
				sFile += CPaintManagerUI::GetResourceZip();
				HZIP hz = NULL;
				if( CPaintManagerUI::IsCachedResourceZip() ) 
					hz = (HZIP)CPaintManagerUI::GetResourceZipHandle();
				else
					hz = OpenZip(sFile.GetData(),CPaintManagerUI::GetResourceZipPassword());
				if( hz == NULL ) 
					break;
				ZIPENTRY ze; 
				int i; 
				if( FindZipItem(hz, pstrApngPath, true, &i, &ze) != 0 ) 
					break;
				dwSize = ze.unc_size;
				if( dwSize == 0 ) 
					break;
				pData = new BYTE[ dwSize ];
				int res = UnzipItem(hz, i, pData, dwSize);
				if( res != 0x00000000 && res != 0x00000600) 
				{
					delete[] pData;
					pData = NULL;
					if( !CPaintManagerUI::IsCachedResourceZip() ) CloseZip(hz);
					break;
				}
				if( !CPaintManagerUI::IsCachedResourceZip() ) CloseZip(hz);
			}

		} while (0);

		while (!pData)
		{
			//读不到图片, 则直接去读取bitmap.m_lpstr指向的路径
			HANDLE hFile = ::CreateFile(pstrApngPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, \
				FILE_ATTRIBUTE_NORMAL, NULL);
			if( hFile == INVALID_HANDLE_VALUE ) break;
			dwSize = ::GetFileSize(hFile, NULL);
			if( dwSize == 0 ) 
			{
				CloseHandle(hFile);
				break;
			}

			DWORD dwRead = 0;
			pData = new BYTE[ dwSize ];
			::ReadFile( hFile, pData, dwSize, &dwRead, NULL );
			::CloseHandle( hFile );

			if( dwRead != dwSize ) {
				delete[] pData;
				pData = NULL;
			}
			break;
		}
		if (!pData)
		{
			return ;
		}

		LoadApngFromMemory(pData, dwSize);
		delete[] pData;
	}

	void CApngAnimUI::BlendOver(BYTE** rows_dst, BYTE** rows_src, UINT32 x, UINT32 y, UINT32 w, UINT32 h)
	{
		UINT32 i, j;
		int u, v, al;

		for (j = 0; j < h; j++)
		{
			BYTE* sp = rows_src[j];
			BYTE* dp = rows_dst[j + y] + x * 4;

			for (i = 0; i < w; i++, sp += 4, dp += 4)
			{
				if (sp[3] == 255)
					memcpy(dp, sp, 4);
				else if (sp[3] != 0)
				{
					if (dp[3] != 0)
					{
						u = sp[3] * 255;
						v = (255 - sp[3]) * dp[3];
						al = u + v;
						dp[0] = (sp[0] * u + dp[0] * v) / al;
						dp[1] = (sp[1] * u + dp[1] * v) / al;
						dp[2] = (sp[2] * u + dp[2] * v) / al;
						dp[3] = al / 255;
					}
					else
						memcpy(dp, sp, 4);
				}
			}
		}
	}
	void CApngAnimUI::pngReadCallback(png_structp png_ptr, png_bytep data, png_size_t length)
	{
		typedef struct tagImgdata
		{
			BYTE* pData;
			DWORD size;
			DWORD offset;
		}Imgdata;
		Imgdata* isource = (Imgdata*)png_get_io_ptr(png_ptr);

		if(isource->offset + length <= isource->size)
		{
			memcpy(data, isource->pData+isource->offset, length);
			isource->offset += length;
		}
	}
	void CApngAnimUI::LoadApngFromMemory( LPVOID pBuf,size_t dwSize )
	{
		if (png_sig_cmp((png_const_bytep)pBuf, 0, 8))
			return ;//不是png或者apng
		
		UINT32 i, j;
		png_bytepp rows_image;
		png_bytepp rows_frame;
		BYTE* p_image = NULL;
		BYTE* p_frame = NULL;
		BYTE* p_temp = NULL;

		png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		png_infop info_ptr = png_create_info_struct(png_ptr);
		if (png_ptr && info_ptr)
		{
			if (setjmp(png_jmpbuf(png_ptr)))
			{
				png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
				return ;
			}

			typedef struct tagImgdata
			{
				BYTE* pData;
				DWORD size;
				DWORD offset;
			}Imgdata;
			Imgdata pngData;
			pngData.pData = (BYTE*)pBuf;
			pngData.size = dwSize;
			pngData.offset = 0;
			png_set_read_fn(png_ptr,&pngData,pngReadCallback);
			png_set_sig_bytes(png_ptr, 0);
			png_read_info(png_ptr, info_ptr);

			if (!png_get_valid(png_ptr, info_ptr, PNG_INFO_acTL))
			{				
				png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
				return ;
			}

			UINT32 nFrameCount = 1;
			m_nPlays = 0;
			png_get_acTL(png_ptr, info_ptr, &nFrameCount, &m_nPlays);
			if (nFrameCount <= 1)
			{			
				png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
				return ;
			}

			png_set_expand(png_ptr);
			png_set_strip_16(png_ptr);
			png_set_gray_to_rgb(png_ptr);
			png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
			png_set_bgr(png_ptr);
			png_set_interlace_handling(png_ptr);
			png_read_update_info(png_ptr, info_ptr);

			m_nPngWidth = png_get_image_width(png_ptr, info_ptr);
			m_nPngHeight = png_get_image_height(png_ptr, info_ptr);
			m_nChannels = png_get_channels(png_ptr, info_ptr);
			m_nRowSize = png_get_rowbytes(png_ptr, info_ptr);
			DWORD nFrameSize = m_nPngHeight * m_nRowSize;
			p_image = (BYTE*)malloc(nFrameSize);
			p_frame = (BYTE*)malloc(nFrameSize);
			p_temp = (BYTE*)malloc(nFrameSize);
			rows_image = (png_bytepp)malloc(m_nPngHeight * sizeof(png_bytep));
			rows_frame = (png_bytepp)malloc(m_nPngHeight * sizeof(png_bytep));
			bool bFirstFrameIsHidden = false;
			if (png_get_first_frame_is_hidden(png_ptr, info_ptr)!=0)
				bFirstFrameIsHidden = true;
			if (p_image && p_frame && p_temp && rows_image && rows_frame)
			{
				png_uint_32 x0 = 0;
				png_uint_32 y0 = 0;
				png_uint_32 w0 = m_nPngWidth;
				png_uint_32 h0 = m_nPngHeight;
				unsigned short delay_num = 1;
				unsigned short delay_den = 10;
				BYTE dop = 0;
				BYTE bop = 0;
				UINT32 first = ((bFirstFrameIsHidden == true) ? 1 : 0);

				for (j = 0; j < m_nPngHeight; j++)
					rows_image[j] = p_image + j * m_nRowSize;

				for (j = 0; j < m_nPngHeight; j++)
					rows_frame[j] = p_frame + j * m_nRowSize;

				for (i = 0; i < nFrameCount; i++)
				{
					png_read_frame_head(png_ptr, info_ptr);
					png_get_next_frame_fcTL(png_ptr, info_ptr, &w0, &h0, &x0, &y0, &delay_num, &delay_den, &dop, &bop);

					if (i == first)
					{
						bop = PNG_BLEND_OP_SOURCE;
						if (dop == PNG_DISPOSE_OP_PREVIOUS)
							dop = PNG_DISPOSE_OP_BACKGROUND;
					}

					png_read_image(png_ptr, rows_frame);

					if (dop == PNG_DISPOSE_OP_PREVIOUS)
						memcpy(p_temp, p_image, nFrameSize);

					if (bop == PNG_BLEND_OP_OVER)
					{
						BlendOver(rows_image, rows_frame, x0, y0, w0, h0);
					}
					else
					{
						for (j = 0; j < h0; j++)
							memcpy(rows_image[j + y0] + x0 * 4, rows_frame[j], w0 * 4);
					}

					ApngFrame* pFrame = new ApngFrame;
					pFrame->nFrameID = i;
					pFrame->nDelayMS = delay_num * 1000 / delay_den;
					pFrame->mallocByte(m_nPngHeight*m_nRowSize);
					LPBYTE pBuf = pFrame->pframeBits;
					for (UINT32 iRow = 0; iRow < m_nPngHeight; ++iRow)
					{						
						memcpy(pBuf, rows_image[iRow], m_nRowSize);
						pBuf += m_nRowSize;
					}

					m_ApngImage.Add(pFrame);			

					if (dop == PNG_DISPOSE_OP_PREVIOUS)
					{
						memcpy(p_image, p_temp, nFrameSize);
					}
					else if (dop == PNG_DISPOSE_OP_BACKGROUND)
					{
						for (j = 0; j < h0; j++)
							memset(rows_image[j + y0] + x0 * 4, 0, w0 * 4);
					}
				}

				png_read_end(png_ptr, info_ptr);
				free(rows_frame);
				free(rows_image);
				free(p_temp);
				free(p_frame);
				free(p_image);
			}
		}
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	}
}

#endif //SUPPORT_APNG