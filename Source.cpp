//�w�b�_�[�t�@�C���ǂݍ���
#include "DxLib.h"

//�}�N����`
#define GAME_WIDTH 800    //��ʂ̉�
#define GAME_HEIGHT 600	  //��ʂ̏c
#define GAME_COLOR 32     //��ʂ̃J���[�r�b�g

#define GAME_WINDOW_NAME                     "GAME_TITLE_NAME"
#define GAME_WINDOW_MODECHANGE TRUE		//TRUE:�E�B���h�E���[�h FALSE:�t���X�N���[��

#define SET_WINDOW_ST_MODE_DEFAULT           0		//�f�t�H���g
#define SET_WINDOW_ST_MODE_TITLE_NONE        1		//�^�C�g���o�[����
#define SET_WINDOW_ST_MODE_TITLE_FLAME_NONE  2		//�^�C�g���o�[�ƃt���[������
#define SET_WINDOW_ST_MODE_FLAME_NONE        3		//�t���[������

#define GAME_FPS_SPEED                       60

//�񋓌^
enum GAME_SCENE {
	GAME_SCENE_TITLE,	//�^�C�g�����
	GAME_SCENE_PLAY,	//�v���C���
	GAME_SCENE_END		//�G���h���
};

//�O���[�o���ϐ�
//�E�B���h�E�֌W
WNDPROC WndProc;			//�E�B���h�E�v���V�[�W���̃A�h���X
BOOL IsWM_CREATE = FALSE; //WM_CREATE������ɓ��삵�������肷��

//�L�[�{�[�h�֌W
char AllKeyState[256];		//�S�L�[�̏�Ԃ��͂���

//FPS�֌W
int StartTimeFps;					//����J�n����
int CountFps;						//�J�E���^
float CalcFps;						//�v�Z����
int SampleNumFps = GAME_FPS_SPEED;	//���ς��Ƃ�T���v����

//�V�[���֌W
int GameSceneNow = (int)GAME_SCENE_TITLE;	//�ŏ��̃Q�[����ʂ��^�C�g���ɐݒ�

//�v���g�^�C�v�錾
LRESULT CALLBACK MY_WNDPROC(HWND, UINT, WPARAM, LPARAM);//����E�B���h�E�v���V�[�W��

VOID MY_FPS_UPDATE(VOID);		//FPS�l���v���A�X�V����֐�
VOID MY_FPS_DRAW(VOID);			//FPS�l��`�悷��֐�
VOID MY_FPS_WAIT(VOID);			//FPS�l���v�����A�҂֐�
VOID MY_DRAW_PLAY_INFO(VOID);	//�v���C��ʂ̏���`�悷��֐�

VOID MY_ALL_KEYDOWN_UPDATE(VOID);//�L�[�̓��͏�Ԃ��X�V����֐�

VOID MY_GAME_TITLE(VOID);		//�^�C�g����ʂ̊֐�
VOID MY_GAME_PLAY(VOID);		//�v���C��ʂ̊֐�
VOID MY_GAME_END(VOID);			//�G���h��ʂ̊֐�

VOID MY_DRAW_STRING_CENTER_CENTER(char[][128], int, char *, int);	//��ʒ����ɕ�����`�悷��֐�
VOID MY_DRAW_SET_DEFAULT_FONT(BOOL);	//�������f�t�H���g�t�H���g�ɐݒ肷��֐�


//�v���O�����ōŏ��Ɏ��s�����֐�
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	ChangeWindowMode(TRUE);
	SetGraphMode(GAME_WIDTH, GAME_HEIGHT, GAME_COLOR);
	SetWindowStyleMode(SET_WINDOW_ST_MODE_TITLE_NONE);
	SetMainWindowText(TEXT(GAME_WINDOW_NAME));

	//�t�b�N��WM_CLOSE�Ȃǂ̃��b�Z�[�W�������|���Ď擾
	SetHookWinProc(MY_WNDPROC);	//�E�B���h�E�v���V�[�W���̐ݒ�

	if (DxLib_Init() == -1) { return -1; }//�c�w���C�u��������������

	SetDrawScreen(DX_SCREEN_BACK);//Draw�n�֐��͗���ʂɕ`��


	//�������[�v
	while (TRUE)
	{
		if (ProcessMessage() != 0) { break; }	//���b�Z�[�W�����̌��ʂ��G���[�̂Ƃ��A�����I��

		if (ClearDrawScreen() != 0) { break; }	//��ʂ������ł��Ȃ������Ƃ��A�����I��

		MY_ALL_KEYDOWN_UPDATE();				//�����Ă���L�[��Ԃ��擾

		//�V�[���I��
		switch (GameSceneNow)
		{
		case(int)GAME_SCENE_TITLE:	//�^�C�g����ʂ̏�����������

			MY_GAME_TITLE();	//�^�C�g����ʂ̏���

			break;//�^�C�g����ʂ̏��������܂�

		case(int)GAME_SCENE_PLAY://�v���C��ʂ̏�����������

			MY_GAME_PLAY();		//�v���C��ʂ̏���

			break;					//�v���C��ʂ̏��������܂�

		case(int)GAME_SCENE_END://�G���h��ʂ̏�����������

			MY_GAME_END();		//�G���h��ʂ̏���

			break;					//�G���h��ʂ̏��������܂�

		default:

			break;
		}

		MY_FPS_UPDATE();	//FPS�̏���[�X�V]

		MY_FPS_DRAW();		//FPS�̏���[�`��]

		ScreenFlip();		//���j�^�̃��t���b�V�����[�g�̑����ŗ���ʂ��ĕ`��

		MY_FPS_WAIT();		//FPS�̏���[�҂�]
	}

	DxLib_End();		//�c�w���C�u�����g�p�̏I������

	return 0;
}

//�L�[�̓��͏�Ԃ��X�V����֐�
VOID MY_ALL_KEYDOWN_UPDATE(VOID)
{
	char TempKey[256];			//�ꎞ�I�ɁA���݂̃L�[�̓��͏�Ԃ��i�[����

	GetHitKeyStateAll(TempKey); // �S�ẴL�[�̓��͏�Ԃ𓾂�

	//�v���O�����̌��l�^�͂�����Fhttps://dixq.net/g/02_09.html
	//���t�@�����X�͂�����@�@�@�Fhttps://dxlib.xsrv.jp/function/dxfunc_input.html#R5N2

	for (int i = 0; i < 256; i++)
	{
		if (TempKey[i] != 0)	//������Ă���L�[�̃L�[�R�[�h�������Ă���Ƃ�
		{
			AllKeyState[i]++;	//������Ă���
		}
		else
		{
			AllKeyState[i] = 0;	//������Ă��Ȃ�
		}
	}
	return;
}

//FPS�l���v���A�X�V����֐�
VOID MY_FPS_UPDATE(VOID)
{
	if (CountFps == 0) //1�t���[���ڂȂ玞�����L��
	{
		StartTimeFps = GetNowCount();
	}

	if (CountFps == SampleNumFps) //60�t���[���ڂȂ畽�ς��v�Z
	{
		int now = GetNowCount();
		CalcFps = 1000.f / ((now - StartTimeFps) / (float)SampleNumFps);
		CountFps = 0;
		StartTimeFps = now;
	}
	CountFps++;
	return;
}

//FPS�l��`�悷��֐�
VOID MY_FPS_DRAW(VOID)
{

	//�������`��
	DrawFormatString(0, GAME_HEIGHT - 20, GetColor(0, 200, 255), "FPS:%.1f", CalcFps);
	return;
}

//FPS�l���v�����A�҂֐�
VOID MY_FPS_WAIT(VOID)
{
	int resultTime = GetNowCount() - StartTimeFps;					//������������
	int waitTime = CountFps * 1000 / GAME_FPS_SPEED - resultTime;	//�҂ׂ�����

	if (waitTime > 0)	//�������镪�A�ҋ@
	{
		Sleep(waitTime);
	}
	return;
}

//�^�C�g����ʂ̊֐�
VOID MY_GAME_TITLE(VOID)
{
	if (AllKeyState[KEY_INPUT_RETURN] != 0)//�G���^�[�L�[��������Ă�����
	{
		GameSceneNow = (int)GAME_SCENE_PLAY;	//�V�[�����v���C��ʂɂ���
	}

	char StrGameTitle[2][128] = { "�ӂ���������","���H" };
	char StrFontTitle[128] = { "MS �S�V�b�N" };//�啶�����p�uMS�v�A���p�u�󔒁v�A�S�p�u�S�V�b�N�v

	MY_DRAW_STRING_CENTER_CENTER(&StrGameTitle[0], 2, StrFontTitle, 64);	//��ʒ����ɕ`��

	DrawString(0, 0, "�^�C�g�����(�G���^�[�L�[�������Ă�������)", GetColor(255, 255, 255));

	return;
}

//�v���C��ʂ̊֐�
VOID MY_GAME_PLAY(VOID)
{
	if (AllKeyState[KEY_INPUT_SPACE] != 0) //�G���^�[�L�[��������Ă�����
	{
		GameSceneNow = (int)GAME_SCENE_END; //�V�[�����v���C��ʂɂ���
	}
	DrawString(0, 0, "�v���C���(�X�y�[�X�L�[�������Ă�������)", GetColor(255, 255, 255));

	return;
}

//�G���h��ʂ̊֐�
VOID MY_GAME_END(VOID)
{
	if (AllKeyState[KEY_INPUT_BACK] != 0) //�o�b�N�X�y�[�X�L�[��������Ă�����
	{
		GameSceneNow = (int)GAME_SCENE_TITLE; //�V�[�����^�C�g����ʂɂ���
	}
	DrawString(0, 0, "�G���h���(�o�b�N�X�y�[�X�������Ă�������)", GetColor(255, 255, 255));

	return;
}

//�E�B���h�E�v���V�[�W���֐�
LRESULT CALLBACK MY_WNDPROC(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{

	case WM_CREATE:	//�E�B���h�E�̐�����������


		IsWM_CREATE = TRUE;	//WM_CREATE����I��
		return 0;

	case WM_CLOSE:		//����{�^�����������Ƃ�

		MessageBox(hwnd, TEXT("�Q�[�����I�����܂�"), TEXT("�I�����b�Z�[�W"), MB_OK);
		break;

	case WM_RBUTTONDOWN:	//�}�E�X�̉E�{�^�����������Ƃ�

		SendMessage(hwnd, WM_CLOSE, 0, 0);		//WM_CLOSE���b�Z�[�W���L���[�ɒǉ�
		break;

	case WM_LBUTTONDOWN:	//�}�E�X�̍��{�^�����������Ƃ�

		//WM_NCLBUTTONDOWN(�^�C�g���o�[�Ń}�E�X�̍��{�^����������)���b�Z�[�W�������ɔ��s
		PostMessage(hwnd, WM_NCLBUTTONDOWN, (WPARAM)HTCAPTION, lp);
		break;

	case WM_DESTROY:	//�E�B���h�E���j�����ꂽ(�Ȃ��Ȃ���)�Ƃ�

		PostQuitMessage(0);		//���b�Z�[�W�L���[�� WM_QUIT �𑗂�
		return 0;
	}

	//�f�t�H���g�̃E�B���h�E�v���V�[�W���֐����Ăяo��
	return DefWindowProc(hwnd, msg, wp, lp);
}

//�������f�t�H���g�t�H���g�ɐݒ肷��֐�
//���@���FBOOL�F�A���`�G�C���A�X������
VOID MY_DRAW_SET_DEFAULT_FONT(BOOL anti)
{
	//�f�t�H���g�͈ȉ��̃t�H���g�ƃT�C�Y�ł���ƁA�ݒ肷��
	ChangeFont("MS �S�V�b�N", DX_CHARSET_DEFAULT);
	SetFontSize(16);

	if (anti)
	{
		ChangeFontType(DX_FONTTYPE_ANTIALIASING);
	}
	return;
}

//��ʒ����ɕ�����`�悷��֐�
//���@���Fchar [][]�F�\�����镶����̐擪�A�h���X
//���@���Fint�@ �@ �F�\������s��
//���@���Fchar *�@ �F�t�H���g�̖��O
//���@���Fint�@ �@ �F�����̑傫��
//�߂�l�F�Ȃ�
VOID MY_DRAW_STRING_CENTER_CENTER(char str[][128], int row, char *fontname, int size)
{
	ChangeFont(fontname, DX_CHARSET_DEFAULT);		//�f�t�H���g�̃t�H���g��ύX����
	SetFontSize(size);								//�f�t�H���g�̃t�H���g�T�C�Y��ς���
	ChangeFontType(DX_FONTTYPE_ANTIALIASING_EDGE);	//�t�H���g�����ꂢ�ɂ݂���

	int StrHeightSum = 0;
	for (int sum = 0; sum < row; sum++)	//�s�S�̂̍������W�v
	{
		StrHeightSum += GetFontSize();	//�f�t�H���g�̃t�H���g�̍������W�v
	}

	for (int rCnt = 0; rCnt < row; rCnt++)	//�t�H���g��`��
	{
		int StrWidth = GetDrawFormatStringWidth(&str[rCnt][0]);	//�f�t�H���g�̃t�H���g�̉������擾
		int StrHeight = GetFontSize();

		DrawFormatString(
			GAME_WIDTH / 2 - StrWidth / 2,
			GAME_HEIGHT / 2 - StrHeightSum + StrHeight * rCnt,	//��ʂ̒��S�@-�@�s�S�̂̕����̍����@+�@�P�s�̍���
			GetColor(255, 255, 255), &str[rCnt][0]);	//�f�t�H���g�̃t�H���g�ŕ`��
	}
	MY_DRAW_SET_DEFAULT_FONT(TRUE);//�f�t�H���g�t�H���g�ɐݒ肷��
	return;
}