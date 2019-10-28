//ヘッダーファイル読み込み
#include "DxLib.h"

//マクロ定義
#define GAME_WIDTH 800    //画面の横
#define GAME_HEIGHT 600	  //画面の縦
#define GAME_COLOR 32     //画面のカラービット

#define GAME_WINDOW_NAME                     "GAME_TITLE_NAME"
#define GAME_WINDOW_MODECHANGE TRUE		//TRUE:ウィンドウモード FALSE:フルスクリーン

#define SET_WINDOW_ST_MODE_DEFAULT           0		//デフォルト
#define SET_WINDOW_ST_MODE_TITLE_NONE        1		//タイトルバー無し
#define SET_WINDOW_ST_MODE_TITLE_FLAME_NONE  2		//タイトルバーとフレーム無し
#define SET_WINDOW_ST_MODE_FLAME_NONE        3		//フレーム無し

#define GAME_FPS_SPEED                       60

//列挙型
enum GAME_SCENE {
	GAME_SCENE_TITLE,	//タイトル画面
	GAME_SCENE_PLAY,	//プレイ画面
	GAME_SCENE_END		//エンド画面
};

//グローバル変数
//ウィンドウ関係
WNDPROC WndProc;			//ウィンドウプロシージャのアドレス
BOOL IsWM_CREATE = FALSE; //WM_CREATEが正常に動作したか判定する

//キーボード関係
char AllKeyState[256];		//全キーの状態がはいる

//FPS関係
int StartTimeFps;					//測定開始時刻
int CountFps;						//カウンタ
float CalcFps;						//計算結果
int SampleNumFps = GAME_FPS_SPEED;	//平均をとるサンプル数

//シーン関係
int GameSceneNow = (int)GAME_SCENE_TITLE;	//最初のゲーム画面をタイトルに設定

//プロトタイプ宣言
LRESULT CALLBACK MY_WNDPROC(HWND, UINT, WPARAM, LPARAM);//自作ウィンドウプロシージャ

VOID MY_FPS_UPDATE(VOID);		//FPS値を計測、更新する関数
VOID MY_FPS_DRAW(VOID);			//FPS値を描画する関数
VOID MY_FPS_WAIT(VOID);			//FPS値を計測し、待つ関数
VOID MY_DRAW_PLAY_INFO(VOID);	//プレイ画面の情報を描画する関数

VOID MY_ALL_KEYDOWN_UPDATE(VOID);//キーの入力状態を更新する関数

VOID MY_GAME_TITLE(VOID);		//タイトル画面の関数
VOID MY_GAME_PLAY(VOID);		//プレイ画面の関数
VOID MY_GAME_END(VOID);			//エンド画面の関数

VOID MY_DRAW_STRING_CENTER_CENTER(char[][128], int, char *, int);	//画面中央に文字を描画する関数
VOID MY_DRAW_SET_DEFAULT_FONT(BOOL);	//文字をデフォルトフォントに設定する関数


//プログラムで最初に実行される関数
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	ChangeWindowMode(TRUE);
	SetGraphMode(GAME_WIDTH, GAME_HEIGHT, GAME_COLOR);
	SetWindowStyleMode(SET_WINDOW_ST_MODE_TITLE_NONE);
	SetMainWindowText(TEXT(GAME_WINDOW_NAME));

	//フック→WM_CLOSEなどのメッセージを引っ掛けて取得
	SetHookWinProc(MY_WNDPROC);	//ウィンドウプロシージャの設定

	if (DxLib_Init() == -1) { return -1; }//ＤＸライブラリ初期化処理

	SetDrawScreen(DX_SCREEN_BACK);//Draw系関数は裏画面に描画


	//無限ループ
	while (TRUE)
	{
		if (ProcessMessage() != 0) { break; }	//メッセージ処理の結果がエラーのとき、強制終了

		if (ClearDrawScreen() != 0) { break; }	//画面を消去できなかったとき、強制終了

		MY_ALL_KEYDOWN_UPDATE();				//押しているキー状態を取得

		//シーン選択
		switch (GameSceneNow)
		{
		case(int)GAME_SCENE_TITLE:	//タイトル画面の処理ここから

			MY_GAME_TITLE();	//タイトル画面の処理

			break;//タイトル画面の処理ここまで

		case(int)GAME_SCENE_PLAY://プレイ画面の処理ここから

			MY_GAME_PLAY();		//プレイ画面の処理

			break;					//プレイ画面の処理ここまで

		case(int)GAME_SCENE_END://エンド画面の処理ここから

			MY_GAME_END();		//エンド画面の処理

			break;					//エンド画面の処理ここまで

		default:

			break;
		}

		MY_FPS_UPDATE();	//FPSの処理[更新]

		MY_FPS_DRAW();		//FPSの処理[描画]

		ScreenFlip();		//モニタのリフレッシュレートの速さで裏画面を再描画

		MY_FPS_WAIT();		//FPSの処理[待つ]
	}

	DxLib_End();		//ＤＸライブラリ使用の終了処理

	return 0;
}

//キーの入力状態を更新する関数
VOID MY_ALL_KEYDOWN_UPDATE(VOID)
{
	char TempKey[256];			//一時的に、現在のキーの入力状態を格納する

	GetHitKeyStateAll(TempKey); // 全てのキーの入力状態を得る

	//プログラムの元ネタはこちら：https://dixq.net/g/02_09.html
	//リファレンスはこちら　　　：https://dxlib.xsrv.jp/function/dxfunc_input.html#R5N2

	for (int i = 0; i < 256; i++)
	{
		if (TempKey[i] != 0)	//押されているキーのキーコードを押しているとき
		{
			AllKeyState[i]++;	//押されている
		}
		else
		{
			AllKeyState[i] = 0;	//押されていない
		}
	}
	return;
}

//FPS値を計測、更新する関数
VOID MY_FPS_UPDATE(VOID)
{
	if (CountFps == 0) //1フレーム目なら時刻を記憶
	{
		StartTimeFps = GetNowCount();
	}

	if (CountFps == SampleNumFps) //60フレーム目なら平均を計算
	{
		int now = GetNowCount();
		CalcFps = 1000.f / ((now - StartTimeFps) / (float)SampleNumFps);
		CountFps = 0;
		StartTimeFps = now;
	}
	CountFps++;
	return;
}

//FPS値を描画する関数
VOID MY_FPS_DRAW(VOID)
{

	//文字列を描画
	DrawFormatString(0, GAME_HEIGHT - 20, GetColor(0, 200, 255), "FPS:%.1f", CalcFps);
	return;
}

//FPS値を計測し、待つ関数
VOID MY_FPS_WAIT(VOID)
{
	int resultTime = GetNowCount() - StartTimeFps;					//かかった時間
	int waitTime = CountFps * 1000 / GAME_FPS_SPEED - resultTime;	//待つべき時間

	if (waitTime > 0)	//早すぎる分、待機
	{
		Sleep(waitTime);
	}
	return;
}

//タイトル画面の関数
VOID MY_GAME_TITLE(VOID)
{
	if (AllKeyState[KEY_INPUT_RETURN] != 0)//エンターキーが押されていた時
	{
		GameSceneNow = (int)GAME_SCENE_PLAY;	//シーンをプレイ画面にする
	}

	char StrGameTitle[2][128] = { "ふぇぇぇぇぇ","え？" };
	char StrFontTitle[128] = { "MS ゴシック" };//大文字半角「MS」、半角「空白」、全角「ゴシック」

	MY_DRAW_STRING_CENTER_CENTER(&StrGameTitle[0], 2, StrFontTitle, 64);	//画面中央に描画

	DrawString(0, 0, "タイトル画面(エンターキーを押してください)", GetColor(255, 255, 255));

	return;
}

//プレイ画面の関数
VOID MY_GAME_PLAY(VOID)
{
	if (AllKeyState[KEY_INPUT_SPACE] != 0) //エンターキーが押されていた時
	{
		GameSceneNow = (int)GAME_SCENE_END; //シーンをプレイ画面にする
	}
	DrawString(0, 0, "プレイ画面(スペースキーを押してください)", GetColor(255, 255, 255));

	return;
}

//エンド画面の関数
VOID MY_GAME_END(VOID)
{
	if (AllKeyState[KEY_INPUT_BACK] != 0) //バックスペースキーが押されていた時
	{
		GameSceneNow = (int)GAME_SCENE_TITLE; //シーンをタイトル画面にする
	}
	DrawString(0, 0, "エンド画面(バックスペースを押してください)", GetColor(255, 255, 255));

	return;
}

//ウィンドウプロシージャ関数
LRESULT CALLBACK MY_WNDPROC(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{

	case WM_CREATE:	//ウィンドウの生成＆初期化


		IsWM_CREATE = TRUE;	//WM_CREATE正常終了
		return 0;

	case WM_CLOSE:		//閉じるボタンを押したとき

		MessageBox(hwnd, TEXT("ゲームを終了します"), TEXT("終了メッセージ"), MB_OK);
		break;

	case WM_RBUTTONDOWN:	//マウスの右ボタンを押したとき

		SendMessage(hwnd, WM_CLOSE, 0, 0);		//WM_CLOSEメッセージをキューに追加
		break;

	case WM_LBUTTONDOWN:	//マウスの左ボタンを押したとき

		//WM_NCLBUTTONDOWN(タイトルバーでマウスの左ボタンを押した)メッセージをすぐに発行
		PostMessage(hwnd, WM_NCLBUTTONDOWN, (WPARAM)HTCAPTION, lp);
		break;

	case WM_DESTROY:	//ウィンドウが破棄された(なくなった)とき

		PostQuitMessage(0);		//メッセージキューに WM_QUIT を送る
		return 0;
	}

	//デフォルトのウィンドウプロシージャ関数を呼び出す
	return DefWindowProc(hwnd, msg, wp, lp);
}

//文字をデフォルトフォントに設定する関数
//引　数：BOOL：アンチエイリアスをする
VOID MY_DRAW_SET_DEFAULT_FONT(BOOL anti)
{
	//デフォルトは以下のフォントとサイズであると、設定する
	ChangeFont("MS ゴシック", DX_CHARSET_DEFAULT);
	SetFontSize(16);

	if (anti)
	{
		ChangeFontType(DX_FONTTYPE_ANTIALIASING);
	}
	return;
}

//画面中央に文字を描画する関数
//引　数：char [][]：表示する文字列の先頭アドレス
//引　数：int　 　 ：表示する行数
//引　数：char *　 ：フォントの名前
//引　数：int　 　 ：文字の大きさ
//戻り値：なし
VOID MY_DRAW_STRING_CENTER_CENTER(char str[][128], int row, char *fontname, int size)
{
	ChangeFont(fontname, DX_CHARSET_DEFAULT);		//デフォルトのフォントを変更する
	SetFontSize(size);								//デフォルトのフォントサイズを変える
	ChangeFontType(DX_FONTTYPE_ANTIALIASING_EDGE);	//フォントをきれいにみせる

	int StrHeightSum = 0;
	for (int sum = 0; sum < row; sum++)	//行全体の高さを集計
	{
		StrHeightSum += GetFontSize();	//デフォルトのフォントの高さを集計
	}

	for (int rCnt = 0; rCnt < row; rCnt++)	//フォントを描画
	{
		int StrWidth = GetDrawFormatStringWidth(&str[rCnt][0]);	//デフォルトのフォントの横幅を取得
		int StrHeight = GetFontSize();

		DrawFormatString(
			GAME_WIDTH / 2 - StrWidth / 2,
			GAME_HEIGHT / 2 - StrHeightSum + StrHeight * rCnt,	//画面の中心　-　行全体の文字の高さ　+　１行の高さ
			GetColor(255, 255, 255), &str[rCnt][0]);	//デフォルトのフォントで描画
	}
	MY_DRAW_SET_DEFAULT_FONT(TRUE);//デフォルトフォントに設定する
	return;
}