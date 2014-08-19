#include "GenerateJSON.hpp"
#include "../common/unicode.hpp"

namespace
{
	int ADFUNC_DXconvAnsiToWide(int destsize, WCHAR* wstrDest, const CHAR* strSrc )
	{
		// 引数の*がNULLの場合はエラー
		if( strSrc==NULL )
			return 0;
		int ct = strlen(strSrc) + 1;
		if( wstrDest == NULL )
// ※ ここから 日本語OS以外でもshift-jisを扱えるようにコードページ指定に変更(他国語OS対応)
//			return MultiByteToWideChar( CP_ACP, 0, strSrc, ct, NULL, 0 );
			return MultiByteToWideChar( 932, 0, strSrc, ct, NULL, 0 );
//		int nResult = MultiByteToWideChar( CP_ACP, 0, strSrc, ct, wstrDest, destsize );
		int nResult = MultiByteToWideChar( 932, 0, strSrc, ct, wstrDest, destsize );
// ※ ここまで
		wstrDest[destsize-1] = 0;

		if( nResult == 0 )
			return 0;
		return nResult;
	}

	TCHAR* _ftot_s( TCHAR* pString, size_t Size, float Value, int FracDigits )
	{
		if( pString == NULL )
			goto _error;
		TCHAR* pSource = pString;
		if( *(long*)&Value < 0 )
		{
			if( (size_t)( pString - pSource ) >= Size )
				goto _error;
			*pString++ = '-';
			*(long*)&Value ^= 0x80000000;
		}
		TCHAR* pBegin  = pString;
		int   Integer = (int)Value;
		for( int i = 10; i <= Integer; i *= 10, pString++ );
		if( (size_t)( pString - pSource ) >= Size )
			goto _error;
		TCHAR* pCode = pString++;
		*pCode-- = '0' + (char)( Integer % 10 );
		for( int i = Integer / 10; i != 0; i /= 10 )
			*pCode-- = '0' + (char)( i % 10 );
		if( FracDigits > 0 )
		{
			if( (size_t)( pString - pSource ) >= Size )
				goto _error;
			*pString++ = '.';
			while( FracDigits-- > 1 )
			{
				Value -= Integer;
				Integer = (int)( Value *= 10.0f );
				if( (size_t)( pString - pSource ) >= Size )
					goto _error;
				*pString++ = '0' + (char)Integer;
			}
			Integer = (int)( ( Value - Integer ) * 10.0f + 0.5f );
			if( Integer < 10 )
			{
				if( (size_t)( pString - pSource ) >= Size )
					goto _error;
				*pString++ = '0' + (char)Integer;
			}
			else
			{
				pCode = pString - 1;
				if( (size_t)( pString - pSource ) >= Size )
					goto _error;
				*pString++ = '0';
				do
				{
					if( *pCode == '.' )
						pCode--;
					if( *pCode != '9' )
					{
						*pCode += 1;
						break;
					}
					else
						*pCode = '0';
					if( pCode == pBegin )
					{
						if( (size_t)( pString - pSource ) >= Size )
							goto _error;
						TCHAR* pEnd = pString++;
						while( pEnd-- != pCode )
							*( pEnd + 1 ) = *pEnd;
						*pCode = '1';
					}
				} while( *pCode-- == '0' );
			}
		}
		if( (size_t)( pString - pSource ) >= Size )
			goto _error;
		*pString = '\0';
		return pSource;
_error:
		__debugbreak();
		return NULL;
	}

	BOOL DeleteDirectory( LPCTSTR lpPathName )
	{
		// 入力値チェック
		if( NULL == lpPathName )
		{
			return FALSE;
		}

		// ディレクトリ名の保存（終端に'\'がないなら付ける）
		TCHAR szDirectoryPathName[_MAX_PATH];
		_tcsncpy_s( szDirectoryPathName, _MAX_PATH, lpPathName, _TRUNCATE );
		if( '/' != szDirectoryPathName[_tcslen(szDirectoryPathName) - 1] )
		{	// 一番最後に'\'がないなら付加する。
			_tcsncat_s( szDirectoryPathName, _MAX_PATH, _T("/"), _TRUNCATE );
		}

		// ディレクトリ内のファイル走査用のファイル名作成
		TCHAR szFindFilePathName[_MAX_PATH];
		_tcsncpy_s( szFindFilePathName, _MAX_PATH, szDirectoryPathName, _TRUNCATE );
		_tcsncat_s( szFindFilePathName, _MAX_PATH, _T("*"), _TRUNCATE );

		// ディレクトリ内のファイル走査開始
		WIN32_FIND_DATA		fd;
		HANDLE hFind = FindFirstFile( szFindFilePathName, &fd );
		if( INVALID_HANDLE_VALUE == hFind )
		{	// 走査対象フォルダが存在しない。
			return FALSE;
		}

		do
		{
			//if( '.' != fd.cFileName[0] )
			if( 0 != _tcscmp( fd.cFileName, _T(".") )		// カレントフォルダ「.」と
				&& 0 != _tcscmp( fd.cFileName, _T("..") ) )	// 親フォルダ「..」は、処理をスキップ
			{
				TCHAR szFoundFilePathName[_MAX_PATH];
				_tcsncpy_s( szFoundFilePathName, _MAX_PATH, szDirectoryPathName, _TRUNCATE );
				_tcsncat_s( szFoundFilePathName, _MAX_PATH, fd.cFileName, _TRUNCATE );

				if( FILE_ATTRIBUTE_DIRECTORY & fd.dwFileAttributes )
				{	// ディレクトリなら再起呼び出しで削除
					if( !DeleteDirectory( szFoundFilePathName ) )
					{
						FindClose( hFind );
						return FALSE;
					}
				}
				else
				{	// ファイルならWin32API関数を用いて削除
					if( !DeleteFile( szFoundFilePathName ) )
					{
						FindClose( hFind );
						return FALSE;
					}
				}
			}
		} while( FindNextFile( hFind, &fd ) );

		FindClose( hFind );

		return RemoveDirectory( lpPathName );
	}

	int Trim(char *s) {
    int i;
    int count = 0;

    /* 空ポインタか? */
    if ( s == NULL ) { /* yes */
        return -1;
    }

    /* 文字列長を取得する */
    i = strlen(s);

    /* 末尾から順に空白でない位置を探す */
    while ( --i >= 0 && s[i] == ' ' ) count++;

    /* 終端ナル文字を付加する */
    s[i+1] = '\0';

    /* 先頭から順に空白でない位置を探す */
    i = 0;
    while ( s[i] != '\0' && s[i] == ' ' ) i++;
    strcpy(s, &s[i]);

    return i + count;
	}
};

#define MAX_PATH_L (MAX_PATH * 2)

JsonGen::JsonGen()
{
	HANDLE hFind,hPmdFind,hTxtFind;							// Directory Pmd Text
	WIN32_FIND_DATA win32fd_dir,win32fd_pmd,win32fd_txt;	// Directory Pmd Text
	TCHAR tcsTmpPath[MAX_PATH_L] = {0};
	TCHAR tcsTmpPath_Pmd[MAX_PATH_L] = {0};
	TCHAR tcsTmpDir[MAX_PATH_L] = {0};

	std::vector<std::wstring> pmd_paths;
	std::wstring prejson;
	DxLib::VECTOR prePos = {0},curPos = {0},lclPos = {0};
	DxLib::MATRIX chglcl = {0};

	char pmd_model_name_[MAX_PATH_L] = {0};
	TCHAR pmd_author_name_[MAX_PATH_L] = {0};

	std::wstring pmx_model_name_;  // ※ pmx対応の為追加
	std::wstring pmx_author_name_; // ※ pmx対応の為追加 


	int exist_num_pmd_ = 0;

	int model_handle_ = 0;

	FILE *json_file = 0;

	// system/models/内検索　列挙
	hFind = FindFirstFile(_T("./models/?*"), &win32fd_dir);

	if (hFind == INVALID_HANDLE_VALUE) {
		return;
	}

	do {
		if (win32fd_dir.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
			_tcscmp(win32fd_dir.cFileName,_T("."))!=0 &&
			_tcscmp(win32fd_dir.cFileName,_T(".."))!=0 &&
			_tcscmp(win32fd_dir.cFileName,_T(".svn"))!=0) {
			_tcscpy_s(tcsTmpDir,_T("./models/"));
			_tcscat_s(tcsTmpDir,win32fd_dir.cFileName);
			_tcscat_s(tcsTmpDir,_T("/"));

			ZeroMemory(tcsTmpPath,MAX_PATH_L);
			_tcscpy_s(tcsTmpPath,tcsTmpDir);
			_tcscat_s(tcsTmpPath,_T("info.json"));

			// info.jsonが存在しない場合、pmdに従って作成
			if(!PathFileExists(tcsTmpPath))
			{
				ZeroMemory(tcsTmpPath_Pmd,MAX_PATH_L);
				_tcscpy_s(tcsTmpPath_Pmd,tcsTmpDir);
//				_tcscat_s(tcsTmpPath_Pmd,_T("*.pmd"));
				_tcscat_s(tcsTmpPath_Pmd,_T("*.pm?")); // pmxも対応するため変更
				hPmdFind = FindFirstFile(tcsTmpPath_Pmd, &win32fd_pmd);
//				if(hPmdFind == (HANDLE)0xffffffff)
				if(hPmdFind == INVALID_HANDLE_VALUE) // ※ 64bitでビルドしても問題ないように修正
				{
					FindClose(hPmdFind);
					continue;
				}

				TCHAR cur_dir[MAX_PATH_L];
				ZeroMemory(cur_dir,MAX_PATH_L);
				GetCurrentDirectory(MAX_PATH_L,cur_dir);
				bool flag = false;
				/*for(int k = MAX_PATH_L - 1;k != 0;--k)
				{
					if( cur_dir[k] == _T('/') )
					{
						cur_dir[k] = 0;
						break;
					}
				}*/
				do {
					std::wstring tmp_path = cur_dir;
					tmp_path += _T("/models/");
					tmp_path += win32fd_dir.cFileName;
					tmp_path += _T("/");
					tmp_path += win32fd_pmd.cFileName;
					pmd_paths.push_back(tmp_path);
				} while (FindNextFile(hPmdFind, &win32fd_pmd));
				FindClose(hPmdFind);

				for(unsigned int i = 0;i < pmd_paths.size();++i){
// ※ ここから  pmx対応のため追加
					if (pmd_paths[i].substr(pmd_paths[i].size()-4,4) == L".pmx") {
						// PMX
						int fd = _topen(pmd_paths[i].c_str(),O_RDONLY);
						char readbuf[1024];
						read(fd,readbuf,9);
						// ヘッダー確認
						if( readbuf[ 0 ] != 'P' || readbuf[ 1 ] != 'M' || readbuf[ 2 ] != 'X' || readbuf[ 3 ] != ' ' ) {
							close(fd);
							continue;
						}
						// Ver2.0 かを確認
						if( readbuf[ 4 ] != 0x00 || readbuf[ 5 ] != 0x00 || readbuf[ 6 ] != 0x00 || readbuf[ 7 ] != 0x40 ) {
							close(fd);
							continue;
						}

						// 追加情報のサイズを取得し読み込み
						auto AddHeadDataSize = readbuf[ 8 ] ;
						read(fd,readbuf,8);
						if (AddHeadDataSize > 8) {
							// 追加情報が8バイトより大きい場合は残りをスキップ
							_lseek(fd,AddHeadDataSize - 8,SEEK_CUR);
						}
						auto EncodeType =  readbuf[0]; // エンコード方式 0:UTF16 , 1:UTF8

						// モデル情報読み込み
						DWORD size;
						read(fd,(char *) &size,4);  // テキストデータ サイズ取得
						if (size <= 1024) {
							read(fd,readbuf,size); // 文字列情報を読み込み
						} else {
							// 1024バイトより大きい場合は1024バイト読み込んで残りはスキップ
							read(fd,readbuf,1024); // 文字列情報を読み込み
							_lseek(fd,size - 1024,SEEK_CUR);
							size=1024;
						}
						// モデル名取得
						pmx_model_name_ = L"";
						switch (EncodeType) {
						case 0: // UTF16
							for (int idx=0;idx<size;idx+=2) {
								pmx_model_name_ +=  (unsigned char)readbuf[idx] + ((unsigned char)readbuf[idx+1]<<8);
							}
							break;
						case 1: // UTF8
							if (size < 1024) {
								readbuf[size] = 0x00;
							} else {
								readbuf[1023] = 0x00;
							}
							pmx_model_name_ = unicode::ToWString(readbuf);
							break;
						}
						// モデル名(英)をスキップ
						read(fd,(char *) &size,4);  // テキストデータ サイズ取得
						_lseek(fd,size,SEEK_CUR);

						// コメント取得
						read(fd,(char *) &size,4);  // テキストデータ サイズ取得
						if (size <= 1024) {
							read(fd,readbuf,size); // 文字列情報を読み込み
						} else {
							// 1024バイトより大きい場合は1024バイト読み込んで残りはスキップ
							read(fd,readbuf,1024); // 文字列情報を読み込み
							_lseek(fd,size - 1024,SEEK_CUR);
							size=1024;
						}
						close(fd);

						std::wstring pmx_model_comment = L"";
						switch (EncodeType) {
						case 0: // UTF16
							for (int idx=0;idx<size;idx+=2) {
								pmx_model_comment +=  (unsigned char)readbuf[idx] + ((unsigned char)readbuf[idx+1]<<8);
							}
							break;
						case 1: // UTF8
							if (size < 1024) {
								readbuf[size] = 0x00;
							} else {
								readbuf[1023] = 0x00;
							}
							pmx_model_comment = unicode::ToWString(readbuf);
							break;
						}

						// コメントからモデラー名を取得
						pmx_author_name_ = L"Unknown";
						unsigned int cnt = 0;
						while(cnt < pmx_model_comment.size()){
							if(pmx_model_comment.substr(cnt,5) == L"モデリング") break;
							else	++cnt;
						}
						if(cnt != pmx_model_comment.size()) {
							for(cnt;cnt<pmx_model_comment.size() && pmx_model_comment.substr(cnt,1) != L"：" && pmx_model_comment.substr(cnt,1) != L":"; ++cnt);
							for(cnt;cnt<pmx_model_comment.size() && (pmx_model_comment.substr(cnt+1,1) == L" "||pmx_model_comment.substr(cnt+1,1) == L"　"); ++cnt);
							int tmp_str_cnt = 1;
							for(tmp_str_cnt;cnt+tmp_str_cnt<pmx_model_comment.size() && (pmx_model_comment.substr(cnt+tmp_str_cnt,1) !=L"\n" && pmx_model_comment.substr(cnt+tmp_str_cnt,1) !=L"\r" && pmx_model_comment.substr(cnt+tmp_str_cnt,1) != L" " && pmx_model_comment.substr(cnt+tmp_str_cnt,1) != L".");++tmp_str_cnt);
							// 作者取得
							pmx_author_name_ = pmx_model_comment.substr(cnt+1,tmp_str_cnt-1);
						}

					} else if (pmd_paths[i].substr(pmd_paths[i].size()-4,4) == L".pmd") {
// ※ ここまで
						// PMD
						char pmd_info[PMDINFO_SIZE+1];
						int fd = _topen(pmd_paths[i].c_str(),O_RDONLY);
						read(fd,pmd_info,PMDINFO_SIZE);
						close(fd);

						// モデル名取得
						strcpy_s(pmd_model_name_,pmd_info+7);
						Trim(pmd_model_name_);
						unsigned int cnt = 0x1b;
						size_t info_size = ADFUNC_DXconvAnsiToWide(0,0,pmd_info+cnt);
						TCHAR *pmd_info_t = new TCHAR[info_size + 1];
						ADFUNC_DXconvAnsiToWide(info_size,pmd_info_t,pmd_info+cnt);
						cnt = 0;
						while(cnt < info_size){
							if(!_tcsncmp(_T("モデリング"),pmd_info_t+cnt,5))break;
							else	++cnt;
						}
						if(cnt == info_size)
						{
							_tcscpy_s(pmd_author_name_,_T("Unknown"));
						}else{
//							for(cnt;cnt<info_size && *(pmd_info_t+cnt) != _T('：'); ++cnt);
							for(cnt;cnt<info_size && *(pmd_info_t+cnt) != _T('：') && *(pmd_info_t+cnt) != _T(':'); ++cnt); // ※ 半角':'も調べるように追加
							for(cnt;cnt<info_size && (*(pmd_info_t+cnt+1) == _T(' ')||*(pmd_info_t+cnt+1) == _T('　')); ++cnt);  // ※ スペースを飛ばすように追加
							int tmp_str_cnt = 1;
							for(tmp_str_cnt;cnt+tmp_str_cnt<info_size && (*(pmd_info_t+cnt+tmp_str_cnt) != _T('\n') && *(pmd_info_t+cnt+tmp_str_cnt) != _T(' ') && *(pmd_info_t+cnt+tmp_str_cnt) != _T('.'));++tmp_str_cnt);
							// 作者取得
							_tcsncpy_s(pmd_author_name_,pmd_info_t+cnt+1,tmp_str_cnt-1);
						}
						delete []pmd_info_t;
// ※ ここから pmx対応のため追加
					} else {
						continue; // pmx,pmd 以外の場合はループを抜ける
					}
// ※ ここまで

						TCHAR tmp_mv1_path[MAX_PATH_L] = {0};
						_tcscpy_s(tmp_mv1_path,pmd_paths[i].c_str());

						model_handle_ = MV1LoadModel( tmp_mv1_path );
						int tex_num = MV1GetTextureNum( model_handle_ );
						int frame_num = MV1GetFrameNum( model_handle_ );


						// ボーンに基づいたモデルの高さ設定
						for(int j = 0;j < frame_num;++j)
						{
							curPos = MV1GetFramePosition( model_handle_, j );
							chglcl = MV1GetFrameLocalMatrix( model_handle_, j );
							lclPos.x = chglcl.m[3][0];
							lclPos.y = chglcl.m[3][1];
							lclPos.z = chglcl.m[3][2];

							if(	prePos.y < curPos.y &&
								lclPos.x == curPos.x &&
								lclPos.y == curPos.y &&
								lclPos.z == curPos.z)prePos = curPos;
						}
					// JSONの構築
					prejson = _T("{\n\t\"name\": \"");
					std::wstring premodelname = _T("char:");

// ※ ここから pmx対応のため追加
					if (pmd_paths[i].substr(pmd_paths[i].size()-4,4) == L".pmx") {
						premodelname += pmx_model_name_;
						premodelname += _T(":");
						premodelname += pmx_author_name_;
					} else if (pmd_paths[i].substr(pmd_paths[i].size()-4,4) == L".pmd") { 
// ※ ここまで
						size_t tmp_w_s_m = ADFUNC_DXconvAnsiToWide(0,0,pmd_model_name_);
						TCHAR *tmp_w_m = new TCHAR[tmp_w_s_m + 1];
						ADFUNC_DXconvAnsiToWide(tmp_w_s_m,tmp_w_m,pmd_model_name_);
						premodelname += tmp_w_m;
						premodelname += _T(":");
						premodelname += pmd_author_name_;
// ※ ここから pmx対応のため追加
						pmx_model_name_ = tmp_w_m; // 後で使われていてスコープ外になったのでここに入れておくw
						delete [] tmp_w_m;         // 元のオブジェクトは開放
					}
// ※ ここまで
					premodelname += _T("式");

// ※ここから モデル名が64バイトを超える場合は先頭から64バイト以内のみにする
					auto premodelname_ = unicode::ToString(premodelname);
                    if (premodelname_.size() > 64) {
                        for (int i=20;i<=premodelname.size();i++){
                            premodelname_ = unicode::ToString(premodelname.substr(0,i));
                            if (premodelname_.size() > 64){
                                premodelname = premodelname.substr(0,i-1);
                                break;
                            }
                        }
                    }
					prejson += premodelname;
					prejson += _T("\",\n\t\"character\":\n\t\t{\n\t\t\t\"height\":");
// ※ ここまで
					TCHAR tmp_f[32];
					_ftot_s(tmp_f,32,floor(prePos.y*2)/10.0f,2);
					prejson += tmp_f;
					prejson += _T(",\n\t\t\t\"motions\":\n\t\t\t\t{\n\t\t\t\t\t\"stand\":\"basic_stand.vmd\",\n\t\t\t\t\t\"walk\": \t\"basic_walk.vmd\",\n\t\t\t\t\t\"run\":\t\"basic_run.vmd\"\n\t\t\t\t}\n\t\t}\n}");
					TCHAR tmp_dir[MAX_PATH_L];
					_tcscpy_s(tmp_dir,_T("./models/"));


// ※ ここから pmx対応のため追加
					if (pmd_paths[i].substr(pmd_paths[i].size()-4,4) == L".pmx") {
						_tcscat_s(tmp_dir,pmx_author_name_.c_str());
						_tcscat_s(tmp_dir,_T("式"));
						_tcscat_s(tmp_dir,_T("/"));
						if(!PathIsDirectory(tmp_dir)){
							_wmkdir(tmp_dir);
						}
						_tcscpy_s(tmp_dir,_T("./models/"));
						_tcscat_s(tmp_dir,pmx_author_name_.c_str());
						_tcscat_s(tmp_dir,_T("式"));
						_tcscat_s(tmp_dir,_T("/"));
						_tcscat_s(tmp_dir,pmx_model_name_.c_str());
					} else if (pmd_paths[i].substr(pmd_paths[i].size()-4,4) == L".pmd") { 
// ※ ここまで
						_tcscat_s(tmp_dir,pmd_author_name_);
						_tcscat_s(tmp_dir,_T("式"));
						_tcscat_s(tmp_dir,_T("/"));
						if(!PathIsDirectory(tmp_dir)){
							_wmkdir(tmp_dir);
						}
						_tcscpy_s(tmp_dir,_T("./models/"));
						_tcscat_s(tmp_dir,pmd_author_name_);
						_tcscat_s(tmp_dir,_T("式"));
						_tcscat_s(tmp_dir,_T("/"));
// ※ ここから pmx対応のため追加
//						_tcscat_s(tmp_dir,tmp_w_m); // スコープ外になったので pmx_model_name_ に代入してそちらを利用
						_tcscat_s(tmp_dir,pmx_model_name_.c_str());
					}
					_tcscat_s(tmp_dir,_T("/"));
					_wmkdir(tmp_dir);
//					delete [] tmp_w_m;  // スコープ外になったのでスコープ内で開放してここはコメント化
					//delete [] tmp_w_a;
// ※ ここまで
					TCHAR json_path[MAX_PATH_L];
					_tcscpy_s(json_path,tmp_dir);
					_tcscat_s(json_path,_T("info.json"));
					_tfopen_s(&json_file,json_path, _T("w, ccs=UTF-8"));
					// ※ ここから ファイルオープンに失敗していてランタイムエラーが出ることがあるので修正
//					fseek(json_file, 0, SEEK_SET);
//					_ftprintf_s(json_file,prejson.c_str());
//					fclose(json_file);
					if (json_file != 0) {
						fseek(json_file, 0, SEEK_SET);
						_ftprintf_s(json_file,prejson.c_str());
						fclose(json_file);
					}

					TCHAR tmp_src[MAX_PATH_L];
					TCHAR tmp_cpy[MAX_PATH_L];

					// テクスチャのコピー
					for(int t = 0;t < tex_num;++t)
					{
						_tcscpy_s(tmp_src,tcsTmpDir);
						_tcscat_s(tmp_src,MV1GetTextureName( model_handle_, t));
						_tcscpy_s(tmp_cpy,tmp_dir);
						_tcscat_s(tmp_cpy,MV1GetTextureName( model_handle_, t));
// ※ ここから テクスチャーコピー先のディレクトリが無い場合は作成する
						for (unsigned int idx = 0;idx<sizeof(tmp_cpy) && *(tmp_cpy+idx) != _T('\0');idx++) {
							if (*(tmp_cpy+idx) == _T('\\')||*(tmp_cpy+idx) == _T('/')) {
								TCHAR dirchk[MAX_PATH_L];
								_tcsncpy_s(dirchk,tmp_cpy,idx+1);
								if(!PathIsDirectory(dirchk)){
									_wmkdir(dirchk);
								}
							}
						}
// ※ ここまで
						CopyFile(tmp_src,tmp_cpy,TRUE);
					}
					TCHAR tmp_txt_f[MAX_PATH_L];
					_tcscpy_s(tmp_txt_f,tcsTmpDir);
					_tcscat_s(tmp_txt_f,_T("*.txt"));
					hTxtFind = FindFirstFile(tmp_txt_f,&win32fd_txt);
					do{
						_tcscpy_s(tmp_src,tcsTmpDir);
						_tcscat_s(tmp_src,win32fd_txt.cFileName);
						_tcscpy_s(tmp_cpy,tmp_dir);
						_tcscat_s(tmp_cpy,win32fd_txt.cFileName);
						CopyFile(tmp_src,tmp_cpy,TRUE);
					}while(FindNextFile(hTxtFind, &win32fd_txt));
					FindClose(hTxtFind);
					TCHAR tmp_type[128] = {0};
					_tsplitpath(pmd_paths[i].c_str(),NULL,NULL,tmp_cpy,tmp_type);
					_tcscat_s(tmp_cpy,tmp_type);
					_tcscpy_s(tmp_type,tmp_dir);
					_tcscat_s(tmp_type,tmp_cpy);
					CopyFile(pmd_paths[i].c_str(),tmp_type,TRUE);
				}
				pmd_paths.clear();
				if (_tcscmp(tcsTmpDir,_T("./models/キャラクター/")) !=0){ // ※ キャラクター直下の場合は削除しない
					DeleteDirectory(tcsTmpDir);
				}
			}
		}
	} while (FindNextFile(hFind, &win32fd_dir));

	FindClose(hFind);
}