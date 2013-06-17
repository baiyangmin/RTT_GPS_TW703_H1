#ifndef _H_M66_
#define _H_M66_

/*
1.将gsm封装成一个设备，内部gsm状态机由一个线程处理
  应用程序只需要打开设备，进行操作即可。

2.通过AT命令读取模块类型? 这样更换模块就不需要刷新
  程序，也便于比较模块。有多大实际意义?

3.模块的状态或信息，比如中心下发信息、接收呼叫/短信
  天线开短路、socket断开等 (不同的模块不同,要抽象出
  来)，如何通知APP? 使用回调函数。

4.有效的socket管理
*/


#define GSM_UART_NAME	"uart4"

//#define MAX_SOCKETS	6	//EM310定义3  MG323定义6

/*
定义使用的模块型号
*/
//#define MG323
#define M66
//#define MC323
//#define EM310



/*
GSM支持操作功能的列表
不同的模块支持的命令不同，比如录音命令 TTS命令
此处不能统一成一个gsm.h,而应依据模块的不同而不同。
但上层软件如何控制?

控制接口应不应该把功能分的足够详细
如何实现一键拨号:一个命令/函数控制建立连接
*/
typedef enum
{
	CTL_STATUS=1,		//查询GSM状态
	CTL_AT_CMD, 		//发送AT命令
	CTL_PPP,			//PPP链接维护
	CTL_SOCKET, 		//建立socket
	CTL_DNS,			//进行DNS解析
	CTL_TXRX_COUNT, 	//发送接收的字节数
	CTL_CONNECT,		//直接建立连接

}T_GSM_CONTROL_CMD;


typedef enum
{
	GSM_STATE_GET=0,		/*查询GSM状态*/
	GSM_IDLE=1,			/*空闲*/
	GSM_POWERON,		/*上电过程并完成模块的AT命令初始化过程*/
	GSM_AT,				/*处于AT命令空闲，可以设置socket参数，收发短信*/
	GSM_AT_SEND,		/*处于AT收发状态*/
	GSM_GPRS,		/*登录GPRS中*/
	GSM_TCPIP,			/*已经登网，可以进行socket控制*/
	GSM_SOCKET_PROC,	/*正在进行socket控制*/
	GSM_ERR_POWERON,
	GSM_ERR_GPRS,
	GSM_ERR_TCPIP,
	GSM_POWEROFF,		/*已经断电*/	
}T_GSM_STATE;


typedef enum
{
	SOCKET_STATE=0,		/*查询socket状态*/
	SOCKET_IDLE = 1,            /*无需启动*/
	SOCKET_ERR,
	SOCKET_START,				/*启动连接远程*/
	SOCKET_DNS,                 /*DNS查询中*/
	SOCKET_DNS_ERR,
	SOCKET_CONNECT,             /*连接中*/
	SOCKET_CONNECT_ERR,         /*连接错误，对方不应答*/
	SOCKET_READY,               /*已完成，可以建立链接*/
	SOCKET_CLOSE,
}T_SOCKET_STATE;


enum RESP_TYPE
{
	RESP_TYPE_FUNC			=1,
	RESP_TYPE_FUNC_WITHOK	=2,
	RESP_TYPE_STR			=3,
	RESP_TYPE_STR_WITHOK	=4,
};



/*最大支持4个链接*/
#define MAX_GSM_SOCKET 4

typedef struct
{
	T_SOCKET_STATE	state;          /*连接状态*/
	uint8_t			linkno;			/*所使用的link号*/
	char			type;           /*连接类型 'u':udp client 't':TCP client  'U' udp server*/
	char			ipstr[64];    /*域名或地址*/
	char			ip_addr[16];     /*dns后的IP xxx.xxx.xxx.xxx*/
	uint16_t		port;           /*端口*/
	//MsgList			* msglist_tx;
}GSM_SOCKET;

typedef rt_err_t ( *RESP_FUNC )( char *s, uint16_t len );


void gsm_init(void);

void ctl_gprs( char* apn, char* user, char*psw ,uint8_t fdial );

void ctl_socket( uint8_t linkno,char type, char* remoteip, uint16_t remoteport,uint8_t fconnect );

rt_size_t socket_write( uint8_t linkno, uint8_t* buff, rt_size_t count );

rt_err_t gsm_send( char *atcmd,
                   RESP_FUNC respfunc,
                   char * compare_str,
                   uint8_t type,
                   uint32_t timeout,
                   uint8_t retry );

T_GSM_STATE gsmstate( T_GSM_STATE cmd );
T_SOCKET_STATE socketstate( T_SOCKET_STATE cmd );


#endif

