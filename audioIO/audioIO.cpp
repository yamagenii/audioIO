// audioIO.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include <windows.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>

#pragma comment (lib, "winmm.lib")

using namespace std;

HWAVEIN      hwi = NULL;                   // WAVEハンドル
LPBYTE       lpWaveDataFront = NULL;       // バッファ本体のポインタ[ヒープ領域]
LPBYTE       lpWaveDataBack = NULL;        // バッファ本体のポインタ[ヒープ領域]
static vector<BYTE> lpWaveDataSave;        // 可変バッファ[ヒープ領域]
static vector<__int16> lpWaveDataSaveInt;        // 可変バッファ[ヒープ領域]
WAVEFORMATEX wf = { 0 };                   // デバイスオープン時に使う、設定構造体
WAVEHDR      whf = { 0 };                  // デバイスに与えるバッファの設定構造体
WAVEHDR      whb = { 0 };                  // デバイスに与えるバッファの設定構造体
static boolean dataResetFlg = false;       //バッファリセットフラグ
static LPBYTE lpWaveDataTmp;               //リセットバッファの指定ポインタ
boolean clap = false;                      //拍点用
ofstream ofs("test.csv");				   //ファイル出力ストリーム


										   // waveInOpenが利用するコールバック関数
void CALLBACK waveInProc(
	HWAVEIN hwi,
	UINT uMsg,
	DWORD dwInstance,
	DWORD dwParam1,
	DWORD dwParam2
	)
{
	switch (uMsg)
	{
	case WIM_OPEN://waveInOpen時
		cout << "waveInOpen" << endl;
		break;

	case WIM_CLOSE://waveInClose時
		cout << "waveInClose" << endl;
		break;

	case WIM_DATA://バッファがいっぱいになると呼ばれる、ここでwave系関数を呼ぶとデッドロック
		int bsize = ((LPWAVEHDR)dwParam1)->dwBytesRecorded;//バッファサイズ
		for (int dwCount = 0; dwCount < bsize; dwCount++) {
			//セーブ用の可変バッファに挿入
			lpWaveDataSave.push_back(((LPWAVEHDR)dwParam1)->lpData[dwCount]);
			lpWaveDataSaveInt.push_back(((LPWAVEHDR)dwParam1)->lpData[dwCount]);
			//ofs << num << endl;
			//拍点検出アルゴリズム
			if (180 < (short)(((LPWAVEHDR)dwParam1)->lpData[dwCount])) {
				clap = true;
			}
		}
		if (clap) cout << "clap!" << endl;
		clap = false;

		//セーブ用バッファの登録とフラグを立てる スレッドなので順番に注意（mainの処理からバッファが先）
		lpWaveDataTmp = (LPBYTE)(((LPWAVEHDR)dwParam1)->lpData);
		dataResetFlg = true;
		//cout << "バッファがいっぱいになりました" << endl;
		break;
	}
}

// waveOutOpenが利用するコールバック関数
void CALLBACK waveOutProc(
	HWAVEOUT hwo,
	UINT uMsg,
	DWORD dwInstance,
	DWORD dwParam1,
	DWORD dwParam2
	)
{
	switch (uMsg)
	{
	case WOM_OPEN:
		cout << "waveOutOpen" << endl;
		break;
	case WOM_CLOSE:
		cout << "waveOutClose" << endl;
		break;
	case WOM_DONE:
		cout << "バッファをすべて再生しました" << endl;
		break;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	/*******************************************************************************
	録音
	********************************************************************************/


	// デバイスオープン時の設定
	wf.wFormatTag = WAVE_FORMAT_PCM;   // PCM形式
	wf.nChannels = 1;                 // ステレオかモノラルか
									  //wf.nSamplesPerSec = 22050;             // サンプリングレート 22.05KHz
	wf.nSamplesPerSec = 44100;             // サンプリングレート 22.05KHz
	wf.wBitsPerSample = 16;                 // 量子化レベル
	wf.nBlockAlign = wf.wBitsPerSample * wf.nChannels / 8;  // バイトあたりのビット数[PCMの仕様]
	wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;    // 1 秒あたりのバイト数
																// モノラル+サンプル22050+量子化8bitなので 一秒間に22050byteのデータが発生する
																// サンプル x nBlockAlign = 一秒間のデータ量なので nBlockAlignは１
																//UINT test = waveInGetNumDevs();
																// デバイスオープン
	if (waveInOpen(
		&hwi,               // ハンドル
							//WAVE_MAPPER,        // デバイスID。 WAVE_MAPPERデフォルトのデバイスを指定 付属のマイクプラグ
							//WAVE_MAPPER,                  //よくわからないけどCH2はデバイスIDが4
		1,				//waveInGetNumDevs(),//デバイスの数を取得する関数
		&wf,                // オープン時の設定
		(DWORD)waveInProc,  // コールバック関数
		0,                  // しらん
		CALLBACK_FUNCTION   // コールバック関数の属性
		) != MMSYSERR_NOERROR)
		cout << "デバイスのオープンに失敗しました" << endl;

	float dwRecordSecond = 1;      // バッファ秒数 0.01まで確認
	DWORD dwDataSize;

	dwDataSize = (DWORD)((float)(wf.nAvgBytesPerSec) * (dwRecordSecond));   // 一秒間のデータ量 x 秒数
	lpWaveDataFront = (LPBYTE)new BYTE[dwDataSize];
	lpWaveDataBack = (LPBYTE)new BYTE[dwDataSize];
	//lpWaveDataSave = (LPBYTE)new BYTE[dwDataSize*2];

	// バッファ情報セット
	whf.lpData = (LPSTR)lpWaveDataFront;
	whf.dwBufferLength = dwDataSize;
	whf.dwFlags = 0;

	// バッファ情報セット
	whb.lpData = (LPSTR)lpWaveDataBack;
	whb.dwBufferLength = dwDataSize;
	whb.dwFlags = 0;

	// バッファの初期化
	waveInPrepareHeader(hwi, &whf, sizeof(WAVEHDR));
	waveInPrepareHeader(hwi, &whb, sizeof(WAVEHDR));

	// バッファの関連付け
	waveInAddBuffer(hwi, &whf, sizeof(WAVEHDR));
	waveInAddBuffer(hwi, &whb, sizeof(WAVEHDR));

	// 録音開始
	waveInStart(hwi);

	//無限ループ中にバッファ交換フラグの確認waveInAddBufferでバッファ内上書き
	while (1) {
		if (dataResetFlg) {
			if (lpWaveDataTmp == lpWaveDataFront) {
				;
				waveInAddBuffer(hwi, &whf, sizeof(WAVEHDR));
			}
			else if (lpWaveDataTmp == lpWaveDataBack) {
				waveInAddBuffer(hwi, &whb, sizeof(WAVEHDR));
			}
			else {
				cout << "バッファエラー" << endl;
			}
			dataResetFlg = false;
		}

		if (GetAsyncKeyState(VK_UP)) {
			break;
		}

	}
	rewind(stdin);
	getchar(); // 5秒待機すると、 "バッファがいっぱいになった" が表示されるはず

			   // 録音終了
	waveInReset(hwi);
	waveInUnprepareHeader(hwi, &whf, sizeof(WAVEHDR));
	waveInUnprepareHeader(hwi, &whb, sizeof(WAVEHDR));
	waveInClose(hwi);

	/*for (int i = 0; i < dwDataSize;i++) {
	lpWaveDataSave.push_back(whf.lpData[i]);
	}
	for (int i = 0; i < dwDataSize; i++) {
	lpWaveDataSave.push_back(whb.lpData[i]);
	}*/
	/*******************************************************************************
	再生
	********************************************************************************/

	HWAVEOUT hwo;   // ハンドル
	WAVEHDR whdr;       // 

						// デバイスを開く
	if (waveOutOpen(
		&hwo,           // ハンドル
		WAVE_MAPPER,        // デフォルトのデバイスを指定
		&wf,                // waveInOpenのとおなじでおｋ
		(DWORD)waveOutProc, // コールバック関数
		0,                  // しらん
		CALLBACK_FUNCTION   // コールバック関数の属性
		)
		!= MMSYSERR_NOERROR)
		cout << "Error" << endl;


	// 音声ファイルのパラメータを設定
	whdr.lpData = (LPSTR)(lpWaveDataSave.data());          // 音声データの入ってるポインタ
	whdr.dwBufferLength = lpWaveDataSave.size();                     // 音声データのサイズ
	whdr.dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;  // 再生オプション
	whdr.dwLoops = 1;                              // ループ回数

												   // 初期化
	waveOutPrepareHeader(hwo, &whdr, sizeof(WAVEHDR));

	// 音声データブロックの書き込み
	waveOutWrite(hwo, &whdr, whdr.dwBufferLength);

	rewind(stdin);
	getchar();  // 再生中待機

	waveOutReset(hwo);
	waveOutUnprepareHeader(hwo, &whdr, sizeof(whdr));
	waveOutClose(hwo);
	return 0;
}

