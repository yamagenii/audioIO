// audioIO.cpp : �R���\�[�� �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//

#include "stdafx.h"
#include <windows.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>

#pragma comment (lib, "winmm.lib")

using namespace std;

HWAVEIN      hwi = NULL;                   // WAVE�n���h��
LPBYTE       lpWaveDataFront = NULL;       // �o�b�t�@�{�̂̃|�C���^[�q�[�v�̈�]
LPBYTE       lpWaveDataBack = NULL;        // �o�b�t�@�{�̂̃|�C���^[�q�[�v�̈�]
static vector<BYTE> lpWaveDataSave;        // �σo�b�t�@[�q�[�v�̈�]
static vector<__int16> lpWaveDataSaveInt;        // �σo�b�t�@[�q�[�v�̈�]
WAVEFORMATEX wf = { 0 };                   // �f�o�C�X�I�[�v�����Ɏg���A�ݒ�\����
WAVEHDR      whf = { 0 };                  // �f�o�C�X�ɗ^����o�b�t�@�̐ݒ�\����
WAVEHDR      whb = { 0 };                  // �f�o�C�X�ɗ^����o�b�t�@�̐ݒ�\����
static boolean dataResetFlg = false;       //�o�b�t�@���Z�b�g�t���O
static LPBYTE lpWaveDataTmp;               //���Z�b�g�o�b�t�@�̎w��|�C���^
boolean clap = false;                      //���_�p
ofstream ofs("test.csv");				   //�t�@�C���o�̓X�g���[��


										   // waveInOpen�����p����R�[���o�b�N�֐�
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
	case WIM_OPEN://waveInOpen��
		cout << "waveInOpen" << endl;
		break;

	case WIM_CLOSE://waveInClose��
		cout << "waveInClose" << endl;
		break;

	case WIM_DATA://�o�b�t�@�������ς��ɂȂ�ƌĂ΂��A������wave�n�֐����ĂԂƃf�b�h���b�N
		int bsize = ((LPWAVEHDR)dwParam1)->dwBytesRecorded;//�o�b�t�@�T�C�Y
		for (int dwCount = 0; dwCount < bsize; dwCount++) {
			//�Z�[�u�p�̉σo�b�t�@�ɑ}��
			lpWaveDataSave.push_back(((LPWAVEHDR)dwParam1)->lpData[dwCount]);
			lpWaveDataSaveInt.push_back(((LPWAVEHDR)dwParam1)->lpData[dwCount]);
			//ofs << num << endl;
			//���_���o�A���S���Y��
			if (180 < (short)(((LPWAVEHDR)dwParam1)->lpData[dwCount])) {
				clap = true;
			}
		}
		if (clap) cout << "clap!" << endl;
		clap = false;

		//�Z�[�u�p�o�b�t�@�̓o�^�ƃt���O�𗧂Ă� �X���b�h�Ȃ̂ŏ��Ԃɒ��Ӂimain�̏�������o�b�t�@����j
		lpWaveDataTmp = (LPBYTE)(((LPWAVEHDR)dwParam1)->lpData);
		dataResetFlg = true;
		//cout << "�o�b�t�@�������ς��ɂȂ�܂���" << endl;
		break;
	}
}

// waveOutOpen�����p����R�[���o�b�N�֐�
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
		cout << "�o�b�t�@�����ׂčĐ����܂���" << endl;
		break;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	/*******************************************************************************
	�^��
	********************************************************************************/


	// �f�o�C�X�I�[�v�����̐ݒ�
	wf.wFormatTag = WAVE_FORMAT_PCM;   // PCM�`��
	wf.nChannels = 1;                 // �X�e���I�����m������
									  //wf.nSamplesPerSec = 22050;             // �T���v�����O���[�g 22.05KHz
	wf.nSamplesPerSec = 44100;             // �T���v�����O���[�g 22.05KHz
	wf.wBitsPerSample = 16;                 // �ʎq�����x��
	wf.nBlockAlign = wf.wBitsPerSample * wf.nChannels / 8;  // �o�C�g������̃r�b�g��[PCM�̎d�l]
	wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;    // 1 �b������̃o�C�g��
																// ���m����+�T���v��22050+�ʎq��8bit�Ȃ̂� ��b�Ԃ�22050byte�̃f�[�^����������
																// �T���v�� x nBlockAlign = ��b�Ԃ̃f�[�^�ʂȂ̂� nBlockAlign�͂P
																//UINT test = waveInGetNumDevs();
																// �f�o�C�X�I�[�v��
	if (waveInOpen(
		&hwi,               // �n���h��
							//WAVE_MAPPER,        // �f�o�C�XID�B WAVE_MAPPER�f�t�H���g�̃f�o�C�X���w�� �t���̃}�C�N�v���O
							//WAVE_MAPPER,                  //�悭�킩��Ȃ�����CH2�̓f�o�C�XID��4
		1,				//waveInGetNumDevs(),//�f�o�C�X�̐����擾����֐�
		&wf,                // �I�[�v�����̐ݒ�
		(DWORD)waveInProc,  // �R�[���o�b�N�֐�
		0,                  // �����
		CALLBACK_FUNCTION   // �R�[���o�b�N�֐��̑���
		) != MMSYSERR_NOERROR)
		cout << "�f�o�C�X�̃I�[�v���Ɏ��s���܂���" << endl;

	float dwRecordSecond = 1;      // �o�b�t�@�b�� 0.01�܂Ŋm�F
	DWORD dwDataSize;

	dwDataSize = (DWORD)((float)(wf.nAvgBytesPerSec) * (dwRecordSecond));   // ��b�Ԃ̃f�[�^�� x �b��
	lpWaveDataFront = (LPBYTE)new BYTE[dwDataSize];
	lpWaveDataBack = (LPBYTE)new BYTE[dwDataSize];
	//lpWaveDataSave = (LPBYTE)new BYTE[dwDataSize*2];

	// �o�b�t�@���Z�b�g
	whf.lpData = (LPSTR)lpWaveDataFront;
	whf.dwBufferLength = dwDataSize;
	whf.dwFlags = 0;

	// �o�b�t�@���Z�b�g
	whb.lpData = (LPSTR)lpWaveDataBack;
	whb.dwBufferLength = dwDataSize;
	whb.dwFlags = 0;

	// �o�b�t�@�̏�����
	waveInPrepareHeader(hwi, &whf, sizeof(WAVEHDR));
	waveInPrepareHeader(hwi, &whb, sizeof(WAVEHDR));

	// �o�b�t�@�̊֘A�t��
	waveInAddBuffer(hwi, &whf, sizeof(WAVEHDR));
	waveInAddBuffer(hwi, &whb, sizeof(WAVEHDR));

	// �^���J�n
	waveInStart(hwi);

	//�������[�v���Ƀo�b�t�@�����t���O�̊m�FwaveInAddBuffer�Ńo�b�t�@���㏑��
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
				cout << "�o�b�t�@�G���[" << endl;
			}
			dataResetFlg = false;
		}

		if (GetAsyncKeyState(VK_UP)) {
			break;
		}

	}
	rewind(stdin);
	getchar(); // 5�b�ҋ@����ƁA "�o�b�t�@�������ς��ɂȂ���" ���\�������͂�

			   // �^���I��
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
	�Đ�
	********************************************************************************/

	HWAVEOUT hwo;   // �n���h��
	WAVEHDR whdr;       // 

						// �f�o�C�X���J��
	if (waveOutOpen(
		&hwo,           // �n���h��
		WAVE_MAPPER,        // �f�t�H���g�̃f�o�C�X���w��
		&wf,                // waveInOpen�̂Ƃ��Ȃ��ł���
		(DWORD)waveOutProc, // �R�[���o�b�N�֐�
		0,                  // �����
		CALLBACK_FUNCTION   // �R�[���o�b�N�֐��̑���
		)
		!= MMSYSERR_NOERROR)
		cout << "Error" << endl;


	// �����t�@�C���̃p�����[�^��ݒ�
	whdr.lpData = (LPSTR)(lpWaveDataSave.data());          // �����f�[�^�̓����Ă�|�C���^
	whdr.dwBufferLength = lpWaveDataSave.size();                     // �����f�[�^�̃T�C�Y
	whdr.dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;  // �Đ��I�v�V����
	whdr.dwLoops = 1;                              // ���[�v��

												   // ������
	waveOutPrepareHeader(hwo, &whdr, sizeof(WAVEHDR));

	// �����f�[�^�u���b�N�̏�������
	waveOutWrite(hwo, &whdr, whdr.dwBufferLength);

	rewind(stdin);
	getchar();  // �Đ����ҋ@

	waveOutReset(hwo);
	waveOutUnprepareHeader(hwo, &whdr, sizeof(whdr));
	waveOutClose(hwo);
	return 0;
}

