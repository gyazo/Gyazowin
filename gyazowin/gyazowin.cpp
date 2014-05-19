// gyazowin.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "gyazowin.h"

// グローバル変数:
HINSTANCE hInst;							// 現在のインターフェイス
TCHAR *szTitle			= _T("Gyazo");		// タイトル バーのテキスト
TCHAR *szWindowClass	= _T("GYAZOWIN");	// メイン ウィンドウ クラス名
TCHAR *szWindowClassL	= _T("GYAZOWINL");	// レイヤー ウィンドウ クラス名
HWND hLayerWnd;

int ofX, ofY;	// 画面オフセット

// プロトタイプ宣言
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	LayerWndProc(HWND, UINT, WPARAM, LPARAM);

int					GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

BOOL				isPng(LPCTSTR fileName);
VOID				drawRubberband(HDC hdc, LPRECT newRect, BOOL erase);
VOID				execUrl(const char* str);
VOID				setClipBoardText(const char* str);
BOOL				convertPNG(LPCTSTR destFile, LPCTSTR srcFile);
BOOL				savePNG(LPCTSTR fileName, HBITMAP newBMP);
BOOL				uploadFile(HWND hwnd, LPCTSTR fileName);
std::string			getId();
BOOL				saveId(const WCHAR* str);

// エントリーポイント
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;

	TCHAR	szThisPath[MAX_PATH];
	DWORD   sLen;

	// 自身のディレクトリを取得する
	sLen = GetModuleFileName(NULL, szThisPath, MAX_PATH);
	for(unsigned int i = sLen; i >= 0; i--) {
		if(szThisPath[i] == _T('\\')) {
			szThisPath[i] = _T('\0');
			break;
		}
	}

	// カレントディレクトリを exe と同じ場所に設定
	SetCurrentDirectory(szThisPath);

	// 引数にファイルが指定されていたら
	if ( 2 == __argc )
	{
		// ファイルをアップロードして終了
		if (isPng(__targv[1])) {
			// PNG はそのままupload
			uploadFile(NULL, __targv[1]);
		}else {
			// PNG 形式に変換
			TCHAR tmpDir[MAX_PATH], tmpFile[MAX_PATH];
			GetTempPath(MAX_PATH, tmpDir);
			GetTempFileName(tmpDir, _T("gya"), 0, tmpFile);
			
			if (convertPNG(tmpFile, __targv[1])) {
				//アップロード
				uploadFile(NULL, tmpFile);
			} else {
				// PNGに変換できなかった...
				MessageBox(NULL, _T("Cannot convert this image"), szTitle, 
					MB_OK | MB_ICONERROR);
			}
			DeleteFile(tmpFile);
		}
		return TRUE;
	}

	// ウィンドウクラスを登録
	MyRegisterClass(hInstance);

	// アプリケーションの初期化を実行します:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}
	
	// メイン メッセージ ループ:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int) msg.wParam;
}

// ヘッダを見て PNG 画像かどうか(一応)チェック
BOOL isPng(LPCTSTR fileName)
{
	unsigned char pngHead[] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
	unsigned char readHead[8];
	
	FILE *fp = NULL;
	
	if (0 != _tfopen_s(&fp, fileName, _T("rb")) ||
		8 != fread(readHead, 1, 8, fp)) {
		// ファイルが読めない	
		return FALSE;
	}
	fclose(fp);
	
	// compare
	for(unsigned int i=0;i<8;i++)
		if(pngHead[i] != readHead[i]) return FALSE;

	return TRUE;

}

// ウィンドウクラスを登録
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASS wc;

	// メインウィンドウ
	wc.style         = 0;							// WM_PAINT を送らない
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GYAZOWIN));
	wc.hCursor       = LoadCursor(NULL, IDC_CROSS);	// + のカーソル
	wc.hbrBackground = 0;							// 背景も設定しない
	wc.lpszMenuName  = 0;
	wc.lpszClassName = szWindowClass;

	RegisterClass(&wc);

	// レイヤーウィンドウ
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = LayerWndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GYAZOWIN));
	wc.hCursor       = LoadCursor(NULL, IDC_CROSS);	// + のカーソル
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = szWindowClassL;

	return RegisterClass(&wc);
}


// インスタンスの初期化（全画面をウィンドウで覆う）
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
//	HWND hLayerWnd;
	hInst = hInstance; // グローバル変数にインスタンス処理を格納します。

	int x, y, w, h;

	// 仮想スクリーン全体をカバー
	x = GetSystemMetrics(SM_XVIRTUALSCREEN);
	y = GetSystemMetrics(SM_YVIRTUALSCREEN);
	w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	h = GetSystemMetrics(SM_CYVIRTUALSCREEN);

	// x, y のオフセット値を覚えておく
	ofX = x; ofY = y;

	// 完全に透過したウィンドウを作る
	hWnd = CreateWindowEx(
		WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_TOPMOST
#if(_WIN32_WINNT >= 0x0500)
		| WS_EX_NOACTIVATE
#endif
		,
		szWindowClass, NULL, WS_POPUP,
		0, 0, 0, 0,
		NULL, NULL, hInstance, NULL);

	// 作れなかった...?
	if (!hWnd) return FALSE;
	
	// 全画面を覆う
	MoveWindow(hWnd, x, y, w, h, FALSE);
	
	// nCmdShow を無視 (SW_MAXIMIZE とかされると困る)
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	// ESCキー検知タイマー
	SetTimer(hWnd, 1, 100, NULL);


	// レイヤーウィンドウの作成
	hLayerWnd = CreateWindowEx(
	 WS_EX_TOOLWINDOW
#if(_WIN32_WINNT >= 0x0500)
		| WS_EX_LAYERED | WS_EX_NOACTIVATE
#endif
		,
		szWindowClassL, NULL, WS_POPUP,
		100, 100, 300, 300,
		hWnd, NULL, hInstance, NULL);

    SetLayeredWindowAttributes(hLayerWnd, RGB(255, 0, 0), 100, LWA_COLORKEY|LWA_ALPHA);

	


	
	return TRUE;
}

// 指定されたフォーマットに対応する Encoder の CLSID を取得する
// Cited from MSDN Library: Retrieving the Class Identifier for an Encoder
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
   UINT  num = 0;          // number of image encoders
   UINT  size = 0;         // size of the image encoder array in bytes

   ImageCodecInfo* pImageCodecInfo = NULL;

   GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   GetImageEncoders(num, size, pImageCodecInfo);

   for(UINT j = 0; j < num; ++j)
   {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }    
   }

   free(pImageCodecInfo);
   return -1;  // Failure
}

// ラバーバンドを描画.
VOID drawRubberband(HDC hdc, LPRECT newRect, BOOL erase)
{
	
	static BOOL firstDraw = TRUE;	// 1 回目は前のバンドの消去を行わない
	static RECT lastRect  = {0};	// 最後に描画したバンド
	static RECT clipRect  = {0};	// 最後に描画したバンド
	
	if(firstDraw) {
		// レイヤーウィンドウを表示
		ShowWindow(hLayerWnd, SW_SHOW);
		UpdateWindow(hLayerWnd);

		firstDraw = FALSE;
	}

	if (erase) {
		// レイヤーウィンドウを隠す
		ShowWindow(hLayerWnd, SW_HIDE);
		
	}

	// 座標チェック
	clipRect = *newRect;
	if ( clipRect.right  < clipRect.left ) {
		int tmp = clipRect.left;
		clipRect.left   = clipRect.right;
		clipRect.right  = tmp;
	}
	if ( clipRect.bottom < clipRect.top  ) {
		int tmp = clipRect.top;
		clipRect.top    = clipRect.bottom;
		clipRect.bottom = tmp;
	}
	MoveWindow(hLayerWnd,  clipRect.left, clipRect.top, 
			clipRect.right-  clipRect.left + 1, clipRect.bottom - clipRect.top + 1,true);

	
	return;

/* rakusai 2009/11/2

	// XOR で描画
	int hPreRop = SetROP2(hdc, R2_XORPEN);

	// 点線
	HPEN hPen = CreatePen(PS_DOT , 1, 0);
	SelectObject(hdc, hPen);
	SelectObject(hdc, GetStockObject(NULL_BRUSH));

	if(!firstDraw) {
		// 前のを消す
		Rectangle(hdc, lastRect.left, lastRect.top, 
			lastRect.right + 1, lastRect.bottom + 1);
	} else {
		firstDraw = FALSE;
	}
	
	// 新しい座標を記憶
	lastRect = *newRect;
	
	


	if (!erase) {

		// 枠を描画
		Rectangle(hdc, lastRect.left, lastRect.top, 
			lastRect.right + 1, lastRect.bottom + 1);

	}


	// 後処理
	SetROP2(hdc, hPreRop);
	DeleteObject(hPen);

*/

}

// PNG 形式に変換
BOOL convertPNG(LPCTSTR destFile, LPCTSTR srcFile)
{
	BOOL				res = FALSE;

	GdiplusStartupInput	gdiplusStartupInput;
	ULONG_PTR			gdiplusToken;
	CLSID				clsidEncoder;

	// GDI+ の初期化
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	Image *b = new Image(srcFile, 0);

	if (0 == b->GetLastStatus()) {
		if (GetEncoderClsid(L"image/png", &clsidEncoder)) {
			// save!
			if (0 == b->Save(destFile, &clsidEncoder, 0) ) {
					// 保存できた
					res = TRUE;
			}
		}
	}

	// 後始末
	delete b;
	GdiplusShutdown(gdiplusToken);

	return res;
}

// PNG 形式で保存 (GDI+ 使用)
BOOL savePNG(LPCTSTR fileName, HBITMAP newBMP)
{
	BOOL				res = FALSE;

	GdiplusStartupInput	gdiplusStartupInput;
	ULONG_PTR			gdiplusToken;
	CLSID				clsidEncoder;

	// GDI+ の初期化
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	
	// HBITMAP から Bitmap を作成
	Bitmap *b = new Bitmap(newBMP, NULL);
	
	if (GetEncoderClsid(L"image/png", &clsidEncoder)) {
		// save!
		if (0 ==
			b->Save(fileName, &clsidEncoder, 0) ) {
				// 保存できた
				res = TRUE;
		}
	}
	
	// 後始末
	delete b;
	GdiplusShutdown(gdiplusToken);

	return res;
}

// レイヤーウィンドウプロシージャ
LRESULT CALLBACK LayerWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	RECT clipRect	= {0, 0, 500, 500};
	HBRUSH hBrush;
	HPEN hPen;
	HFONT hFont;


	switch (message)
	{
	case WM_ERASEBKGND:
		 GetClientRect(hWnd, &clipRect);
		
		hdc = GetDC(hWnd);
        hBrush = CreateSolidBrush(RGB(100,100,100));
        SelectObject(hdc, hBrush);
		hPen = CreatePen(PS_DASH,1,RGB(255,255,255));
		SelectObject(hdc, hPen);
		Rectangle(hdc,0,0,clipRect.right,clipRect.bottom);

		//矩形のサイズを出力
		int fHeight;
		fHeight = -MulDiv(8, GetDeviceCaps(hdc, LOGPIXELSY), 72);
		hFont = CreateFont(fHeight,    //フォント高さ
			0,                    //文字幅
			0,                    //テキストの角度
			0,                    //ベースラインとｘ軸との角度
			FW_REGULAR,            //フォントの重さ（太さ）
			FALSE,                //イタリック体
			FALSE,                //アンダーライン
			FALSE,                //打ち消し線
			ANSI_CHARSET,    //文字セット
			OUT_DEFAULT_PRECIS,    //出力精度
			CLIP_DEFAULT_PRECIS,//クリッピング精度
			PROOF_QUALITY,        //出力品質
			FIXED_PITCH | FF_MODERN,//ピッチとファミリー
			L"Tahoma");    //書体名

		SelectObject(hdc, hFont);
		// show size
		int iWidth, iHeight;
		iWidth  = clipRect.right  - clipRect.left;
		iHeight = clipRect.bottom - clipRect.top;

		wchar_t sWidth[200], sHeight[200];
		swprintf_s(sWidth, L"%d", iWidth);
		swprintf_s(sHeight, L"%d", iHeight);

		int w,h,h2;
		w = -fHeight * 2.5 + 8;
		h = -fHeight * 2 + 8;
		h2 = h + fHeight;

		SetBkMode(hdc,TRANSPARENT);
		SetTextColor(hdc,RGB(0,0,0));
		TextOut(hdc, clipRect.right-w+1,clipRect.bottom-h+1,(LPCWSTR)sWidth,wcslen(sWidth));
		TextOut(hdc, clipRect.right-w+1,clipRect.bottom-h2+1,(LPCWSTR)sHeight,wcslen(sHeight));
		SetTextColor(hdc,RGB(255,255,255));
		TextOut(hdc, clipRect.right-w,clipRect.bottom-h,(LPCWSTR)sWidth,wcslen(sWidth));
		TextOut(hdc, clipRect.right-w,clipRect.bottom-h2,(LPCWSTR)sHeight,wcslen(sHeight));

		DeleteObject(hPen);
		DeleteObject(hBrush);
		DeleteObject(hFont);
		ReleaseDC(hWnd, hdc);

		return TRUE;

        break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;

}

// ウィンドウプロシージャ
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	
	static BOOL onClip		= FALSE;
	static BOOL firstDraw	= TRUE;
	static RECT clipRect	= {0, 0, 0, 0};
	
	switch (message)
	{
	case WM_RBUTTONDOWN:
		// キャンセル
		DestroyWindow(hWnd);
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;

	case WM_TIMER:
		// ESCキー押下の検知
		if (GetKeyState(VK_ESCAPE) & 0x8000){
			DestroyWindow(hWnd);
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_MOUSEMOVE:
		if (onClip) {
			// 新しい座標をセット
			clipRect.right  = LOWORD(lParam) + ofX;
			clipRect.bottom = HIWORD(lParam) + ofY;
			
			hdc = GetDC(NULL);
			drawRubberband(hdc, &clipRect, FALSE);

			ReleaseDC(NULL, hdc);
		}
		break;
	

	case WM_LBUTTONDOWN:
		{
			// クリップ開始
			onClip = TRUE;
			
			// 初期位置をセット
			clipRect.left = LOWORD(lParam) + ofX;
			clipRect.top  = HIWORD(lParam) + ofY;
			


			// マウスをキャプチャ
			SetCapture(hWnd);
		}
		break;

	case WM_LBUTTONUP:
		{
			// クリップ終了
			onClip = FALSE;
			
			// マウスのキャプチャを解除
			ReleaseCapture();
		
			// 新しい座標をセット
			clipRect.right  = LOWORD(lParam) + ofX;
			clipRect.bottom = HIWORD(lParam) + ofY;

			// 画面に直接描画，って形
			HDC hdc = GetDC(NULL);

			// 線を消す
			drawRubberband(hdc, &clipRect, TRUE);

			// 座標チェック
			if ( clipRect.right  < clipRect.left ) {
				int tmp = clipRect.left;
				clipRect.left   = clipRect.right;
				clipRect.right  = tmp;
			}
			if ( clipRect.bottom < clipRect.top  ) {
				int tmp = clipRect.top;
				clipRect.top    = clipRect.bottom;
				clipRect.bottom = tmp;
			}
			
			// 画像のキャプチャ
			int iWidth, iHeight;
			iWidth  = clipRect.right  - clipRect.left + 1;
			iHeight = clipRect.bottom - clipRect.top  + 1;

			if(iWidth == 0 || iHeight == 0) {
				// 画像になってない, なにもしない
				ReleaseDC(NULL, hdc);
				DestroyWindow(hWnd);
				break;
			}

			// ビットマップバッファを作成
			HBITMAP newBMP = CreateCompatibleBitmap(hdc, iWidth, iHeight);
			HDC	    newDC  = CreateCompatibleDC(hdc);
			
			// 関連づけ
			SelectObject(newDC, newBMP);

			// 画像を取得
			BitBlt(newDC, 0, 0, iWidth, iHeight, 
				hdc, clipRect.left, clipRect.top, SRCCOPY);
			
			// ウィンドウを隠す!
			ShowWindow(hWnd, SW_HIDE);
			/*
			// 画像をクリップボードにコピー
			if ( OpenClipboard(hWnd) ) {
				// 消去
				EmptyClipboard();
				// セット
				SetClipboardData(CF_BITMAP, newBMP);
				// 閉じる
				CloseClipboard();
			}
			*/
			
			// テンポラリファイル名を決定
			TCHAR tmpDir[MAX_PATH], tmpFile[MAX_PATH];
			GetTempPath(MAX_PATH, tmpDir);
			GetTempFileName(tmpDir, _T("gya"), 0, tmpFile);
			
			if (savePNG(tmpFile, newBMP)) {

				// うｐ
				if (!uploadFile(hWnd, tmpFile)) {
					// アップロードに失敗...
					// エラーメッセージは既に表示されている

					/*
					TCHAR sysDir[MAX_PATH];
					if (SUCCEEDED(StringCchCopy(sysDir, MAX_PATH, tmpFile)) &&
						SUCCEEDED(StringCchCat(sysDir, MAX_PATH, _T(".png")))) {
						
						MoveFile(tmpFile, sysDir);
						SHELLEXECUTEINFO lsw = {0};
						
						lsw.hwnd	= hWnd;
						lsw.cbSize	= sizeof(SHELLEXECUTEINFO);
						lsw.lpVerb	= _T("open");
						lsw.lpFile	= sysDir;

						ShellExecuteEx(&lsw);
					}
					*/
				}
			} else {
				// PNG保存失敗...
				MessageBox(hWnd, _T("Cannot save png image"), szTitle, 
					MB_OK | MB_ICONERROR);
			}

			// 後始末
			DeleteFile(tmpFile);
			
			DeleteDC(newDC);
			DeleteObject(newBMP);

			ReleaseDC(NULL, hdc);
			DestroyWindow(hWnd);
			PostQuitMessage(0);
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// クリップボードに文字列をコピー
VOID setClipBoardText(const char* str)
{

	HGLOBAL hText;
	char    *pText;
	size_t  slen;

	slen  = strlen(str) + 1; // NULL

	hText = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, slen * sizeof(TCHAR));

	pText = (char *)GlobalLock(hText);
	strncpy_s(pText, slen, str, slen);
	GlobalUnlock(hText);
	
	// クリップボードを開く
	OpenClipboard(NULL);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hText);
	CloseClipboard();

	// 解放
	GlobalFree(hText);
}

// 指定された URL (char*) をブラウザで開く
VOID execUrl(const char* str)
{
	size_t  slen;
	size_t  dcount;
	slen  = strlen(str) + 1; // NULL

	TCHAR *wcUrl = (TCHAR *)malloc(slen * sizeof(TCHAR));
	
	// ワイド文字に変換
	mbstowcs_s(&dcount, wcUrl, slen, str, slen);
	
	// open コマンドを実行
	SHELLEXECUTEINFO lsw = {0};
	lsw.cbSize = sizeof(SHELLEXECUTEINFO);
	lsw.lpVerb = _T("open");
	lsw.lpFile = wcUrl;

	ShellExecuteEx(&lsw);

	free(wcUrl);
}

// ID を生成・ロードする
std::string getId()
{

    TCHAR idFile[_MAX_PATH];
	TCHAR idDir[_MAX_PATH];

    SHGetSpecialFolderPath( NULL, idFile, CSIDL_APPDATA, FALSE );

	 _tcscat_s( idFile, _T("\\Gyazo"));
	 _tcscpy_s( idDir, idFile);
	 _tcscat_s( idFile, _T("\\id.txt"));

	const TCHAR*	 idOldFile			= _T("id.txt");
	BOOL oldFileExist = FALSE;

	std::string idStr;

	// まずはファイルから ID をロード
	std::ifstream ifs;

	ifs.open(idFile);
	if (! ifs.fail()) {
		// ID を読み込む
		ifs >> idStr;
		ifs.close();
	} else{		
		std::ifstream ifsold;
		ifsold.open(idOldFile);
		if (! ifsold.fail()) {
			// 同一ディレクトリからID を読み込む(旧バージョンとの互換性)
			ifsold >> idStr;
			ifsold.close();
		}
	}

	return idStr;
}

// Save ID
BOOL saveId(const WCHAR* str)
{

    TCHAR idFile[_MAX_PATH];
	TCHAR idDir[_MAX_PATH];

    SHGetSpecialFolderPath( NULL, idFile, CSIDL_APPDATA, FALSE );

	 _tcscat_s( idFile, _T("\\Gyazo"));
	 _tcscpy_s( idDir, idFile);
	 _tcscat_s( idFile, _T("\\id.txt"));

	const TCHAR*	 idOldFile			= _T("id.txt");

	size_t  slen;
	size_t  dcount;
	slen  = _tcslen(str) + 1; // NULL

	char *idStr = (char *)malloc(slen * sizeof(char));
	// バイト文字に変換
	wcstombs_s(&dcount, idStr, slen, str, slen);

	// ID を保存する
	CreateDirectory(idDir,NULL);
	std::ofstream ofs;
	ofs.open(idFile);
	if (! ofs.fail()) {
		ofs << idStr;
		ofs.close();

		// 旧設定ファイルの削除
		if (PathFileExists(idOldFile)){
			DeleteFile(idOldFile);
		}
	}else{
		free(idStr);
		return FALSE;
	}

	free(idStr);
	return TRUE;
}

// PNG ファイルをアップロードする.
BOOL uploadFile(HWND hwnd, LPCTSTR fileName)
{
	const TCHAR* UPLOAD_SERVER	= _T("upload.gyazo.com");
	const TCHAR* UPLOAD_PATH	= _T("/upload.cgi");

	const char*  sBoundary = "----BOUNDARYBOUNDARY----";		// boundary
	const char   sCrLf[]   = { 0xd, 0xa, 0x0 };					// 改行(CR+LF)
	const TCHAR* szHeader  = 
		_T("Content-type: multipart/form-data; boundary=----BOUNDARYBOUNDARY----");

	std::ostringstream	buf;	// 送信メッセージ
	std::string			idStr;	// ID
	
	// ID を取得
	idStr = getId();

	// メッセージの構成
	// -- "id" part
	buf << "--";
	buf << sBoundary;
	buf << sCrLf;
	buf << "content-disposition: form-data; name=\"id\"";
	buf << sCrLf;
	buf << sCrLf;
	buf << idStr;
	buf << sCrLf;

	// -- "imagedata" part
	buf << "--";
	buf << sBoundary;
	buf << sCrLf;
	buf << "content-disposition: form-data; name=\"imagedata\"; filename=\"gyazo.com\"";
	buf << sCrLf;
	//buf << "Content-type: image/png";	// 一応
	//buf << sCrLf;
	buf << sCrLf;

	// 本文: PNG ファイルを読み込む
	std::ifstream png;
	png.open(fileName, std::ios::binary);
	if (png.fail()) {
		MessageBox(hwnd, _T("PNG open failed"), szTitle, MB_ICONERROR | MB_OK);
		png.close();
		return FALSE;
	}
	buf << png.rdbuf();		// read all & append to buffer
	png.close();

	// 最後
	buf << sCrLf;
	buf << "--";
	buf << sBoundary;
	buf << "--";
	buf << sCrLf;

	// メッセージ完成
	std::string oMsg(buf.str());

	// WinInet を準備 (proxy は 規定の設定を利用)
	HINTERNET hSession    = InternetOpen(szTitle, 
		INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if(NULL == hSession) {
		MessageBox(hwnd, _T("Cannot configure wininet"),
			szTitle, MB_ICONERROR | MB_OK);
		return FALSE;
	}
	
	// 接続先
	HINTERNET hConnection = InternetConnect(hSession, 
		UPLOAD_SERVER, INTERNET_DEFAULT_HTTP_PORT,
		NULL, NULL, INTERNET_SERVICE_HTTP, 0, NULL);
	if(NULL == hSession) {
		MessageBox(hwnd, _T("Cannot initiate connection"),
			szTitle, MB_ICONERROR | MB_OK);
		return FALSE;
	}

	// 要求先の設定
	HINTERNET hRequest    = HttpOpenRequest(hConnection,
		_T("POST"), UPLOAD_PATH, NULL,
		NULL, NULL, INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD, NULL);
	if(NULL == hSession) {
		MessageBox(hwnd, _T("Cannot compose post request"),
			szTitle, MB_ICONERROR | MB_OK);
		return FALSE;
	}

	// User-Agentを指定
	const TCHAR* ua = _T("User-Agent: Gyazowin/1.0\r\n");
	BOOL bResult = HttpAddRequestHeaders(
		hRequest, ua, _tcslen(ua), 
		HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
	if (FALSE == bResult) {
		MessageBox(hwnd, _T("Cannot set user agent"),
			szTitle, MB_ICONERROR | MB_OK);
		return FALSE;
	}
	
	// 要求を送信
	if (HttpSendRequest(hRequest,
                    szHeader,
					lstrlen(szHeader),
                    (LPVOID)oMsg.c_str(),
					(DWORD) oMsg.length()))
	{
		// 要求は成功
		
		DWORD resLen = 8;
		TCHAR resCode[8];

		// status code を取得
		HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE, resCode, &resLen, 0);
		if( _ttoi(resCode) != 200 ) {
			// upload 失敗 (status error)
			MessageBox(hwnd, _T("Failed to upload (unexpected result code, under maintainance?)"),
				szTitle, MB_ICONERROR | MB_OK);
		} else {
			// upload succeeded

			// get new id
			DWORD idLen = 100;
			TCHAR newid[100];
			
			memset(newid, 0, idLen*sizeof(TCHAR));	
			_tcscpy_s(newid, _T("X-Gyazo-Id"));

			HttpQueryInfo(hRequest, HTTP_QUERY_CUSTOM, newid, &idLen, 0);
			if (GetLastError() != ERROR_HTTP_HEADER_NOT_FOUND && idLen != 0) {
				//save new id
				saveId(newid);
			}

			// 結果 (URL) を読取る
			DWORD len;
			char  resbuf[1024];
			std::string result;
			
			// そんなに長いことはないけどまあ一応
			while(InternetReadFile(hRequest, (LPVOID) resbuf, 1024, &len) 
				&& len != 0)
			{
				result.append(resbuf, len);
			}

			// 取得結果は NULL terminate されていないので
			result += '\0';

			// クリップボードに URL をコピー
			setClipBoardText(result.c_str());
			
			// URL を起動
			execUrl(result.c_str()); 

			return TRUE;
		}
	} else {
		// アップロード失敗...
		MessageBox(hwnd, _T("Failed to upload"), szTitle, MB_ICONERROR | MB_OK);
	}

	return FALSE;

}