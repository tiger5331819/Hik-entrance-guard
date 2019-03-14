#include <stdio.h> 
#include <iostream> 
#include "Windows.h" 
#include "HCNetSDK.h" 
using namespace std;

BOOL CALLBACK MSesGCallback(LONG lCommand, NET_DVR_ALARMER *pAlarmer, char *pAlarmInfo, DWORD dwBufLen, void*
	pUser)
{  //回调函数中不可有耗时较长的操作，不能调用该 SDK（HCNetSDK.dll）本身的接口。  //以下代码仅供参考，实际应用中不建议在回调函数中直接处理数据保存文件，  
   //例如可以使用消息的方式(PostMessage)在消息响应函数里进行处理。 
	switch (lCommand)
	{
	case COMM_ALARM_ACS://门禁主机报警信息 
	{
		int i;
		NET_DVR_ACS_ALARM_INFO struAcsAlarmInfo = { 0 };
		memcpy(&struAcsAlarmInfo, pAlarmInfo, sizeof(struAcsAlarmInfo));    //按需处理报警信息结构体中的信息...... 
		break;
	}  case COMM_ID_INFO_ALARM://门禁身份证刷卡信息 
	{NET_DVR_ID_CARD_INFO_ALARM struID_CardInfo = { 0 };
	memcpy(&struID_CardInfo, pAlarmInfo, sizeof(struID_CardInfo));    //按需处理报警信息结构体中的信息...... 
	break;
	}  case COMM_PASSNUM_INFO_ALARM://门禁通行人数信息 
	{
		NET_DVR_PASSNUM_INFO_ALARM struPassnumInfo = { 0 };
		memcpy(&struPassnumInfo, pAlarmInfo, sizeof(struPassnumInfo));    //按需处理报警信息结构体中的信息...... 
		break;
	}
	default:
		break;
	}
	return true;
}
int main()
{
	//---------------------------------------  //初始化 
	NET_DVR_Init();
	//设置连接超时时间与重连功能 
	NET_DVR_SetConnectTime(2000, 1);
	NET_DVR_SetReconnect(10000, true);

	//---------------------------------------  //注册设备 
	LONG lUserID;  //登录参数，包括设备地址、登录用户、密码等 
	NET_DVR_USER_LOGIN_INFO struLoginInfo = { 0 };  struLoginInfo.bUseAsynLogin = 0; //同步登录方式  strcpy(struLoginInfo.sDeviceAddress, "192.0.0.64"); //设备 IP 地址  struLoginInfo.wPort = 8000; //设备服务端口  strcpy(struLoginInfo.sUserName, "admin"); //登录用户名  strcpy(struLoginInfo.sPassword, "abcd1234"); //登录密码 

	//设备信息, 输出参数 
	NET_DVR_DEVICEINFO_V40 struDeviceInfoV40 = { 0 };

	lUserID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);
	if (lUserID < 0)
	{
		printf("Login failed, error code: %d\n", NET_DVR_GetLastError());
		NET_DVR_Cleanup();
		return 0;
	}

	//设置报警回调函数，刷卡等事件都会触发报警回调函数 
	NET_DVR_SetDVRMessageCallBack_V31(MSesGCallback, NULL);  //启用布防 
	NET_DVR_SETUPALARM_PARAM struSetupParam = { 0 };
	struSetupParam.dwSize = sizeof(NET_DVR_SETUPALARM_PARAM);

	LONG  lHandle = NET_DVR_SetupAlarmChan_V41(lUserID, &struSetupParam);
	if (lHandle < 0)
	{
		printf("NET_DVR_SetupAlarmChan_V41 error, %d\n", NET_DVR_GetLastError());
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
		return 0;
	}
	//等待 60s，等待接收设备上传报警 
	Sleep(60000);  //撤销布防 
	if (!NET_DVR_CloseAlarmChan_V30(lHandle))
	{
		printf("NET_DVR_CloseAlarmChan_V30 error, %d\n", NET_DVR_GetLastError());
		NET_DVR_Logout(lUserID);
		NET_DVR_Cleanup();
		return 0;
	}  //注销用户 
	NET_DVR_Logout(lUserID);  //释放 SDK 资源 
	NET_DVR_Cleanup();
	return 0;
}